// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX2_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX2_HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../array_helpers.hpp"

namespace aie::detail {

template<unsigned Vectorization, unsigned Stage, typename Input, typename Output>
struct fft_dit<Vectorization, Stage, 2, Input, Output, cint16> : public fft_dit_common<Vectorization, Stage, 2, Input, Output, cint16>
{
    using   input_type = Input;
    using  output_type = Output;
    using twiddle_type = cint16;

    static constexpr unsigned native_mul_elems    = 16;
    static constexpr unsigned native_input_elems  = native_vector_length_v<input_type>;
    static constexpr unsigned native_output_elems = native_vector_length_v<output_type>;

    using input_vector  = vector<input_type, native_input_elems>;
    using input_ptr     = typename input_vector::storage_t;
    using output_vector = vector<output_type, native_output_elems>;
    using output_ptr    = typename output_vector::storage_t;

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cnt_(0), cntw_(0)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             output_type * __restrict out,
             unsigned n)
    {
        input_ptr *           pi  = (input_ptr *) x;
        output_ptr * restrict po  = (output_ptr *) out;
        twiddle_type *        ptw = (twiddle_type *) tw0;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [a, c] = load_data(pi);
            auto w      = load_twiddles(ptw);

            accum<cacc64, 16> o0 = ::mac_elem_16_conf(c, w, a, 0, 0, cmplx_mask_, 0, 0);
            accum<cacc64, 16> o1 = ::msc_elem_16_conf(c, w, a, 0, 0, cmplx_mask_, 0, 0);

            if constexpr (native_output_elems == native_mul_elems) {
                *po   = o0.template to_vector<output_type>(shift_);  po +=  n/32;
                *po   = o1.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 2*n);
            }
            else {
                *po++ = o0.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o0.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/16-1;
                *po++ = o1.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o1.template extract<8>(1).template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 4*n);
            }
        }
    }

private:
    __aie_inline
    auto load_data(input_ptr *& pi)
    {
        using return_acc_type = accum<cacc64, native_mul_elems>;
        using return_vec_type = vector<input_type, native_mul_elems>;
        return_acc_type a;
        return_vec_type c;

        if constexpr (std::is_same_v<input_type, cint16>) {
            if      constexpr (Stage == 0) {
                a.from_vector(input_vector(*pi), shift_tw_);  pi += Vectorization/16;
                c = *pi;                                      pi  = ::add_2d_ptr(pi, 1, Vectorization/16-1, cnt_, 1-Vectorization/16);
            }
            else {
                vector<input_type, 16> d0 = *pi++;
                vector<input_type, 16> d1 = *pi++;

                constexpr unsigned step = native_mul_elems >> Stage;
                auto [a_tmp, c_tmp] = interleave_unzip<input_type, 16>::run(d0, d1, step);

                a.from_vector(a_tmp, shift_tw_);
                c = c_tmp;
            }
        }
        else {
            using acc_type = accum<cacc64, 8>;
            using vec_type = vector<input_type, 8>;

            if      constexpr (Stage == 0) {
                a.insert(0, acc_type(vec_type(pi[0]), shift_tw_));
                a.insert(1, acc_type(vec_type(pi[1]), shift_tw_));  pi += 2*Vectorization/16;

                c.insert(0, pi[0]);
                c.insert(1, pi[1]);  pi = ::add_2d_ptr(pi, 2, Vectorization/16-1, cnt_, 2-2*Vectorization/16);
            }
            else if constexpr (Stage == 1) {
                a.insert(0, acc_type(vec_type(pi[0]), shift_tw_));
                a.insert(1, acc_type(vec_type(pi[2]), shift_tw_));

                c.insert(0, pi[1]);
                c.insert(1, pi[3]);
                pi += 4;
            }
            else {
                vector<input_type, 8> d0 = *pi++;
                vector<input_type, 8> d1 = *pi++;
                vector<input_type, 8> d2 = *pi++;
                vector<input_type, 8> d3 = *pi++;

                constexpr unsigned lo_mode = Stage == 2 ?   DINTLV_lo_256o512 :
                                             Stage == 3 ?   DINTLV_lo_128o256 :
                                           /*Stage == 4 ?*/ DINTLV_lo_64o128  ;
                constexpr unsigned hi_mode = lo_mode + 1;

                a.insert(0, acc_type(shfl(d0, d1, lo_mode), shift_tw_));
                a.insert(1, acc_type(shfl(d2, d3, lo_mode), shift_tw_));
                c.insert(0, shfl(d0, d1, hi_mode));
                c.insert(1, shfl(d2, d3, hi_mode));
            }
        }

        return std::make_pair(a, c);
    }

    __aie_inline
    auto load_twiddles(twiddle_type *& ptw)
    {
        vector<twiddle_type, 16> tw;

        if      constexpr (Stage == 0) {
            tw = broadcast<twiddle_type, 16>::run(*ptw);
            ptw = ::add_2d_ptr(ptw, 1, Vectorization/16-1, cntw_, 0);
        }
        else if constexpr (Stage == 1) {
#if __AIE_ARCH__ == 21
            tw = broadcast_2c16_T32_4x4(ptw[0], ptw[1]);
            tw = ::shuffle(tw, tw, INTLV_lo_128o256);
#else
            tw = broadcast_2c16_T32_8x2(ptw[0], ptw[1]);
#endif
            ptw += 2;
        }
        else if constexpr (Stage == 2) {
            tw = vector<twiddle_type, 4>(*(v4cint16*)ptw).template grow<16>();
            tw = ::shuffle(tw, tw, T32_2x16_lo);
            tw = ::shuffle(tw, tw, T64_2x8_lo);
            ptw += 4;
        }
        else if constexpr (Stage == 3) {
            tw = vector<twiddle_type, 8>(*(v8cint16*)ptw).template grow<16>();
            tw = ::shuffle(tw, tw, INTLV_lo_32o64);
            ptw += 8;
        }
        else if constexpr (Stage == 4) {
            tw = *(v16cint16*)ptw;
            ptw += 16;
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return tw;
    }

    unsigned shift_tw_, shift_;
    int cmplx_mask_;
    addr_t cnt_, cntw_;
};

#if __AIE_API_CBF16_SUPPORT__
template<unsigned Vectorization, unsigned Stage>
struct fft_dit<Vectorization, Stage, 2, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, Stage, 2, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;

    static constexpr unsigned native_elems = 16;

    using input_vector  = vector<input_type, native_elems>;
    using input_ptr     = typename input_vector::storage_t;
    using output_vector = vector<output_type, native_elems>;
    using output_ptr    = typename output_vector::storage_t;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cnt_(0), cntw_(0)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             output_type * __restrict out,
             unsigned n)
    {
        input_ptr *           pi  = (input_ptr *) x;
        output_ptr * restrict po  = (output_ptr *) out;
        twiddle_type *        ptw = (twiddle_type *) tw0;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [a, c] = load_data(pi);
            auto w      = load_twiddles(ptw);

            accum<caccfloat, 16> o0 = ::mac_elem_16_conf(c, w, a, 0, cmplx_mask_, 0, 0);
            accum<caccfloat, 16> o1 = ::msc_elem_16_conf(c, w, a, 0, cmplx_mask_, 0, 0);

            *po = o0.template to_vector<output_type>();  po +=  (n/2) / native_elems;
            *po = o1.template to_vector<output_type>();  po  = ::byte_incr(po, 64 - 2*n);
        }
    }

private:
    __aie_inline
    auto load_data(input_ptr *& pi)
    {
        accum<caccfloat, native_elems> a;
        vector<input_type, native_elems> c;

        if constexpr (Stage == 0) {
            a.from_vector(input_vector(*pi));  pi += Vectorization/16;
            c = *pi;                           pi  = ::add_2d_ptr(pi, 1, Vectorization/16-1, cnt_, 1-Vectorization/16);
        }
        else {
            vector<input_type, 16> d0 = *pi++;
            vector<input_type, 16> d1 = *pi++;

            constexpr unsigned step = native_elems >> Stage;
            auto [a_tmp, c_tmp] = interleave_unzip<input_type, 16>::run(d0, d1, step);

            a.from_vector(a_tmp);
            c = c_tmp;
        }

        return std::make_pair(a, c);
    }

    __aie_inline
    auto load_twiddles(twiddle_type *& ptw)
    {
        vector<twiddle_type, 16> tw;

        if      constexpr (Stage == 0) {
            tw = broadcast<twiddle_type, 16>::run(*ptw);
            ptw = ::add_2d_ptr(ptw, 1, Vectorization/16-1, cntw_, 0);
        }
        else if constexpr (Stage == 1) {
            tw = broadcast_2c16_T32_8x2(ptw[0], ptw[1]);
            ptw += 2;
        }
        else if constexpr (Stage == 2) {
            tw = vector<twiddle_type, 4>(*(v4cbfloat16*)ptw).template grow<16>();
            tw = ::shuffle(tw, tw, T32_2x16_lo);
            tw = ::shuffle(tw, tw, T64_2x8_lo);
            ptw += 4;
        }
        else if constexpr (Stage == 3) {
            tw = vector<twiddle_type, 8>(*(v8cbfloat16*)ptw).template grow<16>();
            tw = ::shuffle(tw, tw, INTLV_lo_32o64);
            ptw += 8;
        }
        else if constexpr (Stage == 4) {
            tw = *(v16cbfloat16*)ptw;
            ptw += 16;
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return tw;
    }

    int cmplx_mask_;
    addr_t cnt_, cntw_;
};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template<unsigned Vectorization, unsigned Stage>
struct fft_dit<Vectorization, Stage, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, Stage, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;

    static constexpr unsigned native_mul_elems = 16;
    static constexpr unsigned input_elems      = std::min(native_mul_elems, native_vector_length_v<input_type>);
    static constexpr unsigned output_elems     = std::min(native_mul_elems, native_vector_length_v<output_type>);

    using input_vector  = vector<input_type, input_elems>;
    using input_ptr     = typename input_vector::native_type;
    using output_vector = vector<output_type, output_elems>;
    using output_ptr    = typename output_vector::native_type;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cnt_(0), cntw_(0)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             output_type * __restrict out,
             unsigned n)
    {
        output_ptr * __restrict po  = (output_ptr *) out;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [a, c] = load_data(x);
            auto w      = load_twiddles(tw0);

            accum<caccfloat, 16> o0 = ::mac_elem_16_conf(c, w, a, 0, cmplx_mask_, 0, 0);
            accum<caccfloat, 16> o1 = ::msc_elem_16_conf(c, w, a, 0, cmplx_mask_, 0, 0);

            *po++ = o0.template extract<8>(0).template to_vector<output_type>();
            *po   = o0.template extract<8>(1).template to_vector<output_type>();  po +=  n / native_mul_elems - 1;
            *po++ = o1.template extract<8>(0).template to_vector<output_type>();
            *po   = o1.template extract<8>(1).template to_vector<output_type>();  po  = ::byte_incr(po, 64 - 4*n);
        }
    }

private:
    __aie_inline
    auto load_data(const input_type * __restrict& pi)
    {
        accum<caccfloat, native_mul_elems> a;
        vector<input_type, native_mul_elems> c;

        if constexpr (Stage == 0) {
            vector<input_type, native_mul_elems> tmp;
            tmp.load(pi);  pi += Vectorization;
            c.load(pi);    pi  = ::add_2d_ptr(pi, 16, Vectorization/16-1, cnt_, 16-Vectorization);
            a.from_vector(tmp);
        }
        else {
            vector<input_type, 16> d0, d1;
            d0.load(pi); pi += 16;
            d1.load(pi); pi += 16;

            constexpr unsigned step = native_mul_elems >> Stage;
            auto [a_tmp, c_tmp] = interleave_unzip<input_type, 16>::run(d0, d1, step);

            a.from_vector(a_tmp);
            c = c_tmp;
        }

        return std::make_pair(a, c);
    }

    __aie_inline
    auto load_twiddles(const twiddle_type * __restrict& ptw)
    {
        vector<twiddle_type, 16> tw;

        if      constexpr (Stage == 0) {
            tw = broadcast<twiddle_type, 16>::run(*ptw);
            ptw = ::add_2d_ptr(ptw, 1, Vectorization/16-1, cntw_, 0);
        }
        else if constexpr (Stage == 1) {
            tw = broadcast_2c32_T64_8x2(ptw[0], ptw[1]);
            ptw += 2;
        }
        else if constexpr (Stage == 2) {
            auto tmp = vector<twiddle_type, 4>(*(v4cfloat*)ptw).template grow_replicate<8>();
            tw.insert(0, ::shuffle(tmp, tmp, T64_4x4_lo));
            tw.insert(1, ::shuffle(tmp, tmp, T64_4x4_hi));
            ptw += 4;
        }
        else if constexpr (Stage == 3) {
            auto tmp = vector<twiddle_type, 8>(*(v8cfloat*)ptw);
            auto [tmp_a, tmp_b] = interleave_zip<twiddle_type, 8>::run(tmp, tmp, 1);
            tw = ::concat(tmp_a, tmp_b);
            ptw += 8;
        }
        else if constexpr (Stage == 4) {
            tw = *(v16cfloat*)ptw;
            ptw += 16;
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return tw;
    }

    int cmplx_mask_;
    addr_t cnt_, cntw_;
};
#endif

} // namespace aie::detail

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif // __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX2_HPP__
