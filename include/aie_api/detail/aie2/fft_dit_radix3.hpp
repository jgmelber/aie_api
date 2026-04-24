// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX3_HPP__
#define __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX3_HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../array_helpers.hpp"

namespace aie::detail {

template<unsigned Vectorization, typename Input, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 0, 3, Input, Output, Twiddle> : public fft_dit_common<Vectorization, 0, 3, Input, Output, Twiddle>
{
    using   input_type = Input;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    using native_input_type = std::conditional_t<std::is_same_v<Input, cint32>, v8cint32, v8cint16>;

    struct input_data
    {
        vector<input_type, 8> a;
        vector<input_type, 8> b;
        vector<input_type, 8> c;
        twiddle_type tw1, tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr,
                                              const twiddle_type * __restrict ptw1,
                                              const twiddle_type * __restrict ptw2,
                                              unsigned r) :
            ptr_begin_((const native_input_type __aie_dm_resource_a *)(ptr)),
            ptw1_(ptw1),
            ptw2_(ptw2),
            r_(r),
            cnt_(0),
            cnt_tw1_(0),
            cnt_tw2_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 1+2*r_/8, r_ /8-1, cnt_, 1);
            ptw1_       = ::add_2d_ptr(ptw1_, 1, r_ /8-1, cnt_tw1_, 0);
            ptw2_       = ::add_2d_ptr(ptw2_, 1, r_ /8-1, cnt_tw2_, 0);

            return *this;
        }

        __aie_inline
        stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*()
        {
            return { *ptr_begin_, *(ptr_begin_+ r_ / 8), *(ptr_begin_+ 2 * r_ / 8), *ptw1_, *ptw2_ };
        }

    private:
        const native_input_type __aie_dm_resource_a * __restrict ptr_begin_; //TODO: this DM_bankA annotation seems to be ignored
        const twiddle_type * __restrict ptw1_;
        const twiddle_type * __restrict ptw2_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data,
                               const twiddle_type * __restrict ptw1,
                               const twiddle_type * __restrict ptw2)
    {
        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv)
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data)
    {
        output_data ret;

        accum<cacc64, 8> o0, o1, o2;

        int cmplx_mask       = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_conjy = (inv_ ? OP_TERM_NEG_COMPLEX             : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        constexpr unsigned twiddle_elems = std::is_same_v<Input, cint16> && std::is_same_v<Twiddle, cint16> ? 16 : 8;
        vector<twiddle_type, twiddle_elems> w1, w2;
        w1.insert(0, broadcast<twiddle_type, 8>::run(data.tw1));
        w2.insert(0, broadcast<twiddle_type, 8>::run(data.tw2));

        accum<cacc64, 8> a_acc = accum<cacc64, 8>(data.a, shift_tw_);
        accum<cacc64, 8> d_acc, e_acc;

        if      constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Twiddle, cint16>) {
            w1.insert(1, zeros<twiddle_type, 8>::run());
            w2.insert(1, zeros<twiddle_type, 8>::run());

            d_acc = ::mul_elem_8_2_conf(data.b.template grow<16>(), w1, cmplx_mask, 0);
            e_acc = ::mul_elem_8_2_conf(data.c.template grow<16>(), w2, cmplx_mask, 0);
        }
        else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Twiddle, cint16>) {
            d_acc = ::mul_elem_8_conf(data.b, w1.template grow<16>(), cmplx_mask, 0);
            e_acc = ::mul_elem_8_conf(data.c, w2.template grow<16>(), cmplx_mask, 0);
        }
        else if constexpr (std::is_same_v<Input, cint16> && std::is_same_v<Twiddle, cint32>) {
            d_acc = ::mul_elem_8_conf(w1, data.b.template grow<16>(), (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_X : OP_TERM_NEG_COMPLEX),             0);
            e_acc = ::mul_elem_8_conf(w2, data.c.template grow<16>(), (inv_ ? OP_TERM_NEG_COMPLEX             : OP_TERM_NEG_COMPLEX_CONJUGATE_X), 0);
        }
        else if constexpr (std::is_same_v<Input, cint32> && std::is_same_v<Twiddle, cint32>) {
            d_acc = ::mul_elem_8_conf(data.b, w1, cmplx_mask, 0);
            e_acc = ::mul_elem_8_conf(data.c, w2, cmplx_mask, 0);
        }

        vector<cint32, 8> d = d_acc.template to_vector<cint32>(shift_tw_);
        vector<cint32, 8> e = e_acc.template to_vector<cint32>(shift_tw_);

        o0 = add_accum<cacc64, 8>::run(a_acc, false, d_acc);
        o0 = add_accum<cacc64, 8>::run(o0,    false, e_acc);

        constexpr unsigned twiddle_elems2 = std::is_same_v<Twiddle, cint16> ? 16 : 8;
        if constexpr (std::is_same_v<Twiddle, cint32>) {
            o1 = ::msc_elem_8_conf(d, qbuf_.template grow<twiddle_elems2>(), a_acc,    0, cmplx_mask,       0, 0);
            o1 = ::msc_elem_8_conf(e, qbuf_.template grow<twiddle_elems2>(), o1,       0, cmplx_mask_conjy, 0, 0);

            o2 = ::msc_elem_8_conf(d, qbuf_.template grow<twiddle_elems2>(), a_acc,    0, cmplx_mask_conjy, 0, 0);
            o2 = ::msc_elem_8_conf(e, qbuf_.template grow<twiddle_elems2>(), o2,       0, cmplx_mask,       0, 0);
        }
        else {
            o1 = ::msc_elem_8_conf(d, qbuf_.template grow<twiddle_elems2>(), a_acc, 0, 0, cmplx_mask,       0, 0);
            o1 = ::msc_elem_8_conf(e, qbuf_.template grow<twiddle_elems2>(), o1,    0, 0, cmplx_mask_conjy, 0, 0);

            o2 = ::msc_elem_8_conf(d, qbuf_.template grow<twiddle_elems2>(), a_acc, 0, 0, cmplx_mask_conjy, 0, 0);
            o2 = ::msc_elem_8_conf(e, qbuf_.template grow<twiddle_elems2>(), o2,    0, 0, cmplx_mask,       0, 0);
        }

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;

    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          inv_(inv)
    {
        twiddle_type k;

        if constexpr (std::is_same_v<twiddle_type, cint16>)
            k = as_cint16((1u << (shift_tw_ - 1)) | ((28378u >> (15 - shift_tw_)) << 16)); // 0.5 + 0.5j*sqrt(3)
        else if constexpr (std::is_same_v<twiddle_type, cint32>)
            k = twiddle_type(1u << (shift_tw_ - 1), (1859775393u >> (31 - shift_tw_)));

        qbuf_ = broadcast<twiddle_type, 8>::run(k);
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 8> qbuf_;
};

#if __AIE_API_CBF16_SUPPORT__
template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 3, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 0, 3, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> a;
        vector<input_type, 8> b;
        vector<input_type, 8> c;
        twiddle_type tw1, tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr,
                                              const twiddle_type * __restrict ptw1,
                                              const twiddle_type * __restrict ptw2,
                                              unsigned r) :
            ptr_begin_((const v8cbfloat16 __aie_dm_resource_a *)(ptr)),
            ptw1_(ptw1),
            ptw2_(ptw2),
            r_(r),
            cnt_(0),
            cnt_tw1_(0),
            cnt_tw2_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 1+2*r_/8, r_ /8-1, cnt_, 1);
            ptw1_       = ::add_2d_ptr(ptw1_, 1, r_ /8-1, cnt_tw1_, 0);
            ptw2_       = ::add_2d_ptr(ptw2_, 1, r_ /8-1, cnt_tw2_, 0);

            return *this;
        }

        __aie_inline
        stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*()
        {
            return { *ptr_begin_, *(ptr_begin_+ r_ / 8), *(ptr_begin_+ 2 * r_ / 8), *ptw1_, *ptw2_ };
        }

    private:
        const v8cbfloat16 __aie_dm_resource_a * __restrict ptr_begin_; //TODO: this DM_bankA annotation seems to be ignored
        const twiddle_type * __restrict ptw1_;
        const twiddle_type * __restrict ptw2_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data,
                               const twiddle_type * __restrict ptw1,
                               const twiddle_type * __restrict ptw2)
    {
        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv)
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data)
    {
        vector<twiddle_type, 8> w1 = broadcast<twiddle_type, 8>::run(data.tw1);
        vector<twiddle_type, 8> w2 = broadcast<twiddle_type, 8>::run(data.tw2);

        accum<caccfloat, 8> a_acc(data.a);
        accum<caccfloat, 8> d_acc = ::mul_elem_8_2_conf(::concat(data.b, zeros_), ::concat(w1, zeros_), cmplx_mask_, 0);
        accum<caccfloat, 8> e_acc = ::mul_elem_8_2_conf(::concat(data.c, zeros_), ::concat(w2, zeros_), cmplx_mask_, 0);

        vector<twiddle_type, 8> d = d_acc.template to_vector<twiddle_type>();
        vector<twiddle_type, 8> e = e_acc.template to_vector<twiddle_type>();

        accum<caccfloat, 8> o0 = add_accum<caccfloat, 8>::run(a_acc, false, d_acc);
                            o0 = add_accum<caccfloat, 8>::run(o0,    false, e_acc);
        accum<caccfloat, 8> o1 = ::msc_elem_8_2_conf(::concat(d, zeros_), ::concat(qbuf_, zeros_), a_acc, 0, cmplx_mask_,       0, 0);
                            o1 = ::msc_elem_8_2_conf(::concat(e, zeros_), ::concat(qbuf_, zeros_), o1,    0, cmplx_mask_conjy_, 0, 0);
        accum<caccfloat, 8> o2 = ::msc_elem_8_2_conf(::concat(d, zeros_), ::concat(qbuf_, zeros_), a_acc, 0, cmplx_mask_conjy_, 0, 0);
                            o2 = ::msc_elem_8_2_conf(::concat(e, zeros_), ::concat(qbuf_, zeros_), o2,    0, cmplx_mask_,       0, 0);

        output_data ret;

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();
        ret[2] = o2.to_vector<output_type>();

        return ret;
    }

    __aie_inline fft_dit() = default;

    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : cmplx_mask_      (inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_conjy_(inv ? OP_TERM_NEG_COMPLEX             : OP_TERM_NEG_COMPLEX_CONJUGATE_Y)

    {
        // 0.5 + 0.5j*sqrt(3)
        qbuf_ = broadcast<twiddle_type, 8>::run(twiddle_type{0.5f, 0.863281250f});
        zeros_ = zeros<twiddle_type, 8>::run();
    }
private:
    int cmplx_mask_, cmplx_mask_conjy_;
    vector<twiddle_type, 8> qbuf_;
    vector<twiddle_type, 8> zeros_;
};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 3, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 3, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> a;
        vector<input_type, 8> b;
        vector<input_type, 8> c;
        twiddle_type tw1, tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr,
                                              const twiddle_type * __restrict ptw1,
                                              const twiddle_type * __restrict ptw2,
                                              unsigned r) :
            ptr_begin_((const v8cfloat __aie_dm_resource_a *)(ptr)),
            ptw1_(ptw1),
            ptw2_(ptw2),
            r_(r),
            cnt_(0),
            cnt_tw1_(0),
            cnt_tw2_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 1+2*r_/8, r_ /8-1, cnt_, 1);
            ptw1_       = ::add_2d_ptr(ptw1_, 1, r_ /8-1, cnt_tw1_, 0);
            ptw2_       = ::add_2d_ptr(ptw2_, 1, r_ /8-1, cnt_tw2_, 0);

            return *this;
        }

        __aie_inline
        stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*()
        {
            return { *ptr_begin_, *(ptr_begin_+ r_ / 8), *(ptr_begin_+ 2 * r_ / 8), *ptw1_, *ptw2_ };
        }

    private:
        const v8cfloat __aie_dm_resource_a * __restrict ptr_begin_; //TODO: this DM_bankA annotation seems to be ignored
        const twiddle_type * __restrict ptw1_;
        const twiddle_type * __restrict ptw2_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data,
                               const twiddle_type * __restrict ptw1,
                               const twiddle_type * __restrict ptw2)
    {
        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv)
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data)
    {
        vector<twiddle_type, 8> w1 = broadcast<twiddle_type, 8>::run(data.tw1);
        vector<twiddle_type, 8> w2 = broadcast<twiddle_type, 8>::run(data.tw2);

        accum<caccfloat, 8> a_acc(data.a);
        accum<caccfloat, 8> d_acc = ::mul_elem_8_conf(data.b, w1, cmplx_mask_, 0);
        accum<caccfloat, 8> e_acc = ::mul_elem_8_conf(data.c, w2, cmplx_mask_, 0);

        vector<twiddle_type, 8> d = d_acc.template to_vector<twiddle_type>();
        vector<twiddle_type, 8> e = e_acc.template to_vector<twiddle_type>();

        accum<caccfloat, 8> o0 = add_accum<caccfloat, 8>::run(a_acc, false, d_acc);
                            o0 = add_accum<caccfloat, 8>::run(o0,    false, e_acc);
        accum<caccfloat, 8> o1 = ::msc_elem_8_conf(d, qbuf_, a_acc, 0, cmplx_mask_,       0, 0);
                            o1 = ::msc_elem_8_conf(e, qbuf_, o1,    0, cmplx_mask_conjy_, 0, 0);
        accum<caccfloat, 8> o2 = ::msc_elem_8_conf(d, qbuf_, a_acc, 0, cmplx_mask_conjy_, 0, 0);
                            o2 = ::msc_elem_8_conf(e, qbuf_, o2,    0, cmplx_mask_,       0, 0);

        output_data ret;

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();
        ret[2] = o2.to_vector<output_type>();

        return ret;
    }

    __aie_inline fft_dit() = default;

    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : cmplx_mask_      (inv ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX),
          cmplx_mask_conjy_(inv ? OP_TERM_NEG_COMPLEX             : OP_TERM_NEG_COMPLEX_CONJUGATE_Y)

    {
        // 0.5 + 0.5j*sqrt(3)
        qbuf_ = broadcast<twiddle_type, 8>::run(twiddle_type{0.5f, 0.866025388f});
    }
private:
    int cmplx_mask_, cmplx_mask_conjy_;
    vector<twiddle_type, 8> qbuf_;
};
#endif

}

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif // __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX3_HPP__
