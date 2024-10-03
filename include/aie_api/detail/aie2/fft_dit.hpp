// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FFT_DIT_HPP__
#define __AIE_API_DETAIL_AIE2_FFT_DIT_HPP__

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
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0);
            auto it_out0  = iterator(out);
            auto it_out1  = iterator(out + n / radix);

            for (int j = 0; j < block_size; ++j)
                chess_prepare_for_pipelining
                chess_loop_range(1,)
            {
                const auto out = fft.dit(*it_stage++);
                *it_out0++ = out[0];
                *it_out1++ = out[1];
            }
        }
    };

    template <unsigned Vectorization, typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage<3, Vectorization, Input, Output, Twiddle>
    {
        //TODO: Check if three iterator version performs better
        static constexpr unsigned radix = 3;
        static constexpr unsigned one_third_Q15 = 10923;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1);
#if 1
            auto it_out   = iterator(out);

            for (int j = 0; j < block_size; ++j)
                chess_prepare_for_pipelining
                chess_loop_range(1,)
            {
                const auto out = fft.dit(*it_stage++);
                *it_out = out[0]; it_out +=    block_size;
                *it_out = out[1]; it_out +=    block_size;
                *it_out = out[2]; it_out += -2*block_size + 1;
            }
#else
            auto it_out0  = iterator(out);
            auto it_out1  = iterator(out +     n_div_3);
            auto it_out2  = iterator(out + 2 * n_div_3);

            for (int j = 0; j < block_size; ++j)
                chess_prepare_for_pipelining
                chess_loop_range(1,)
            {
                const auto out = fft.dit(*it_stage++);
                *it_out0++ = out[0];
                *it_out1++ = out[1];
                *it_out2++ = out[2];
            }
#endif
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
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1, tw2);
            auto it_out   = iterator(out);

            for (int j = 0; j < block_size; ++j)
                chess_prepare_for_pipelining
                chess_loop_range(1,)
            {
                const auto out = fft.dit(*it_stage++);
                *it_out = out[0]; it_out +=    block_size;
                *it_out = out[1]; it_out +=    block_size;
                *it_out = out[2]; it_out +=    block_size;
                *it_out = out[3]; it_out += -3*block_size + 1;
            }
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
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1, tw2, tw3);
            auto it_out   = iterator(out);

            for (int j = 0; j < block_size; ++j)
                chess_prepare_for_pipelining
                chess_loop_range(1,)
            {
                const auto out = fft.dit(*it_stage++);
                *it_out = out[0]; it_out +=    block_size;
                *it_out = out[1]; it_out +=    block_size;
                *it_out = out[2]; it_out +=    block_size;
                *it_out = out[3]; it_out +=    block_size;
                *it_out = out[4]; it_out += -4*block_size + 1;
            }
        }
    };
}

#include "fft_dit_radix2.hpp"
#include "fft_dit_radix3.hpp"
#include "fft_dit_radix4.hpp"
#include "fft_dit_radix5.hpp"

#endif
