// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_FFT_DIT_ACC48_HPP__
#define __AIE_API_DETAIL_AIE1_FFT_DIT_ACC48_HPP__

#include "../array_helpers.hpp"

namespace aie::detail {

// Radix 2

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 2, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 2, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8>   lo;
        vector<input_type, 8>   hi;
        vector<twiddle_type, 8> tw;
        int zoffs;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw, unsigned r) :
            ptr_begin1_((const v8cint16 *)(ptr)),
            ptr_begin2_((const v8cint16 *)(ptr + r)),
            ptr_end1_((const v8cint16 *)(ptr + r)),
            ptr_end2_((const v8cint16 *)(ptr + r * 2)),
            ptw_((const v8cint16 *)ptw),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            zoffs_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_ = ::fft_data_incr(ptr_begin1_, r_ / 8, ptr_end1_, cnt1_);
            ptr_begin2_ = ::fft_data_incr(ptr_begin2_, r_ / 8, ptr_end2_, cnt2_);
            ptw_   = ::fft_twiddle_incr(ptw_, cnt2_, 3);
            zoffs_ = ::fft_zoffs(cnt1_, 3, 1, 1);

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
            return { *ptr_begin1_, *ptr_begin2_, *ptw_, zoffs_ };
        }

    private:
        const v8cint16 * __restrict ptr_begin1_;
        const v8cint16 * __restrict ptr_begin2_;
        const v8cint16 *            ptr_end1_;
        const v8cint16 *            ptr_end2_;
        const v8cint16 * __restrict ptw_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw, unsigned vectorization)
    {
        set_rdx(2);

        return stage_iterator(data, ptw, vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;
        accum<cacc48, 8> acc_lo, acc_hi;

        vector<input_type, 16> ybuf;
        vector<input_type, 8> __aie_register(wr2) temp = data.hi;
        ybuf.insert(0, temp);

        acc_lo = ::mac8(::ups(data.lo, shift_tw_), ybuf, 0, 0x76543210,    false, data.tw, 0, data.zoffs, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::msc8_sym(acc_lo                   , ybuf, 0, 0x76543210, 0, false, data.tw, 0, data.zoffs, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 1, 2, cint16, cint16, cint16> : public fft_dit_common<Vectorization, 1, 2, cint16, cint16, cint16>
{
    using   input_type = cint16;
    using  output_type = cint16;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8>   lo;
        vector<input_type, 8>   hi;
        vector<twiddle_type, 8> tw;
        int zoffs;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_begin1_((const v8cint16 *)(ptr)),
            ptr_begin2_((const v8cint16 *)(ptr + 8)),
            ptr_end1_((const v8cint16 *)(ptr + 8)),
            ptw_((const v8cint16 *)ptw),
            cnt_(0),
            zoffs_(0x11110000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_ = ::fft_data_incr(ptr_begin1_, 1, ptr_end1_, cnt_);
            ptr_begin2_ += 2;
            ptw_        = ::fft_twiddle_incr(ptw_, cnt_, 2);
            zoffs_      = ::fft_zoffs(cnt_, 2, 1, 1);

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
        reference operator*() const
        {
            return { ptr_begin1_[0], ptr_begin2_[0], *ptw_, zoffs_ };
        }

    private:
        const v8cint16 * __restrict ptr_begin1_;
        const v8cint16 * __restrict ptr_begin2_;
        const v8cint16 *            ptr_end1_;
        const v8cint16 * __restrict ptw_;
        umod_t cnt_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;
        accum<cacc48, 8> acc_lo, acc_hi;

        vector<input_type, 16> __aie_register(xb) ybufb;
        ybufb = data.lo.template grow<16>();
        ybufb.insert(1, data.hi);
        vector<input_type, 16> ybuf = ybufb;

        acc_lo = ::mul8(            ybuf, 0, 0xba983210,              one_, 0, 0x00000000);
        acc_lo = ::mac8(acc_lo,     ybuf, 0, 0xfedc7654,    false, data.tw, 0, data.zoffs, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::msc8_sym(acc_lo, ybuf, 0, 0xfedc7654, 0, false, data.tw, 0, data.zoffs, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv), one_(1 << shift_tw)
    {}
private:
    unsigned shift_;
    bool inv_;
    vector<int32, 8> one_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 2, 2, cint16, cint16, cint16> : public fft_dit_common<Vectorization, 2, 2, cint16, cint16, cint16>
{
    using   input_type = cint16;
    using  output_type = cint16;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8>   lo;
        vector<input_type, 8>   hi;
        vector<twiddle_type, 4> tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        constexpr stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_((const v8cint16 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_ += 2;
            ++ptw_;
            return *this;
        }

        __aie_inline
        constexpr stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*()
        {
            return { ptr_[0], ptr_[1], *ptw_ };
        }

    private:
        const v8cint16 * __restrict ptr_;
        const_vector_iterator<cint16, 4> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;
        accum<cacc48, 8> acc_lo, acc_hi;

        vector<input_type, 16> __aie_register(xb) ybufb;
        ybufb = data.lo.template grow<16>();
        ybufb.insert(1, data.hi);
        vector<input_type, 16> ybuf = ybufb;

        acc_lo = ::mul8(            ybuf, 0, 0xdc985410,                                 one_, 0, 0x00000000);
        acc_lo = ::mac8(acc_lo,     ybuf, 0, 0xfeba7632,    false, data.tw.template grow<8>(), 0, 0x33221100, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::msc8_sym(acc_lo, ybuf, 0, 0xfeba7632, 0, false, data.tw.template grow<8>(), 0, 0x33221100, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv), one_(1 << shift_tw)
    {}
private:
    unsigned shift_;
    bool inv_;
    vector<int32, 8> one_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 3, 2, cint16, cint16, cint16> : public fft_dit_common<Vectorization, 3, 2, cint16, cint16, cint16>
{
    using   input_type = cint16;
    using  output_type = cint16;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 3, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8>   lo;
        vector<input_type, 8>   hi;
        vector<twiddle_type, 8> tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        constexpr stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_((const v8cint16 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_ += 2;
            ++ptw_;
            return *this;
        }

        __aie_inline
        constexpr stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*()
        {
            return { ptr_[0], ptr_[1], *ptw_ };
        }

    private:
        const v8cint16 * __restrict ptr_;
        const_vector_iterator<cint16, 8> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;
        accum<cacc48, 8> acc_lo, acc_hi;

        vector<input_type, 16> __aie_register(xb) ybufb;
        ybufb = data.lo.template grow<16>();
        ybufb.insert(1, data.hi);
        vector<input_type, 16> ybuf = ybufb;

        acc_lo = ::mul8(            ybuf, 0, 0xeca86420,              one_, 0, 0x00000000);
        acc_lo = ::mac8(acc_lo,     ybuf, 0, 0xfdb97531,    false, data.tw, 0, 0x76543210, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::msc8_sym(acc_lo, ybuf, 0, 0xfdb97531, 0, false, data.tw, 0, 0x76543210, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv), one_(1 << shift_tw)
    {}
private:
    unsigned shift_;
    bool inv_;
    vector<int32, 8> one_;
};

// Radix 3

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 3, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 3, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
        vector<twiddle_type, 4> tw2;
        vector<twiddle_type, 4> tw1;
        int zoffs;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, unsigned r) :
            ptr_begin0_((const v4cint16 *)(ptr)),
            ptr_begin1_((const v4cint16 *)(ptr + r)),
            ptr_begin2_((const v4cint16 *)(ptr + r * 2)),
            ptr_end0_((const v4cint16 *)(ptr + r)),
            ptr_end1_((const v4cint16 *)(ptr + r * 2)),
            ptr_end2_((const v4cint16 *)(ptr + r * 3)),
            ptw2_((const v4cint16 *)ptw2),
            r_(r),
            cnt_(0),
            zoffs_(0x00000000),
            twdiff_((const v4cint16 *)ptw1 - (const v4cint16 *)ptw2)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr),  "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw1), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw2), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin0_   = ::fft_data_incr(ptr_begin0_, r_/4,ptr_end0_     );
            ptr_begin1_   = ::fft_data_incr(ptr_begin1_, r_/4,ptr_end1_,cnt_);
            ptr_begin2_   = ::fft_data_incr(ptr_begin2_, r_/4,ptr_end2_     ); 

            ptw2_  = ::fft_twiddle_incr(ptw2_,cnt_,2);

            zoffs_ = ((cnt_ & 3) + 0x40000) * 0x11111111;

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
        reference operator*() const
        {
            return { *ptr_begin0_,  *ptr_begin1_, *ptr_begin2_, *ptw2_, *(ptw2_ + twdiff_), zoffs_};
        }

    private:
        const v4cint16 * __restrict ptr_begin0_;
        const v4cint16 * __restrict ptr_begin1_;
        const v4cint16 * __restrict ptr_begin2_;
        const v4cint16 *            ptr_end0_;
        const v4cint16 *            ptr_end1_;
        const v4cint16 *            ptr_end2_;
        const v4cint16 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt_;
        int zoffs_;
        int twdiff_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2)
    {
        set_rdx(3);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               unsigned vectorization)
    {
        set_rdx(3);

        return stage_iterator(data, ptw1, ptw2, vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;

        vector<twiddle_type, 8>  zbuf;
        vector<input_type, 16> ybuf;
        vector<cint32, 8>  ubuf;
        accum<cacc48, 4> o0, o1, o2;
        accum<cacc48, 8> wbc;
        zbuf.insert(0, data.tw1);
        zbuf.insert(1, data.tw2);
        ybuf.insert(0, data.dat1);
        ybuf.insert(1, data.dat2);

        wbc = ::mul8(::concat(::undef_v16cint16(), ybuf), 16, 0x76543210, 0, zbuf, 0, data.zoffs, inv_);
                        
        vector<input_type, 4> a = data.dat0;
        ubuf = wbc.to_vector<cint32>(shift_tw_); 

        o0 = ::msc4         (::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, ::as_v16int16(qbuf_), 8, 0x0000,  0);
        o1 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, qbuf_,                0, 0x0000,  inv_);
        o2 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, qbuf_,                0, 0x0000, !inv_);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        qbuf_[0] = as_cint16((1 << (shift_tw_ - 1)) | ((28378 >> (15 - shift_tw_)) << 16)); // 0.5 + 0.5j*sqrt(3)
        qbuf_[4] = as_cint16(1 << shift_tw_);  // 1.0 + 0.0j
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 8>  qbuf_;
};

// Radix 5

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 5, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 5, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 5, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {   //Data numbering refers to points on the unit circle which are computed in pairs, the load sequence helps the compiler schedule the pointer modifications
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat4;
        vector<input_type, 4>   dat3;
        vector<input_type, 4>   dat2;
        vector<input_type, 4>   dat5;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw4;
        vector<twiddle_type, 4> tw3;
        vector<twiddle_type, 4> tw2;
        int zoffs;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, 
                                              const twiddle_type * __restrict ptw3, const twiddle_type * __restrict ptw4, unsigned r) :
            ptr_begin0_((const v4cint16 *)(ptr + r)),
            ptr_begin1_((const v4cint16 *)(ptr)),
            ptr_end0_((const v4cint16 *)(ptr + r * 3)),
            ptr_end1_((const v4cint16 *)(ptr + r)),
            ptw2_((const v4cint16 *)ptw2),
            r_(r),
            cnt_(0),
            zoffs_(0x00000000),
            //Helping the compiler to form the correct pointer chain by providing it with the relative offsets from ptw2
            td1_((const v4cint16 *)ptw1 - (const v4cint16 *)ptw2),
            td4_((const v4cint16 *)ptw4 - (const v4cint16 *)ptw2),
            td3_((const v4cint16 *)ptw3 - (const v4cint16 *)ptw2)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr),  "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw1), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw2), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw3), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw4), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin0_ += r_/4; 
            ptr_begin0_  = ::fft_data_incr(ptr_begin0_, r_/4,ptr_end0_, cnt_); 
            ptr_begin0_ += -(int)(r_/4);
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, r_/4,ptr_end1_     );

            ptw2_ = ::fft_twiddle_incr(ptw2_,cnt_,2);

            zoffs_ = ::fft_zoffs(cnt_,3,0,1);

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
        reference operator*() const
        {
            return { *ptr_begin0_, *(ptr_begin0_ - (int)(r_/4)), *(ptr_begin0_ + 3*r_/4), *(ptr_begin0_ + 2*r_/4), *(ptr_begin0_ + r_/4),  *ptr_begin1_,
                     *(ptw2_+td1_), *(ptw2_ + td4_), *(ptw2_ + td3_), *ptw2_, zoffs_};
        }

    private:
        const v4cint16 * __restrict ptr_begin0_;
        const v4cint16 * __restrict ptr_begin1_;
        const v4cint16 *            ptr_end0_;
        const v4cint16 *            ptr_end1_;
        const v4cint16 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt_;
        int zoffs_;
        int td1_;
        int td4_;
        int td3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw3,
                               const twiddle_type * ptw4)
    {
        set_rdx(5);

        return stage_iterator(data, ptw1, ptw2, ptw3, ptw4, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw3,
                               const twiddle_type * ptw4,
                               unsigned vectorization)
    {
        set_rdx(5);

        return stage_iterator(data, ptw1, ptw2, ptw3, ptw4, vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(shift_tw, shift, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;

        vector<twiddle_type, 8> qbuf;

        accum<cacc48, 4> x1, x2, x3, x4;
        accum<cacc48, 4> o0, o1, o2, o3, o4;
        accum<cacc48, 8> wbc;

        vector<input_type, 16> ybuf;
        vector<cint32, 8> ubuf;
        vector<cint32, 8> vbuf;

        ybuf.insert(0, data.dat1); ybuf.insert(1, data.dat4); 
        qbuf.insert(0, data.tw1);  qbuf.insert(1, data.tw4);
        //Putting ybuf in the upper half will allow XB to be used, otherwise register allocation leads to a worse schedule
        wbc = ::mul8(::concat(::undef_v16cint16(), ybuf), 16, 0x76543210, 0, qbuf, 0, data.zoffs, inv_);
        x1 = wbc.extract<4>(0);    x4 = wbc.extract<4>(1);

        ybuf.insert(0, data.dat3); ybuf.insert(1, data.dat2); 
        qbuf.insert(0, data.tw3);  qbuf.insert(1, data.tw2);
        //Putting ybuf in the upper half will allow XB to be used, otherwise register allocation leads to a worse schedule
        wbc = ::mul8(::concat(::undef_v16cint16(), ybuf), 16, 0x76543210, 0, qbuf, 0, data.zoffs, inv_);
        x3 = wbc.extract<4>(0);    x2 = wbc.extract<4>(1);

        vector<input_type, 4> a = data.dat5; 
        ubuf = concat(lsrs(x1,shift_),lsrs(x4,shift_));
        o0 = ::msc4         (::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, as_v16int16(zbuf_), 8, 0x0000, 0); 
        o1 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_,              0, 0x0000,  inv_);
        o4 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_,              0, 0x0000, !inv_);
        o2 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_,              1, 0x0000,  inv_);
        o3 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_,              1, 0x0000, !inv_);

        vbuf = concat(lsrs(x2,shift_),lsrs(x3,shift_));
        o0 = ::msc4         (o0,                  vbuf, 0, 0x3210, 4, 0, as_v16int16(zbuf_), 8, 0x0000, 0); 
        o1 = ::msc4_conj_sym(o1,                  vbuf, 0, 0x3210, 4, 0, zbuf_,              1, 0x0000,  inv_);
        o4 = ::msc4_conj_sym(o4,                  vbuf, 0, 0x3210, 4, 0, zbuf_,              1, 0x0000, !inv_);
        o2 = ::msc4_conj_sym(o2,                  vbuf, 0, 0x3210, 4, 0, zbuf_,              0, 0x0000, !inv_);
        o3 = ::msc4_conj_sym(o3,                  vbuf, 0, 0x3210, 4, 0, zbuf_,              0, 0x0000,  inv_);

        ret[0] = o0.to_vector<output_type>(shift_); 
        ret[1] = o1.to_vector<output_type>(shift_); 
        ret[2] = o2.to_vector<output_type>(shift_); 
        ret[3] = o3.to_vector<output_type>(shift_); 
        ret[4] = o4.to_vector<output_type>(shift_); 

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        zbuf_[0] = as_cint16(((-10126 >> (15 - shift_tw_)) & 0xFFFF) | ((31164 >> (15 - shift_tw_)) << 16)); // -exp(-2j*pi/5)
        zbuf_[1] = as_cint16( ( 26510 >> (15 - shift_tw_))           | ((19261 >> (15 - shift_tw_)) << 16)); // -(-exp(-2j*pi/5))^2
        zbuf_[4] = as_cint16(1 << shift_tw_);              // 1.0 + 0.0j
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 8>  zbuf_;
};

}

#endif
