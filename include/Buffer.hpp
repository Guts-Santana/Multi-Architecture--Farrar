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


#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdlib>
#include <iostream>
#include <new>

#define ALWAYS_INLINE __attribute__((always_inline)) inline


/*
 * Contiguous, 64-byte aligned array of `count` vectors of VL lanes each.
 * Vector i lives at data() + i * VL, so walking the segments is a single
 * sequential stream with no pointer chasing.
 */
template <typename Backend>
class Buffer {
    using VecType  = typename Backend::VecType;
    using Traits   = typename Backend::Traits;
    using ElemType = typename Traits::ElemType;

    private:
        ElemType* lanes = nullptr;
        size_t VL = 0;
        size_t count = 0;
        size_t capacity = 0;

    public:

        Buffer() = default;

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        ALWAYS_INLINE Buffer(Buffer&& other) noexcept
            : lanes(other.lanes), VL(other.VL), count(other.count), capacity(other.capacity) {
            other.lanes = nullptr;
            other.VL = other.count = other.capacity = 0;
        }

        ALWAYS_INLINE Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                release();
                swap(other);
            }
            return *this;
        }

        ALWAYS_INLINE ~Buffer(){
            release();
        }

        // Reallocates only when the current allocation is too small.
        void resize(size_t vectors, size_t vl) {
            size_t elems = vectors * vl;
            if (elems > capacity) {
                release();
                if (posix_memalign((void**)&lanes, 64, elems * sizeof(ElemType)) != 0) {
                    lanes = nullptr;
                    throw std::bad_alloc();
                }
                capacity = elems;
            }
            VL = vl;
            count = vectors;
        }

        void fill(ElemType value) {
            VecType vec = Traits::set(value, VL);
            for (size_t i = 0; i < count; ++i) {
                Traits::store(lanes + i * VL, vec, VL);
            }
        }

        void release() {
            if (lanes) {
                free(lanes);
            }
            lanes = nullptr;
            VL = count = capacity = 0;
        }

        ALWAYS_INLINE ElemType& at(size_t vec, size_t lane){
            return lanes[vec * VL + lane];
        }

        ALWAYS_INLINE const ElemType& at(size_t vec, size_t lane) const{
            return lanes[vec * VL + lane];
        }

        ALWAYS_INLINE ElemType* data(){
            return lanes;
        }

        ALWAYS_INLINE const ElemType* data() const{
            return lanes;
        }

        ALWAYS_INLINE size_t size() const{
            return count;
        }

        ALWAYS_INLINE void swap(Buffer& other) noexcept{
            std::swap(lanes, other.lanes);
            std::swap(VL, other.VL);
            std::swap(count, other.count);
            std::swap(capacity, other.capacity);
        }

        ALWAYS_INLINE VecType load(size_t vec) const{
            return Traits::load(lanes + vec * VL, VL);
        }

        ALWAYS_INLINE void store(size_t vec, VecType v){
            Traits::store(lanes + vec * VL, v, VL);
        }

        void print(size_t vec) const{
            for (size_t i = 0; i < VL; ++i) {
                std::cout << at(vec, i) << " ";
            }
            std::cout << '\n';
        }
};

#endif
