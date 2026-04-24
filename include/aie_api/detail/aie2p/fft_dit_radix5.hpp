// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX5_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX5_HPP__

#include <algorithm>

#include "../array_helpers.hpp"

namespace aie::detail {

template<unsigned Vectorization, typename Input, typename Output>
struct fft_dit<Vectorization, 0, 5, Input, Output, cint16> : public fft_dit_common<Vectorization, 0, 5, Input, Output, cint16>
{
    using   input_type = Input;
    using  output_type = Output;
    using twiddle_type = cint16;

    static constexpr unsigned native_mul_elems = 16;
    static constexpr unsigned input_elems      = std::min(native_mul_elems, native_vector_length_v<input_type>);
    static constexpr unsigned output_elems     = std::min(native_mul_elems, native_vector_length_v<output_type>);

    using input_vector  = vector<input_type, input_elems>;
    using input_ptr     = typename input_vector::native_type;
    using output_vector = vector<output_type, output_elems>;
    using output_ptr    = typename output_vector::native_type;

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw), shift_(shift),
          cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_conjy_(inv ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y),
          cnt_(0), cnt_tw1_(0), cnt_tw2_(0), cnt_tw3_(0), cnt_tw4_(0)
    {
        twiddle_type k1 = as_cint16(((-10126 >> (15 - shift_tw_)) & 0xFFFF) | ((31164 >> (15 - shift_tw_)) << 16)); // -exp(-2j*pi/5)
        twiddle_type k2 = as_cint16( ( 26510 >> (15 - shift_tw_))           | ((19261 >> (15 - shift_tw_)) << 16)); // -(-exp(-2j*pi/5))^2
        q1_ = broadcast<twiddle_type, 16>::run(k1);
        q2_ = broadcast<twiddle_type, 16>::run(k2);
    }

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             const twiddle_type * __restrict tw2,
             const twiddle_type * __restrict tw3,
             output_type * __restrict out,
             unsigned n)
    {
        output_ptr * restrict po   = (output_ptr *) out;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [w1, w2, w3, w4]      = load_twiddles(tw0, tw1, tw2, tw3);
            auto [d0, d1, d2, d3, d4]  = load_data(x);

            accum<cacc64, 16> a_acc;
                              a_acc.from_vector(d0, shift_tw_);
            accum<cacc64, 16> d_acc = ::mul_elem_16_conf(d1, w1, cmplx_mask_, 0);
            accum<cacc64, 16> e_acc = ::mul_elem_16_conf(d2, w2, cmplx_mask_, 0);
            accum<cacc64, 16> f_acc = ::mul_elem_16_conf(d3, w3, cmplx_mask_, 0);
            accum<cacc64, 16> g_acc = ::mul_elem_16_conf(d4, w4, cmplx_mask_, 0);

            vector<cint32, 16> d = d_acc.template to_vector<cint32>(shift_tw_);
            vector<cint32, 16> e = e_acc.template to_vector<cint32>(shift_tw_);
            vector<cint32, 16> f = f_acc.template to_vector<cint32>(shift_tw_);
            vector<cint32, 16> g = g_acc.template to_vector<cint32>(shift_tw_);

            accum<cacc64, 16> o0 = add_accum<cacc64, 16>::run(a_acc, false, d_acc);
                              o0 = add_accum<cacc64, 16>::run(o0,    false, e_acc);
                              o0 = add_accum<cacc64, 16>::run(o0,    false, f_acc);
                              o0 = add_accum<cacc64, 16>::run(o0,    false, g_acc);

            accum<cacc64, 16> o1 = ::msc_elem_16_conf(d, q1_, a_acc, 0, 0, cmplx_mask_,       0, 0);
                              o1 = ::msc_elem_16_conf(e, q2_, o1,    0, 0, cmplx_mask_,       0, 0);
                              o1 = ::msc_elem_16_conf(f, q2_, o1,    0, 0, cmplx_mask_conjy_, 0, 0);
                              o1 = ::msc_elem_16_conf(g, q1_, o1,    0, 0, cmplx_mask_conjy_, 0, 0);

            accum<cacc64, 16> o2 = ::msc_elem_16_conf(d, q2_, a_acc, 0, 0, cmplx_mask_,       0, 0);
                              o2 = ::msc_elem_16_conf(e, q1_, o2,    0, 0, cmplx_mask_conjy_, 0, 0);
                              o2 = ::msc_elem_16_conf(f, q1_, o2,    0, 0, cmplx_mask_,       0, 0);
                              o2 = ::msc_elem_16_conf(g, q2_, o2,    0, 0, cmplx_mask_conjy_, 0, 0);

            accum<cacc64, 16> o3 = ::msc_elem_16_conf(d, q2_, a_acc, 0, 0, cmplx_mask_conjy_, 0, 0);
                              o3 = ::msc_elem_16_conf(e, q1_, o3,    0, 0, cmplx_mask_,       0, 0);
                              o3 = ::msc_elem_16_conf(f, q1_, o3,    0, 0, cmplx_mask_conjy_, 0, 0);
                              o3 = ::msc_elem_16_conf(g, q2_, o3,    0, 0, cmplx_mask_,       0, 0);

            accum<cacc64, 16> o4 = ::msc_elem_16_conf(d, q1_, a_acc, 0, 0, cmplx_mask_conjy_, 0, 0);
                              o4 = ::msc_elem_16_conf(e, q2_, o4,    0, 0, cmplx_mask_conjy_, 0, 0);
                              o4 = ::msc_elem_16_conf(f, q2_, o4,    0, 0, cmplx_mask_,       0, 0);
                              o4 = ::msc_elem_16_conf(g, q1_, o4,    0, 0, cmplx_mask_,       0, 0);

            if constexpr (std::is_same_v<output_type, cint16>) {
                *po   = o0.template to_vector<output_type>(shift_);  po +=  n/80;
                *po   = o1.template to_vector<output_type>(shift_);  po +=  n/80;
                *po   = o2.template to_vector<output_type>(shift_);  po +=  n/80;
                *po   = o3.template to_vector<output_type>(shift_);  po +=  n/80;
                *po   = o4.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 4*4*16*this->block_size(n));
            }
            else if constexpr (std::is_same_v<output_type, cint32>) {
                *po++ = o0.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o0.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/40-1;
                *po++ = o1.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o1.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/40-1;
                *po++ = o2.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o2.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/40-1;
                *po++ = o3.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o3.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/40-1;
                *po++ = o4.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o4.template extract<8>(1).template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 8*4*16*this->block_size(n));
            }
        }
    }

private:
    __aie_inline
    auto load_data(const input_type * __restrict& pi)
    {
        using return_vector_type = vector<input_type, native_mul_elems>;

        return_vector_type d0, d1, d2, d3, d4;

        d0.load(pi);
        d1.load(pi +     Vectorization);
        d2.load(pi + 2 * Vectorization);
        d3.load(pi + 3 * Vectorization);
        d4.load(pi + 4 * Vectorization);

        pi = ::add_2d_ptr(pi, 16+4*Vectorization, Vectorization/16-1, cnt_, 16);

        return std::make_tuple(d0, d1, d2, d3, d4);
    }

    __aie_inline
    auto load_twiddles(const twiddle_type * __restrict& ptw1,
                       const twiddle_type * __restrict& ptw2,
                       const twiddle_type * __restrict& ptw3,
                       const twiddle_type * __restrict& ptw4)
    {
//FIXME CRVO-11688: Unexpected: missing anti-dependency for M[7] : <3402> -> <3401>
#if 0 && __AIE_ARCH__ == 22
        twiddle_type w1 = *ptw1;
        twiddle_type w2 = *ptw2;
        twiddle_type w3 = *ptw3;
        twiddle_type w4 = *ptw4;
#else
        vector<twiddle_type, 16> w1 = broadcast<twiddle_type, 16>::run(*ptw1);
        vector<twiddle_type, 16> w2 = broadcast<twiddle_type, 16>::run(*ptw2);
        vector<twiddle_type, 16> w3 = broadcast<twiddle_type, 16>::run(*ptw3);
        vector<twiddle_type, 16> w4 = broadcast<twiddle_type, 16>::run(*ptw4);
#endif

        ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
        ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);
        ptw3 = ::add_2d_ptr(ptw3, 1, Vectorization/16-1, cnt_tw3_, 0);
        ptw4 = ::add_2d_ptr(ptw4, 1, Vectorization/16-1, cnt_tw4_, 0);

        return std::make_tuple(w1, w2, w3, w4);
    }

    unsigned shift_tw_, shift_;
    int cmplx_mask_, cmplx_mask_conjy_;
    addr_t cnt_, cnt_tw1_, cnt_tw2_, cnt_tw3_, cnt_tw4_;
    vector<twiddle_type, 16> q1_, q2_;
};

#if __AIE_API_CBF16_SUPPORT__
template<typename T, unsigned Vectorization>
    requires(utils::is_one_of_v<T, cbfloat16, cfloat>)
struct fft_dit<Vectorization, 0, 5, T, T, T> : public fft_dit_common<Vectorization, 0, 5, T, T, T>
{
    using   input_type = T;
    using  output_type = T;
    using twiddle_type = T;

    static constexpr unsigned native_mul_elems = 16;
    static constexpr unsigned input_elems      = native_mul_elems;
    static constexpr unsigned output_elems     = native_mul_elems;

    using input_vector  = vector<input_type, input_elems>;
    using input_ptr     = typename input_vector::native_type;
    using output_vector = vector<output_type, output_elems>;
    using output_ptr    = typename output_vector::native_type;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_conjy_(inv ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y),
          cnt_(0), cnt_tw1_(0), cnt_tw2_(0), cnt_tw3_(0), cnt_tw4_(0)
    {
        twiddle_type k1; //   -exp(-2j*pi/5)
        twiddle_type k2; // -(-exp(-2j*pi/5))^2

        if constexpr (std::is_same_v<T, cbfloat16>) {
            k1 = twiddle_type{-0.30859375f, 0.94921875f};
            k2 = twiddle_type{ 0.80859375f, 0.5859375f};
        }
        else {
            k1 = twiddle_type{-0.309016994f, 0.951056516f};
            k2 = twiddle_type{ 0.809016994f, 0.587785252f};
        }

        q1_ = broadcast<twiddle_type, 16>::run(k1);
        q2_ = broadcast<twiddle_type, 16>::run(k2);
    }

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             const twiddle_type * __restrict tw2,
             const twiddle_type * __restrict tw3,
             output_type * __restrict out,
             unsigned n)
    {
        output_ptr * restrict po   = (output_ptr *) out;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [w1, w2, w3, w4]      = load_twiddles(tw0, tw1, tw2, tw3);
            auto [d0, d1, d2, d3, d4]  = load_data(x);

            accum<caccfloat, 16> a_acc;
                                 a_acc.from_vector(d0);
            accum<caccfloat, 16> d_acc = ::mul_elem_16_conf(d1, w1, cmplx_mask_, 0);
            accum<caccfloat, 16> e_acc = ::mul_elem_16_conf(d2, w2, cmplx_mask_, 0);
            accum<caccfloat, 16> f_acc = ::mul_elem_16_conf(d3, w3, cmplx_mask_, 0);
            accum<caccfloat, 16> g_acc = ::mul_elem_16_conf(d4, w4, cmplx_mask_, 0);

            vector<twiddle_type, 16> d = d_acc.template to_vector<twiddle_type>();
            vector<twiddle_type, 16> e = e_acc.template to_vector<twiddle_type>();
            vector<twiddle_type, 16> f = f_acc.template to_vector<twiddle_type>();
            vector<twiddle_type, 16> g = g_acc.template to_vector<twiddle_type>();

            accum<caccfloat, 16> o0 = add_accum<caccfloat, 16>::run(a_acc, false, d_acc);
                                 o0 = add_accum<caccfloat, 16>::run(o0,    false, e_acc);
                                 o0 = add_accum<caccfloat, 16>::run(o0,    false, f_acc);
                                 o0 = add_accum<caccfloat, 16>::run(o0,    false, g_acc);

            accum<caccfloat, 16> o1 = ::msc_elem_16_conf(d, q1_, a_acc, 0, cmplx_mask_,       0, 0);
                                 o1 = ::msc_elem_16_conf(e, q2_, o1,    0, cmplx_mask_,       0, 0);
                                 o1 = ::msc_elem_16_conf(f, q2_, o1,    0, cmplx_mask_conjy_, 0, 0);
                                 o1 = ::msc_elem_16_conf(g, q1_, o1,    0, cmplx_mask_conjy_, 0, 0);

            accum<caccfloat, 16> o2 = ::msc_elem_16_conf(d, q2_, a_acc, 0, cmplx_mask_,       0, 0);
                                 o2 = ::msc_elem_16_conf(e, q1_, o2,    0, cmplx_mask_conjy_, 0, 0);
                                 o2 = ::msc_elem_16_conf(f, q1_, o2,    0, cmplx_mask_,       0, 0);
                                 o2 = ::msc_elem_16_conf(g, q2_, o2,    0, cmplx_mask_conjy_, 0, 0);

            accum<caccfloat, 16> o3 = ::msc_elem_16_conf(d, q2_, a_acc, 0, cmplx_mask_conjy_, 0, 0);
                                 o3 = ::msc_elem_16_conf(e, q1_, o3,    0, cmplx_mask_,       0, 0);
                                 o3 = ::msc_elem_16_conf(f, q1_, o3,    0, cmplx_mask_conjy_, 0, 0);
                                 o3 = ::msc_elem_16_conf(g, q2_, o3,    0, cmplx_mask_,       0, 0);

            accum<caccfloat, 16> o4 = ::msc_elem_16_conf(d, q1_, a_acc, 0, cmplx_mask_conjy_, 0, 0);
                                 o4 = ::msc_elem_16_conf(e, q2_, o4,    0, cmplx_mask_conjy_, 0, 0);
                                 o4 = ::msc_elem_16_conf(f, q2_, o4,    0, cmplx_mask_,       0, 0);
                                 o4 = ::msc_elem_16_conf(g, q1_, o4,    0, cmplx_mask_,       0, 0);

            constexpr unsigned incr = std::is_same_v<T, cbfloat16> ? 64 : 128;

            *po = o0.template to_vector<output_type>();  po +=  n/80;
            *po = o1.template to_vector<output_type>();  po +=  n/80;
            *po = o2.template to_vector<output_type>();  po +=  n/80;
            *po = o3.template to_vector<output_type>();  po +=  n/80;
            *po = o4.template to_vector<output_type>();  po  = ::byte_incr(po, incr - 4*incr*this->block_size(n));
        }
    }

private:
    __aie_inline
    auto load_data(const input_type * __restrict& pi)
    {
        using return_vector_type = vector<input_type, native_mul_elems>;

        return_vector_type d0, d1, d2, d3, d4;

        d0.load(pi);
        d1.load(pi +     Vectorization);
        d2.load(pi + 2 * Vectorization);
        d3.load(pi + 3 * Vectorization);
        d4.load(pi + 4 * Vectorization);

        pi = ::add_2d_ptr(pi, 16+4*Vectorization, Vectorization/16-1, cnt_, 16);

        return std::make_tuple(d0, d1, d2, d3, d4);
    }

    __aie_inline
    auto load_twiddles(const twiddle_type * __restrict& ptw1,
                       const twiddle_type * __restrict& ptw2,
                       const twiddle_type * __restrict& ptw3,
                       const twiddle_type * __restrict& ptw4)
    {
//TODO CRVO-11697: Update when scalar mul intrinsics are available
#if 0 && __AIE_ARCH__ == 22
        twiddle_type w1 = *ptw1;
        twiddle_type w2 = *ptw2;
        twiddle_type w3 = *ptw3;
        twiddle_type w4 = *ptw4;
#else
        vector<twiddle_type, 16> w1 = broadcast<twiddle_type, 16>::run(*ptw1);
        vector<twiddle_type, 16> w2 = broadcast<twiddle_type, 16>::run(*ptw2);
        vector<twiddle_type, 16> w3 = broadcast<twiddle_type, 16>::run(*ptw3);
        vector<twiddle_type, 16> w4 = broadcast<twiddle_type, 16>::run(*ptw4);
#endif

        ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
        ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);
        ptw3 = ::add_2d_ptr(ptw3, 1, Vectorization/16-1, cnt_tw3_, 0);
        ptw4 = ::add_2d_ptr(ptw4, 1, Vectorization/16-1, cnt_tw4_, 0);

        return std::make_tuple(w1, w2, w3, w4);
    }

    int cmplx_mask_, cmplx_mask_conjy_;
    addr_t cnt_, cnt_tw1_, cnt_tw2_, cnt_tw3_, cnt_tw4_;
    vector<twiddle_type, 16> q1_, q2_;
};
#endif

}

#endif
