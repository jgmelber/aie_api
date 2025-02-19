// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_FFT_DIT_ACC80_HPP__
#define __AIE_API_DETAIL_AIE1_FFT_DIT_ACC80_HPP__

#include "../vector.hpp"

namespace aie::detail {

// Radix 2

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 2, cint32, cint32, cint16> : public fft_dit_common<Vectorization, 0, 2, cint32, cint32, cint16>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo;
        vector<input_type, 4>   hi;
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
            ptr_begin1_((const v4cint32 *)(ptr)),
            ptr_begin2_((const v4cint32 *)(ptr + r)),
            ptr_end1_((const v4cint32 *)(ptr + r)),
            ptr_end2_((const v4cint32 *)(ptr + r * 2)),
            ptw_((const v8cint16 *)ptw),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            zoffs_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_ = ::fft_data_incr(ptr_begin1_, r_ / 4, ptr_end1_, cnt1_);
            ptr_begin2_ = ::fft_data_incr(ptr_begin2_, r_ / 4, ptr_end2_, cnt2_);
            ptw_        = ::fft_twiddle_incr(ptw_, cnt1_, 3);
            zoffs_      = ::fft_zoffs(cnt2_, 3, 1, 0);

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
            return { *ptr_begin1_, *ptr_begin2_, *ptw_, zoffs_ };
        }

    private:
        const v4cint32 * __restrict ptr_begin1_;
        const v4cint32 * __restrict ptr_begin2_;
        const v4cint32 *            ptr_end1_;
        const v4cint32 *            ptr_end2_;
        const v8cint16 * __restrict ptw_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw, unsigned vectorization)
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

        accum<cacc48, 8> acc = ::butterfly_dit(::concat(data.lo, data.hi), ::undef_v8cint32(), 0x3210, 0x7654, data.tw, 0, data.zoffs, inv_, shift_tw_);

        ret[0] = acc.extract<4>(0).to_vector<output_type>(shift_);
        ret[1] = acc.extract<4>(1).to_vector<output_type>(shift_);

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
struct fft_dit<Vectorization, 1, 2, cint32, cint32, cint16> : public fft_dit_common<Vectorization, 1, 2, cint32, cint32, cint16>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo;
        vector<input_type, 4>   hi;
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
            ptr_begin1_((const v4cint32 *)(ptr)),
            ptr_begin2_((const v4cint32 *)(ptr + 4)),
            ptr_end1_((const v4cint32 *)(ptr + 4)),
            ptw_((const v8cint16 *)ptw),
            cnt_(0),
            zoffs_(0x1100)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, 1, ptr_end1_, cnt_);
            ptr_begin2_ += 2;
            ptw_         = ::fft_twiddle_incr(ptw_, cnt_, 2);
            zoffs_ = ::fft_zoffs(cnt_, 2, 1, 0);

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
        const v4cint32 * __restrict ptr_begin1_;
        const v4cint32 * __restrict ptr_begin2_;
        const v4cint32 *            ptr_end1_;
        const v8cint16 * __restrict ptw_;
        umod_t cnt_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
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
        vector<input_type, 8> xbuf;
        xbuf.insert(0, data.lo);
        xbuf.insert(1, data.hi);

        accum<cacc48, 8> acc = ::butterfly_dit(xbuf , ::undef_v8cint32(), 0x5410, 0x7632, data.tw, 0, data.zoffs, inv_, shift_tw_);

        ret[0] = acc.extract<4>(0).to_vector<input_type>(shift_);
        ret[1] = acc.extract<4>(1).to_vector<input_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 2, cint32, Output, cint16> : public fft_dit_common<Vectorization, 2, 2, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo;
        vector<input_type, 4>   hi;
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
            ptr_((const v4cint32 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
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
        const v4cint32 * __restrict ptr_;
        const_vector_iterator<cint16, 4> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
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

        accum<cacc48, 8> acc = ::butterfly_dit(::concat(data.lo, data.hi), ::undef_v8cint32(), 0x6420, 0x7531, data.tw.template grow<8>(), 0, 0x3210, inv_, shift_tw_);

        ret[0] = acc.extract<4>(0).template to_vector<output_type>(shift_);
        ret[1] = acc.extract<4>(1).template to_vector<output_type>(shift_);

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
struct fft_dit<Vectorization, 0, 2, cint16, cint32, cint32> : public fft_dit_common<Vectorization, 0, 2, cint16, cint32, cint32>
{
    using   input_type = cint16;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> lo;
        vector<input_type, 4> hi;
        vector<twiddle_type, 4> tw;
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
        constexpr stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw, unsigned r) :
            ptr_begin1_((const v4cint16 *)(ptr)),
            ptr_begin2_((const v4cint16 *)(ptr + r)),
            ptr_end1_((const v4cint16 *)(ptr + r)),
            ptr_end2_((const v4cint16 *)(ptr + r * 2)),
            ptw_((const v4cint32 *)ptw),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            zoffs_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
        {
            ptr_begin1_ = ::fft_data_incr(ptr_begin1_, r_ / 2, ptr_end1_, cnt1_);
            ptr_begin2_ = ::fft_data_incr(ptr_begin2_, r_ / 2, ptr_end2_, cnt2_);
            ptw_        = ::fft_twiddle_incr(ptw_, cnt1_, 2);
            zoffs_      = ::fft_zoffs(cnt2_, 3, 0, 0);
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
            return { *ptr_begin1_, *ptr_begin2_, *ptw_, zoffs_ };
        }

    private:
        const v4cint16 * __restrict ptr_begin1_;
        const v4cint16 * __restrict ptr_begin2_;
        const v4cint16 *            ptr_end1_;
        const v4cint16 *            ptr_end2_;
        const v4cint32 * __restrict ptw_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
    {
        set_rdx(2);

        return stage_iterator(data, ptw, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw, unsigned vectorization)
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

        accum<cacc80, 4> acc_lo, acc_hi;

        vector<input_type, 16> ybuf = data.hi.template grow<16>();

        acc_lo = ::lmul4(data.lo.template grow<16>(), 0, 0x3210, one_, 0, 0x0000);
        acc_lo = ::lmac4(acc_lo, ybuf, 0, 0x3210,    false, data.tw, 0, data.zoffs, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::lmsc4_sym(acc_lo,                 ybuf, 0, 0x3210, 0, false, data.tw, 0, data.zoffs, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        one_[0] = twiddle_type((1u << shift_tw) - 1, 0);
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<cint32, 4> one_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 2, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 0, 2, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2> lo;
        vector<input_type, 2> hi;
        vector<twiddle_type, 4>    tw;
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
        constexpr stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw, unsigned r) :
            ptr_begin1_((const v2cint32 *)(ptr)),
            ptr_begin2_((const v2cint32 *)(ptr + r)),
            ptr_end1_((const v2cint32 *)(ptr + r)),
            ptr_end2_((const v2cint32 *)(ptr + r * 2)),
            ptw_((const v4cint32 *)ptw),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            zoffs_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
        {
            ptr_begin1_ = ::fft_data_incr(ptr_begin1_, r_ / 2, ptr_end1_, cnt1_);
            ptr_begin2_ = ::fft_data_incr(ptr_begin2_, r_ / 2, ptr_end2_, cnt2_);
            ptw_        = ::fft_twiddle_incr(ptw_, cnt1_, 2);
            zoffs_      = ::fft_zoffs(cnt2_, 3, 0, 0);
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
            return { *ptr_begin1_, *ptr_begin2_, *ptw_, zoffs_ };
        }

    private:
        const v2cint32 * __restrict ptr_begin1_;
        const v2cint32 * __restrict ptr_begin2_;
        const v2cint32 *            ptr_end1_;
        const v2cint32 *            ptr_end2_;
        const v4cint32 * __restrict ptw_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        int zoffs_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
    {
        set_rdx(2);

        return stage_iterator(data, ptw, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw, unsigned vectorization)
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

        accum<cacc80, 2> acc_lo, acc_hi;

        vector<input_type, 8> ybuf = data.hi.template grow<8>();

        acc_lo = ::lmac2(::lups(data.lo, shift_tw_), ybuf, 0, 0x10,    false, data.tw, 0, data.zoffs, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::lmsc2_sym(acc_lo,                 ybuf, 0, 0x10, 0, false, data.tw, 0, data.zoffs, inv_);
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
struct fft_dit<Vectorization, 1, 2, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 1, 2, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2> lo;
        vector<input_type, 2> hi;
        vector<twiddle_type, 2> tw;
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
            ptr_((const v2cint32 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
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
        const v2cint32 * __restrict ptr_;
        const_vector_iterator<cint32, 2> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
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

        accum<cacc80, 2> acc_lo, acc_hi;

        vector<input_type, 8> ybuf = data.lo.template grow<8>();
                              ybuf.insert(1, data.hi);
        vector<twiddle_type, 4>   twbuf = data.tw.template grow<4>();

        acc_lo = ::lmul2(            ybuf, 0, 0x20,           one_,  0, 0x00);
        acc_lo = ::lmac2(acc_lo,     ybuf, 0, 0x31,    false, twbuf, 0, 0x10, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::lmsc2_sym(acc_lo, ybuf, 0, 0x31, 0, false, twbuf, 0, 0x10, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv)
    {
        one_ = ::upd_elem(one_, 0, twiddle_type((1u << shift_tw) - 1, 0));
    }
private:
    unsigned shift_;
    bool inv_;
    vector<twiddle_type, 4> one_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 1, 2, cint32, cint16, cint32> : public fft_dit_common<Vectorization, 1, 2, cint32, cint16, cint32>
{
    using   input_type = cint32;
    using  output_type = cint16;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> lo;
        vector<input_type, 4> hi;
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
            ptr_((const v4cint32 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
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
        const v4cint32 * __restrict ptr_;
        const_vector_iterator<cint32, 4> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
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

        accum<cacc80, 4> acc_lo, acc_hi;

        vector<input_type, 8> ybuf = data.lo.template grow<8>();
                              ybuf.insert(1, data.hi);
        vector<twiddle_type, 4> twbuf = data.tw;

        acc_lo = ::lmul4(ybuf, 0, 0x6420, one_.template grow<8>(), 0, 0x0000);

        acc_lo.insert(0, ::lmac2(acc_lo.template extract<2>(0), ybuf, 0, 0x31,    false, twbuf, 0, 0x10, inv_));
        acc_lo.insert(1, ::lmac2(acc_lo.template extract<2>(1), ybuf, 4, 0x31,    false, twbuf, 2, 0x10, inv_));
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi.insert(0, ::lmsc2_sym(acc_lo.template extract<2>(0), ybuf, 0, 0x31, 0, false, twbuf, 0, 0x10, inv_));
        acc_hi.insert(1, ::lmsc2_sym(acc_lo.template extract<2>(1), ybuf, 4, 0x31, 4, false, twbuf, 2, 0x10, inv_));
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv)
    {
        one_ = ::upd_elem(one_, 0, int32((1u << shift_tw) - 1));
    }
private:
    unsigned shift_;
    bool inv_;
    vector<int32, 4> one_;
};

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 2, cint16, Output, cint32> : public fft_dit_common<Vectorization, 2, 2, cint16, Output, cint32>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> lo;
        vector<input_type, 4> hi;
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
            ptr_((const v4cint16 *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        constexpr stage_iterator &operator++()
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
        const v4cint16 * __restrict ptr_;
        const_vector_iterator<cint32, 4> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw) const
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

        accum<cacc80, 4> acc_lo, acc_hi;

        vector<input_type, 16> ybuf = data.lo.template grow<16>();
                               ybuf.insert(1, data.hi);

        acc_lo = ::lmul4(            ybuf, 0, 0x6420,              one_, 0, 0x0000);
        acc_lo = ::lmac4(acc_lo,     ybuf, 0, 0x7531,    false, data.tw, 0, 0x3210, inv_);
        ret[0] = acc_lo.to_vector<output_type>(shift_);

        acc_hi = ::lmsc4_sym(acc_lo, ybuf, 0, 0x7531, 0, false, data.tw, 0, 0x3210, inv_);
        ret[1] = acc_hi.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_(shift), inv_(inv)
    {
        one_ = ::upd_elem(one_, 0, twiddle_type((1u << shift_tw) - 1, 0));
    }
private:
    unsigned shift_;
    bool inv_;
    vector<twiddle_type, 4> one_;
};

// Radix 3

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 3, cint32, Output, cint16> : public fft_dit_common<Vectorization, 0, 3, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat0d;
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
            ptr_begin0_((const v4cint32 *)(ptr)),
            ptr_begin0d_((const v4cint32 *)(ptr)),
            ptr_begin1_((const v4cint32 *)(ptr + r)),
            ptr_begin2_((const v4cint32 *)(ptr + r * 2)),
            ptr_end0_((const v4cint32 *)(ptr + r)),
            ptr_end0d_((const v4cint32 *)(ptr + r)),
            ptr_end1_((const v4cint32 *)(ptr + r * 2)),
            ptr_end2_((const v4cint32 *)(ptr + r * 3)),
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
            ptr_begin0_   = ::fft_data_incr(ptr_begin0_,  r_/4, ptr_end0_, cnt_);
            ptr_begin0d_  = ::fft_data_incr(ptr_begin0d_, r_/4, ptr_end0d_     );
            ptr_begin1_   = ::fft_data_incr(ptr_begin1_,  r_/4, ptr_end1_      );
            ptr_begin2_   = ::fft_data_incr(ptr_begin2_,  r_/4, ptr_end2_      ); 

            ptw2_  = ::fft_twiddle_incr(ptw2_,cnt_,2);

            zoffs_ = ::fft_zoffs(cnt_,3,0,0);

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
            return { *ptr_begin0_, *ptr_begin0d_, *ptr_begin1_, *ptr_begin2_, *ptw2_, *(ptw2_ + twdiff_), zoffs_};
        }

    private:
        const v4cint32 * __restrict ptr_begin0_;
        const v4cint32 * __restrict ptr_begin0d_;
        const v4cint32 * __restrict ptr_begin1_;
        const v4cint32 * __restrict ptr_begin2_;
        const v4cint32 *            ptr_end0_;
        const v4cint32 *            ptr_end0d_;
        const v4cint32 *            ptr_end1_;
        const v4cint32 *            ptr_end2_;
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

        vector<twiddle_type, 8> zbuf;
        vector<input_type, 8> ybuf, ubuf;
        ybuf.insert(0 ,data.dat0);
        ybuf.insert(1, data.dat1);
        zbuf.insert(0, data.tw1);

        accum<cacc48, 8> o0;
        accum<cacc48, 4> d, e, o1, o2;

        //Putting ybuf in the upper half will allow XB to be used, otherwise register allocation leads to a worse schedule
        o0 = ::half_butterfly_dit(    ::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, zbuf, 0, data.zoffs, inv_, shift_tw_);
        d = o0.extract<4>(1);
        
        ybuf.insert(1, data.dat2);
        zbuf.insert(1, data.tw2);  
        //Putting ybuf in the upper half will allow XB to be used, otherwise register allocation leads to a worse schedule
        o0 = ::half_butterfly_dit(o0, ::undef_v8cint32(), ybuf,         0xFEDC, zbuf, 4, data.zoffs, inv_);
        e = o0.extract<4>(1);
            
        ubuf.insert(0, d.to_vector<input_type>(shift_));
        ubuf.insert(1, e.to_vector<input_type>(shift_));
        
        vector<input_type, 4> a = data.dat0d;
        o1 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, qbuf_, 0, 0x0000,  inv_);
        o2 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, qbuf_, 0, 0x0000, !inv_);
        
        ret[0] = o0.extract<4>(0).to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);       

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        qbuf_ = upd_elem(qbuf_, 0, as_cint16((1u << (shift_tw_ - 1)) | ((28378u >> (15 - shift_tw_)) << 16))); // 0.5 + 0.5j*sqrt(3)
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 8>  qbuf_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 3, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 0, 3, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2>   dat0;
        vector<input_type, 2>   dat1;
        vector<input_type, 2>   dat2;
        vector<twiddle_type, 2> tw2;
        vector<twiddle_type, 2> tw1;
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
            ptr_begin0_((const v2cint32 *)(ptr)),
            ptr_begin1_((const v2cint32 *)(ptr + r)),
            ptr_begin2_((const v2cint32 *)(ptr + r * 2)),
            ptr_end0_((const v2cint32 *)(ptr + r)),
            ptr_end1_((const v2cint32 *)(ptr + r * 2)),
            ptr_end2_((const v2cint32 *)(ptr + r * 3)),
            ptw2_((const v2cint32 *)ptw2),
            r_(r),
            cnt_(0),
            zoffs_(0x00000000),
            twdiff_((const v2cint32 *)ptw1 - (const v2cint32 *)ptw2)
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
            ptr_begin0_   = ::fft_data_incr(ptr_begin0_, r_/2,ptr_end0_, cnt_);
            ptr_begin1_   = ::fft_data_incr(ptr_begin1_, r_/2,ptr_end1_     );
            ptr_begin2_   = ::fft_data_incr(ptr_begin2_, r_/2,ptr_end2_     ); 

            ptw2_  = ::fft_twiddle_incr(ptw2_,cnt_,1);

            zoffs_ = ::fft_zoffs(cnt_,3,0,0) & 0x11;

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
            return { *ptr_begin0_, *ptr_begin1_, *ptr_begin2_, *ptw2_, *(ptw2_ + twdiff_), zoffs_};
        }

    private:
        const v2cint32 * __restrict ptr_begin0_;
        const v2cint32 * __restrict ptr_begin1_;
        const v2cint32 * __restrict ptr_begin2_;
        const v2cint32 *            ptr_end0_;
        const v2cint32 *            ptr_end1_;
        const v2cint32 *            ptr_end2_;
        const v2cint32 * __restrict ptw2_;
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

        vector<twiddle_type, 4> zbuf;
        vector<input_type, 8> ybuf, ubuf;
        ybuf.insert(0, data.dat1);
        ybuf.insert(1, data.dat2);
        zbuf.insert(0, data.tw1);
        zbuf.insert(1, data.tw2);

        accum<cacc80, 2> a = ::lups(data.dat0, shift_tw_);

        accum<cacc80, 2> o0, o1, o2;
        accum<cacc80, 2> d, e;

        d = ::lmul2(ybuf, 0, 0x10, false, zbuf, 0, data.zoffs, inv_);
        e = ::lmul2(ybuf, 2, 0x10, false, zbuf, 2, data.zoffs, inv_);

        vector<input_type, 8> dv, ev;
        dv.insert(0, d.to_vector<input_type>(shift_tw_));
        ev.insert(0, e.to_vector<input_type>(shift_tw_));

        o0 = ::lmac2(a,  dv, 0, 0x10, false, qbuf_, 1, 0x00, false);
        o0 = ::lmac2(o0, ev, 0, 0x10, false, qbuf_, 1, 0x00, false);

        o1 = ::lmsc2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);
        o1 = ::lmsc2(o1, ev, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);

        o2 = ::lmsc2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);
        o2 = ::lmsc2(o2, ev, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);
        
        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        qbuf_ = ::upd_elem(qbuf_, 0, twiddle_type(1u << (shift_tw_ - 1), (1859775393u >> (31 - shift_tw_)))); // -0.5 - 0.5j*sqrt(3)
        qbuf_ = ::upd_elem(qbuf_, 1, twiddle_type((1u << shift_tw_) - 1, 0));
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 4> qbuf_;
};

// Radix 4

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 4, cint32, cint32, cint16> : public fft_dit_common<Vectorization, 0, 4, cint32, cint32, cint16>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo0;
        vector<input_type, 4>   hi0;
        vector<input_type, 4>   lo1;
        vector<input_type, 4>   hi1;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        int zoffs1;
        int zoffs2;
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
            ptr_begin1_((const v4cint32 *)(ptr)),
            ptr_begin2_((const v4cint32 *)(ptr + r)),
            ptr_begin4_((const v4cint32 *)(ptr + r * 3)),
            ptr_end1_((const v4cint32 *)(ptr + r)),
            ptr_end2_((const v4cint32 *)(ptr + r * 2)),
            ptr_end4_((const v4cint32 *)(ptr + r * 4)),
            ptw1_((const v8cint16 *)ptw1),
            ptw2_((const v8cint16 *)ptw2),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            cnt4_(0),
            cnt3d_(0),
            zoffs1_(0x00000000),
            zoffs2_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr),  "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw1), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw2), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, r_ / 4, ptr_end1_, cnt1_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_, r_ / 4, ptr_end2_, cnt2_);
            ptr_begin4_  = ::fft_data_incr(ptr_begin4_, r_ / 4, ptr_end4_, cnt4_);

            cnt3d_       = ::fft_mod_delay(cnt4_);

            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt1_ , 3);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt3d_, 3);

            zoffs1_      = ::fft_zoffs(cnt2_ , 3, 1, 0);
            zoffs2_      = ::fft_zoffs(cnt3d_, 3, 1, 0);

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
            return { *ptr_begin1_, *(ptr_begin4_-r_/4), *ptr_begin4_, *ptr_begin2_, *ptw1_, *ptw2_, zoffs1_, zoffs2_ };
        }

    private:
        const v4cint32 * __restrict ptr_begin1_;
        const v4cint32 * __restrict ptr_begin2_;
        const v4cint32 * __restrict ptr_begin4_;
        const v4cint32 *            ptr_end1_;
        const v4cint32 *            ptr_end2_;
        const v4cint32 *            ptr_end4_;
        const v8cint16 * __restrict ptw1_;
        const v8cint16 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        umod_t cnt4_;
        umod_t cnt3d_;
        int zoffs1_;
        int zoffs2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy,
                               unsigned vectorization)
    {
        set_rdx(4);

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
        accum<cacc48, 8> a0, a1, o0, o1;

        vector<input_type, 8> xbuf;
        uint8_t  shft = chess_copy(shift_tw_); //CRVO-1460
        vector<output_type, 8> __aie_register(xd) ybuf;

        xbuf.insert(0, data.lo0);
        xbuf.insert(1, data.hi0);
        a0 = ::butterfly_dit(xbuf,      ::undef_v8cint32(), 0x3210, 0x7654, data.tw1, 0, data.zoffs1, inv_, shift_tw_);

        xbuf.insert(1, data.hi1);
        xbuf.insert(0, data.lo1);
        a1 = ::butterfly_dit(xbuf,      ::undef_v8cint32(), 0x7654, 0x3210, data.tw1, 0, data.zoffs1, inv_, shift_tw_);

        ybuf.insert(0, a0.extract<4>(0).to_vector<output_type>(shft));
        ybuf.insert(1, a1.extract<4>(0).to_vector<output_type>(shft));

        o0 = ::butterfly_dit(ybuf,      ::undef_v8cint32(), 0x3210, 0x7654, data.tw2, 0, data.zoffs2, inv_, shift_tw_);

        ybuf.insert(0, a0.extract<4>(1).to_vector<output_type>(shft));
        ybuf.insert(1, a1.extract<4>(1).to_vector<output_type>(shft));

        o1 = ::butterfly_dit_minj(ybuf, ::undef_v8cint32(), 0x3210, 0x7654, data.tw2, 0, data.zoffs2, inv_, shift_tw_);

        ret[0] = o0.extract<4>(0).to_vector<output_type>(shift_);
        ret[1] = o1.extract<4>(0).to_vector<output_type>(shift_);
        ret[2] = o0.extract<4>(1).to_vector<output_type>(shift_);
        ret[3] = o1.extract<4>(1).to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 1, 4, cint32, Output, cint16> : public fft_dit_common<Vectorization, 1, 4, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo0;
        vector<input_type, 4>   hi0;
        vector<input_type, 4>   lo1;
        vector<input_type, 4>   hi1;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2) :
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw1_((const v4cint16 *)ptw1),
            ptw2_((const v4cint16 *)ptw2)
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
            ptr_begin_  += 4;
            ptw1_       += 1;
            ptw2_       += 1;
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
            return { *ptr_begin_, *(ptr_begin_+2), *(ptr_begin_+1), *(ptr_begin_+3), *ptw1_, *ptw2_};
        }

    private:
        const v4cint32 * __restrict ptr_begin_;
        const v4cint16 * __restrict ptw1_;
        const v4cint16 * __restrict ptw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2);
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
        accum<cacc48, 8> a0, a1, o0, o1;

        vector<input_type, 8> xbuf;
        vector<input_type, 8> __aie_register(xd) ybuf;
        uint8_t  shft = chess_copy(shift_tw_); //CRVO-1460

        xbuf.insert(0, data.lo0);
        xbuf.insert(1, data.hi0);
        a0 = ::butterfly_dit(xbuf,      ::undef_v8cint32(), 0x5410, 0x7632, data.tw1.template grow<8>(), 0, 0x2200, inv_, shift_tw_);

        xbuf.insert(0, data.lo1);
        xbuf.insert(1, data.hi1);
        a1 = ::butterfly_dit(xbuf,      ::undef_v8cint32(), 0x5410, 0x7632, data.tw1.template grow<8>(), 0, 0x3311, inv_, shift_tw_);

        ybuf.insert(0, a0.template extract<4>(0).to_vector<input_type>(shft));
        ybuf.insert(1, a1.template extract<4>(0).to_vector<input_type>(shft));

        o0 = ::butterfly_dit(ybuf,      ::undef_v8cint32(), 0x6240, 0x7351, data.tw2.template grow<8>(), 0, 0x3210, inv_, shift_tw_);

        ybuf.insert(0, a0.template extract<4>(1).to_vector<input_type>(shft));
        ybuf.insert(1, a1.template extract<4>(1).to_vector<input_type>(shft));

        o1 = ::butterfly_dit_minj(ybuf, ::undef_v8cint32(), 0x6240, 0x7351, data.tw2.template grow<8>(), 0, 0x3210, inv_, shift_tw_);

        ret[0] = o0.template extract<4>(0).template to_vector<output_type>(shift_);
        ret[2] = o0.template extract<4>(1).template to_vector<output_type>(shift_);
        ret[1] = o1.template extract<4>(0).template to_vector<output_type>(shift_);
        ret[3] = o1.template extract<4>(1).template to_vector<output_type>(shift_);

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
struct fft_dit<Vectorization, 1, 4, cint16, cint16, cint16> : public fft_dit_common<Vectorization, 1, 4, cint16, cint16, cint16>
{
    using   input_type = cint16;
    using  output_type = cint16;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<cint16, 4> dat0;
        vector<cint16, 4> dat1;
        vector<cint16, 4> dat2;
        vector<cint16, 4> dat3;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2) :
            ptr_begin_((const v4cint16 *)(ptr)),
            ptw1_((const v4cint16 *)ptw1),
            ptw2_((const v4cint16 *)ptw2)
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
            ptr_begin_  += 4;
            ptw1_       += 1;
            ptw2_       += 1;
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
            return { *ptr_begin_, *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw1_, *ptw2_};
        }

    private:
        const v4cint16 * __restrict ptr_begin_;
        const v4cint16 * __restrict ptw1_;
        const v4cint16 * __restrict ptw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2);
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

        vector<input_type, 16> xbuf;
        vector<cint32, 8> ybuf;

        xbuf.insert(0, data.dat0);
        xbuf.insert(1, data.dat1);
        xbuf.insert(2, data.dat2);
        xbuf.insert(3, data.dat3);

        vector<input_type, 16> init_buf = ::shuffle16(xbuf, 0, 0xD951C840, 0x00000000);

        accum<cacc48, 8> a0 = ::mac8(::ups(init_buf.template extract<8>(0), shift_tw_), xbuf, 0, 0xFB73EA62, 0, data.tw1.template grow<8>(), 0, 0x32103210, inv_);
        accum<cacc48, 8> a1 = ::msc8(::ups(init_buf.template extract<8>(0), shift_tw_), xbuf, 0, 0xFB73EA62, 0, data.tw1.template grow<8>(), 0, 0x32103210, inv_);

        ybuf.insert(0, a0.template extract<4>(0).to_vector<cint32>(shift_tw_));
        ybuf.insert(1, a0.template extract<4>(1).to_vector<cint32>(shift_tw_));

        accum<cacc48, 8> o0 = ::butterfly_dit(     ::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, data.tw2.template grow<8>(), 0, 0x32103210, inv_, shift_tw_);

        ybuf.insert(0, a1.template extract<4>(0).to_vector<cint32>(shift_tw_));
        ybuf.insert(1, a1.template extract<4>(1).to_vector<cint32>(shift_tw_));

        accum<cacc48, 8> o1 = ::butterfly_dit_minj(::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, data.tw2.template grow<8>(), 0, 0x32103210, inv_, shift_tw_);

        ret[0] = o0.template extract<4>(0).template to_vector<output_type>(shift_);
        ret[2] = o0.template extract<4>(1).template to_vector<output_type>(shift_);
        ret[1] = o1.template extract<4>(0).template to_vector<output_type>(shift_);
        ret[3] = o1.template extract<4>(1).template to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 4, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 4, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo0;
        vector<input_type, 4>   hi0;
        vector<input_type, 4>   lo1;
        vector<input_type, 4>   hi1;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        int zoffs1;
        int zoffs2;
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
            ptr_begin1_((const v4cint16 *)(ptr)),
            ptr_begin2_((const v4cint16 *)(ptr + r)),
            ptr_end1_((const v4cint16 *)(ptr + r)),
            ptr_end2_((const v4cint16 *)(ptr + r * 2)),
            ptw1_((const v8cint16 *)ptw1),
            ptw2_((const v8cint16 *)ptw2),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            cnt2d_(0),
            zoffs1_(0x00000000),
            zoffs2_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr),  "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw1), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw2), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_, r_ / 4, ptr_end2_, cnt2_);
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, r_ / 4, ptr_end1_, cnt1_);

            cnt2d_       = ::fft_mod_delay(cnt2_);

            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt2_ , 3);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt2d_, 3);

            zoffs1_      = ::fft_zoffs(cnt2_,  3, 1, 1);
            zoffs2_      = ::fft_zoffs(cnt2d_, 3, 1, 0);

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
            return { *(ptr_begin2_+ 2*r_/4), *(ptr_begin1_ + 2*r_/4), *ptr_begin2_, *ptr_begin1_, *ptw1_, *ptw2_, zoffs1_, zoffs2_ };
        }

    private:
        const v4cint16 * __restrict ptr_begin1_;
        const v4cint16 * __restrict ptr_begin2_;
        const v4cint16 *            ptr_end1_;
        const v4cint16 *            ptr_end2_;
        const v8cint16 * __restrict ptw1_;
        const v8cint16 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        umod_t cnt2d_;
        int zoffs1_;
        int zoffs2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy,
                               unsigned vectorization)
    {
        set_rdx(4);

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
        accum<cacc48, 8> a0, a1, o0, o1;

        vector<input_type, 8> __aie_register(wd1) xbuf    ;
        vector<input_type, 8> __aie_register(wd0) init_buf;
        uint8_t  shft = chess_copy(shift_tw_); //CRVO-1460
        vector<cint32, 8> __aie_register(xb) ybuf;

        xbuf.insert(1, data.lo0);
        xbuf.insert(0, data.hi0);
        init_buf.insert(1, data.lo1);
        init_buf.insert(0, data.hi1);

        a0 = ::mac8(::ups(init_buf, shift_tw_), ::upd_w(::undef_v16cint16(), 1, xbuf), 8, 0x76543210, 0, data.tw1, 0, data.zoffs1, inv_);
        a1 = ::msc8(::ups(init_buf, shift_tw_), ::upd_w(::undef_v16cint16(), 1, xbuf), 8, 0x76543210, 0, data.tw1, 0, data.zoffs1, inv_);

        ybuf.insert(0, a0.template extract<4>(0).to_vector<cint32>(shft));
        ybuf.insert(1, a0.template extract<4>(1).to_vector<cint32>(shft));

        o0 = ::butterfly_dit(     ::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, data.tw2, 0, data.zoffs2, inv_, shift_tw_);

        ybuf.insert(0, a1.template extract<4>(0).to_vector<cint32>(shft));
        ybuf.insert(1, a1.template extract<4>(1).to_vector<cint32>(shft));

        o1 = ::butterfly_dit_minj(::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, data.tw2, 0, data.zoffs2, inv_, shift_tw_);

        ret[0] = o0.template extract<4>(0).to_vector<output_type>(shift_);
        ret[2] = o0.template extract<4>(1).to_vector<output_type>(shift_);
        ret[1] = o1.template extract<4>(0).to_vector<output_type>(shift_);
        ret[3] = o1.template extract<4>(1).to_vector<output_type>(shift_);

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
struct fft_dit<Vectorization, 0, 4, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 0, 4, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2>   dat0;
        vector<input_type, 2>   dat1;
        vector<input_type, 2>   dat2;
        vector<input_type, 2>   dat3;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
        int zoffs1;
        int zoffs2;
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
            ptr_begin1_((const v2cint32 *)(ptr)),
            ptr_begin2_((const v2cint32 *)(ptr + r)),
            ptr_begin4_((const v2cint32 *)(ptr + r * 3)),
            ptr_end1_((const v2cint32 *)(ptr + r)),
            ptr_end2_((const v2cint32 *)(ptr + r * 2)),
            ptr_end4_((const v2cint32 *)(ptr + r * 4)),
            ptw1_((const v4cint32 *)ptw1),
            ptw2_((const v4cint32 *)ptw2),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            cnt4_(0),
            cnt3d_(0),
            zoffs1_(0x00000000),
            zoffs2_(0x00000000)
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
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, r_ / 2, ptr_end1_, cnt1_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_, r_ / 2, ptr_end2_, cnt2_);
            ptr_begin4_  = ::fft_data_incr(ptr_begin4_, r_ / 2, ptr_end4_, cnt4_);

            cnt3d_       = ::fft_mod_delay(cnt4_);

            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt1_ , 2);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt3d_, 2);

            zoffs1_      = ::fft_zoffs(cnt2_ , 3, 0, 0);
            zoffs2_      = ::fft_zoffs(cnt3d_, 3, 0, 0);

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
            return { *ptr_begin1_, *ptr_begin2_, *(ptr_begin4_-r_/2), *ptr_begin4_, *ptw1_, *ptw2_, zoffs1_, zoffs2_ };
        }

    private:
        const v2cint32 * __restrict ptr_begin1_;
        const v2cint32 * __restrict ptr_begin2_;
        const v2cint32 * __restrict ptr_begin4_;
        const v2cint32 *            ptr_end1_;
        const v2cint32 *            ptr_end2_;
        const v2cint32 *            ptr_end4_;
        const v4cint32 * __restrict ptw1_;
        const v4cint32 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        umod_t cnt4_;
        umod_t cnt3d_;
        int zoffs1_;
        int zoffs2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy,
                               unsigned vectorization)
    {
        set_rdx(4);

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

        vector<input_type, 8> ybuf;
        ybuf.insert(0, data.dat2);
        ybuf.insert(1, data.dat3);

        accum<cacc80, 2> o0, o1, o2, o3;

        accum<cacc80, 2> g, h;
        accum<cacc80, 2> m, n;

        vector<twiddle_type, 4> zbuf;
        zbuf.insert(0, data.tw2);
        accum<cacc80, 2> tw2_min_j = ::lmul2(zbuf.grow<8>(), 0, data.zoffs2, false, min_j_, 0, 0x00, false);
        zbuf.insert(0, tw2_min_j.to_vector<twiddle_type>());

        g = ::lmac2(::lups(data.dat0, shift_tw_), ybuf, 0, 0x10, false, data.tw1, 0, data.zoffs1, inv_);
        h = ::lmsc2(::lups(data.dat0, shift_tw_), ybuf, 0, 0x10, false, data.tw1, 0, data.zoffs1, inv_);

        m = ::lmac2(::lups(data.dat1, shift_tw_), ybuf, 2, 0x10, false, data.tw1, 0, data.zoffs1, inv_);
        n = ::lmsc2(::lups(data.dat1, shift_tw_), ybuf, 2, 0x10, false, data.tw1, 0, data.zoffs1, inv_);

        o0 = ::lmac2(g, m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, data.tw2, 0, data.zoffs2, inv_);
        o1 = ::lmac2(h, n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf,     0, 0x00,        inv_);
        o2 = ::lmsc2(g, m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, data.tw2, 0, data.zoffs2, inv_);
        o3 = ::lmsc2(h, n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf,     0, 0x00,        inv_);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);
        ret[3] = o3.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit()
    {
        min_j_[0] = twiddle_type(0,-1);
    }
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        min_j_[0] = twiddle_type(0,-1);
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 4> min_j_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 1, 4, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 1, 4, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2>   lo0;
        vector<input_type, 2>   hi0;
        vector<input_type, 2>   lo1;
        vector<input_type, 2>   hi1;
        vector<twiddle_type, 2> tw1;
        vector<twiddle_type, 2> tw2;
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
            ptr_begin_((const v2cint32 *)(ptr)),
            ptw1_((const v2cint32 *)ptw1),
            ptw2_((const v2cint32 *)ptw2)
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
            ptr_begin_  += 4;
            ptw1_       += 1;
            ptw2_       += 1;

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
            return { *ptr_begin_, *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw1_, *ptw2_ };
        }

    private:
        const v2cint32 * __restrict ptr_begin_;
        const v2cint32 * __restrict ptw1_;
        const v2cint32 * __restrict ptw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
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

        vector<input_type, 8> ybuf;
        ybuf.insert(0, data.lo0);
        ybuf.insert(1, data.hi0);
        ybuf.insert(2, data.lo1);
        ybuf.insert(3, data.hi1);

        accum<cacc80, 2> o0, o1, o2, o3;

        accum<cacc80, 4> d0 = ::lmul4(ybuf, 0, 0x5140, ones_.template grow<8>(), 1, 0x0000);
        accum<cacc80, 4> d1 = ::lmul4(ybuf, 0, 0x7362, ones_.template grow<8>(), 0, 0x0000);
        ybuf.insert(0, d0.to_vector<input_type>());
        ybuf.insert(1, d1.to_vector<input_type>());

        accum<cacc80, 2> g, h;
        accum<cacc80, 2> m, n;

        vector<twiddle_type, 8> zbuf;
        zbuf.insert(0, data.tw1);
        zbuf.insert(1, data.tw2);
        accum<cacc80, 2> tw2_min_j = ::lmul2(zbuf, 2, 0x10, false, min_j_, 0, 0x00, false);
        zbuf.insert(2, tw2_min_j.to_vector<twiddle_type>());

        g = ::lmac2(d0.extract<2>(0), ybuf, 4, 0x10, false, zbuf.extract<4>(0), 0, 0x10, inv_);
        h = ::lmsc2(d0.extract<2>(0), ybuf, 4, 0x10, false, zbuf.extract<4>(0), 0, 0x10, inv_);

        m = ::lmac2(d0.extract<2>(1), ybuf, 6, 0x10, false, zbuf.extract<4>(0), 0, 0x10, inv_);
        n = ::lmsc2(d0.extract<2>(1), ybuf, 6, 0x10, false, zbuf.extract<4>(0), 0, 0x10, inv_);

        o0 = ::lmac2(g,  m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(0), 2, 0x10, inv_);
        o1 = ::lmac2(h,  n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(1), 0, 0x10, inv_);
        o2 = ::lmsc2(g,  m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(0), 2, 0x10, inv_);
        o3 = ::lmsc2(h,  n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(1), 0, 0x10, inv_);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);
        ret[3] = o3.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        ones_  = ::upd_elem(ones_, 0, 1);
        ones_  = ::upd_elem(ones_, 1, (1u << shift_tw) - 1);
        min_j_ = ::upd_elem(min_j_, 0, twiddle_type(0,-1));
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<int32, 4>  ones_;
    vector<twiddle_type, 4>  min_j_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 1, 4, cint32, cint16, cint32> : public fft_dit_common<Vectorization, 1, 4, cint32, cint16, cint32>
{
    using   input_type = cint32;
    using  output_type = cint16;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   a;
        vector<input_type, 4>   b;
        vector<input_type, 4>   c;
        vector<input_type, 4>   d;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
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
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw1_((const v4cint32 *)ptw1),
            ptw2_((const v4cint32 *)ptw2)
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
            ptr_begin_  += 4;
            ptw1_       += 1;
            ptw2_       += 1;

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
            return { *ptr_begin_, *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw1_, *ptw2_ };
        }

    private:
        const v4cint32 * __restrict ptr_begin_;
        const v4cint32 * __restrict ptw1_;
        const v4cint32 * __restrict ptw2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
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

        vector<input_type, 16> ybuf;
        ybuf.insert(0, data.a);
        ybuf.insert(1, data.b);
        ybuf.insert(2, data.c);
        ybuf.insert(3, data.d);

        accum<cacc80, 4> d0 = ::lmul4(ybuf, 0, 0xC840, ones_.template grow<8>(), 1, 0x0000);
        accum<cacc80, 4> d1 = ::lmul4(ybuf, 1, 0xC840, ones_.template grow<8>(), 1, 0x0000);

        vector<twiddle_type, 8> zbuf;
        zbuf.insert(0, data.tw1);
        zbuf.insert(1, data.tw2);

        accum<cacc80, 4> tw2_min_j_acc = ::lmul4(zbuf, 4, 0x3210, false, min_j_.template grow<8>(), 0, 0x0000, false);
        vector<twiddle_type, 4> tw2_min_j = tw2_min_j_acc.to_vector<twiddle_type>();

        accum<cacc80, 4> g, h;
        accum<cacc80, 4> m, n;

        g.insert(0, ::lmac2(d0.extract<2>(0), ybuf, 0, 0x62, false, zbuf.extract<4>(0), 0, 0x10, inv_));
        g.insert(1, ::lmac2(d0.extract<2>(1), ybuf, 8, 0x62, false, zbuf.extract<4>(0), 2, 0x10, inv_));
        h.insert(0, ::lmsc2(d0.extract<2>(0), ybuf, 0, 0x62, false, zbuf.extract<4>(0), 0, 0x10, inv_));
        h.insert(1, ::lmsc2(d0.extract<2>(1), ybuf, 8, 0x62, false, zbuf.extract<4>(0), 2, 0x10, inv_));

        m.insert(0, ::lmac2(d1.extract<2>(0), ybuf, 0, 0x73, false, zbuf.extract<4>(0), 0, 0x10, inv_));
        m.insert(1, ::lmac2(d1.extract<2>(1), ybuf, 8, 0x73, false, zbuf.extract<4>(0), 2, 0x10, inv_));
        n.insert(0, ::lmsc2(d1.extract<2>(0), ybuf, 0, 0x73, false, zbuf.extract<4>(0), 0, 0x10, inv_));
        n.insert(1, ::lmsc2(d1.extract<2>(1), ybuf, 8, 0x73, false, zbuf.extract<4>(0), 2, 0x10, inv_));

        accum<cacc80, 4> o0, o1, o2, o3;

        o0.insert(0, ::lmac2(g.extract<2>(0), m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(1), 0, 0x10, inv_));
        o0.insert(1, ::lmac2(g.extract<2>(1), m.to_vector<input_type>(shift_tw_).template grow<8>(), 2, 0x10, false, zbuf.extract<4>(1), 2, 0x10, inv_));
        o1.insert(0, ::lmac2(h.extract<2>(0), n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, tw2_min_j,          0, 0x10, inv_));
        o1.insert(1, ::lmac2(h.extract<2>(1), n.to_vector<input_type>(shift_tw_).template grow<8>(), 2, 0x10, false, tw2_min_j,          2, 0x10, inv_));
        o2.insert(0, ::lmsc2(g.extract<2>(0), m.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, zbuf.extract<4>(1), 0, 0x10, inv_));
        o2.insert(1, ::lmsc2(g.extract<2>(1), m.to_vector<input_type>(shift_tw_).template grow<8>(), 2, 0x10, false, zbuf.extract<4>(1), 2, 0x10, inv_));
        o3.insert(0, ::lmsc2(h.extract<2>(0), n.to_vector<input_type>(shift_tw_).template grow<8>(), 0, 0x10, false, tw2_min_j,          0, 0x10, inv_));
        o3.insert(1, ::lmsc2(h.extract<2>(1), n.to_vector<input_type>(shift_tw_).template grow<8>(), 2, 0x10, false, tw2_min_j,          2, 0x10, inv_));

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);
        ret[3] = o3.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        ones_  = ::upd_elem(ones_, 0, 1);
        ones_  = ::upd_elem(ones_, 1, (1u << shift_tw) - 1);
        min_j_ = ::upd_elem(min_j_, 0, cint16(0,-1));
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<int32, 4>  ones_;
    vector<cint16, 4>  min_j_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 4, cint16, cint32, cint32> : public fft_dit_common<Vectorization, 0, 4, cint16, cint32, cint32>
{
    using   input_type = cint16;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
        vector<input_type, 4>   dat3;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
        int zoffs1;
        int zoffs2;
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
            ptr_begin1_((const v4cint16 *)(ptr)),
            ptr_begin2_((const v4cint16 *)(ptr + r)),
            ptr_begin4_((const v4cint16 *)(ptr + r * 3)),
            ptr_end1_((const v4cint16 *)(ptr + r)),
            ptr_end2_((const v4cint16 *)(ptr + r * 2)),
            ptr_end4_((const v4cint16 *)(ptr + r * 4)),
            ptw1_((const v4cint32 *)ptw1),
            ptw2_((const v4cint32 *)ptw2),
            r_(r),
            cnt1_(0),
            cnt2_(0),
            cnt4_(0),
            cnt3d_(0),
            zoffs1_(0x00000000),
            zoffs2_(0x00000000)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr),  "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw1), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<8>(ptw2), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_, r_ / 4, ptr_end1_, cnt1_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_, r_ / 4, ptr_end2_, cnt2_);
            ptr_begin4_  = ::fft_data_incr(ptr_begin4_, r_ / 4, ptr_end4_, cnt4_);

            cnt3d_       = ::fft_mod_delay(cnt4_);

            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt1_ , 2);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt3d_, 2);

            zoffs1_      = ::fft_zoffs(cnt2_ , 3, 0, 0);
            zoffs2_      = ::fft_zoffs(cnt3d_, 3, 0, 0);

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
            return { *ptr_begin1_, *ptr_begin2_, *(ptr_begin4_-r_/4), *ptr_begin4_, *ptw1_, *ptw2_, zoffs1_, zoffs2_ };
        }

    private:
        const v4cint16 * __restrict ptr_begin1_;
        const v4cint16 * __restrict ptr_begin2_;
        const v4cint16 * __restrict ptr_begin4_;
        const v4cint16 *            ptr_end1_;
        const v4cint16 *            ptr_end2_;
        const v4cint16 *            ptr_end4_;
        const v4cint32 * __restrict ptw1_;
        const v4cint32 * __restrict ptw2_;
        unsigned r_;
        umod_t cnt1_;
        umod_t cnt2_;
        umod_t cnt4_;
        umod_t cnt3d_;
        int zoffs1_;
        int zoffs2_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy = nullptr)
    {
        set_rdx(4);

        return stage_iterator(data, ptw1, ptw2, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw1,
                               const twiddle_type * ptw2,
                               const twiddle_type * ptw_dummy,
                               unsigned vectorization)
    {
        set_rdx(4);

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

        vector<input_type, 16> ybuf;
        ybuf.insert(0, data.dat2);
        ybuf.insert(1, data.dat3);

        accum<cacc80, 4> o0, o1, o2, o3;

        accum<cacc80, 4> g, h;
        accum<cacc80, 4> m, n;

        vector<twiddle_type, 4> zbuf;
        zbuf.insert(0, data.tw2);
        accum<cacc48, 4> tw2_min_j = ::mul4(zbuf.grow<8>(), 0, data.zoffs2, min_j_.grow<8>(), 0, 0x0000);
        zbuf.insert(0, tw2_min_j.to_vector<twiddle_type>());

        v4cacc80 d0 = ::lmul4(data.dat0.template grow<16>(), 0, 0x3210, one_, 0, 0x0000);
        v4cacc80 d1 = ::lmul4(data.dat1.template grow<16>(), 0, 0x3210, one_, 0, 0x0000);

        g = ::lmac4(d0, ybuf, 0, 0x3210, false, data.tw1, 0, data.zoffs1, inv_);
        h = ::lmsc4(d0, ybuf, 0, 0x3210, false, data.tw1, 0, data.zoffs1, inv_);

        m = ::lmac4(d1, ybuf, 4, 0x3210, false, data.tw1, 0, data.zoffs1, inv_);
        n = ::lmsc4(d1, ybuf, 4, 0x3210, false, data.tw1, 0, data.zoffs1, inv_);

        auto mv = m.to_vector<output_type>(shift_tw_);
        auto nv = n.to_vector<output_type>(shift_tw_);

        o0.insert(0, ::lmac2(g.template extract<2>(0), mv.template grow<8>(), 0, 0x10, false, data.tw2, 0, data.zoffs2, inv_));
        o0.insert(1, ::lmac2(g.template extract<2>(1), mv.template grow<8>(), 2, 0x10, false, data.tw2, 0, data.zoffs2, inv_));

        o1.insert(0, ::lmac2(h.template extract<2>(0), nv.template grow<8>(), 0, 0x10, false, zbuf,     0, 0x00,        inv_));
        o1.insert(1, ::lmac2(h.template extract<2>(1), nv.template grow<8>(), 2, 0x10, false, zbuf,     0, 0x00,        inv_));

        o2.insert(0, ::lmsc2(g.template extract<2>(0), mv.template grow<8>(), 0, 0x10, false, data.tw2, 0, data.zoffs2, inv_));
        o2.insert(1, ::lmsc2(g.template extract<2>(1), mv.template grow<8>(), 2, 0x10, false, data.tw2, 0, data.zoffs2, inv_));
        
        o3.insert(0, ::lmsc2(h.template extract<2>(0), nv.template grow<8>(), 0, 0x10, false, zbuf,     0, 0x00,        inv_));
        o3.insert(1, ::lmsc2(h.template extract<2>(1), nv.template grow<8>(), 2, 0x10, false, zbuf,     0, 0x00,        inv_));

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);
        ret[2] = o2.to_vector<output_type>(shift_);
        ret[3] = o3.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        min_j_[0] = input_type(0,-1);
        one_[0]  = twiddle_type((1u << shift_tw_) - 1, 0);
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<input_type, 4> min_j_;
    vector<twiddle_type, 4> one_;
};

// Radix 5

template <unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 5, cint32, Output, cint16> : public fft_dit_common<Vectorization, 0, 5, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 5, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    { //Data numbering refers to points on the unit circle which are computed in pairs, the load sequence helps the compiler schedule the pointer modifications
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
        int zoffs_;
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
            ptr_begin0_((const v4cint32 *)(ptr + r)),
            ptr_begin1_((const v4cint32 *)(ptr)),
            ptr_end0_((const v4cint32 *)(ptr + r * 3)),
            ptr_end1_((const v4cint32 *)(ptr + r)),
            ptw2_((const v4cint16 *)ptw2),
            r_(r),
            cnt_(0),
            zoffs_(0x00000000),
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
            //Decoupling code selection from scheduling (prevents the post increment from being bundled with the loads)
            unsigned r_4 = chess_copy(r_/4);
            const v4cint32  * chess_storage(P) ptr_tmp_ = ptr_begin0_;
            ptr_tmp_     += r_4; 
            ptr_tmp_      = ::fft_data_incr(ptr_tmp_,    r_4, ptr_end0_, cnt_); 
            ptr_tmp_     += -(int)(r_4); 
            ptr_begin1_   = ::fft_data_incr(ptr_begin1_, r_4, ptr_end1_     );
            ptr_begin0_   = ptr_tmp_;

            ptw2_  = ::fft_twiddle_incr(ptw2_, cnt_, 2);

            zoffs_ = ::fft_zoffs(cnt_, 3, 0, 0);

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
        const v4cint32 * __restrict ptr_begin0_;
        const v4cint32 * __restrict ptr_begin1_;
        const v4cint32 *            ptr_end0_;
        const v4cint32 *            ptr_end1_;
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

        accum<cacc48, 8> o0;
        accum<cacc48, 4> x1,x2,x3,x4, o1, o2, o3, o4;

        vector<input_type, 8> ybuf;
        vector<cint32, 8> ubuf, vbuf;  
        
        ybuf.insert(1, data.dat1); 
        ybuf.insert(0, data.dat0);
        qbuf.insert(1, data.tw1);
        o0 = ::half_butterfly_dit(    ::undef_v8cint32(), ybuf, 0xBA98, 0xFEDC, qbuf, 4, data.zoffs_, inv_, shift_tw_); x1 = o0.extract<4>(1);
        ybuf.insert(1, data.dat4);
        qbuf.insert(1, data.tw4);
        o0 = ::half_butterfly_dit(o0, ::undef_v8cint32(), ybuf,         0xFEDC, qbuf, 4, data.zoffs_, inv_);            x4 = o0.extract<4>(1);
        ybuf.insert(1, data.dat3);
        qbuf.insert(1, data.tw3);
        o0 = ::half_butterfly_dit(o0, ::undef_v8cint32(), ybuf,         0xFEDC, qbuf, 4, data.zoffs_, inv_);            x3 = o0.extract<4>(1);
        ybuf.insert(1, data.dat2);
        qbuf.insert(1, data.tw2);
        o0 = ::half_butterfly_dit(o0, ::undef_v8cint32(), ybuf,         0xFEDC, qbuf, 4, data.zoffs_, inv_);            x2 = o0.extract<4>(1);

        vector<input_type, 4> a = data.dat5;
        ubuf = ::concat(x1.to_vector<cint32>(shift_), x4.to_vector<cint32>(shift_));
        o1 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_, 0, 0x0000,  inv_);
        o4 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_, 0, 0x0000, !inv_);
        o2 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_, 1, 0x0000,  inv_);
        o3 = ::msc4_conj_sym(::ups(a, shift_tw_), ubuf, 0, 0x3210, 4, 0, zbuf_, 1, 0x0000, !inv_);

        vbuf = ::concat(x2.to_vector<cint32>(shift_), x3.to_vector<cint32>(shift_));
        o1 = ::msc4_conj_sym(o1,                        vbuf, 0, 0x3210, 4, 0, zbuf_, 1, 0x0000,  inv_);
        o4 = ::msc4_conj_sym(o4,                        vbuf, 0, 0x3210, 4, 0, zbuf_, 1, 0x0000, !inv_);
        o2 = ::msc4_conj_sym(o2,                        vbuf, 0, 0x3210, 4, 0, zbuf_, 0, 0x0000, !inv_);
        o3 = ::msc4_conj_sym(o3,                        vbuf, 0, 0x3210, 4, 0, zbuf_, 0, 0x0000,  inv_);

        ret[0] = o0.extract<4>(0).to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_); 
        ret[2] = o2.to_vector<output_type>(shift_); 
        ret[3] = o3.to_vector<output_type>(shift_); 
        ret[4] = o4.to_vector<output_type>(shift_); 

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        zbuf_ = upd_elem(zbuf_,0,as_cint16(((-10126u >> (15 - shift_tw_)) & 0xFFFF) | ((31164u >> (15 - shift_tw_)) << 16))); // -exp(-2j*pi/5)
        zbuf_ = upd_elem(zbuf_,1,as_cint16( ( 26510u >> (15 - shift_tw_))           | ((19261u >> (15 - shift_tw_)) << 16))); // -(-exp(-2j*pi/5))^2
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 8>  zbuf_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 0, 5, cint32, cint32, cint32> : public fft_dit_common<Vectorization, 0, 5, cint32, cint32, cint32>
{
    using   input_type = cint32;
    using  output_type = cint32;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 5, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 2>   dat0;
        vector<input_type, 2>   dat1;
        vector<input_type, 2>   dat2;
        vector<input_type, 2>   dat3;
        vector<input_type, 2>   dat4;
        vector<twiddle_type, 2> tw1;
        vector<twiddle_type, 2> tw4;
        vector<twiddle_type, 2> tw3;
        vector<twiddle_type, 2> tw2;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr,
                                              const twiddle_type * __restrict ptw1,
                                              const twiddle_type * __restrict ptw2,
                                              const twiddle_type * __restrict ptw3,
                                              const twiddle_type * __restrict ptw4,
                                              unsigned r) :
            ptr_begin0_((const v2cint32 *)(ptr)),
            ptr_begin1_((const v2cint32 *)(ptr + r)),
            ptr_begin2_((const v2cint32 *)(ptr + r * 2)),
            ptr_begin3_((const v2cint32 *)(ptr + r * 3)),
            ptr_begin4_((const v2cint32 *)(ptr + r * 4)),
            ptr_end0_((const v2cint32 *)(ptr + r)),
            ptr_end1_((const v2cint32 *)(ptr + r * 2)),
            ptr_end2_((const v2cint32 *)(ptr + r * 3)),
            ptr_end3_((const v2cint32 *)(ptr + r * 4)),
            ptr_end4_((const v2cint32 *)(ptr + r * 5)),
            ptw2_((const v2cint32 *)ptw2),
            r_(r),
            cnt_(0),
            zoffs_(0x00000000),
            td1_((const v2cint32 *)ptw1 - (const v2cint32 *)ptw2),
            td4_((const v2cint32 *)ptw4 - (const v2cint32 *)ptw2),
            td3_((const v2cint32 *)ptw3 - (const v2cint32 *)ptw2)
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
            ptr_begin0_   = ::fft_data_incr(ptr_begin0_, r_/2,ptr_end0_, cnt_);
            ptr_begin1_   = ::fft_data_incr(ptr_begin1_, r_/2,ptr_end1_     );
            ptr_begin2_   = ::fft_data_incr(ptr_begin2_, r_/2,ptr_end2_     ); 
            ptr_begin3_   = ::fft_data_incr(ptr_begin3_, r_/2,ptr_end3_     ); 
            ptr_begin4_   = ::fft_data_incr(ptr_begin4_, r_/2,ptr_end4_     ); 

            ptw2_  = ::fft_twiddle_incr(ptw2_,cnt_,1);

            zoffs_ = ::fft_zoffs(cnt_,3,0,0) & 0x11;

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
            return { *ptr_begin0_, *ptr_begin1_, *ptr_begin2_, *ptr_begin3_, *ptr_begin4_,
                     *(ptw2_+td1_), *(ptw2_ + td4_), *(ptw2_ + td3_), *ptw2_, zoffs_};
        }

    private:
        const v2cint32 * __restrict ptr_begin0_;
        const v2cint32 * __restrict ptr_begin1_;
        const v2cint32 * __restrict ptr_begin2_;
        const v2cint32 * __restrict ptr_begin3_;
        const v2cint32 * __restrict ptr_begin4_;
        const v2cint32 *            ptr_end0_;
        const v2cint32 *            ptr_end1_;
        const v2cint32 *            ptr_end2_;
        const v2cint32 *            ptr_end3_;
        const v2cint32 *            ptr_end4_;
        const v2cint32 * __restrict ptw2_;
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

        vector<twiddle_type, 8> zbuf;
        vector<input_type, 8> ybuf, ubuf;
        ybuf.insert(0, data.dat1);
        ybuf.insert(1, data.dat2);
        ybuf.insert(2, data.dat3);
        ybuf.insert(3, data.dat4);
        zbuf.insert(0, data.tw1);
        zbuf.insert(1, data.tw2);
        zbuf.insert(2, data.tw3);
        zbuf.insert(3, data.tw4);

        accum<cacc80, 2> a = ::lups(data.dat0, shift_tw_);

        accum<cacc80, 2> o0, o1, o2, o3, o4;
        accum<cacc80, 2> d, e, f, g;

        d = ::lmul2(ybuf, 0, 0x10, false, zbuf.template extract<4>(0), 0, data.zoffs, inv_);
        e = ::lmul2(ybuf, 2, 0x10, false, zbuf.template extract<4>(0), 2, data.zoffs, inv_);
        f = ::lmul2(ybuf, 4, 0x10, false, zbuf.template extract<4>(1), 0, data.zoffs, inv_);
        g = ::lmul2(ybuf, 6, 0x10, false, zbuf.template extract<4>(1), 2, data.zoffs, inv_);

        vector<input_type, 8> dv, ev, fv, gv;
        dv.insert(0, d.to_vector<input_type>(shift_tw_));
        ev.insert(0, e.to_vector<input_type>(shift_tw_));
        fv.insert(0, f.to_vector<input_type>(shift_tw_));
        gv.insert(0, g.to_vector<input_type>(shift_tw_));

        o0 = ::lmac2(a,  dv, 0, 0x10, false, one_, 0, 0x00, false);
        o0 = ::lmac2(o0, ev, 0, 0x10, false, one_, 0, 0x00, false);
        o0 = ::lmac2(o0, fv, 0, 0x10, false, one_, 0, 0x00, false);
        o0 = ::lmac2(o0, gv, 0, 0x10, false, one_, 0, 0x00, false);

        o1 = ::lmac2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);
        o1 = ::lmac2(o1, ev, 0, 0x10, false, qbuf_, 0, 0x11,  inv_);
        o1 = ::lmac2(o1, fv, 0, 0x10, false, qbuf_, 0, 0x11, !inv_);
        o1 = ::lmac2(o1, gv, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);

        o2 = ::lmac2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x11,  inv_);
        o2 = ::lmac2(o2, ev, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);
        o2 = ::lmac2(o2, fv, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);
        o2 = ::lmac2(o2, gv, 0, 0x10, false, qbuf_, 0, 0x11, !inv_);

        o3 = ::lmac2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x11, !inv_);
        o3 = ::lmac2(o3, ev, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);
        o3 = ::lmac2(o3, fv, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);
        o3 = ::lmac2(o3, gv, 0, 0x10, false, qbuf_, 0, 0x11,  inv_);

        o4 = ::lmac2(a,  dv, 0, 0x10, false, qbuf_, 0, 0x00, !inv_);
        o4 = ::lmac2(o4, ev, 0, 0x10, false, qbuf_, 0, 0x11, !inv_);
        o4 = ::lmac2(o4, fv, 0, 0x10, false, qbuf_, 0, 0x11,  inv_);
        o4 = ::lmac2(o4, gv, 0, 0x10, false, qbuf_, 0, 0x00,  inv_);

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
        qbuf_[0] = twiddle_type( ( 663608942u >> (31 - shift_tw)), -(2042378317u >> (31 - shift_tw))); // -exp(-2j*pi/5)
        qbuf_[1] = twiddle_type(-(1737350766u >> (31 - shift_tw)), -(1262259217u >> (31 - shift_tw))); // -(-exp(-2j*pi/5))^2
        one_[0]  = twiddle_type((1u << shift_tw) - 1, 0);
    }

private:
    unsigned shift_tw_, shift_;
    bool inv_;
    vector<twiddle_type, 4>  qbuf_;
    vector<twiddle_type, 4>  one_;
};

}

#endif
