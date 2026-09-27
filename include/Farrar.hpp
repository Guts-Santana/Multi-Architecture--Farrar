#ifndef FARRAR_HPP
#define FARRAR_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include "constants.hpp"
#include <vector>

#include "Buffer.hpp"
#include "Backend.hpp"


template <typename Backend>
class Farrar {
protected:
    using VecType    = typename Backend::VecType;
    using Ops        = typename Backend::Ops;
    using Traits     = typename Backend::Traits;
    using BufferType = Buffer<Backend>;

    int gap_open = GAP_OPEN;
    int gap_ext = GAP_EXT;
    int match = MATCH;
    int mismatch = MISMATCH;

    size_t VL;
    size_t segLen;
    int maxScore;
    std::string s0;
    std::string s1;
    std::string ftype;

    using PropagateF = void (Farrar<Backend>::*)(VecType&, VecType&);
    PropagateF propagateF;

    BufferType* pvHStore  = nullptr;
    BufferType* pvHLoad   = nullptr;
    BufferType* pvE       = nullptr;
    BufferType* vProfile  = nullptr;

    static constexpr std::array<char, 5> alphabet = {
        'A', 'C', 'G', 'T', 'N'
    };


    void processPrimaryPass(
        int* column,
        VecType& vF,
        VecType& vMax
    );

    public:
    Farrar(std::string s0, std::string s1, std::string ftype) : maxScore(0), s0(s0), s1(s1), ftype(ftype)
    {
        VL = Traits::maxVL();
        segLen = (s0.length() + VL - 1) / VL;

        if (ftype == "prefix-scan-f" || ftype == "prefix")
        {
            propagateF = &Farrar<Backend>::propagatePrefixScanF;
        }
        else
        {
            propagateF = &Farrar<Backend>::propagateLazyF;
        }
    }


    ~Farrar()
    {
        clearData();
    }

    void setSequences(std::string s0, std::string s1);

    void buildProfile();

    void initMatrices();

    VecType processColumn(int column);

    void processPrimaryPass(int column, VecType &vF, VecType &vMax);

    void propagatePrefixScanF(VecType &vF, VecType &vMax);

    void propagateLazyF(VecType &vF, VecType &vMax);

    int obtainScore();

    void clearData();

    inline int charToIndex(char c)
    {
        switch (c)
        {
        case 'A':
        case 'a':
            return 0;

        case 'C':
        case 'c':
            return 1;

        case 'G':
        case 'g':
            return 2;

        case 'T':
        case 't':
            return 3;

        case 'N':
        case 'n':
            return 4;

        default:
            std::cerr << "Invalid character: "
                      << (int)(unsigned char)c
                      << " (0x"
                      << std::hex << (int)(unsigned char)c
                      << std::dec << ")\n";
            return -1;
        }
    }

};




enum class RvvType {
    vint16m1, vint16m2, vint16m4, vint16m8,
    vint32m1, vint32m2, vint32m4, vint32m8,
    Unknown
};

inline RvvType parseRvvType(const std::string& str) {
    if (str == "vint16m1_t") return RvvType::vint16m1;
    if (str == "vint16m2_t") return RvvType::vint16m2;
    if (str == "vint16m4_t") return RvvType::vint16m4;
    if (str == "vint16m8_t") return RvvType::vint16m8;
    if (str == "vint32m1_t") return RvvType::vint32m1;
    if (str == "vint32m2_t") return RvvType::vint32m2;
    if (str == "vint32m4_t") return RvvType::vint32m4;
    if (str == "vint32m8_t") return RvvType::vint32m8;
    return RvvType::Unknown;
}

#endif
