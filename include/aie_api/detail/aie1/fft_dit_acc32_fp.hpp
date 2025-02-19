// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_FFT_DIT_ACC32_FP_HPP__
#define __AIE_API_DETAIL_AIE1_FFT_DIT_ACC32_FP_HPP__

#include "../vector.hpp"

namespace aie::detail {

template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo;
        vector<input_type, 4>   hi;
        vector<twiddle_type, 4> tw;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw, unsigned r) :
            ptr_begin1_ ((const v4cfloat *)(ptr)),
            ptr_begin1s_((const v4cfloat *)(ptr)),
            ptr_begin2_ ((const v4cfloat *)(ptr + r)),
            ptr_end1_ ((const v4cfloat *)(ptr + r)),
            ptr_end1s_((const v4cfloat *)(ptr + r)),
            ptr_end2_ ((const v4cfloat *)(ptr + r * 2)),
            ptw_((const v4cfloat *)ptw),
            r_(r),
            cnt_(0)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_,  r_ / 4, ptr_end1_);
            ptr_begin1s_ = ::fft_data_incr(ptr_begin1s_, r_ / 4, ptr_end1s_, cnt_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_,  r_ / 4, ptr_end2_);
            ptw_         = ::fft_twiddle_incr(ptw_, cnt_, 2);

            return *this;
        }

        __aie_inline
        stage_iterator operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*() const
        {
            const int zoffs1 = (cnt_ & 3) * 0x22222222;
            const int zoffs2 = zoffs1 + 0x11111111;

            return { *ptr_begin1_, *ptr_begin2_, *ptw_, zoffs1, zoffs2 };
        }

    private:
        const v4cfloat * __restrict ptr_begin1_;
        const v4cfloat * __restrict ptr_begin1s_;
        const v4cfloat * __restrict ptr_begin2_;
        const v4cfloat *            ptr_end1_;
        const v4cfloat *            ptr_end1s_;
        const v4cfloat *            ptr_end2_;
        const v4cfloat *ptw_;
        unsigned r_;
        umod_t cnt_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw)
    {
        set_rdx(2);

        return stage_iterator(data, ptw, Vectorization);
    }

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw,
                               unsigned vectorization)
    {
        set_rdx(2);

        return stage_iterator(data, ptw, vectorization);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(0, 0, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;

        v4cfloat o1 = ::fpmac_conf(data.lo, data.hi.template grow<8>(), 0, 0x76543210, data.tw, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        v4cfloat p1 = ::fpmac_conf(data.lo, data.hi.template grow<8>(), 0, 0x76543210, data.tw, 0, data.zoffs1, false, false, fpadd_sub,    0x00,             fpcmp_nrm);
        ret[0]      = ::fpmac_conf(o1,      data.hi.template grow<8>(), 0, 0x67452301, data.tw, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1]      = ::fpmac_conf(p1,      data.hi.template grow<8>(), 0, 0x67452301, data.tw, 0, data.zoffs2, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 1, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 1, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   lo;
        vector<input_type, 4>   hi;
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
            ptr_begin1_((const v2cfloat *)(ptr)),
            ptr_begin2_((const v2cfloat *)(ptr + 2)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<2>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<2>(ptw), "Insufficient alignment");
#endif
        }

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin1_ += 4;
            ptr_begin2_ += 4;
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
            vector<input_type, 4> lo;
            vector<input_type, 4> hi;

            lo.insert<2>(0, ptr_begin1_[0]);
            lo.insert<2>(1, ptr_begin1_[2]);
            hi.insert<2>(0, ptr_begin2_[0]);
            hi.insert<2>(1, ptr_begin2_[2]);

            return { lo, hi, *ptw_ };
        }

    private:
        const v2cfloat * __restrict ptr_begin1_;
        const v2cfloat * __restrict ptr_begin2_;
        const_vector_iterator<twiddle_type, 2> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw)
    {
        set_rdx(4); //--> 4 narrow loads

        return stage_iterator(data, ptw);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        return fft_dit(0, 0, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;

        v4cfloat o = ::fpmac_conf(data.lo, data.hi.template grow<8>(), 0, 0x76543210, data.tw.template grow<4>(), 0, 0x22220000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        v4cfloat p = ::fpmac_conf(data.lo, data.hi.template grow<8>(), 0, 0x76543210, data.tw.template grow<4>(), 0, 0x22220000, false, false, fpadd_sub,    0x00,             fpcmp_nrm);
        ret[0]     = ::fpmac_conf(o,       data.hi.template grow<8>(), 0, 0x67452301, data.tw.template grow<4>(), 0, 0x33331111, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1]     = ::fpmac_conf(p,       data.hi.template grow<8>(), 0, 0x67452301, data.tw.template grow<4>(), 0, 0x33331111, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

template <unsigned Vectorization>
struct fft_dit<Vectorization, 2, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 2, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
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
            ptr_((const v4cfloat *)(ptr)),
            ptw_(ptw)
        {
#if !AIE_API_DISABLE_ALIGNMENT_ASSERTIONS
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptr), "Insufficient alignment");
            RUNTIME_ASSERT(detail::check_vector_alignment<4>(ptw),  "Insufficient alignment");
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
        const v4cfloat * __restrict ptr_;
        const_vector_iterator<cfloat, 4> ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data,
                               const twiddle_type * ptw) const
    {
        set_rdx(2);

        return stage_iterator(data, ptw);
    }

    __aie_inline
    output_data dit(const input_data &data, unsigned shift_tw, unsigned shift, bool inv) const
    {
        // shift_tw and shift not supported for fp
        return fft_dit(0, 0, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        output_data ret;

        v4cfloat a = ::fpshuffle(::concat(data.lo, data.hi), 0, 0x6420);
        v4cfloat o = ::fpmac_conf(a, ::concat(data.lo, data.hi), 0, 0xFEBA7632, data.tw, 0, 0x66442200, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        v4cfloat p = ::fpmac_conf(a, ::concat(data.lo, data.hi), 0, 0xFEBA7632, data.tw, 0, 0x66442200, false, false, fpadd_sub,    0x00,             fpcmp_nrm);
        ret[0]     = ::fpmac_conf(o, ::concat(data.lo, data.hi), 0, 0xEFAB6723, data.tw, 0, 0x77553311, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1]     = ::fpmac_conf(p, ::concat(data.lo, data.hi), 0, 0xEFAB6723, data.tw, 0, 0x77553311, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 3, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 3, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 3, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
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
            ptr_begin0_ ((const v4cfloat *)(ptr)),
            ptr_begin1_ ((const v4cfloat *)(ptr + r)),
            ptr_begin2_ ((const v4cfloat *)(ptr + r * 2)),
            ptr_end0_ ((const v4cfloat *)(ptr + r)),
            ptr_end1_ ((const v4cfloat *)(ptr + r * 2)),
            ptr_end2_ ((const v4cfloat *)(ptr + r * 3)),
            ptw1_((const v4cfloat *)ptw1),
            ptw2_((const v4cfloat *)ptw2),
            r_(r),
            cnt_(0)
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
            ptr_begin0_  = ::fft_data_incr(ptr_begin0_,  r_ / 4, ptr_end0_);
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_,  r_ / 4, ptr_end1_, cnt_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_,  r_ / 4, ptr_end2_);
            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt_, 2);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt_, 2);

            return *this;
        }

        __aie_inline
        stage_iterator operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*() const
        {
            const int zoffs1 = (cnt_ & 3) * 0x22222222;
            const int zoffs2 = zoffs1 + 0x11111111;

            return { *ptr_begin0_, *ptr_begin1_, *ptr_begin2_, *ptw1_, *ptw2_, zoffs1, zoffs2 };
        }

    private:
        const v4cfloat * __restrict ptr_begin0_;
        const v4cfloat * __restrict ptr_begin1_;
        const v4cfloat * __restrict ptr_begin2_;
        const v4cfloat *            ptr_end0_;
        const v4cfloat *            ptr_end1_;
        const v4cfloat *            ptr_end2_;
        const v4cfloat *ptw1_;
        const v4cfloat *ptw2_;
        unsigned r_;
        umod_t cnt_;
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
        // shift_tw and shift not supported for fp
        return fft_dit(0, 0, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        vector<cfloat, 4> tmp1 = ::fpmul_conf(      data.dat1, 0, 0x76543210, data.tw1, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> tmp2 = ::fpmul_conf(      data.dat2, 0, 0x76543210, data.tw2, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> d    = ::fpmac_conf(tmp1, data.dat1, 0, 0x67452301, data.tw1, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        vector<cfloat, 4> e    = ::fpmac_conf(tmp2, data.dat2, 0, 0x67452301, data.tw2, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);

        output_data ret;

        //                 ____d___          ____e___
        // o0 = i0 + (     tw1 * i1) + (     tw2 * i2)
        ret[0] = ::fpadd(data.dat0, d);
        ret[0] = ::fpadd(ret[0],    e);

        // k2 = conj(k1)
        constexpr int k1_re = 0, k2_re = k1_re;
        constexpr int k1_im = 1, k2_im = k1_im;

        //                 ____d___          ____e___
        // o1 = i0 + (k1 * tw1 * i1) + (k2 * tw2 * i2)
        ret[1] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    d, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    e, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    e, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        //                 ____d___          ____e___
        // o2 = i0 + (k2 * tw1 * i1) + (k1 * tw2 * i2)
        ret[2] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    d, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    e, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    e, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {
        kbuf_[0] = twiddle_type(-0.5f, -0.866025f); // k1 = -0.5 - 0.5j*sqrt(3)
    }
private:
    bool inv_;
    vector<twiddle_type, 2> kbuf_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 5, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 5, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 5, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
        vector<input_type, 4>   dat3;
        vector<input_type, 4>   dat4;
        vector<twiddle_type, 4> tw1;
        vector<twiddle_type, 4> tw2;
        vector<twiddle_type, 4> tw3;
        vector<twiddle_type, 4> tw4;
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
                                              const twiddle_type * __restrict ptw3,
                                              const twiddle_type * __restrict ptw4,
                                              unsigned r) :
            ptr_begin0_ ((const v4cfloat *)(ptr)),
            ptr_begin1_ ((const v4cfloat *)(ptr + r)),
            ptr_begin2_ ((const v4cfloat *)(ptr + r * 2)),
            ptr_begin3_ ((const v4cfloat *)(ptr + r * 3)),
            ptr_begin4_ ((const v4cfloat *)(ptr + r * 4)),
            ptr_end0_ ((const v4cfloat *)(ptr + r)),
            ptr_end1_ ((const v4cfloat *)(ptr + r * 2)),
            ptr_end2_ ((const v4cfloat *)(ptr + r * 3)),
            ptr_end3_ ((const v4cfloat *)(ptr + r * 4)),
            ptr_end4_ ((const v4cfloat *)(ptr + r * 5)),
            ptw1_((const v4cfloat *)ptw1),
            ptw2_((const v4cfloat *)ptw2),
            ptw3_((const v4cfloat *)ptw3),
            ptw4_((const v4cfloat *)ptw4),
            r_(r),
            cnt_(0)
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
            ptr_begin0_  = ::fft_data_incr(ptr_begin0_,  r_ / 4, ptr_end0_);
            ptr_begin1_  = ::fft_data_incr(ptr_begin1_,  r_ / 4, ptr_end1_, cnt_);
            ptr_begin2_  = ::fft_data_incr(ptr_begin2_,  r_ / 4, ptr_end2_);
            ptr_begin3_  = ::fft_data_incr(ptr_begin3_,  r_ / 4, ptr_end3_);
            ptr_begin4_  = ::fft_data_incr(ptr_begin4_,  r_ / 4, ptr_end4_);
            ptw1_        = ::fft_twiddle_incr(ptw1_, cnt_, 2);
            ptw2_        = ::fft_twiddle_incr(ptw2_, cnt_, 2);
            ptw3_        = ::fft_twiddle_incr(ptw3_, cnt_, 2);
            ptw4_        = ::fft_twiddle_incr(ptw4_, cnt_, 2);

            return *this;
        }

        __aie_inline
        stage_iterator operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        __aie_inline
        reference operator*() const
        {
            const int zoffs1 = (cnt_ & 3) * 0x22222222;
            const int zoffs2 = zoffs1 + 0x11111111;

            return { *ptr_begin0_, *ptr_begin1_, *ptr_begin2_, *ptr_begin3_, *ptr_begin4_,
                     *ptw1_, *ptw2_, *ptw3_, *ptw4_, zoffs1, zoffs2 };
        }

    private:
        const v4cfloat * __restrict ptr_begin0_;
        const v4cfloat * __restrict ptr_begin1_;
        const v4cfloat * __restrict ptr_begin2_;
        const v4cfloat * __restrict ptr_begin3_;
        const v4cfloat * __restrict ptr_begin4_;
        const v4cfloat *            ptr_end0_;
        const v4cfloat *            ptr_end1_;
        const v4cfloat *            ptr_end2_;
        const v4cfloat *            ptr_end3_;
        const v4cfloat *            ptr_end4_;
        const v4cfloat *ptw1_;
        const v4cfloat *ptw2_;
        const v4cfloat *ptw3_;
        const v4cfloat *ptw4_;
        unsigned r_;
        umod_t cnt_;
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
        // shift_tw and shift not supported for fp
        return fft_dit(0, 0, inv).dit(data);
    }

    __aie_inline
    output_data dit(const input_data &data) const
    {
        vector<cfloat, 4> tmp1 = ::fpmul_conf(      data.dat1, 0, 0x76543210, data.tw1, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> tmp2 = ::fpmul_conf(      data.dat2, 0, 0x76543210, data.tw2, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> tmp3 = ::fpmul_conf(      data.dat3, 0, 0x76543210, data.tw3, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> tmp4 = ::fpmul_conf(      data.dat4, 0, 0x76543210, data.tw4, 0, data.zoffs1, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        vector<cfloat, 4> d    = ::fpmac_conf(tmp1, data.dat1, 0, 0x67452301, data.tw1, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        vector<cfloat, 4> e    = ::fpmac_conf(tmp2, data.dat2, 0, 0x67452301, data.tw2, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        vector<cfloat, 4> f    = ::fpmac_conf(tmp3, data.dat3, 0, 0x67452301, data.tw3, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        vector<cfloat, 4> g    = ::fpmac_conf(tmp4, data.dat4, 0, 0x67452301, data.tw4, 0, data.zoffs2, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);

        output_data ret;

        //                 ____d___          ____e___          ____f___          ____g___
        // o0 = i0 + (     tw1 * i1) + (     tw2 * i2) + (     tw3 * i3) _ (     tw4 * i4)
        ret[0] = ::fpadd(data.dat0, d);
        ret[0] = ::fpadd(ret[0],    e);
        ret[0] = ::fpadd(ret[0],    f);
        ret[0] = ::fpadd(ret[0],    g);

        // k3 = conj(k2)
        // k4 = conj(k1)
        constexpr int k1_re = 0, k4_re = k1_re;
        constexpr int k1_im = 1, k4_im = k1_im;
        constexpr int k2_re = 2, k3_re = k2_re;
        constexpr int k2_im = 3, k3_im = k2_im;

        //                 ____d___          ____e___          ____f___          ____g___
        // o1 = i0 + (k1 * tw1 * i1) + (k2 * tw2 * i2) + (k3 * tw3 * i3) + (k4 * tw4 * i4)
        ret[1] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    d, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    e, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    e, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    f, 0, 0x76543210, kbuf_.template grow<4>(), k3_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    f, 0, 0x67452301, kbuf_.template grow<4>(), k3_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    g, 0, 0x76543210, kbuf_.template grow<4>(), k4_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[1] = ::fpmac_conf(ret[1],    g, 0, 0x67452301, kbuf_.template grow<4>(), k4_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        //                 ____d___          ____e___          ____f___          ____g___
        // o2 = i0 + (k2 * tw1 * i1) + (k4 * tw2 * i2) + (k1 * tw3 * i3) + (k3 * tw4 * i4)
        ret[2] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    d, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    e, 0, 0x76543210, kbuf_.template grow<4>(), k4_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    e, 0, 0x67452301, kbuf_.template grow<4>(), k4_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    f, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    f, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    g, 0, 0x76543210, kbuf_.template grow<4>(), k3_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[2] = ::fpmac_conf(ret[2],    g, 0, 0x67452301, kbuf_.template grow<4>(), k3_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);

        //                 ____d___          ____e___          ____f___          ____g___
        // o3 = i0 + (k3 * tw1 * i1) + (k1 * tw2 * i2) + (k4 * tw3 * i3) + (k2 * tw4 * i4)
        ret[3] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k3_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    d, 0, 0x67452301, kbuf_.template grow<4>(), k3_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    e, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    e, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    f, 0, 0x76543210, kbuf_.template grow<4>(), k4_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    f, 0, 0x67452301, kbuf_.template grow<4>(), k4_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    g, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[3] = ::fpmac_conf(ret[3],    g, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);

        //                 ____d___          ____e___          ____f___          ____g___
        // o4 = i0 + (k4 * tw1 * i1) + (k3 * tw2 * i2) + (k2 * tw3 * i3) + (k1 * tw4 * i4)
        ret[4] = ::fpmac_conf(data.dat0, d, 0, 0x76543210, kbuf_.template grow<4>(), k4_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    d, 0, 0x67452301, kbuf_.template grow<4>(), k4_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    e, 0, 0x76543210, kbuf_.template grow<4>(), k3_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    e, 0, 0x67452301, kbuf_.template grow<4>(), k3_im, 0x00000000, false, false, fpadd_mixadd, 0xFF * (int)inv_, fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    f, 0, 0x76543210, kbuf_.template grow<4>(), k2_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    f, 0, 0x67452301, kbuf_.template grow<4>(), k2_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    g, 0, 0x76543210, kbuf_.template grow<4>(), k1_re, 0x00000000, false, false, fpadd_add,    0x00,             fpcmp_nrm);
        ret[4] = ::fpmac_conf(ret[4],    g, 0, 0x67452301, kbuf_.template grow<4>(), k1_im, 0x00000000, false, false, fpadd_mixsub, 0xFF * (int)inv_, fpcmp_nrm);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {
        kbuf_[0] = twiddle_type( 0.309017f, -0.951016f); // k1 = exp(-2j*pi/5)
        kbuf_[1] = twiddle_type(-0.809017f, -0.587785f); // k2 = (-exp(-2j*pi/5))^2
    }
private:
    bool inv_;
    vector<twiddle_type, 2> kbuf_;
};

}

#endif
