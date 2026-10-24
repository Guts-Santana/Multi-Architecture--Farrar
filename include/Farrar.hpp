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


#ifndef FARRAR_HPP
#define FARRAR_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Buffer.hpp"
#include "Backend.hpp"

#ifdef FARRAR_WITH_MASA
// cell_t / score_t / INF used by the MASA block interface (alignBlock)
#include "libmasa/libmasaTypes.hpp"
#endif


/*
 * Scores are added to the DP cells, so penalties are negative.
 * A gap of length k costs gapOpen + (k - 1) * gapExt.
 */
struct ScoringScheme {
    int match    = 1;
    int mismatch = -3;
    int gapOpen  = -5;
    int gapExt   = -2;
};


template <typename Backend>
class Farrar {
protected:
    using VecType    = typename Backend::VecType;
    using Ops        = typename Backend::Ops;
    using Traits     = typename Backend::Traits;
    using BufferType = Buffer<Backend>;
    using ElemType   = typename Traits::ElemType;


    private:
        Ops ops;

    int gap_open;
    int gap_ext;
    int match;
    int mismatch;

    size_t VL;
    size_t segLen;
    int maxScore;
    std::string s0;
    std::string s1;
    std::string ftype;

    bool prefixScan;

    // H and E interleaved per segment: H(j) is vector 2*j, E(j) is vector 2*j + 1.
    BufferType pvHE;
    // One row of segLen vectors per residue, plus a last row (all mismatch)
    // used for characters outside the alphabet.
    BufferType vProfile;
    // True F of every cell of the current column (only filled by alignBlock)
    BufferType pvF;
    std::vector<int> s1Index;

#ifdef FARRAR_WITH_MASA
    // Set while alignBlock runs: F and E leave the block through MASA's
    // boundaries, so the F correction must also keep pvF and E up to date
    bool blockMode = false;
#else
    // Standalone builds never align blocks: the MASA-only steps compile away
    static constexpr bool blockMode = false;
#endif

    // "Minus infinity" that still leaves headroom for the non-saturating adds
    static constexpr ElemType LOW = std::numeric_limits<ElemType>::min() / 4;

#ifdef FARRAR_WITH_MASA
    // MASA boundaries are int and may contain -INF (-999999999): clamp into lane range
    static ElemType toLane(int v)
    {
        constexpr int HIGH = std::numeric_limits<ElemType>::max();
        return static_cast<ElemType>(v < LOW ? LOW : (v > HIGH ? HIGH : v));
    }
#endif

    static constexpr std::array<char, 5> alphabet = {
        'A', 'C', 'G', 'T', 'N'
    };
    static constexpr int invalidRow = alphabet.size();

    public:
    Farrar(std::string s0, std::string s1, std::string ftype,
           const ScoringScheme& scoring = ScoringScheme{})
        : maxScore(0), s0(s0), s1(s1), ftype(ftype)
    {
        VL = Traits::maxVL();
        ops.setVL(VL);
        segLen = (s0.length() + VL - 1) / VL;

        prefixScan = (ftype == "prefix-scan-f" || ftype == "prefix");

        setScoring(scoring);
    }

    //Implemented to MASA: the sequences arrive block by block through alignBlock
    Farrar(std::string ftype, const ScoringScheme& scoring = ScoringScheme{})
        : segLen(0), maxScore(0), ftype(ftype)
    {
        VL = Traits::maxVL();
        ops.setVL(VL);

        prefixScan = (ftype == "prefix-scan-f" || ftype == "prefix");

        setScoring(scoring);
    }


    ~Farrar()
    {
        clearData();
    }

    void setSequences(std::string s0, std::string s1);

    void setScoring(int match, int mismatch, int gapOpen, int gapExt);

    void setScoring(const ScoringScheme& scoring)
    {
        setScoring(scoring.match, scoring.mismatch, scoring.gapOpen, scoring.gapExt);
    }

    void buildProfile();

    void initMatrices();

    void encodeTarget();

    template <bool PrefixScan>
    VecType processColumns();

    // diag_h enters lane 0 of the first segment (0 in the standalone run)
    void processPrimaryPass(int profileRow, ElemType diag_h, VecType &vF, VecType &vMax);

    // top_f is the F entering the first row of the column (0 in the standalone run)
    void propagatePrefixScanF(VecType &vF, VecType &vMax, ElemType top_f);

    void propagateLazyF(VecType &vF, VecType &vMax, ElemType top_f);

    int obtainScore();

    void clearData();

#ifdef FARRAR_WITH_MASA
    score_t alignBlock(
        const char* query_seg, int q_len,
        const char* target_seg, int t_len,
        cell_t* row,
        cell_t* col,
        int abs_i, int abs_j
    );

    void initializeStripedLeftColumn(cell_t* col_buffer, int q_len);
    void unstripeRightColumnE(cell_t* col_buffer, int q_len);
    void unstripeRightColumnH(cell_t* col_buffer, int q_len);

    // Build a temporary profile specifically for a block's query segment
    void buildBlockProfile(const char* query_seg, int q_len);
#endif

    static inline int residueIndex(char c)
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
            return -1;
        }
    }

    inline int charToIndex(char c)
    {
        int idx = residueIndex(c);
        if (idx == -1)
        {
            std::cerr << "Invalid character: "
                      << (int)(unsigned char)c
                      << " (0x"
                      << std::hex << (int)(unsigned char)c
                      << std::dec << ")\n";
        }
        return idx;
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
