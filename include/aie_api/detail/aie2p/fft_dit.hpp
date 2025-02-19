// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_HPP__

#if __AIE_API_SUPPORTS_FFT_CONST_PTR__
#define FFT_CONST_CAST(a, b) b
#else
#define FFT_CONST_CAST(a, b) const_cast<a>(b)
#endif

namespace aie::detail {

    template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage<2, Vectorization, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 2;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;

            FFT(shift_tw, shift, inv).run(x, tw0, out, n);
        }
    };

    template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage<4, Vectorization, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 4;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        const Twiddle * __restrict tw2,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;

            FFT(shift_tw, shift, inv).run(x, tw0, tw1, tw2, out, n);
        }
    };

    template <typename T, unsigned N>
    __aie_inline
    vector<T, N> shfl(vector<T, N> v, unsigned mode)
    {
        return ::shuffle(v, mode);
    }

    template <typename T, unsigned N>
    __aie_inline
    vector<T, N> shfl(vector<T, N> v1, vector<T, N> v2, unsigned mode)
    {
        return ::shuffle(v1, v2, mode);
    }

    __aie_inline
    inline cint16 swap16(cint16 in) {
        return cint16{in.imag, in.real};
    }

    __aie_inline
    inline v8cint16 swap16(v8cint16 in) {
        return ::extract_v8cint16(::shuffle(::set_v16cint16(0, in), ::undef_v16cint16(), T16_1x2_flip), 0);
    }

    __aie_inline
    inline v16cint16 swap16(v16cint16 in) {
        return ::shuffle(in, ::undef_v16cint16(), T16_1x2_flip);
    }

    __aie_inline
    inline v32cint16 swap16(v32cint16 in) {
        return ::concat(::shuffle(::extract_v16cint16(in, 0), T16_1x2_flip),
                        ::shuffle(::extract_v16cint16(in, 1), T16_1x2_flip));
    }

    __aie_inline
    inline v16cint16 broadcast_2c16 (cint16 a, cint16 b) {
        using bcast_t = mask64;
        bcast_t tmp = (bcast_t)(unsigned long long)(as_uint32(a) | (((uint64_t)::as_uint32(b))<<32));
        return (v16cint16) ::broadcast_to_v16int32(tmp);
    }

    __aie_inline
    inline v16cint16 broadcast_2c16_T32_4x4 (cint16 a, cint16 b) {
        return ::shuffle(broadcast_2c16(a, b), ::undef_v16cint16(), T32_4x4);
    }
}

#include "fft_dit_radix2.hpp"
#include "fft_dit_radix4.hpp"

#endif
