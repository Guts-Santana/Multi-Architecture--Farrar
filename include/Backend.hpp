#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <immintrin.h>

// #if defined(USE_RVV) && defined(USE_AVX)
// #error "USE_RVV and USE_AVX cannot be enabled simultaneously"
// #endif

// #if !defined(USE_RVV) && !defined(USE_AVX)
// #error "One backend must be selected: USE_RVV or USE_AVX"
// #endif



// ============================================================
// RVV
// ============================================================

#if defined(USE_RVV)

#include "RVV/RvvOps.hpp"
#include "RVV/RvvTraits.hpp"

template <typename TraitsT>
struct BackendConfig
{
    using VecType = typename TraitsT::VecType;
    using Traits  = TraitsT;
    using Ops     = RvvOps;
};

using Rvv16M1Backend = BackendConfig<RvvTraits<vint16m1_t>>;
using Rvv16M2Backend = BackendConfig<RvvTraits<vint16m2_t>>;
using Rvv16M4Backend = BackendConfig<RvvTraits<vint16m4_t>>;
using Rvv16M8Backend = BackendConfig<RvvTraits<vint16m8_t>>;

using Rvv32M1Backend = BackendConfig<RvvTraits<vint32m1_t>>;
using Rvv32M2Backend = BackendConfig<RvvTraits<vint32m2_t>>;
using Rvv32M4Backend = BackendConfig<RvvTraits<vint32m4_t>>;
using Rvv32M8Backend = BackendConfig<RvvTraits<vint32m8_t>>;


#include "AVX/AvxOps.hpp"
#include "AVX/AvxTraits.hpp"


#endif

// ============================================================
// AVX2
// ============================================================


#include "AVX/AvxOps.hpp"
#include "AVX/AvxTraits.hpp"

struct AvxInt16
{
    using VecType = typename Avx256Int16::VecType;
    using Traits  = Avx256Int16;
    using Ops     = Avx16Ops;
};

struct AvxInt32
{
    using VecType = typename Avx256Int32::VecType;
    using Traits  = Avx256Int32;
    using Ops     = Avx32Ops;
};



#endif // BACKEND_HPP