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

#ifndef BACKEND_HPP
#define BACKEND_HPP

#include "config.h"

// ============================================================
// Backend selection validation
// ============================================================

#if defined(USE_RVV) && defined(USE_AVX)

#error "USE_RVV and USE_AVX cannot be enabled simultaneously"

#endif

#if !defined(USE_RVV) && !defined(USE_AVX)

#error "No vector backend selected: use --vec-arch=avx or --vec-arch=rvv"

#endif


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

    static Ops createOps()
    {
        return Ops(TraitsT::maxVL());
    }
};

using RvvInt16M1 = BackendConfig<RvvTraits<vint16m1_t>>;
using RvvInt16M2 = BackendConfig<RvvTraits<vint16m2_t>>;
using RvvInt16M4 = BackendConfig<RvvTraits<vint16m4_t>>;
using RvvInt16M8 = BackendConfig<RvvTraits<vint16m8_t>>;

using RvvInt32M1 = BackendConfig<RvvTraits<vint32m1_t>>;
using RvvInt32M2 = BackendConfig<RvvTraits<vint32m2_t>>;
using RvvInt32M4 = BackendConfig<RvvTraits<vint32m4_t>>;
using RvvInt32M8 = BackendConfig<RvvTraits<vint32m8_t>>;

#endif


// ============================================================
// AVX2
// ============================================================

#if defined(USE_AVX)

#include "AVX/AvxOps.hpp"
#include "AVX/AvxTraits.hpp"

struct AvxInt16
{
    using VecType = Avx256Int16::VecType;
    using Traits  = Avx256Int16;
    using Ops     = Avx16Ops;

    static Ops createOps()
    {
        return Ops{};
    }
};

struct AvxInt32
{
    using VecType = Avx256Int32::VecType;
    using Traits  = Avx256Int32;
    using Ops     = Avx32Ops;

    static Ops createOps()
    {
        return Ops{};
    }
};

#endif // USE_AVX

#endif // BACKEND_HPP