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



#ifndef RVV_OPS_HPP
#define RVV_OPS_HPP

#include <riscv_vector.h>
#include <iostream>

#define ALWAYS_INLINE __attribute__((always_inline)) inline

class RvvOps{
    private:
        size_t VL = 0;

    public:

        RvvOps() {}
        void setVL(size_t vl){
            VL = vl;
        }

    // =========================================================================
    // LMUL = 1 (m1)
    // =========================================================================

    ALWAYS_INLINE vint16m1_t add(vint16m1_t a, vint16m1_t b) {
        return __riscv_vadd_vv_i16m1(a, b, VL);
    }
    ALWAYS_INLINE vint16m1_t add(vint16m1_t a, int16_t value) {
        return __riscv_vadd_vx_i16m1(a, value, VL);
    }
    ALWAYS_INLINE vint16m1_t sub(vint16m1_t a, vint16m1_t b) {
        return __riscv_vsub_vv_i16m1(a, b, VL);
    }
    ALWAYS_INLINE vint16m1_t sub(vint16m1_t a, int16_t value) {
        return __riscv_vsub_vx_i16m1(a, value, VL);
    }
    ALWAYS_INLINE vint16m1_t max(vint16m1_t a, vint16m1_t b) {
        return __riscv_vmax_vv_i16m1(a, b, VL);
    }
    ALWAYS_INLINE vint16m1_t max(vint16m1_t a, int16_t value) {
        return __riscv_vmax_vx_i16m1(a, value, VL);
    }
    ALWAYS_INLINE int16_t maxValue(vint16m1_t a) {
        vint16m1_t init = __riscv_vmv_s_x_i16m1(INT16_MIN, 1);
        vint16m1_t red  = __riscv_vredmax_vs_i16m1_i16m1(a, init, VL);
        return __riscv_vmv_x_s_i16m1_i16(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint16m1_t a, vint16m1_t b) {
        vbool16_t mask = __riscv_vmsgt_vv_i16m1_b16(a, b, VL);
        return __riscv_vcpop_m_b16(mask, VL) > 0;
    }
    ALWAYS_INLINE vint16m1_t shift(vint16m1_t a, int16_t carry) {
        return __riscv_vslide1up_vx_i16m1(a, carry, VL);
    }

    ALWAYS_INLINE vint16m1_t slideup(vint16m1_t a, size_t offset) {
        return __riscv_vslideup_vx_i16m1(a, a, offset, VL);
    }

    ALWAYS_INLINE int16_t lastElement(vint16m1_t a) {
        vint16m1_t tmp = __riscv_vslidedown_vx_i16m1(a, VL - 1, VL);
        return __riscv_vmv_x_s_i16m1_i16(tmp);
    }

    // =========================================================================
    // VINT32
    // =========================================================================

    ALWAYS_INLINE vint32m1_t add(vint32m1_t a, vint32m1_t b) {
        return __riscv_vadd_vv_i32m1(a, b, VL);
    }
    ALWAYS_INLINE vint32m1_t add(vint32m1_t a, int32_t value) {
        return __riscv_vadd_vx_i32m1(a, value, VL);
    }
    ALWAYS_INLINE vint32m1_t sub(vint32m1_t a, vint32m1_t b) {
        return __riscv_vsub_vv_i32m1(a, b, VL);
    }
    ALWAYS_INLINE vint32m1_t sub(vint32m1_t a, int32_t value) {
        return __riscv_vsub_vx_i32m1(a, value, VL);
    }
    ALWAYS_INLINE vint32m1_t max(vint32m1_t a, vint32m1_t b) {
        return __riscv_vmax_vv_i32m1(a, b, VL);
    }
    ALWAYS_INLINE vint32m1_t max(vint32m1_t a, int32_t value) {
        return __riscv_vmax_vx_i32m1(a, value, VL);
    }
    ALWAYS_INLINE int32_t maxValue(vint32m1_t a) {
        vint32m1_t init = __riscv_vmv_s_x_i32m1(INT32_MIN, 1);
        vint32m1_t red  = __riscv_vredmax_vs_i32m1_i32m1(a, init, VL);
        return __riscv_vmv_x_s_i32m1_i32(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint32m1_t a, vint32m1_t b) {
        vbool32_t mask = __riscv_vmsgt_vv_i32m1_b32(a, b, VL);
        return __riscv_vcpop_m_b32(mask, VL) > 0;
    }
    ALWAYS_INLINE vint32m1_t shift(vint32m1_t a, int32_t carry) {
        return __riscv_vslide1up_vx_i32m1(a, carry, VL);
    }

    ALWAYS_INLINE vint32m1_t slideup(vint32m1_t a, size_t offset) {
        return __riscv_vslideup_vx_i32m1(a, a, offset, VL);
    }

    ALWAYS_INLINE int32_t lastElement(vint32m1_t a) {
        vint32m1_t tmp = __riscv_vslidedown_vx_i32m1(a, VL - 1, VL);
        return __riscv_vmv_x_s_i32m1_i32(tmp);
    }

    // =========================================================================
    // LMUL = 2 (m2)
    // =========================================================================

    ALWAYS_INLINE vint16m2_t add(vint16m2_t a, vint16m2_t b) {
        return __riscv_vadd_vv_i16m2(a, b, VL);
    }
    ALWAYS_INLINE vint16m2_t add(vint16m2_t a, int16_t value) {
        return __riscv_vadd_vx_i16m2(a, value, VL);
    }
    ALWAYS_INLINE vint16m2_t sub(vint16m2_t a, vint16m2_t b) {
        return __riscv_vsub_vv_i16m2(a, b, VL);
    }
    ALWAYS_INLINE vint16m2_t sub(vint16m2_t a, int16_t value) {
        return __riscv_vsub_vx_i16m2(a, value, VL);
    }
    ALWAYS_INLINE vint16m2_t max(vint16m2_t a, vint16m2_t b) {
        return __riscv_vmax_vv_i16m2(a, b, VL);
    }
    ALWAYS_INLINE vint16m2_t max(vint16m2_t a, int16_t value) {
        return __riscv_vmax_vx_i16m2(a, value, VL);
    }
    ALWAYS_INLINE int16_t maxValue(vint16m2_t a) {
        vint16m1_t init = __riscv_vmv_s_x_i16m1(INT16_MIN, 1);
        vint16m1_t red  = __riscv_vredmax_vs_i16m2_i16m1(a, init, VL);
        return __riscv_vmv_x_s_i16m1_i16(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint16m2_t a, vint16m2_t b) {
        vbool8_t mask = __riscv_vmsgt_vv_i16m2_b8(a, b, VL);
        return __riscv_vcpop_m_b8(mask, VL) > 0;
    }
    ALWAYS_INLINE vint16m2_t shift(vint16m2_t a, int16_t carry) {
        return __riscv_vslide1up_vx_i16m2(a, carry, VL);
    }
    ALWAYS_INLINE vint16m2_t slideup(vint16m2_t a, size_t offset) {
        return __riscv_vslideup_vx_i16m2(a, a, offset, VL);
    }
    ALWAYS_INLINE int16_t lastElement(vint16m2_t a) {
        vint16m2_t tmp = __riscv_vslidedown_vx_i16m2(a, VL - 1, VL);
        return __riscv_vmv_x_s_i16m2_i16(tmp);
    }

    // =========================================================================
    // VINT32
    // =========================================================================

    ALWAYS_INLINE vint32m2_t add(vint32m2_t a, vint32m2_t b) {
        return __riscv_vadd_vv_i32m2(a, b, VL);
    }
    ALWAYS_INLINE vint32m2_t add(vint32m2_t a, int32_t value) {
        return __riscv_vadd_vx_i32m2(a, value, VL);
    }
    ALWAYS_INLINE vint32m2_t sub(vint32m2_t a, vint32m2_t b) {
        return __riscv_vsub_vv_i32m2(a, b, VL);
    }
    ALWAYS_INLINE vint32m2_t sub(vint32m2_t a, int32_t value) {
        return __riscv_vsub_vx_i32m2(a, value, VL);
    }
    ALWAYS_INLINE vint32m2_t max(vint32m2_t a, vint32m2_t b) {
        return __riscv_vmax_vv_i32m2(a, b, VL);
    }
    ALWAYS_INLINE vint32m2_t max(vint32m2_t a, int32_t value) {
        return __riscv_vmax_vx_i32m2(a, value, VL);
    }
    ALWAYS_INLINE int32_t maxValue(vint32m2_t a) {
        vint32m1_t init = __riscv_vmv_s_x_i32m1(INT32_MIN, 1);
        vint32m1_t red  = __riscv_vredmax_vs_i32m2_i32m1(a, init, VL);
        return __riscv_vmv_x_s_i32m1_i32(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint32m2_t a, vint32m2_t b) {
        vbool16_t mask = __riscv_vmsgt_vv_i32m2_b16(a, b, VL);
        return __riscv_vcpop_m_b16(mask, VL) > 0;
    }
    ALWAYS_INLINE vint32m2_t shift(vint32m2_t a, int32_t carry) {
        return __riscv_vslide1up_vx_i32m2(a, carry, VL);
    }
    ALWAYS_INLINE vint32m2_t slideup(vint32m2_t a, size_t offset) {
        return __riscv_vslideup_vx_i32m2(a, a, offset, VL);
    }

    ALWAYS_INLINE int32_t lastElement(vint32m2_t a) {
        vint32m2_t tmp = __riscv_vslidedown_vx_i32m2(a, VL - 1, VL);
        return __riscv_vmv_x_s_i32m2_i32(tmp);
    }

    // =========================================================================
    // LMUL = 4 (m4)
    // =========================================================================

    ALWAYS_INLINE vint16m4_t add(vint16m4_t a, vint16m4_t b) {
        return __riscv_vadd_vv_i16m4(a, b, VL);
    }
    ALWAYS_INLINE vint16m4_t add(vint16m4_t a, int16_t value) {
        return __riscv_vadd_vx_i16m4(a, value, VL);
    }
    ALWAYS_INLINE vint16m4_t max(vint16m4_t a, vint16m4_t b) {
        return __riscv_vmax_vv_i16m4(a, b, VL);
    }
    ALWAYS_INLINE vint16m4_t max(vint16m4_t a, int16_t value) {
        return __riscv_vmax_vx_i16m4(a, value, VL);
    }
    ALWAYS_INLINE int16_t maxValue(vint16m4_t a) {
        vint16m1_t init = __riscv_vmv_s_x_i16m1(INT16_MIN, 1);
        vint16m1_t red  = __riscv_vredmax_vs_i16m4_i16m1(a, init, VL);
        return __riscv_vmv_x_s_i16m1_i16(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint16m4_t a, vint16m4_t b) {
        vbool4_t mask = __riscv_vmsgt_vv_i16m4_b4(a, b, VL);
        return __riscv_vcpop_m_b4(mask, VL) > 0;
    }
    ALWAYS_INLINE vint16m4_t shift(vint16m4_t a, int16_t carry) {
        return __riscv_vslide1up_vx_i16m4(a, carry, VL);
    }
    ALWAYS_INLINE vint16m4_t slideup(vint16m4_t a, size_t offset) {
        return __riscv_vslideup_vx_i16m4(a, a, offset, VL);
    }

    ALWAYS_INLINE int16_t lastElement(vint16m4_t a) {
        vint16m4_t tmp = __riscv_vslidedown_vx_i16m4(a, VL - 1, VL);
        return __riscv_vmv_x_s_i16m4_i16(tmp);
    }

    // =========================================================================
    // VINT32
    // =========================================================================

    ALWAYS_INLINE vint32m4_t add(vint32m4_t a, vint32m4_t b) {
        return __riscv_vadd_vv_i32m4(a, b, VL);
    }
    ALWAYS_INLINE vint32m4_t add(vint32m4_t a, int32_t value) {
        return __riscv_vadd_vx_i32m4(a, value, VL);
    }
    ALWAYS_INLINE vint32m4_t sub(vint32m4_t a, vint32m4_t b) {
        return __riscv_vsub_vv_i32m4(a, b, VL);
    }
    ALWAYS_INLINE vint32m4_t sub(vint32m4_t a, int32_t value) {
        return __riscv_vsub_vx_i32m4(a, value, VL);
    }
    ALWAYS_INLINE vint32m4_t max(vint32m4_t a, vint32m4_t b) {
        return __riscv_vmax_vv_i32m4(a, b, VL);
    }
    ALWAYS_INLINE vint32m4_t max(vint32m4_t a, int32_t value) {
        return __riscv_vmax_vx_i32m4(a, value, VL);
    }
    ALWAYS_INLINE int32_t maxValue(vint32m4_t a) {
        vint32m1_t init = __riscv_vmv_s_x_i32m1(INT32_MIN, 1);
        vint32m1_t red  = __riscv_vredmax_vs_i32m4_i32m1(a, init, VL);
        return __riscv_vmv_x_s_i32m1_i32(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint32m4_t a, vint32m4_t b) {
        vbool8_t mask = __riscv_vmsgt_vv_i32m4_b8(a, b, VL);
        return __riscv_vcpop_m_b8(mask, VL) > 0;
    }
    ALWAYS_INLINE vint32m4_t shift(vint32m4_t a, int32_t carry) {
        return __riscv_vslide1up_vx_i32m4(a, carry, VL);
    }
    ALWAYS_INLINE vint32m4_t slideup(vint32m4_t a, size_t offset) {
        return __riscv_vslideup_vx_i32m4(a, a, offset, VL);
    }

    ALWAYS_INLINE int32_t lastElement(vint32m4_t a) {
        vint32m4_t tmp = __riscv_vslidedown_vx_i32m4(a, VL - 1, VL);
        return __riscv_vmv_x_s_i32m4_i32(tmp);
    }

    // =========================================================================
    // LMUL = 8 (m8)
    // =========================================================================

    ALWAYS_INLINE vint16m8_t add(vint16m8_t a, vint16m8_t b) {
        return __riscv_vadd_vv_i16m8(a, b, VL);
    }
    ALWAYS_INLINE vint16m8_t add(vint16m8_t a, int16_t value) {
        return __riscv_vadd_vx_i16m8(a, value, VL);
    }
    ALWAYS_INLINE vint16m8_t max(vint16m8_t a, vint16m8_t b) {
        return __riscv_vmax_vv_i16m8(a, b, VL);
    }
    ALWAYS_INLINE vint16m8_t max(vint16m8_t a, int16_t value) {
        return __riscv_vmax_vx_i16m8(a, value, VL);
    }
    ALWAYS_INLINE int16_t maxValue(vint16m8_t a) {
        vint16m1_t init = __riscv_vmv_s_x_i16m1(INT16_MIN, 1);
        vint16m1_t red  = __riscv_vredmax_vs_i16m8_i16m1(a, init, VL);
        return __riscv_vmv_x_s_i16m1_i16(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint16m8_t a, vint16m8_t b) {
        vbool2_t mask = __riscv_vmsgt_vv_i16m8_b2(a, b, VL);
        return __riscv_vcpop_m_b2(mask, VL) > 0;
    }
    ALWAYS_INLINE vint16m8_t shift(vint16m8_t a, int16_t carry) {
        return __riscv_vslide1up_vx_i16m8(a, carry, VL);
    }
    ALWAYS_INLINE vint16m8_t slideup(vint16m8_t a, size_t offset) {
        return __riscv_vslideup_vx_i16m8(a, a, offset, VL);
    }

    ALWAYS_INLINE int16_t lastElement(vint16m8_t a) {
        vint16m8_t tmp = __riscv_vslidedown_vx_i16m8(a, VL - 1, VL);
        return __riscv_vmv_x_s_i16m8_i16(tmp);
    }

    // =========================================================================
    // VINT32
    // =========================================================================

    ALWAYS_INLINE vint32m8_t add(vint32m8_t a, vint32m8_t b) {
        return __riscv_vadd_vv_i32m8(a, b, VL);
    }
    ALWAYS_INLINE vint32m8_t add(vint32m8_t a, int32_t value) {
        return __riscv_vadd_vx_i32m8(a, value, VL);
    }
    ALWAYS_INLINE vint32m8_t sub(vint32m8_t a, vint32m8_t b) {
        return __riscv_vsub_vv_i32m8(a, b, VL);
    }
    ALWAYS_INLINE vint32m8_t sub(vint32m8_t a, int32_t value) {
        return __riscv_vsub_vx_i32m8(a, value, VL);
    }
    ALWAYS_INLINE vint32m8_t max(vint32m8_t a, vint32m8_t b) {
        return __riscv_vmax_vv_i32m8(a, b, VL);
    }
    ALWAYS_INLINE vint32m8_t max(vint32m8_t a, int32_t value) {
        return __riscv_vmax_vx_i32m8(a, value, VL);
    }
    ALWAYS_INLINE int32_t maxValue(vint32m8_t a) {
        vint32m1_t init = __riscv_vmv_s_x_i32m1(INT32_MIN, 1);
        vint32m1_t red  = __riscv_vredmax_vs_i32m8_i32m1(a, init, VL);
        return __riscv_vmv_x_s_i32m1_i32(red);
    }
    ALWAYS_INLINE bool anyBiggerElement(vint32m8_t a, vint32m8_t b) {
        vbool4_t mask = __riscv_vmsgt_vv_i32m8_b4(a, b, VL);
        return __riscv_vcpop_m_b4(mask, VL) > 0;
    }
    ALWAYS_INLINE vint32m8_t shift(vint32m8_t a, int32_t carry) {
        return __riscv_vslide1up_vx_i32m8(a, carry, VL);
    }
    ALWAYS_INLINE vint32m8_t slideup(vint32m8_t a, size_t offset) {
        return __riscv_vslideup_vx_i32m8(a, a, offset, VL);
    }
    ALWAYS_INLINE int32_t lastElement(vint32m8_t a) {
        vint32m8_t tmp = __riscv_vslidedown_vx_i32m8(a, VL - 1, VL);
        return __riscv_vmv_x_s_i32m8_i32(tmp);
    }
};

#endif
