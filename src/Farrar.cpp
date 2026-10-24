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

        processPrimaryPass(s1Index[i], vF, vMax);
        if constexpr (PrefixScan)
            propagatePrefixScanF(vF, vMax);
        else
            propagateLazyF(vF, vMax);

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
void Farrar<Backend>::processPrimaryPass(int profileRow, VecType &vF, VecType &vMax)
{
    VecType vE;
    VecType vH = pvHE.load(2 * (segLen - 1));
    vH = ops.shift(vH, 0);

    size_t baseIndex = static_cast<size_t>(profileRow) * segLen;

    for (size_t j = 0; j < segLen; j++)
    {
        // Previous column's H(j) must be read before it is overwritten.
        VecType vHPrev = pvHE.load(2 * j);

        vH = ops.add(vH, vProfile.load(baseIndex + j));

        vE = pvHE.load(2 * j + 1);

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
void Farrar<Backend>::propagatePrefixScanF(VecType &vF, VecType &vMax)
{
    long long accumulatedDecay;
    vF = ops.shift(vF, 0);
    // H >= 0, so an F <= 0 can never improve it. Keeping F >= 0 bounds every
    // value below so the clamped decay cannot wrap around.
    vF = ops.max(vF, 0);
    VecType vH, vShift;

    for (size_t offset = 1; offset < VL; offset <<= 1)
    {
        vShift = ops.slideup(vF, offset);

        // Lane k + offset starts offset * segLen rows below lane k.
        accumulatedDecay = static_cast<long long>(offset) * segLen * gap_ext;
        accumulatedDecay = std::max<long long>(accumulatedDecay, std::numeric_limits<ElemType>::min());
        vShift = ops.add(vShift, static_cast<ElemType>(accumulatedDecay));

        vF = ops.max(vF, vShift);
    }

    // F decays by gap_ext per segment; once every lane is <= 0 the remaining
    // segments are unchanged, and stopping there also keeps vF from wrapping.
    int maxF = static_cast<int>(ops.maxValue(vF));
    size_t limit = (maxF > 0) ? std::min(segLen, static_cast<size_t>(maxF / -gap_ext) + 1) : 0;

    for (size_t j = 0; j < limit; j++)
    {
        vH = pvHE.load(2 * j);
        vH = ops.max(vH, vF);
        pvHE.store(2 * j, vH);
        vMax = ops.max(vMax, vH);
        vF = ops.add(vF, gap_ext);
    }
}

template <typename Backend>
void Farrar<Backend>::propagateLazyF(VecType &vF, VecType &vMax)
{
    vF = ops.shift(vF, 0);
    size_t j = 0;
    VecType vHStore = pvHE.load(0);
    int vFCarry;
    while (ops.anyBiggerElement(vF, ops.add(vHStore, gap_open)))
    {
        vHStore = pvHE.load(2 * j);
        vHStore = ops.max(vHStore, vF);
        pvHE.store(2 * j, vHStore);
        vMax = ops.max(vMax, vHStore);

        j++;
        vF = ops.add(vF, gap_ext);

        if (j >= segLen)
        {
            vFCarry = ops.lastElement(vF);
            vF = ops.shift(vF, vFCarry);
            j = 0;
        }
    }
}

template <typename Backend>
void Farrar<Backend>::clearData()
{
    pvHE.release();
    vProfile.release();
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