#ifndef AVX_OPS_HPP
#define AVX_OPS_HPP

#include <iostream>
#include <immintrin.h>
#include <assert.h>

#define ALWAYS_INLINE __attribute__((always_inline)) inline


class Avx16Ops{
public:
    using VecType = __m256i;
    using ElemType = int16_t;

    static ALWAYS_INLINE void setVL(size_t)
    {
    }

    static ALWAYS_INLINE VecType add(VecType a, VecType b)
    {
        return _mm256_add_epi16(a, b);
    }

    static ALWAYS_INLINE VecType add(VecType a, ElemType value)
    {
        return _mm256_add_epi16(a, _mm256_set1_epi16(value));
    }

    static ALWAYS_INLINE VecType sub(VecType a, VecType b)
    {
        return _mm256_sub_epi16(a, b);
    }

    static ALWAYS_INLINE VecType sub(VecType a, ElemType value)
    {
        return _mm256_sub_epi16(a, _mm256_set1_epi16(value));
    }

    static ALWAYS_INLINE VecType max(VecType a, VecType b)
    {
        return _mm256_max_epi16(a, b);
    }

    static ALWAYS_INLINE VecType max(VecType a, ElemType value)
    {
        return _mm256_max_epi16(a, _mm256_set1_epi16(value));
    }

    static ALWAYS_INLINE ElemType maxValue(VecType a)
    {
        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i v = _mm_max_epi16(low, high);

        v = _mm_max_epi16(v, _mm_srli_si128(v, 8));
        v = _mm_max_epi16(v, _mm_srli_si128(v, 4));
        v = _mm_max_epi16(v, _mm_srli_si128(v, 2));

        return static_cast<ElemType>(_mm_extract_epi16(v, 0));
    }

    static ALWAYS_INLINE bool anyBiggerElement(VecType a, VecType b)
    {
        VecType cmp = _mm256_cmpgt_epi16(a, b);

        return !_mm256_testz_si256(cmp, cmp);
    }

    static ALWAYS_INLINE VecType shift(VecType a, ElemType carry)
    {
        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i lowShift  = _mm_slli_si128(low, 2);
        __m128i highShift = _mm_slli_si128(high, 2);

        ElemType boundary =
            static_cast<ElemType>(_mm_extract_epi16(low, 7));

        highShift = _mm_insert_epi16(highShift, boundary, 0);
        lowShift  = _mm_insert_epi16(lowShift, carry, 0);

        return _mm256_set_m128i(highShift, lowShift);
    }

    //Auxiliar Function
    template <int Offset>
    static ALWAYS_INLINE VecType slideupImpl(VecType a)
    {
        static_assert(Offset >= 0 && Offset <= 16);

        constexpr int Bytes = Offset * sizeof(ElemType);

        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i zero = _mm_setzero_si128();

        __m128i resultLow;
        __m128i resultHigh;

        if constexpr (Bytes == 0)
        {
            return a;
        }
        else if constexpr (Bytes < 16)
        {
            resultLow = _mm_slli_si128(low, Bytes);

            resultHigh = _mm_slli_si128(high, Bytes);

            // Bring the upper part of the low 128-bit lane
            // into the lower part of the high 128-bit lane.
            resultHigh = _mm_or_si128(
                resultHigh,
                _mm_srli_si128(low, 16 - Bytes)
            );
        }
        else if constexpr (Bytes == 16)
        {
            resultLow  = zero;
            resultHigh = low;
        }
        else
        {
            resultLow = zero;

            resultHigh = _mm_slli_si128(
                low,
                Bytes - 16
            );
        }

        return _mm256_set_m128i(resultHigh, resultLow);
    }

    static ALWAYS_INLINE VecType slideup(VecType a, size_t offset)
    {
        switch (offset)
        {
            case 0:
                return a;

            case 1:
                return slideupImpl<1>(a);

            case 2:
                return slideupImpl<2>(a);

            case 4:
                return slideupImpl<4>(a);

            case 8:
                return slideupImpl<8>(a);

            case 16:
                return slideupImpl<16>(a);

            default:
                assert(false && "Unsupported AVX16 slideup offset");
                return _mm256_setzero_si256();
        }
    }

    static ALWAYS_INLINE ElemType lastElement(VecType a)
    {
        return static_cast<ElemType>(
            _mm256_extract_epi16(a, 15)
        );
    }
};

class Avx32Ops {
public:
    using VecType  = __m256i;
    using ElemType = int32_t;

    static ALWAYS_INLINE void setVL(size_t)
    {
    }

    static ALWAYS_INLINE VecType add(VecType a, VecType b)
    {
        return _mm256_add_epi32(a, b);
    }

    static ALWAYS_INLINE VecType add(VecType a, ElemType value)
    {
        return _mm256_add_epi32(a, _mm256_set1_epi32(value));
    }

    static ALWAYS_INLINE VecType sub(VecType a, VecType b)
    {
        return _mm256_sub_epi32(a, b);
    }

    static ALWAYS_INLINE VecType sub(VecType a, ElemType value)
    {
        return _mm256_sub_epi32(a, _mm256_set1_epi32(value));
    }

    static ALWAYS_INLINE VecType max(VecType a, VecType b)
    {
        return _mm256_max_epi32(a, b);
    }

    static ALWAYS_INLINE VecType max(VecType a, ElemType value)
    {
        return _mm256_max_epi32(a, _mm256_set1_epi32(value));
    }

    static ALWAYS_INLINE ElemType maxValue(VecType a)
    {
        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i v = _mm_max_epi32(low, high);

        v = _mm_max_epi32(v, _mm_srli_si128(v, 8));
        v = _mm_max_epi32(v, _mm_srli_si128(v, 4));

        return _mm_cvtsi128_si32(v);
    }

    static ALWAYS_INLINE bool anyBiggerElement(VecType a, VecType b)
    {
        VecType cmp = _mm256_cmpgt_epi32(a, b);

        return !_mm256_testz_si256(cmp, cmp);
    }

    static ALWAYS_INLINE VecType shift(VecType a, ElemType carry)
    {
        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i lowShift  = _mm_slli_si128(low, 4);
        __m128i highShift = _mm_slli_si128(high, 4);

        ElemType boundary =
            _mm_extract_epi32(low, 3);

        highShift = _mm_insert_epi32(highShift, boundary, 0);
        lowShift  = _mm_insert_epi32(lowShift, carry, 0);

        return _mm256_set_m128i(highShift, lowShift);
    }

    //Auxiliar Function
    template <int Offset>
    static ALWAYS_INLINE VecType slideupImpl(VecType a)
    {
        static_assert(Offset >= 0 && Offset <= 8);

        constexpr int Bytes = Offset * sizeof(ElemType);

        __m128i low  = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);

        __m128i zero = _mm_setzero_si128();

        __m128i resultLow;
        __m128i resultHigh;

        if constexpr (Bytes == 0)
        {
            return a;
        }
        else if constexpr (Bytes < 16)
        {
            resultLow = _mm_slli_si128(low, Bytes);

            resultHigh = _mm_slli_si128(high, Bytes);

            resultHigh = _mm_or_si128(
                resultHigh,
                _mm_srli_si128(low, 16 - Bytes)
            );
        }
        else
        {
            resultLow  = zero;
            resultHigh = low;
        }

        return _mm256_set_m128i(resultHigh, resultLow);
    }

    static ALWAYS_INLINE VecType slideup(VecType a, size_t offset)
    {
        switch (offset)
        {
            case 0:
                return a;

            case 1:
                return slideupImpl<1>(a);

            case 2:
                return slideupImpl<2>(a);

            case 4:
                return slideupImpl<4>(a);

            case 8:
                return slideupImpl<8>(a);

            default:
                assert(false && "Unsupported AVX32 slideup offset");
                return _mm256_setzero_si256();
        }
    }

    static ALWAYS_INLINE ElemType lastElement(VecType a)
    {
        return _mm256_extract_epi32(a, 7);
    }
};
    

#endif
