// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_SHIFT__HPP__
#define __AIE_API_DETAIL_SHIFT__HPP__

#include <bitset>
#include <cmath>
#include <cstdlib>

#include "vector.hpp"

namespace aie::detail {

#if __AIE_ARCH__ == 10

static constexpr int max_shift = 62;
static constexpr int min_shift = -1;

#elif __AIE_ARCH__ == 20

static constexpr int max_shift = 59;
static constexpr int min_shift = -4;

#endif

template <typename T, unsigned TypeBits, unsigned Elems>
struct shift_bits_impl
{
#ifdef __AIE_API_PROVIDE_DEFAULT_SCALAR_IMPLEMENTATION__
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        for (unsigned i = 0; i < Elems; ++i) {
            const T temp = v[i];

            if constexpr (vector_type::is_complex()) {
                std::bitset<vector_type::type_bits() / 2> bits_real(temp.real);
                std::bitset<vector_type::type_bits() / 2> bits_imag(temp.imag);

                bits_real = (bits_real << upshift) >> downshift;
                bits_imag = (bits_imag << upshift) >> downshift;

                ret[i] = T(bits_real.to_ulong(), bits_imag.to_ulong());
            }
            else {
                const std::bitset<vector_type::type_bits()> bits(temp);
                // TODO: fix the unsigned -> T workaround
                ret[i] = (T)unsigned(((bits << upshift) >> downshift).to_ulong());
            }
        }

        return ret;
    }
#endif
};

template <typename T, unsigned TypeBits, unsigned Elems>
struct shift_bits
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        return shift_bits_impl<T, TypeBits, Elems>::run(v, upshift, downshift);
    }
};

template <typename T, unsigned Elems>
using shift = shift_bits<T, type_bits_v<T>, Elems>;

}

#if __AIE_ARCH__ == 10

#include "aie1/shift.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/shift.hpp"

#endif

#endif
