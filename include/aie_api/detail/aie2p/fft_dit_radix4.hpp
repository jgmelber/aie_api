// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX4_HPP__
#define __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX4_HPP__

#include "../array_helpers.hpp"

namespace aie::detail {

template<unsigned Vectorization, unsigned Stage, typename Output>
struct fft_dit<Vectorization, Stage, 4, cint32, Output, cint16> : public fft_dit_common<Vectorization, Stage, 4, cint32, Output, cint16>
{
    using   input_type = cint32;
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
          cmplx_mask_mj_(inv ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y),
          cnt0_(0), cnt1_(0),
          cnt_tw0_(0), cnt_tw1_(0), cnt_tw2_(0)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             const twiddle_type * __restrict tw2,
             output_type * __restrict out,
             unsigned n)
    {
        input_ptr *           pi   = (input_ptr *) x;
        input_ptr *           pi1  = (input_ptr *) (x + Vectorization);
        output_ptr * restrict po   = (output_ptr *) out;
        // Note reversal of twiddle args
        twiddle_type *        ptw0 = (twiddle_type *) tw1;
        twiddle_type *        ptw1 = (twiddle_type *) tw0;
        twiddle_type *        ptw2 = (twiddle_type *) tw2;

        if constexpr (Stage == 1) {
            ptw0 += 1;
            ptw1 += 1;
            ptw2 += 1;
        }

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            accum<cacc64, 16> a;
            vector<input_type, 16> b, c, d;
            
            if constexpr (Stage == 0)
                std::tie(a, b, c, d) = load_data(pi, pi1);
            else
                std::tie(a, b, c, d) = load_data(pi);

            auto [w1, w2, w3] = load_twiddles(ptw0, ptw1, ptw2);

            accum<cacc64, 16> g = ::mac_elem_16_conf(c,        w2,  a, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 16> h = ::msc_elem_16_conf(c,        w2,  a, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 16> k = ::mul_elem_16_conf(b,        w1,           cmplx_mask_,    0);
            accum<cacc64, 16> l = ::mul_elem_16_conf(b, swap16(w1),          cmplx_mask_mj_, 0);
            
            accum<cacc64, 16> o0 = ::addmac_elem_16_conf(d,        w3,  g, k, 0, 0, cmplx_mask_,    0, 0, 0);
            accum<cacc64, 16> o1 = ::addmsc_elem_16_conf(d, swap16(w3), h, l, 0, 0, cmplx_mask_mj_, 0, 0, 0);
            accum<cacc64, 16> o2 = ::addmsc_elem_16_conf(d,        w3,  g, k, 0, 0, cmplx_mask_,    0, 0, 1);
            accum<cacc64, 16> o3 = ::addmac_elem_16_conf(d, swap16(w3), h, l, 0, 0, cmplx_mask_mj_, 0, 0, 1);
        
            if constexpr (std::is_same_v<output_type, cint16>) {
                *po   = o0.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o1.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o2.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o3.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 3*n);
            }
            else if constexpr (std::is_same_v<output_type, cint32>) {
                *po++ = o0.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o0.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o1.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o1.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o2.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o2.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o3.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o3.template extract<8>(1).template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 6*n);
            }
        }
    }

private:
    __aie_inline
    auto load_data(input_ptr *& pi0, input_ptr *& pi1) requires (Stage == 0)
    {
        using return_acc_type = accum<cacc64, native_mul_elems>;
        using return_vec_type = vector<input_type, native_mul_elems>;
        return_acc_type a;
        return_vec_type a_tmp;
        return_vec_type b, c, d;

        a_tmp.insert(0, pi0[0]);
        a_tmp.insert(1, pi0[1]);  pi0 += 2*Vectorization/8;

        b.insert(0, pi1[0]);
        b.insert(1, pi1[1]);      pi1 += 2*Vectorization/8;

        c.insert(0, pi0[0]);
        c.insert(1, pi0[1]);      pi0 = ::add_2d_ptr(pi0, 2*Vectorization/16+2, Vectorization/16-1, cnt0_, 2-2*Vectorization/8);

        d.insert(0, pi1[0]);
        d.insert(1, pi1[1]);      pi1 = ::add_2d_ptr(pi1, 2*Vectorization/16+2, Vectorization/16-1, cnt1_, 2-2*Vectorization/8);

        a.from_vector(a_tmp, shift_tw_);

        return std::make_tuple(a, b, c, d);
    }

    __aie_inline
    auto load_data(input_ptr *& pi) requires (Stage != 0)
    {
        using ptr_type = std::conditional_t<std::is_same_v<input_type, cint32>, v4cint32, v4cint16>;

        using return_acc_type = accum<cacc64, native_mul_elems>;
        using return_vec_type = vector<input_type, native_mul_elems>;
        return_acc_type a;
        return_vec_type b, c, d;

        if constexpr (Stage == 1) {
            return_vec_type a_tmp;

            ptr_type * tmp = (ptr_type *)pi;

            a_tmp.insert(0, *tmp);    tmp += 4;
            a_tmp.insert(1, *tmp);    tmp += 4;
            a_tmp.insert(2, *tmp);    tmp += 4;
            a_tmp.insert(3, *tmp);    tmp -= 10;

            c.insert(0, *tmp);        tmp += 4;
            c.insert(1, *tmp);        tmp += 4;
            c.insert(2, *tmp);        tmp += 4;
            c.insert(3, *tmp);        tmp -= 13;

            b.insert(0, *tmp);        tmp += 4;
            b.insert(1, *tmp);        tmp += 4;
            b.insert(2, *tmp);        tmp += 4;
            b.insert(3, *tmp);        tmp -= 10;

            d.insert(0, *tmp);        tmp += 4;
            d.insert(1, *tmp);        tmp += 4;
            d.insert(2, *tmp);        tmp += 4;
            d.insert(3, *tmp);        tmp += 1;

            a.from_vector(a_tmp, shift_tw_);

            pi = (input_ptr*)tmp;
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return std::make_tuple(a, b, c, d);
    }

    __aie_inline
    auto load_twiddles(twiddle_type *& ptw0, twiddle_type *& ptw1, twiddle_type *& ptw2)
    {
        if      constexpr (Stage == 0) {
            vector<twiddle_type, 16> tw0, tw1, tw2;

            tw0 = broadcast<twiddle_type, 16>::run(*ptw0);    ptw0 = ::add_2d_ptr(ptw0, 1, Vectorization/16-1, cnt_tw0_, 0);
            tw1 = broadcast<twiddle_type, 16>::run(*ptw1);    ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
            tw2 = broadcast<twiddle_type, 16>::run(*ptw2);    ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);

            return std::make_tuple(tw0, tw1, tw2);
        }
        else if constexpr (Stage == 1) {
            vector<twiddle_type, 16> tw0, tw1, tw2;

            int tw_step = 1;
            ptw0 = chess_copy(ptw0); ptw1 = chess_copy(ptw1); ptw2 = chess_copy(ptw2);

            cint16 wa, wb, wc, wd;
            wa = ptw0[-tw_step];
            wb = *ptw0;      ptw0 += 2*tw_step;    ptw0 = chess_copy(ptw0);
            wc = ptw0[-tw_step];
            wd = *ptw0;      ptw0 += 2*tw_step;    ptw0 = chess_copy(ptw0);
            tw0 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );
            wa = ptw1[-tw_step];
            wb = *ptw1;      ptw1 += 2*tw_step;    ptw1 = chess_copy(ptw1);
            wc = ptw1[-tw_step];
            wd = *ptw1;      ptw1 += 2*tw_step;    ptw1 = chess_copy(ptw1);
            tw1 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );
            wa = ptw2[-tw_step];
            wb = *ptw2;      ptw2 += 2*tw_step;    ptw2 = chess_copy(ptw2);
            wc = ptw2[-tw_step];
            wd = *ptw2;      ptw2 += 2*tw_step;    ptw2 = chess_copy(ptw2);
            tw2 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );

            return std::make_tuple(tw0, tw1, tw2);
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }
    }

    unsigned shift_tw_, shift_;
    int cmplx_mask_;
    int cmplx_mask_mj_;
    addr_t cnt0_, cnt1_;
    addr_t cnt_tw0_, cnt_tw1_, cnt_tw2_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 4, cint32, Output, cint16> : public fft_dit_common<Vectorization, 2, 4, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;

    static constexpr unsigned native_mul_elems    = 8;
    static constexpr unsigned native_input_elems  = native_vector_length_v<input_type>;
    static constexpr unsigned native_output_elems = native_vector_length_v<output_type>;

    using input_vector  = vector<input_type, native_mul_elems>;
    using input_ptr     = typename input_vector::storage_t;
    using output_vector = vector<output_type, native_mul_elems>;
    using output_ptr    = typename output_vector::storage_t;

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_mj_(inv ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             const twiddle_type * __restrict tw2,
             output_type * __restrict out,
             unsigned n)
    {
        input_ptr *           pi   = (input_ptr *) x;
        output_ptr * restrict po   = (output_ptr *) out;
        // Note reversal of twiddle args
        twiddle_type *        ptw0 = (twiddle_type *) tw1;
        twiddle_type *        ptw1 = (twiddle_type *) tw0;
        twiddle_type *        ptw2 = (twiddle_type *) tw2;

        const int po_incr = std::is_same_v<output_type, cint16> ? 32 - 3*n
                                                                : 64 - 6*n;

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            auto [a, b, c, d] = load_data(pi);
            auto [w1, w2, w3] = load_twiddles(ptw0, ptw1, ptw2);

            accum<cacc64, 8> g = ::mac_elem_8_conf(c,        w2,  a, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 8> h = ::msc_elem_8_conf(c,        w2,  a, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 8> k = ::mul_elem_8_conf(b,        w1,           cmplx_mask_,    0);
            accum<cacc64, 8> l = ::mul_elem_8_conf(b, swap16(w1),          cmplx_mask_mj_, 0);
            
            accum<cacc64, 8> o0 = ::addmac_elem_8_conf(d,        w3,  g,                                          k, 0, 0, cmplx_mask_,    0, 0, 0);
            accum<cacc64, 8> o1 = ::addmsc_elem_8_conf(d, swap16(w3), h,                                          l, 0, 0, cmplx_mask_mj_, 0, 0, 0);
            accum<cacc64, 8> o2 = ::addmsc_elem_8_conf(d,        w3,  g, k, 0, 0, cmplx_mask_,    0, 0, 1);
            accum<cacc64, 8> o3 = ::addmac_elem_8_conf(d, swap16(w3), h, l, 0, 0, cmplx_mask_mj_, 0, 0, 1);
        
            *po = o0.template to_vector<output_type>(shift_);  po +=  n/32;
            *po = o1.template to_vector<output_type>(shift_);  po +=  n/32;
            *po = o2.template to_vector<output_type>(shift_);  po +=  n/32;
            *po = o3.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, po_incr);
        }
    }

private:
    __aie_inline
    auto load_data(input_ptr *& pi)
    {
        using acc_type = accum<cacc64, 8>;
        using vec_type = vector<input_type, 8>;

        vec_type dat0 = *pi++;
        vec_type dat1 = *pi++;
        vec_type dat2 = *pi++;
        vec_type dat3 = *pi++;

        auto [s0, s1] = interleave_unzip<input_type, 8>::run(dat0, dat1, 1);
        auto [s2, s3] = interleave_unzip<input_type, 8>::run(dat2, dat3, 1);

        auto [a_tmp, c] = interleave_unzip<input_type, 8>::run(s0, s2, 1);
        auto [b, d]     = interleave_unzip<input_type, 8>::run(s1, s3, 1);

        acc_type a(a_tmp, shift_tw_);

        return std::make_tuple(a, b, c, d);
    }

    __aie_inline
    auto load_twiddles(twiddle_type *& ptw0, twiddle_type *& ptw1, twiddle_type *& ptw2)
    {
        constexpr unsigned lanes = 8;
        vector<twiddle_type, lanes> tw0, tw1, tw2;

        tw0.insert(0, *(v8cint16*)ptw0);    ptw0 += 8;
        tw1.insert(0, *(v8cint16*)ptw1);    ptw1 += 8;
        tw2.insert(0, *(v8cint16*)ptw2);    ptw2 += 8;

        return std::make_tuple(tw0, tw1, tw2);
    }

    unsigned shift_tw_, shift_;
    int cmplx_mask_;
    int cmplx_mask_mj_;
};

template<unsigned Vectorization, unsigned Stage, typename Output>
struct fft_dit<Vectorization, Stage, 4, cint16, Output, cint16> : public fft_dit_common<Vectorization, Stage, 4, cint16, Output, cint16>
{
    using   input_type = cint16;
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
          inv_(inv),
          cmplx_mask_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_mj_(inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY),
          cnt0_(0), cnt1_(0),
          cnt_tw0_(0), cnt_tw1_(0), cnt_tw2_(0)
    {}

    __aie_inline
    void run(const input_type * __restrict x,
             const twiddle_type * __restrict tw0,
             const twiddle_type * __restrict tw1,
             const twiddle_type * __restrict tw2,
             output_type * __restrict out,
             unsigned n)
    {
        input_ptr *           pi   = (input_ptr *) x;
        input_ptr *           pi1  = (input_ptr *) (x + Vectorization);
        output_ptr * restrict po   = (output_ptr *) out;
        // Note reversal of twiddle args
        twiddle_type *        ptw0 = (twiddle_type *) tw1;
        twiddle_type *        ptw1 = (twiddle_type *) tw0;
        twiddle_type *        ptw2 = (twiddle_type *) tw2;

        // Required for stage 1 only
        v16cint16 minus_ones;

        if constexpr (Stage == 1) {
            ptw0 += 1;
            ptw1 += 1;
            ptw2 += 1;

            using limits = std::numeric_limits<int16_t>;
            const cint16 minus_one{int16_t(limits::min() >> (limits::digits - shift_tw_)), 0};
            minus_ones = ::broadcast_to_v16cint16(minus_one);
        }

        for (unsigned int j = 0; j < this->block_size(n); ++j)
            chess_prepare_for_pipelining
            chess_loop_range(1,)
        {
            using a_type = std::conditional_t<Stage == 1, vector<input_type, 16>,
                                                          accum<cacc64, 16>>;
            a_type a;
            vector<input_type, 16> b, c, d;
            
            if constexpr (Stage == 0)
                std::tie(a, b, c, d) = load_data(pi, pi1);
            else
                std::tie(a, b, c, d) = load_data(pi);

            auto [w1, w2, w3] = load_twiddles(ptw0, ptw1, ptw2);

            accum<cacc64, 16> g, h;
            if constexpr (Stage == 1) {
                v32cint16 w21 = ::concat(w2, *(chess_protect_access v16cint16*)&minus_ones);
                v32cint16 ca  = ::concat(c, a);
                g = ::mul_elem_16_2_conf( ca, w21, (inv_ ? 0xD4 : 0xC6), 0 );
                h = ::mul_elem_16_2_conf( ca, w21, (inv_ ? 0xE7 : 0xF5), 0 );
            }
            else {
                g = ::mac_elem_16_conf(c, w2,  a, 0, 0, cmplx_mask_, 0, 0);
                h = ::msc_elem_16_conf(c, w2,  a, 0, 0, cmplx_mask_, 0, 0);
            }

            auto bd = b.template grow<32>();
            bd.insert(1, d);
            auto w13 = w1.template grow<32>();
            w13.insert(1, w3);
            
            accum<cacc64, 16> o0 = ::mac_elem_16_2_conf(bd,        w13,  g, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 16> o1 = ::mac_elem_16_2_conf(bd, swap16(w13), h, 0, 0, cmplx_mask_mj_, 0, 0);
            accum<cacc64, 16> o2 = ::msc_elem_16_2_conf(bd,        w13,  g, 0, 0, cmplx_mask_,    0, 0);
            accum<cacc64, 16> o3 = ::msc_elem_16_2_conf(bd, swap16(w13), h, 0, 0, cmplx_mask_mj_, 0, 0);
        
            if constexpr (std::is_same_v<output_type, cint16>) {
                *po   = o0.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o1.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o2.template to_vector<output_type>(shift_);  po +=  n/64;
                *po   = o3.template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 3*n);
            }
            else if constexpr (std::is_same_v<output_type, cint32>) {
                *po++ = o0.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o0.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o1.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o1.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o2.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o2.template extract<8>(1).template to_vector<output_type>(shift_);  po += n/32-1;
                *po++ = o3.template extract<8>(0).template to_vector<output_type>(shift_);
                *po   = o3.template extract<8>(1).template to_vector<output_type>(shift_);  po  = ::byte_incr(po, 64 - 6*n);
            }
        }
    }

private:
    __aie_inline
    auto load_data(input_ptr *& pi0, input_ptr *& pi1) requires (Stage == 0)
    {
        using return_acc_type = accum<cacc64, native_mul_elems>;
        using return_vec_type = vector<input_type, native_mul_elems>;
        return_acc_type a;
        return_vec_type a_tmp;
        return_vec_type b, c, d;

        a_tmp = *pi0;    pi0 += Vectorization/8;
        b     = *pi1;    pi1 += Vectorization/8;
        c     = *pi0;    pi0 = ::add_2d_ptr(pi0, Vectorization/16+1, Vectorization/16-1, cnt0_, 1-Vectorization/8);
        d     = *pi1;    pi1 = ::add_2d_ptr(pi1, Vectorization/16+1, Vectorization/16-1, cnt1_, 1-Vectorization/8);

        a.from_vector(a_tmp, shift_tw_);

        return std::make_tuple(a, b, c, d);
    }

    __aie_inline
    auto load_data(input_ptr *& pi) requires (Stage != 0)
    {
        using return_acc_type = accum<cacc64, native_mul_elems>;
        using return_vec_type = vector<input_type, native_mul_elems>;
        using a_type          = std::conditional_t<Stage == 1, return_vec_type,
                                                               return_acc_type>;
        a_type a;
        return_vec_type b, c, d;

        if constexpr (Stage == 1) {
            v8cint16 * tmp = (v8cint16*)pi;

            vector<cint16, 16> dat0, dat1, dat2, dat3;

            dat0.insert(0, *tmp);    tmp += 2;
            dat0.insert(1, *tmp);    tmp += 2;
            dat1.insert(0, *tmp);    tmp += 2;
            dat1.insert(1, *tmp);    tmp -= 5;
            dat2.insert(0, *tmp);    tmp += 2;
            dat2.insert(1, *tmp);    tmp += 2;
            dat3.insert(0, *tmp);    tmp += 2;
            dat3.insert(1, *tmp);    tmp += 1;

            std::tie(a, b) = interleave_unzip<input_type, 16>::run(dat0, dat1, 4);
            std::tie(c, d) = interleave_unzip<input_type, 16>::run(dat2, dat3, 4);

            pi = (input_ptr*)tmp;
        }
        else if constexpr (Stage == 2) {
            using vec_type = vector<input_type, 16>;

            vec_type dat0 = *pi++;
            vec_type dat1 = *pi++;
            vec_type dat2 = *pi++;
            vec_type dat3 = *pi++;

            vec_type s0 = ::shuffle(dat0, dat1, T32_8x4_lo);
            vec_type s1 = ::shuffle(dat0, dat1, T32_8x4_hi);
            vec_type s2 = ::shuffle(dat2, dat3, T32_8x4_lo);
            vec_type s3 = ::shuffle(dat2, dat3, T32_8x4_hi);

            vec_type a_tmp;

            std::tie(a_tmp, b) = interleave_unzip<input_type, 16>::run(s0, s2, 8);
            std::tie(c, d)     = interleave_unzip<input_type, 16>::run(s1, s3, 8);

            a.from_vector(a_tmp, shift_tw_);
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return std::make_tuple(a, b, c, d);
    }

    __aie_inline
    auto load_twiddles(twiddle_type *& ptw0, twiddle_type *& ptw1, twiddle_type *& ptw2)
    {
        vector<twiddle_type, 16> tw0, tw1, tw2;

        if      constexpr (Stage == 0) {
            tw0 = broadcast<twiddle_type, 16>::run(*ptw0);    ptw0 = ::add_2d_ptr(ptw0, 1, Vectorization/16-1, cnt_tw0_, 0);
            tw1 = broadcast<twiddle_type, 16>::run(*ptw1);    ptw1 = ::add_2d_ptr(ptw1, 1, Vectorization/16-1, cnt_tw1_, 0);
            tw2 = broadcast<twiddle_type, 16>::run(*ptw2);    ptw2 = ::add_2d_ptr(ptw2, 1, Vectorization/16-1, cnt_tw2_, 0);
        }
        else if constexpr (Stage == 1) {
            vector<twiddle_type, 16> tw0, tw1, tw2;

            int tw_step = 1;
            ptw0 = chess_copy(ptw0); ptw1 = chess_copy(ptw1); ptw2 = chess_copy(ptw2);

            cint16 wa, wb, wc, wd;
            wa = ptw0[-tw_step];
            wb = *ptw0;      ptw0 += 2*tw_step;    ptw0 = chess_copy(ptw0);
            wc = ptw0[-tw_step];
            wd = *ptw0;      ptw0 += 2*tw_step;    ptw0 = chess_copy(ptw0);
            tw0 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );
            wa = ptw1[-tw_step];
            wb = *ptw1;      ptw1 += 2*tw_step;    ptw1 = chess_copy(ptw1);
            wc = ptw1[-tw_step];
            wd = *ptw1;      ptw1 += 2*tw_step;    ptw1 = chess_copy(ptw1);
            tw1 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );
            wa = ptw2[-tw_step];
            wb = *ptw2;      ptw2 += 2*tw_step;    ptw2 = chess_copy(ptw2);
            wc = ptw2[-tw_step];
            wd = *ptw2;      ptw2 += 2*tw_step;    ptw2 = chess_copy(ptw2);
            tw2 = ::shuffle( broadcast_2c16_T32_4x4( wa, wb ), broadcast_2c16_T32_4x4( wc, wd ), T256_2x2_lo );

            return std::make_tuple(tw0, tw1, tw2);
        }
        else if constexpr (Stage == 2) {
            tw0 = *(v16cint16*)ptw0;    ptw0 += 16;
            tw1 = *(v16cint16*)ptw1;    ptw1 += 16;
            tw2 = *(v16cint16*)ptw2;    ptw2 += 16;
        }
        else {
            UNREACHABLE_MSG("Unreachable");
        }

        return std::make_tuple(tw0, tw1, tw2);
    }

    unsigned shift_tw_, shift_, inv_;
    int cmplx_mask_;
    int cmplx_mask_mj_;
    addr_t cnt0_, cnt1_;
    addr_t cnt_tw0_, cnt_tw1_, cnt_tw2_;
};

} // namespace aie::detail

#endif  // __AIE_API_DETAIL_AIE2P_FFT_DIT_RADIX4_HPP__
