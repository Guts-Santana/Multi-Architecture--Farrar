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

#ifndef SVE_OPS_HPP
#define SVE_OPS_HPP

#include <arm_sve.h>
#include <cstddef>
#include <cstdint>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

class Sve16Ops
{
    size_t VL = 0;

    ALWAYS_INLINE svbool_t active() const
    {
        return svwhilelt_b16(uint64_t{0}, static_cast<uint64_t>(VL));
    }

public:
    using VecType = svint16_t;
    using ElemType = int16_t;

    ALWAYS_INLINE void setVL(size_t vl)
    {
        VL = vl;
    }

    ALWAYS_INLINE VecType add(VecType a, VecType b)
    {
        return svadd_s16_x(active(), a, b);
    }

    ALWAYS_INLINE VecType add(VecType a, ElemType value)
    {
        return svadd_n_s16_x(active(), a, value);
    }

    ALWAYS_INLINE VecType sub(VecType a, VecType b)
    {
        return svsub_s16_x(active(), a, b);
    }

    ALWAYS_INLINE VecType sub(VecType a, ElemType value)
    {
        return svsub_n_s16_x(active(), a, value);
    }

    ALWAYS_INLINE VecType max(VecType a, VecType b)
    {
        return svmax_s16_x(active(), a, b);
    }

    ALWAYS_INLINE VecType max(VecType a, ElemType value)
    {
        return svmax_n_s16_x(active(), a, value);
    }

    ALWAYS_INLINE ElemType maxValue(VecType a)
    {
        return svmaxv_s16(active(), a);
    }

    ALWAYS_INLINE bool anyBiggerElement(VecType a, VecType b)
    {
        const svbool_t pg = active();
        return svptest_any(pg, svcmpgt_s16(pg, a, b));
    }

    ALWAYS_INLINE VecType shift(VecType a, ElemType carry)
    {
        return svinsr_n_s16(a, carry);
    }

    ALWAYS_INLINE VecType slideup(VecType a, size_t offset)
    {
        if (offset == 0)
            return a;
        if (offset >= VL)
            return svdup_n_s16(0);

        const size_t byteOffset = offset * sizeof(ElemType);
        const svuint8_t laneIndices = svindex_u8(0, 1);
        const svuint8_t byteIndices = svindex_u8(
            static_cast<uint8_t>(0 - byteOffset), 1);
        const svuint8_t shifted = svtbl_u8(svreinterpret_u8_s16(a), byteIndices);
        const svbool_t validBytes = svcmpge_n_u8(
            svptrue_b8(), laneIndices, static_cast<uint8_t>(byteOffset));

        return svreinterpret_s16_u8(
            svsel_u8(validBytes, shifted, svdup_n_u8(0)));
    }

    ALWAYS_INLINE ElemType lastElement(VecType a)
    {
        return svlastb_s16(active(), a);
    }
};

class Sve32Ops
{
    size_t VL = 0;

    ALWAYS_INLINE svbool_t active() const
    {
        return svwhilelt_b32(uint64_t{0}, static_cast<uint64_t>(VL));
    }

public:
    using VecType = svint32_t;
    using ElemType = int32_t;

    ALWAYS_INLINE void setVL(size_t vl)
    {
        VL = vl;
    }

    ALWAYS_INLINE VecType add(VecType a, VecType b)
    {
        return svadd_s32_x(active(), a, b);
    }

    ALWAYS_INLINE VecType add(VecType a, ElemType value)
    {
        return svadd_n_s32_x(active(), a, value);
    }

    ALWAYS_INLINE VecType sub(VecType a, VecType b)
    {
        return svsub_s32_x(active(), a, b);
    }

    ALWAYS_INLINE VecType sub(VecType a, ElemType value)
    {
        return svsub_n_s32_x(active(), a, value);
    }

    ALWAYS_INLINE VecType max(VecType a, VecType b)
    {
        return svmax_s32_x(active(), a, b);
    }

    ALWAYS_INLINE VecType max(VecType a, ElemType value)
    {
        return svmax_n_s32_x(active(), a, value);
    }

    ALWAYS_INLINE ElemType maxValue(VecType a)
    {
        return svmaxv_s32(active(), a);
    }

    ALWAYS_INLINE bool anyBiggerElement(VecType a, VecType b)
    {
        const svbool_t pg = active();
        return svptest_any(pg, svcmpgt_s32(pg, a, b));
    }

    ALWAYS_INLINE VecType shift(VecType a, ElemType carry)
    {
        return svinsr_n_s32(a, carry);
    }

    ALWAYS_INLINE VecType slideup(VecType a, size_t offset)
    {
        if (offset == 0)
            return a;
        if (offset >= VL)
            return svdup_n_s32(0);

        const size_t byteOffset = offset * sizeof(ElemType);
        const svuint8_t laneIndices = svindex_u8(0, 1);
        const svuint8_t byteIndices = svindex_u8(
            static_cast<uint8_t>(0 - byteOffset), 1);
        const svuint8_t shifted = svtbl_u8(svreinterpret_u8_s32(a), byteIndices);
        const svbool_t validBytes = svcmpge_n_u8(
            svptrue_b8(), laneIndices, static_cast<uint8_t>(byteOffset));

        return svreinterpret_s32_u8(
            svsel_u8(validBytes, shifted, svdup_n_u8(0)));
    }

    ALWAYS_INLINE ElemType lastElement(VecType a)
    {
        return svlastb_s32(active(), a);
    }
};

#endif
