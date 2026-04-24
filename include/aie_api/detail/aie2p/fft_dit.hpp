// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../blend.hpp"
#include "../shuffle.hpp"

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
    struct fft_dit_stage<3, Vectorization, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 3;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;

            FFT(shift_tw, shift, inv).run(x, tw0, tw1, out, n);
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

    template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage<5, Vectorization, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 5;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        const Twiddle * __restrict tw2,
                        const Twiddle * __restrict tw3,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;

            FFT(shift_tw, shift, inv).run(x, tw0, tw1, tw2, tw3, out, n);
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

    template <typename T>
        requires (is_complex_v<T>)
    __aie_inline
    T swap16(T in) {
        return T{in.imag, in.real};
    }

    template <typename T, unsigned N>
        requires (is_complex_v<T> && type_bits_v<T> == 32)
    __aie_inline
    vector<T, N> swap16(vector<T, N> in) {
        if constexpr (N <= 16) {
            return vector<T, 16>(::shuffle(in.template grow<16>(), T16_1x2_flip)).template extract<N>(0);
        }
        else if constexpr (N == 32) {
            return ::concat(::shuffle(in.template extract<16>(0), T16_1x2_flip),
                            ::shuffle(in.template extract<16>(1), T16_1x2_flip));
        }
    }

    template <typename T, unsigned N>
        requires (is_complex_v<T> && type_bits_v<T> == 64)
    __aie_inline
    vector<T, N> swap32(vector<T, N> in) {
        using U = utils::get_complex_component_type_t<T>;
        auto tmp1 = shuffle_up<U, 2*N>::run(in.template cast_to<U>(), 1);
        auto tmp2 = shuffle_down<U, 2*N>::run(in.template cast_to<U>(), 1);
        return select<U, 2*N>::run(tmp1, tmp2, mask<2*N>::from_uint32(0x55555555u)).template cast_to<T>();
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

#if __AIE_ARCH__ == 22
    __aie_inline
    inline v16cint16 broadcast_2c16_T32_8x2 (cint16 a, cint16 b) {
        return ::shuffle(broadcast_2c16(a, b), ::undef_v16cint16(), T32_8x2);
    }

#if __AIE_API_CBF16_SUPPORT__
    __aie_inline
    inline v16cbfloat16 broadcast_2c16 (cbfloat16 a, cbfloat16 b) {
        using bcast_t = mask64;
        bcast_t tmp = (bcast_t)(unsigned long long)(__builtin_bit_cast(uint32, a) | (((uint64_t)__builtin_bit_cast(uint32, b))<<32));
        return (v16cbfloat16) ::broadcast_to_v16int32(tmp);
    }

    __aie_inline
    inline v16cbfloat16 broadcast_2c16_T32_4x4 (cbfloat16 a, cbfloat16 b) {
        return ::shuffle(broadcast_2c16(a, b), ::undef_v16cbfloat16(), T32_4x4);
    }

    __aie_inline
    inline v16cbfloat16 broadcast_2c16_T32_8x2 (cbfloat16 a, cbfloat16 b) {
        return ::shuffle(broadcast_2c16(a, b), ::undef_v16cbfloat16(), T32_8x2);
    }
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
    __aie_inline
    inline v8cfloat broadcast_2c32 (cfloat a, cfloat b) {
        auto tmp = vector<cfloat, 8>(a, b).cast_to<int32>();
             tmp = ::broadcast_elem_128(tmp, 0);
        return tmp.cast_to<cfloat>();
    }

    __aie_inline
    inline v8cfloat broadcast_2c32_T64_4x2 (cfloat a, cfloat b) {
        return ::shuffle(broadcast_2c32(a, b), ::undef_v8cfloat(), T64_4x2);
    }

    __aie_inline
    inline vector<cfloat,16> broadcast_2c32_T64_8x2 (cfloat a, cfloat b) {
        vector<cfloat, 8> tmp = broadcast_2c32(a, b);
        return ::concat(::shuffle(tmp, tmp, T64_8x2_lo),
                        ::shuffle(tmp, tmp, T64_8x2_hi));
    }
#endif
#endif
}

#include "fft_dit_radix2.hpp"
#include "fft_dit_radix3.hpp"
#include "fft_dit_radix4.hpp"
#include "fft_dit_radix5.hpp"

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif // __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX2_HPP__
