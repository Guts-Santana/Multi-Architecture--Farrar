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

#ifndef SVE_TRAITS_HPP
#define SVE_TRAITS_HPP

#include <arm_sve.h>
#include <cstddef>
#include <cstdint>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

struct SveInt16Traits
{
    using VecType = svint16_t;
    using ElemType = int16_t;

    static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
    {
        const svbool_t pg = svwhilelt_b16(uint64_t{0}, static_cast<uint64_t>(vl));
        return svld1_s16(pg, ptr);
    }

    static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
    {
        const svbool_t pg = svwhilelt_b16(uint64_t{0}, static_cast<uint64_t>(vl));
        svst1_s16(pg, ptr, vec);
    }

    static ALWAYS_INLINE VecType set(ElemType value, size_t)
    {
        return svdup_n_s16(value);
    }

    static ALWAYS_INLINE size_t maxVL()
    {
        return svcnth();
    }
};

struct SveInt32Traits
{
    using VecType = svint32_t;
    using ElemType = int32_t;

    static ALWAYS_INLINE VecType load(const ElemType *ptr, size_t vl)
    {
        const svbool_t pg = svwhilelt_b32(uint64_t{0}, static_cast<uint64_t>(vl));
        return svld1_s32(pg, ptr);
    }

    static ALWAYS_INLINE void store(ElemType *ptr, VecType vec, size_t vl)
    {
        const svbool_t pg = svwhilelt_b32(uint64_t{0}, static_cast<uint64_t>(vl));
        svst1_s32(pg, ptr, vec);
    }

    static ALWAYS_INLINE VecType set(ElemType value, size_t)
    {
        return svdup_n_s32(value);
    }

    static ALWAYS_INLINE size_t maxVL()
    {
        return svcntw();
    }
};

#endif
