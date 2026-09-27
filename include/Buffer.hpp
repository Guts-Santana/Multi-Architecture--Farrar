#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iostream>
#include <new>

#define ALWAYS_INLINE __attribute__((always_inline)) inline



template <typename Backend>
class Buffer {
    using VecType  = typename Backend::VecType;
    using Traits   = typename Backend::Traits;
    using ElemType = typename Traits::ElemType;

    private:
        ElemType* lanes = nullptr;
        size_t VL = 0;

        void allocate(size_t vl) {
            this->VL = vl;
            if (posix_memalign((void**)&lanes, 64, vl * sizeof(ElemType)) != 0) {
                lanes = nullptr;
                throw std::bad_alloc();
            }
        }

    public:

        ALWAYS_INLINE Buffer(size_t vl, ElemType value = 0){
            allocate(vl);
            if (lanes) {

                VecType vec = Traits::set(value, VL);
                Traits::store(lanes, vec, VL);
            }
        }

        ALWAYS_INLINE Buffer(const Buffer& other) {
            allocate(other.VL);
            if (lanes && other.lanes) {

                VecType vec = Traits::load(other.lanes, other.VL);
                Traits::store(lanes, vec, other.VL);
            }
        }

        ALWAYS_INLINE Buffer(Buffer&& other) noexcept : lanes(other.lanes), VL(other.VL) {
            other.lanes = nullptr;
            other.VL = 0;
        }

        ALWAYS_INLINE ~Buffer(){
            if (lanes) {
                free(lanes);
            }
        }

        ALWAYS_INLINE Buffer& operator=(const Buffer& other) {
            if (this != &other) {
                if (this->VL != other.VL) {
                    if (lanes){
                        free(lanes);
                    }
                    allocate(other.VL);
                }
                if (lanes && other.lanes && VL > 0) {
                    VecType vec = Traits::load(other.lanes, other.VL);
                    Traits::store(lanes, vec, other.VL);
                }
            }
            return *this;
        }

        ALWAYS_INLINE Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                if (lanes) free(lanes);
                lanes = other.lanes;
                VL = other.VL;
                other.lanes = nullptr;
                other.VL = 0;
            }
            return *this;
        }

        ALWAYS_INLINE ElemType& operator[](size_t i){
            return lanes[i];
        }

        ALWAYS_INLINE const ElemType& operator[](size_t i) const{
            return lanes[i];
        }

        ALWAYS_INLINE size_t size() const{
            return VL; 
        }

        ALWAYS_INLINE void swap(Buffer& other) noexcept{
            std::swap(lanes, other.lanes);
            std::swap(VL, other.VL);
        }

        ALWAYS_INLINE VecType load() const{
            return Traits::load(lanes, VL);
        }

        ALWAYS_INLINE void store(VecType vec){
            Traits::store(lanes, vec, VL);
        }

        ALWAYS_INLINE void print() const{
            for (size_t i = 0; i < VL; ++i) {
                std::cout << lanes[i] << " ";
            }
            std::cout << '\n';
        }
};

#endif
