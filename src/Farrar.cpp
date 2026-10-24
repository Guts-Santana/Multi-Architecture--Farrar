/*
 * Multi-Architecture Farrar
 * Copyright (C) 2026 Gustavo Santana Lima
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "Farrar.hpp"

template <typename Backend>
void Farrar<Backend>::setSequences(std::string s0, std::string s1)
{
    this->s0 = s0;
    this->s1 = s1;
}

template <typename Backend>
void Farrar<Backend>::setScoring(int match, int mismatch, int gapOpen, int gapExt)
{
    if (match <= 0)
        throw std::invalid_argument("match score must be positive");
    if (mismatch > 0)
        throw std::invalid_argument("mismatch score must be zero or negative");
    // A non-negative extension would never decay F, so its propagation would not end.
    if (gapOpen >= 0 || gapExt >= 0)
        throw std::invalid_argument("gap open and gap extension must be negative");

    for (int value : {match, mismatch, gapOpen, gapExt})
    {
        if (value < std::numeric_limits<ElemType>::min() || value > std::numeric_limits<ElemType>::max())
            throw std::invalid_argument("score " + std::to_string(value) + " does not fit the vector element type");
    }

    this->match = match;
    this->mismatch = mismatch;
    this->gap_open = gapOpen;
    this->gap_ext = gapExt;
}


template <typename Backend>
int Farrar<Backend>::obtainScore()
{
    initMatrices();
    buildProfile();
    encodeTarget();

    VecType vMaxBlock = prefixScan ? processColumns<true>() : processColumns<false>();

    maxScore = static_cast<int>(ops.maxValue(vMaxBlock));
    return maxScore;
}

template <typename Backend>
template <bool PrefixScan>
typename Farrar<Backend>::VecType
Farrar<Backend>::processColumns()
{
    VecType vMaxBlock = Traits::set(0, VL);
    for (size_t i = 0; i < s1Index.size(); i++)
    {
        VecType vF = Traits::set(0, VL);
        VecType vMax = Traits::set(0, VL);

        processPrimaryPass(s1Index[i], 0, vF, vMax);
        if constexpr (PrefixScan)
            propagatePrefixScanF(vF, vMax, 0);
        else
            propagateLazyF(vF, vMax, 0);

        vMaxBlock = ops.max(vMaxBlock, vMax);
    }
    return vMaxBlock;
}

template <typename Backend>
void Farrar<Backend>::encodeTarget()
{
    s1Index.resize(s1.length());
    for (size_t i = 0; i < s1.length(); i++)
    {
        int idx = charToIndex(s1[i]);
        s1Index[i] = (idx == -1) ? invalidRow : idx;
    }
}

template <typename Backend>
void Farrar<Backend>::buildProfile()
{
    vProfile.resize((alphabet.size() + 1) * segLen, VL);
    vProfile.fill(mismatch);

    // Single sequential pass over s0: lane j of segment i holds s0[j * segLen + i].
    size_t length = s0.length();
    for (size_t j = 0; j < VL; j++)
    {
        for (size_t i = 0; i < segLen; i++)
        {
            size_t idx = j * segLen + i;
            if (idx >= length)
                break;

            int resIdx = residueIndex(s0[idx]);
            if (resIdx == -1)
                continue;

            vProfile.at(static_cast<size_t>(resIdx) * segLen + i, j) = match;
        }
    }
}

template <typename Backend>
void Farrar<Backend>::initMatrices()
{
    segLen = (s0.length() + VL - 1) / VL;

    pvHE.resize(2 * segLen, VL);
    pvHE.fill(0);
}

template <typename Backend>
void Farrar<Backend>::processPrimaryPass(int profileRow, ElemType diag_h, VecType &vF, VecType &vMax)
{
    VecType vE;
    VecType vH = pvHE.load(2 * (segLen - 1));
    vH = ops.shift(vH, diag_h);

    size_t baseIndex = static_cast<size_t>(profileRow) * segLen;

    for (size_t j = 0; j < segLen; j++)
    {
        // Previous column's H(j) must be read before it is overwritten.
        VecType vHPrev = pvHE.load(2 * j);

        vH = ops.add(vH, vProfile.load(baseIndex + j));

        vE = pvHE.load(2 * j + 1);

        // F of this segment computed so far (cross-lane part is fixed by the F correction)
        if (blockMode)
            pvF.store(j, vF);

        vH = ops.max(vH, vE);
        vH = ops.max(vH, vF);
        vH = ops.max(vH, 0);

        vMax = ops.max(vMax, vH);

        pvHE.store(2 * j, vH);

        vH = ops.add(vH, gap_open);
        vE = ops.add(vE, gap_ext);
        vE = ops.max(vE, vH);
        pvHE.store(2 * j + 1, vE);

        vF = ops.add(vF, gap_ext);
        vF = ops.max(vF, vH);
        vH = vHPrev;
    }
}

template <typename Backend>
void Farrar<Backend>::propagatePrefixScanF(VecType &vF, VecType &vMax, ElemType top_f)
{
    long long accumulatedDecay;
    vF = ops.shift(vF, top_f);
    // H >= 0, so an F <= 0 can never improve it. Keeping F >= 0 bounds every
    // value below so the clamped decay cannot wrap around. In block mode F
    // leaves the block through MASA's row, so its true (negative) value is kept.
    if (!blockMode)
        vF = ops.max(vF, 0);
    VecType vH, vShift;

    for (size_t offset = 1; offset < VL; offset <<= 1)
    {
        vShift = ops.slideup(vF, offset);

        // Lane k + offset starts offset * segLen rows below lane k.
        accumulatedDecay = static_cast<long long>(offset) * segLen * gap_ext;
        accumulatedDecay = std::max<long long>(accumulatedDecay,
            blockMode ? LOW : std::numeric_limits<ElemType>::min());
        vShift = ops.add(vShift, static_cast<ElemType>(accumulatedDecay));

        if (blockMode)
        {
            // AVX slideup fills lanes < offset with 0 (a fake F of 0): push them to LOW
            VecType vFillMask = ops.add(ops.slideup(Traits::set(-LOW, VL), offset), LOW);
            vShift = ops.add(vShift, vFillMask);
        }

        vF = ops.max(vF, vShift);
    }

    // F decays by gap_ext per segment; once every lane is <= 0 the remaining
    // segments are unchanged, and stopping there also keeps vF from wrapping.
    // Block mode needs the corrected F and E of every segment, so it sweeps them all.
    size_t limit = segLen;
    if (!blockMode)
    {
        int maxF = static_cast<int>(ops.maxValue(vF));
        limit = (maxF > 0) ? std::min(segLen, static_cast<size_t>(maxF / -gap_ext) + 1) : 0;
    }

    for (size_t j = 0; j < limit; j++)
    {
        vH = pvHE.load(2 * j);
        vH = ops.max(vH, vF);
        pvHE.store(2 * j, vH);
        vMax = ops.max(vMax, vH);
        if (blockMode)
        {
            pvF.store(j, ops.max(pvF.load(j), vF));
            pvHE.store(2 * j + 1, ops.max(pvHE.load(2 * j + 1), ops.add(vH, gap_open)));
        }
        vF = ops.add(vF, gap_ext);
    }
}

template <typename Backend>
void Farrar<Backend>::propagateLazyF(VecType &vF, VecType &vMax, ElemType top_f)
{
    vF = ops.shift(vF, top_f);
    size_t j = 0;
    VecType vHStore = pvHE.load(0);
    int vFCarry;
    // Segment 0 has no in-lane F from the primary pass: always merge it
    if (blockMode)
        pvF.store(0, ops.max(pvF.load(0), vF));
    while (ops.anyBiggerElement(vF, ops.add(vHStore, gap_open)))
    {
        vHStore = pvHE.load(2 * j);
        vHStore = ops.max(vHStore, vF);
        pvHE.store(2 * j, vHStore);
        vMax = ops.max(vMax, vHStore);
        if (blockMode)
        {
            pvF.store(j, ops.max(pvF.load(j), vF));
            // H may have grown: the E prepared for the next column must follow
            pvHE.store(2 * j + 1, ops.max(pvHE.load(2 * j + 1), ops.add(vHStore, gap_open)));
        }

        j++;
        vF = ops.add(vF, gap_ext);

        if (j >= segLen)
        {
            if (blockMode)
            {
                // Lane k continues into lane k+1; nothing flows from the bottom row back into row 0
                vF = ops.shift(vF, LOW);
                j = 0;
                pvF.store(0, ops.max(pvF.load(0), vF));
            }
            else
            {
                vFCarry = ops.lastElement(vF);
                vF = ops.shift(vF, vFCarry);
                j = 0;
            }
        }
    }
}

#ifdef FARRAR_WITH_MASA

template <typename Backend>
void Farrar<Backend>::initializeStripedLeftColumn(cell_t* col_buffer, int q_len)
{
    // 1. Reset all lanes (important for the padding area at the end of the query)
    for (size_t idx = 0; idx < segLen; ++idx)
    {
        pvHE.store(2 * idx, Traits::set(0, VL));
        pvHE.store(2 * idx + 1, Traits::set(LOW, VL));
    }

    // 2. Interleave MASA's left boundary into the striped SIMD buffers.
    //    MASA keeps E[i][j0-1]; Farrar's E keeps the E already prepared for
    //    the current column: E[i][j0] = max(H[i][j0-1] + open, E[i][j0-1] + ext)
    for (int i = 0; i < q_len; ++i)
    {
        size_t lane = i / segLen;
        size_t idx  = i % segLen;

        int h = toLane(col_buffer[i + 1].h);
        int e = toLane(col_buffer[i + 1].e);
        pvHE.at(2 * idx, lane)     = h;
        pvHE.at(2 * idx + 1, lane) = toLane(std::max(h + gap_open, e + gap_ext));
    }
}

template <typename Backend>
void Farrar<Backend>::unstripeRightColumnE(cell_t* col_buffer, int q_len)
{
    // Must be called right before the last column is processed: at that point
    // E holds exactly E[i][j1-1], which is what MASA expects in col[].e
    for (int i = 0; i < q_len; ++i)
    {
        col_buffer[i + 1].e = pvHE.at(2 * (i % segLen) + 1, i / segLen);
    }
}

template <typename Backend>
void Farrar<Backend>::unstripeRightColumnH(cell_t* col_buffer, int q_len)
{
    for (int i = 0; i < q_len; ++i)
    {
        col_buffer[i + 1].h = pvHE.at(2 * (i % segLen), i / segLen);
    }
}

template <typename Backend>
void Farrar<Backend>::buildBlockProfile(const char* query_seg, int q_len)
{
    vProfile.resize((alphabet.size() + 1) * segLen, VL);
    // Padding rows (i >= q_len) get LOW, otherwise their diagonal (a cell of the
    // left boundary column, outside the block) could beat the block's best score
    vProfile.fill(LOW);

    for (int i = 0; i < q_len; ++i)
    {
        int char_idx = charToIndex(query_seg[i]);
        size_t lane = i / segLen;
        size_t idx  = i % segLen;

        // The last row (characters outside the alphabet) never matches
        for (size_t c = 0; c <= alphabet.size(); ++c)
        {
            vProfile.at(c * segLen + idx, lane) = (static_cast<int>(c) == char_idx) ? match : mismatch;
        }
    }
}

template <typename Backend>
score_t Farrar<Backend>::alignBlock(
    const char* query_seg, int q_len,
    const char* target_seg, int t_len,
    cell_t* row,
    cell_t* col,
    int abs_i, int abs_j)
{
    blockMode = true;

    // Buffer::resize only reallocates when the block is larger than the last one
    segLen = (q_len + VL - 1) / VL;
    pvHE.resize(2 * segLen, VL);
    pvF.resize(segLen, VL);

    buildBlockProfile(query_seg, q_len);
    initializeStripedLeftColumn(col, q_len);

    score_t block_best;
    block_best.score = -INF;
    block_best.i = -1;
    block_best.j = -1;

    size_t last_lane = (q_len - 1) / segLen;
    size_t last_idx  = (q_len - 1) % segLen;

    // H[i0-1][j-1]: diagonal of the first row of column j
    ElemType diag_h = toLane(col[0].h);
    // H[i0-1][j1-1] is the diagonal cell of the right neighbour block
    int corner_h = row[t_len - 1].h;

    for (int j = 0; j < t_len; ++j)
    {
        int top_h = toLane(row[j].h);
        int top_f = toLane(row[j].f);
        // F entering the first row: F[i0][j] = max(H[i0-1][j] + open, F[i0-1][j] + ext)
        ElemType first_f = toLane(std::max(top_h + gap_open, top_f + gap_ext));

        VecType vF = Traits::set(LOW, VL);
        VecType vMax = Traits::set(0, VL);

        // E currently holds E[*][j1-1]: this is MASA's right column E
        if (j == t_len - 1)
            unstripeRightColumnE(col, q_len);

        int idx = charToIndex(target_seg[j]);
        processPrimaryPass((idx == -1) ? invalidRow : idx, diag_h, vF, vMax);
        if (prefixScan)
            propagatePrefixScanF(vF, vMax, first_f);
        else
            propagateLazyF(vF, vMax, first_f);

        diag_h = top_h;

        // Overwrite row boundary for MASA (last row of the block)
        row[j].h = pvHE.at(2 * last_idx, last_lane);
        row[j].f = pvF.at(last_idx, last_lane);

        int col_max = ops.maxValue(vMax);
        if (col_max > block_best.score)
        {
            block_best.score = col_max;
            block_best.j = abs_j + j;
            // Locate the row of the maximum (first one, padding rows excluded)
            for (int i = 0; i < q_len; ++i)
            {
                if (pvHE.at(2 * (i % segLen), i / segLen) == col_max)
                {
                    block_best.i = abs_i + i;
                    break;
                }
            }
        }
    }

    // Overwrite column boundary for MASA
    unstripeRightColumnH(col, q_len);
    col[0].h = corner_h;

    blockMode = false;
    return block_best;
}

#endif // FARRAR_WITH_MASA

template <typename Backend>
void Farrar<Backend>::clearData()
{
    pvHE.release();
    vProfile.release();
    pvF.release();
    s1Index.clear();
}

// template class Farrar<vint16m1_t>;
// template class Farrar<vint16m2_t>;
// template class Farrar<vint16m4_t>;
// template class Farrar<vint16m8_t>;

// template class Farrar<vint32m1_t>;
// template class Farrar<vint32m2_t>;
// template class Farrar<vint32m4_t>;
// template class Farrar<vint32m8_t>;

#if defined(USE_RVV)

template class Farrar<RvvInt16M1>;
template class Farrar<RvvInt16M2>;
template class Farrar<RvvInt16M4>;
template class Farrar<RvvInt16M8>;

template class Farrar<RvvInt32M1>;
template class Farrar<RvvInt32M2>;
template class Farrar<RvvInt32M4>;
template class Farrar<RvvInt32M8>;

#endif

#if defined(USE_AVX)

template class Farrar<AvxInt16>;
template class Farrar<AvxInt32>;

#endif