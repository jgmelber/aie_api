// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX3_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX3_HPP__

#include <algorithm>

#include "../array_helpers.hpp"

namespace aie::detail {

template<unsigned Vectorization, typename Input, typename Output>
struct fft_dit<Vectorization, 0, 3, Input, Output, cint16> : public fft_dit_common<Vectorization, 0, 3, Input, Output, cint16>
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
          cnt_(0), cnt_tw1_(0), cnt_tw2_(0)
    {
        twiddle_type k = as_cint16((1u << (shift_tw_ - 1)) | ((28378u >> (15 - shift_tw_)) << 16)); // 0.5 + 0.5j*sqrt(3)
        qbuf_ = broadcast<twiddle_type, 16>::run(k);
    }

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             output_type * __restrict out,
             unsigned n)
    {
        output_ptr * __restrict po = (output_ptr *) out;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [w1, w2]  = load_twiddles(tw0, tw1);
            auto [a, b, c] = load_data(x);

            accum<cacc64, 16> a_acc(a, shift_tw_);
            accum<cacc64, 16> d_acc = ::mul_elem_16_conf(b, w1, cmplx_mask_, 0);
            accum<cacc64, 16> e_acc = ::mul_elem_16_conf(c, w2, cmplx_mask_, 0);

            vector<cint32, 16> d = d_acc.template to_vector<cint32>(shift_tw_);
            vector<cint32, 16> e = e_acc.template to_vector<cint32>(shift_tw_);

            accum<cacc64, 16> o0 = add_accum<cacc64, 16>::run(a_acc, false, d_acc);
                              o0 = add_accum<cacc64, 16>::run(o0,    false, e_acc);
            accum<cacc64, 16> o1 = ::msc_elem_16_conf(d, qbuf_, a_acc, 0, 0, cmplx_mask_,       0, 0);
                              o1 = ::msc_elem_16_conf(e, qbuf_, o1,    0, 0, cmplx_mask_conjy_, 0, 0);
            accum<cacc64, 16> o2 = ::msc_elem_16_conf(d, qbuf_, a_acc, 0, 0, cmplx_mask_conjy_, 0, 0);
                              o2 = ::msc_elem_16_conf(e, qbuf_, o2,    0, 0, cmplx_mask_,       0, 0);

            if constexpr (std::is_same_v<output_type, cint16>) {
                *po   = o0.template to_vector<output_type>(shift_);  po +=  n/48;
                *po   = o1.template to_vector<output_type>(shift_);  po +=  n/48;
                *po   = o2.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 2*4*16*this->block_size(n));
            }
            else if constexpr (std::is_same_v<output_type, cint32>) {
                *po++ = o0.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o0.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/24-1;
                *po++ = o1.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o1.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/24-1;
                *po++ = o2.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o2.template extract<8>(1).template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 4*4*16*this->block_size(n));
            }
        }
    }

private:
    __aie_inline
    auto load_data(const input_type * __restrict& pi)
    {
        using return_vector_type = vector<input_type, native_mul_elems>;

        return_vector_type a, b, c;

        a.load(pi);
        b.load(pi +     Vectorization);
        c.load(pi + 2 * Vectorization);

        pi = ::add_2d_ptr(pi, 16+2*Vectorization, Vectorization/16-1, cnt_,16);

        return std::make_tuple(a, b, c);
    }

    __aie_inline
    auto load_twiddles(const twiddle_type * __restrict& ptw1,
                       const twiddle_type * __restrict& ptw2)
    {
#if __AIE_ARCH__ == 22
        twiddle_type w1 = *ptw1;
        twiddle_type w2 = *ptw2;
#else
        vector<twiddle_type, 16> w1 = broadcast<twiddle_type, 16>::run(*ptw1);
        vector<twiddle_type, 16> w2 = broadcast<twiddle_type, 16>::run(*ptw2);
#endif

        ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
        ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);

        return std::make_pair(w1, w2);
    }

    unsigned shift_tw_, shift_;
    int cmplx_mask_, cmplx_mask_conjy_;
    addr_t cnt_, cnt_tw1_, cnt_tw2_;
    vector<twiddle_type, 16> qbuf_;
};

#if __AIE_API_CBF16_SUPPORT__
template<typename T, unsigned Vectorization>
    requires(utils::is_one_of_v<T, cbfloat16, cfloat>)
struct fft_dit<Vectorization, 0, 3, T, T, T> : public fft_dit_common<Vectorization, 0, 3, T, T, T>
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
          cnt_(0), cnt_tw1_(0), cnt_tw2_(0)
    {
        // 0.5 + 0.5j*sqrt(3)
        if constexpr (std::is_same_v<T, cbfloat16>)
            qbuf_ = broadcast<twiddle_type, 16>::run(twiddle_type{0.5f, 0.863281250f});
        else
            qbuf_ = broadcast<twiddle_type, 16>::run(twiddle_type{0.5f, 0.866025388f});
    }

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             output_type * __restrict out,
             unsigned n)
    {
        output_ptr * __restrict po = (output_ptr *) out;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [w1, w2]  = load_twiddles(tw0, tw1);
            auto [a, b, c] = load_data(x);

            accum<caccfloat, 16> a_acc(a);
            accum<caccfloat, 16> d_acc = ::mul_elem_16_conf(b, w1, cmplx_mask_, 0);
            accum<caccfloat, 16> e_acc = ::mul_elem_16_conf(c, w2, cmplx_mask_, 0);

            vector<twiddle_type, 16> d = d_acc.template to_vector<twiddle_type>();
            vector<twiddle_type, 16> e = e_acc.template to_vector<twiddle_type>();

            accum<caccfloat, 16> o0 = add_accum<caccfloat, 16>::run(a_acc, false, d_acc);
                                 o0 = add_accum<caccfloat, 16>::run(o0,    false, e_acc);
            accum<caccfloat, 16> o1 = ::msc_elem_16_conf(d, qbuf_, a_acc, 0, cmplx_mask_,       0, 0);
                                 o1 = ::msc_elem_16_conf(e, qbuf_, o1,    0, cmplx_mask_conjy_, 0, 0);
            accum<caccfloat, 16> o2 = ::msc_elem_16_conf(d, qbuf_, a_acc, 0, cmplx_mask_conjy_, 0, 0);
                                 o2 = ::msc_elem_16_conf(e, qbuf_, o2,    0, cmplx_mask_,       0, 0);

            constexpr unsigned incr = std::is_same_v<T, cbfloat16> ? 64 : 128;

            *po   = o0.template to_vector<output_type>();  po +=  n/48;
            *po   = o1.template to_vector<output_type>();  po +=  n/48;
            *po   = o2.template to_vector<output_type>();  po  = ::byte_incr(po, incr - 2*incr*this->block_size(n));
        }
    }

private:
    __aie_inline
    auto load_data(const input_type * __restrict& pi)
    {
        using return_vector_type = vector<input_type, native_mul_elems>;

        return_vector_type a, b, c;

        a.load(pi);
        b.load(pi +     Vectorization);
        c.load(pi + 2 * Vectorization);

        pi = ::add_2d_ptr(pi, 16+2*Vectorization, Vectorization/16-1, cnt_,16);

        return std::make_tuple(a, b, c);
    }

    __aie_inline
    auto load_twiddles(const twiddle_type * __restrict& ptw1,
                       const twiddle_type * __restrict& ptw2)
    {
//TODO CRVO-11697: Update when scalar mul intrinsics are available
#if 0 && __AIE_ARCH__ == 22
        twiddle_type w1 = *ptw1;
        twiddle_type w2 = *ptw2;
#else
        vector<twiddle_type, 16> w1 = broadcast<twiddle_type, 16>::run(*ptw1);
        vector<twiddle_type, 16> w2 = broadcast<twiddle_type, 16>::run(*ptw2);
#endif

        ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
        ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);

        return std::make_pair(w1, w2);
    }

    int cmplx_mask_, cmplx_mask_conjy_;
    addr_t cnt_, cnt_tw1_, cnt_tw2_;
    //TODO CRVO-11697: Should leverage scalar broadcast
    vector<twiddle_type, 16> qbuf_;
};
#endif

}

#endif
