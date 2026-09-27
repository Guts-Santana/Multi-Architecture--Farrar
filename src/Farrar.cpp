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

    Ops::setVL(VL);
    initMatrices();
    buildProfile();
    VecType vMaxBlock = Traits::set(0, VL);
    for (int i = 0; i < s1.length(); i++)
    {
        vMaxBlock = Ops::max(vMaxBlock, processColumn(i));
    }
    maxScore = static_cast<int>(Ops::maxValue(vMaxBlock));
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
    vH = Ops::shift(vH, 0);

    std::swap(pvHStore, pvHLoad);

    int profileIndex = charToIndex(s1[column]);
    int baseIndex = profileIndex * segLen;

    for (int j = 0; j < segLen; j++)
    {
        if (profileIndex == -1)
        {
            vH = Ops::add(vH, mismatch);
        }
        else
        {
            vProfileTemp = vProfile[baseIndex + j].load();
            vH = Ops::add(vH, vProfileTemp);
        }

        vE = pvE[j].load();

        vH = Ops::max(vH, vE);
        vH = Ops::max(vH, vF);
        vH = Ops::max(vH, 0);

        vMax = Ops::max(vMax, vH);

        pvHStore[j].store(vH);

        vH = Ops::add(vH, gap_open);
        vE = Ops::add(vE, gap_ext);
        vE = Ops::max(vE, vH);
        pvE[j].store(vE);

        vF = Ops::add(vF, gap_ext);
        vF = Ops::max(vF, vH);
        vH = pvHLoad[j].load();
    }
}

template <typename Backend>
void Farrar<Backend>::propagatePrefixScanF(VecType &vF, VecType &vMax)
{
    int accumulatedDecay;
    vF = Ops::shift(vF, 0);
    VecType vH, vShift;

    for (size_t offset = 1; offset < VL; offset <<= 1)
    {
        vShift = Ops::slideup(vF, offset);

        accumulatedDecay = offset * gap_ext;
        vShift = Ops::add(vShift, accumulatedDecay);

        vF = Ops::max(vF, vShift);
    }

    for (size_t j = 0; j < segLen; j++)
    {
        vH = pvHStore[j].load();
        vH = Ops::max(vH, vF);
        pvHStore[j].store(vH);
        vMax = Ops::max(vMax, vH);
        vF = Ops::add(vF, gap_ext);
    }
}

template <typename Backend>
void Farrar<Backend>::propagateLazyF(VecType &vF, VecType &vMax)
{
    vF = Ops::shift(vF, 0);
    size_t j = 0;
    VecType vHStore = pvHStore[j].load();
    int vFCarry;
    while (Ops::anyBiggerElement(vF, Ops::add(vHStore, gap_open)))
    {
        vHStore = pvHStore[j].load();
        vHStore = Ops::max(vHStore, vF);
        pvHStore[j].store(vHStore);
        vMax = Ops::max(vMax, vHStore);

        j++;
        vF = Ops::add(vF, gap_ext);

        if (j >= segLen)
        {
            vFCarry = Ops::lastElement(vF);
            vF = Ops::shift(vF, vFCarry);
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

template class Farrar<AvxInt16>;
template class Farrar<AvxInt32>;
