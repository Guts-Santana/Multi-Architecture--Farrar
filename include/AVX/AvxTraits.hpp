#ifndef AVX_TRAITS_HPP
#define AVX_TRAITS_HPP

#include <immintrin.h>
#include <cstdint>
#include <cstddef>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

/*------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------INT16-----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------*/

struct Avx256Int16
{

    using VecType = __m256i;
    using ElemType = int16_t;

    static constexpr size_t VL = 16;

    static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
    {
        return _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(ptr));
    }

    static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(ptr), vec);
    }

    static ALWAYS_INLINE VecType set(ElemType value, size_t vl)
    {
        return _mm256_set1_epi16(value);
    }

    static constexpr size_t maxVL()
    {
        return 16;
    }
};

/*------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------INT32-----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------*/

struct Avx256Int32
{

    using VecType = __m256i;
    using ElemType = int32_t;

    static constexpr size_t VL = 8;

    static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
    {
        return _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(ptr));
    }

    static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
    {
        _mm256_storeu_si256(reinterpret_cast<__m256i *>(ptr), vec);
    }

    static ALWAYS_INLINE VecType set(ElemType value, size_t vl)
    {
        return _mm256_set1_epi32(value);
    }

    static constexpr size_t maxVL()
    {
        return 8;
    }
};

/*------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------INT16-----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------*/

// struct Avx512Int16
// {

//     using VecType = __m512i;
//     using ElemType = int16_t;

//     static constexpr size_t VL = 32;

//     static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
//     {
//         return _mm512_loadu_si512(reinterpret_cast<const void *>(ptr));
//     }

//     static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
//     {
//         _mm512_storeu_si512(reinterpret_cast<void *>(ptr), vec);
//     }

//     static ALWAYS_INLINE VecType set(ElemType value, size_t vl)
//     {
//         return _mm512_set1_epi16(value);
//     }

//     static constexpr size_t maxVL()
//     {
//         return 32;
//     }
// };

/*------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------INT32-----------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------------*/

// struct Avx512Int32
// {

//     using VecType = __m512i;
//     using ElemType = int32_t;

//     static constexpr size_t VL = 16;

//     static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
//     {
//         return _mm512_loadu_si512(reinterpret_cast<const void *>(ptr));
//     }

//     static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
//     {
//         _mm512_storeu_si512(reinterpret_cast<void *>(ptr), vec);
//     }

//     static ALWAYS_INLINE VecType set(ElemType value, size_t vl)
//     {
//         return _mm512_set1_epi32(value);
//     }

//     static constexpr size_t maxVL()
//     {
//         return 16;
//     }
// };

#endif