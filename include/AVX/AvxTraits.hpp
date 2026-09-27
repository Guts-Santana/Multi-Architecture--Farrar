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