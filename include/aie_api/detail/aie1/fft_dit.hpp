// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_FFT_DIT_HPP__
#define __AIE_API_DETAIL_AIE1_FFT_DIT_HPP__

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

            //Output locations are separated by n / Radix elements
            unsigned n_div_3 = (n * one_third_Q15) >> 15; 
            int block_size   = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1);
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
                        const Twiddle * __restrict /*tw2*/,
                        unsigned n, unsigned shift_tw, unsigned shift, bool inv, Output * out)
        {
            constexpr unsigned stage = fft_get_stage<Input, Output, Twiddle>(radix, Vectorization);
            using FFT = fft_dit<Vectorization, stage, radix, Input, Output, Twiddle>;
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            int block_size = FFT::block_size(n);

            auto it_stage  = fft.begin_stage(x, tw0, tw1);
            
            if constexpr (Vectorization == 1) {
                auto it_out0  = iterator(out);
                auto it_out1  = iterator(out +     n / radix);
                auto it_out2  = iterator(out + 2 * n / radix);
                auto it_out3  = iterator(out + 3 * n / radix);

                for (int j = 0; j < block_size; ++j)
                    chess_prepare_for_pipelining
                    chess_loop_range(1,)
                {
                    const auto out = fft.dit(*it_stage++);
                    *it_out0++ = out[0];
                    *it_out1++ = out[1];
                    *it_out2++ = out[2];
                    *it_out3++ = out[3];
                }
            }
            else { //Currently worse performance if using the 4 pointer version on Vectorization > 1
                auto it_out0  = iterator(out);
                auto it_out1  = iterator(out + 2 * n / radix);

                for (int j = 0; j < block_size; ++j)
                    chess_prepare_for_pipelining
                    chess_loop_range(1,)
                {
                    const auto out = fft.dit(*it_stage++);
                    *it_out0 = out[0]; it_out0 +=  block_size;
                    *it_out0 = out[1]; it_out0 += -block_size + 1;
                    *it_out1 = out[2]; it_out1 +=  block_size;
                    *it_out1 = out[3]; it_out1 += -block_size + 1;
                }
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

            //Output locations are separated by n / Radix elements, divide by the vector size used by the iterator
            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1, tw2, tw3);
            auto it_out  = iterator(out);

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

    // Dynamic vectorization

    template <typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage_dyn_vec<2, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 2;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        unsigned n, unsigned vectorization,
                        unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            if (vectorization == 1) {
                fft_dit_stage<radix, 1, Input, Output, Twiddle>::run(x, tw0, n, shift_tw, shift, inv, out);
            }
            else if (vectorization == 2) {
                fft_dit_stage<radix, 2, Input, Output, Twiddle>::run(x, tw0, n, shift_tw, shift, inv, out);
            }
            else if (std::is_same_v<Input, cint16> && vectorization == 4) {
                fft_dit_stage<radix, 4, Input, Output, Twiddle>::run(x, tw0, n, shift_tw, shift, inv, out);
            }
            else {
                constexpr unsigned vec_dummy = 8;
                constexpr unsigned stage = 0;
                using FFT = fft_dit<vec_dummy, stage, radix, Input, Output, Twiddle>;
                using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

                FFT fft(shift_tw, shift, inv);

                int block_size = FFT::block_size(n);

                auto it_stage = fft.begin_stage(x, tw0, vectorization);
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
        }
    };

    template <typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage_dyn_vec<3, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 3;
        static constexpr unsigned one_third_Q15 = 10923;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        unsigned n, unsigned vectorization,
                        unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            if constexpr (std::is_same_v<Twiddle, cint32>)
                REQUIRES_MSG(vectorization >= 2, "Only vectorizations >= 2 are supported");
            else
                REQUIRES_MSG(vectorization >= 4, "Only vectorizations >= 4 are supported");

            constexpr unsigned vec_dummy = 4;
            constexpr unsigned stage = 0;
            using FFT = fft_dit<vec_dummy, stage, radix, Input, Output, Twiddle>;
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            //Output locations are separated by n / Radix elements
            unsigned n_div_3 = (n * one_third_Q15) >> 15; 
            int block_size   = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1, vectorization);
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
        }
    };

    template <typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage_dyn_vec<4, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 4;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        const Twiddle * __restrict tw2,
                        unsigned n, unsigned vectorization,
                        unsigned shift_tw, unsigned shift, bool inv, Output * out)
        {
            if (vectorization == 1) {
                fft_dit_stage<radix, 1, Input, Output, Twiddle>::run(x, tw0, tw1, tw2, n, shift_tw, shift, inv, out);
            }
            else {
                constexpr unsigned vec_dummy = 4;
                constexpr unsigned stage = 0;
                using FFT = fft_dit<vec_dummy, stage, radix, Input, Output, Twiddle>;
                using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

                FFT fft(shift_tw, shift, inv);

                int block_size = FFT::block_size(n);

                auto it_stage  = fft.begin_stage(x, tw0, tw1, tw2, vectorization);
                
                auto it_out0  = iterator(out);
                auto it_out1  = iterator(out + 2 * n / radix);

                for (int j = 0; j < block_size; ++j)
                    chess_prepare_for_pipelining
                    chess_loop_range(1,)
                {
                    const auto out = fft.dit(*it_stage++);
                    *it_out0 = out[0]; it_out0 +=  block_size;
                    *it_out0 = out[1]; it_out0 += -block_size + 1;
                    *it_out1 = out[2]; it_out1 +=  block_size;
                    *it_out1 = out[3]; it_out1 += -block_size + 1;
                }
            }
        }
    };

    template <typename Input, typename Output, typename Twiddle>
    struct fft_dit_stage_dyn_vec<5, Input, Output, Twiddle>
    {
        static constexpr unsigned radix = 5;

        __aie_inline
        static void run(const Input * __restrict x,
                        const Twiddle * __restrict tw0,
                        const Twiddle * __restrict tw1,
                        const Twiddle * __restrict tw2,
                        const Twiddle * __restrict tw3,
                        unsigned n, unsigned vectorization,
                        unsigned shift_tw, unsigned shift, bool inv, Output * __restrict out)
        {
            if constexpr (std::is_same_v<Twiddle, cint32>)
                REQUIRES_MSG(vectorization >= 2, "Only vectorizations >= 2 are supported");
            else
                REQUIRES_MSG(vectorization >= 4, "Only vectorizations >= 4 are supported");

            constexpr unsigned vec_dummy = 4;
            constexpr unsigned stage = 0;
            using FFT = fft_dit<vec_dummy, stage, radix, Input, Output, Twiddle>;
            using iterator = restrict_vector_iterator<Output, FFT::out_vector_size, 1, aie_dm_resource::none>;

            FFT fft(shift_tw, shift, inv);

            //Output locations are separated by n / Radix elements, divide by the vector size used by the iterator
            int block_size = FFT::block_size(n);

            auto it_stage = fft.begin_stage(x, tw0, tw1, tw2, tw3, vectorization);
            auto it_out  = iterator(out);

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

#include "fft_dit_acc48.hpp"
#include "fft_dit_acc80.hpp"
#include "fft_dit_acc32_fp.hpp"

#endif
