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
int Farrar<Backend>::obtainScore()
{
    initMatrices();
    buildProfile();
    VecType vMaxBlock = Traits::set(0, VL);
    for (int i = 0; i < s1.length(); i++)
    {
        vMaxBlock = ops.max(vMaxBlock, processColumn(i));
    }
    maxScore = static_cast<int>(ops.maxValue(vMaxBlock));
    return maxScore;
}

template <typename Backend>
void Farrar<Backend>::buildProfile()
{

    size_t total_blocks = alphabet.size() * segLen;

    posix_memalign((void **)&vProfile, 64, total_blocks * sizeof(BufferType));

    for (size_t k = 0; k < total_blocks; ++k)
    {
        new (&vProfile[k]) BufferType(VL, 0);
    }

    for (char residue : alphabet)
    {
        int resIdx = charToIndex(residue);
        if (resIdx == -1)
            continue;

        size_t baseOffset = static_cast<size_t>(resIdx) * segLen;

        for (size_t i = 0; i < segLen; i++)
        {
            for (size_t j = 0; j < VL; j++)
            {
                size_t idx = j * segLen + i;
                vProfile[baseOffset + i][j] = (idx < s0.length() && s0[idx] == residue) ? match : mismatch;
            }
        }
    }
}

template <typename Backend>
void Farrar<Backend>::initMatrices()
{
    segLen = (s0.length() + VL - 1) / VL;

    clearData();

    posix_memalign((void **)&pvHStore, 64, segLen * sizeof(BufferType));
    posix_memalign((void **)&pvHLoad, 64, segLen * sizeof(BufferType));
    posix_memalign((void **)&pvE, 64, segLen * sizeof(BufferType));

    for (size_t j = 0; j < segLen; ++j)
    {
        new (&pvHStore[j]) BufferType(VL, 0);
        new (&pvHLoad[j]) BufferType(VL, 0);
        new (&pvE[j]) BufferType(VL, 0);
    }
}

template <typename Backend>
typename Farrar<Backend>::VecType
Farrar<Backend>::processColumn(int column)
{
    VecType vF = Traits::set(0, VL);
    VecType vMax = Traits::set(0, VL);

    processPrimaryPass(column, vF, vMax);
    (this->*propagateF)(vF, vMax);

    return vMax;
}

template <typename Backend>
void Farrar<Backend>::processPrimaryPass(int column, VecType &vF, VecType &vMax)
{
    VecType vE;
    VecType vProfileTemp;
    VecType vH = pvHStore[segLen - 1].load();
    vH = ops.shift(vH, 0);

    std::swap(pvHStore, pvHLoad);

    int profileIndex = charToIndex(s1[column]);
    int baseIndex = profileIndex * segLen;

    for (int j = 0; j < segLen; j++)
    {
        if (profileIndex == -1)
        {
            vH = ops.add(vH, mismatch);
        }
        else
        {
            vProfileTemp = vProfile[baseIndex + j].load();
            vH = ops.add(vH, vProfileTemp);
        }

        vE = pvE[j].load();

        vH = ops.max(vH, vE);
        vH = ops.max(vH, vF);
        vH = ops.max(vH, 0);

        vMax = ops.max(vMax, vH);

        pvHStore[j].store(vH);

        vH = ops.add(vH, gap_open);
        vE = ops.add(vE, gap_ext);
        vE = ops.max(vE, vH);
        pvE[j].store(vE);

        vF = ops.add(vF, gap_ext);
        vF = ops.max(vF, vH);
        vH = pvHLoad[j].load();
    }
}

template <typename Backend>
void Farrar<Backend>::propagatePrefixScanF(VecType &vF, VecType &vMax)
{
    int accumulatedDecay;
    vF = ops.shift(vF, 0);
    VecType vH, vShift;

    for (size_t offset = 1; offset < VL; offset <<= 1)
    {
        vShift = ops.slideup(vF, offset);

        accumulatedDecay = offset * gap_ext;
        vShift = ops.add(vShift, accumulatedDecay);

        vF = ops.max(vF, vShift);
    }

    for (size_t j = 0; j < segLen; j++)
    {
        vH = pvHStore[j].load();
        vH = ops.max(vH, vF);
        pvHStore[j].store(vH);
        vMax = ops.max(vMax, vH);
        vF = ops.add(vF, gap_ext);
    }
}

template <typename Backend>
void Farrar<Backend>::propagateLazyF(VecType &vF, VecType &vMax)
{
    vF = ops.shift(vF, 0);
    size_t j = 0;
    VecType vHStore = pvHStore[j].load();
    int vFCarry;
    while (ops.anyBiggerElement(vF, ops.add(vHStore, gap_open)))
    {
        vHStore = pvHStore[j].load();
        vHStore = ops.max(vHStore, vF);
        pvHStore[j].store(vHStore);
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
    if (pvHStore)
    {
        free(pvHStore);
        pvHStore = nullptr;

        free(pvHLoad);
        pvHLoad = nullptr;

        free(pvE);
        pvE = nullptr;

        free(vProfile);
        vProfile = nullptr;
    }
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