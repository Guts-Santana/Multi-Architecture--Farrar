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


#ifndef RVV_TRAITS_HPP
#define RVV_TRAITS_HPP

#include <riscv_vector.h>

#define ALWAYS_INLINE __attribute__((always_inline)) inline


template <typename VecType>
struct RvvTraits;

template <>
struct RvvTraits<vint16m1_t> {
    using ElemType = int16_t;
    using VecType  = vint16m1_t;

    static ALWAYS_INLINE vint16m1_t load(const int16_t* ptr, size_t vl) {
        return __riscv_vle16_v_i16m1(ptr, vl);
    }
    static ALWAYS_INLINE void store(int16_t* ptr, vint16m1_t vec, size_t vl) {
        __riscv_vse16_v_i16m1(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint16m1_t set(int16_t value, size_t VL){
        return __riscv_vmv_v_x_i16m1(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e16m1();
    }
};

template <>
struct RvvTraits<vint16m2_t> {
    using ElemType = int16_t;
    using VecType  = vint16m2_t;

    static ALWAYS_INLINE vint16m2_t load(const int16_t* ptr, size_t vl) {
        return __riscv_vle16_v_i16m2(ptr, vl);
    }
    static ALWAYS_INLINE void store(int16_t* ptr, vint16m2_t vec, size_t vl) {
        __riscv_vse16_v_i16m2(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint16m2_t set(int16_t value, size_t VL){
        return __riscv_vmv_v_x_i16m2(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e16m2();
    }
};

template <>
struct RvvTraits<vint16m4_t> {
    using ElemType = int16_t;
    using VecType  = vint16m4_t;
    
    static ALWAYS_INLINE vint16m4_t load(const int16_t* ptr, size_t vl) {
        return __riscv_vle16_v_i16m4(ptr, vl);
    }
    static ALWAYS_INLINE void store(int16_t* ptr, vint16m4_t vec, size_t vl) {
        __riscv_vse16_v_i16m4(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint16m4_t set(int16_t value, size_t VL){
        return __riscv_vmv_v_x_i16m4(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e16m4();
    }
};

template <>
struct RvvTraits<vint16m8_t> {
    using ElemType = int16_t;
    using VecType  = vint16m8_t;
    
    static ALWAYS_INLINE vint16m8_t load(const int16_t* ptr, size_t vl) {
        return __riscv_vle16_v_i16m8(ptr, vl);
    }
    static ALWAYS_INLINE void store(int16_t* ptr, vint16m8_t vec, size_t vl) {
        __riscv_vse16_v_i16m8(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint16m8_t set(int16_t value, size_t VL){
        return __riscv_vmv_v_x_i16m8(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e16m8();
    }
};


// Using INT32 for comparisons;

template <>
struct RvvTraits<vint32m1_t> {
    using ElemType = int32_t;
    using VecType  = vint32m1_t;
    
    static ALWAYS_INLINE vint32m1_t load(const int32_t* ptr, size_t vl) {
        return __riscv_vle32_v_i32m1(ptr, vl);
    }
    static ALWAYS_INLINE void store(int32_t* ptr, vint32m1_t vec, size_t vl) {
        __riscv_vse32_v_i32m1(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint32m1_t set(int32_t value, size_t VL){
        return __riscv_vmv_v_x_i32m1(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e32m1();
    }
};

template <>
struct RvvTraits<vint32m2_t> {
    using ElemType = int32_t;
    using VecType  = vint32m2_t;
    
    static ALWAYS_INLINE vint32m2_t load(const int32_t* ptr, size_t vl) {
        return __riscv_vle32_v_i32m2(ptr, vl);
    }
    static ALWAYS_INLINE void store(int32_t* ptr, vint32m2_t vec, size_t vl) {
        __riscv_vse32_v_i32m2(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint32m2_t set(int32_t value, size_t VL){
        return __riscv_vmv_v_x_i32m2(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e32m2();
    }
};

template <>
struct RvvTraits<vint32m4_t> {
    using ElemType = int32_t;
    using VecType  = vint32m4_t;
    
    static ALWAYS_INLINE vint32m4_t load(const int32_t* ptr, size_t vl) {
        return __riscv_vle32_v_i32m4(ptr, vl);
    }
    static ALWAYS_INLINE void store(int32_t* ptr, vint32m4_t vec, size_t vl) {
        __riscv_vse32_v_i32m4(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint32m4_t set(int32_t value, size_t VL){
        return __riscv_vmv_v_x_i32m4(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e32m4();
    }
};

template <>
struct RvvTraits<vint32m8_t> {
    using ElemType = int32_t;
    using VecType  = vint32m8_t;
    
    static ALWAYS_INLINE vint32m8_t load(const int32_t* ptr, size_t vl) {
        return __riscv_vle32_v_i32m8(ptr, vl);
    }
    static ALWAYS_INLINE void store(int32_t* ptr, vint32m8_t vec, size_t vl) {
        __riscv_vse32_v_i32m8(ptr, vec, vl);
    }
    static ALWAYS_INLINE vint32m8_t set(int32_t value, size_t VL){
        return __riscv_vmv_v_x_i32m8(value, VL);
    }
    static ALWAYS_INLINE size_t maxVL() {
        return __riscv_vsetvlmax_e32m8();
    }
};


#endif