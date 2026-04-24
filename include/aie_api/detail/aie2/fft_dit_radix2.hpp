// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX2_HPP__
#define __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX2_HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../array_helpers.hpp"
#include "../broadcast.hpp"
#include "../transpose.hpp"

namespace aie::detail {

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 2, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 2, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw;
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
            ptr_begin_((const v8cint16 __aie_dm_resource_a *)(ptr)),
            ptw_((const cint16 *)ptw),
            r_(r),
            cnt_(0),
            cnt_tw_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 1+r_ /8, r_ /8-1, cnt_, 1);
            ptw_        = ::add_2d_ptr(ptw_, 1, r_ /8-1, cnt_tw_, 0);

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
            return { *ptr_begin_, *(ptr_begin_+ r_ / 8), *ptw_ };
        }

    private:
        const v8cint16 __aie_dm_resource_a * __restrict ptr_begin_; //TODO: this DM_bankA annotation seems to be ignored
        const cint16 * __restrict ptw_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw, Vectorization);
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

        accum<cacc64, 8> o0, o1;

        vector<cint16, 8>  a;
        vector<cint16, 16> w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        a = data.lo;
        v16cint16 c = ::concat(data.hi, c_);

        w = broadcast<cint16, 16>::run(data.tw);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);

        o0 = ::mac_elem_8_2_conf(c, w, o0, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, o1, 0, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline
    fft_dit()
        : c_(zeros<cint16, 8>::run())
    {}

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          inv_(inv),
          c_(zeros<cint16, 8>::run())
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    v8cint16 c_;
};

inline v16cint16 broadcast_2c16 (cint16 a, cint16 b) {
    return (v16cint16) ::broadcast_to_v16int32((unsigned long long)(as_uint32(a) | ((uint64_t)::as_uint32(b)<<32)));
}
template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 1, 2, cint16, Output, cint16> : public fft_dit_common<Vectorization, 1, 2, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw1;
        twiddle_type tw2;
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
            ptr_begin_((const v8cint16 *)(ptr)),
            ptw_((const cint16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  += 2;
            ptw_        += 2;
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
            return { *ptr_begin_, *(ptr_begin_+ 1), *ptw_, *(ptw_ + 1)};
        }

    private:
        const v8cint16 * __restrict ptr_begin_;
        const cint16 * __restrict ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<cacc64, 8> o0, o1;
        vector<cint16, 8> a;
        vector<cint16, 16> w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        vector<cint16, 16> d0 = data.lo.template grow<16>();
        vector<cint16, 16> d1 = data.hi.template grow<16>();

        v16cint16 c = ::shuffle(d0, d1, T128_2x4_lo);
        a = ::extract_v8cint16(c, 0);

        w = transpose<twiddle_type, 16>::run(broadcast_2c16(data.tw1 , data.tw2), 4, 4);

        w = ::insert(w, 0, ::ssrs(zeros_, 0));       zeros_ = chess_copy(zeros_);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);
        o0 = ::mac_elem_8_2_conf(c, w, o0, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, o1, 0, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline
    fft_dit()
        : zeros_(zeros_acc<AccumClass::CInt, 64, 8>::run())
    {}

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          inv_(inv),
          zeros_(zeros_acc<AccumClass::CInt, 64, 8>::run())
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    v8cacc64 zeros_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 2, cint16, Output, cint16> : public fft_dit_common<Vectorization, 2, 2, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        v4cint16 tw; //TODO: can we use API vector directly? giving an error currently, otherwise we could probably compine stage 2 and stage 3?
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
            ptr_begin_((const v8cint16 *)(ptr)),
            ptw_((const v4cint16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  += 2;
            ptw_++;
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
            return { *ptr_begin_, *(ptr_begin_+ 1), *ptw_ };
        }

    private:
        const v8cint16 * __restrict ptr_begin_;
        const v4cint16 * __restrict ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw);
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
        accum<cacc64, 8> o0, o1;

        vector<cint16, 8> a;
        vector<cint16,16> w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        vector<cint16,16> d0 = ::set_v16cint16(0, data.lo);
        d0           = ::insert(d0, 1, data.hi);

        vector<cint16, 16> c = ::shuffle(d0, zeros<cint16, 16>::run(), T64_8x2_hi);
        a = ::extract_v8cint16(::shuffle(d0, zeros<cint16, 16>::run(), T64_8x2_lo), 0);

        w = ::set_v16cint16(0, ::set_v8cint16(0, data.tw));
#if AIE_API_NATIVE
        // Initialize upper lanes to prevent tool warnings
        w = ::insert(w, 1, ::extract_v8cint16(::broadcast_zero_to_v16cint16(), 0));
#endif
        w = ::shuffle(w, w, T32_2x16_lo);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);
        o0 = ::mac_elem_8_2_conf(c, w, o0, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, o1, 0, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 3, 2, cint16, Output, cint16> : public fft_dit_common<Vectorization, 3, 2, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_begin_((const v8cint16 *)(ptr)),
            ptw_((const v8cint16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  += 2;;
            ptw_++;

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
            return { *ptr_begin_, *(ptr_begin_+ 1), *ptw_};
        }

    private:
        const v8cint16 * __restrict ptr_begin_;
        const v8cint16 * __restrict ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<cacc64, 8> o0, o1;

        vector<cint16, 8>  a;
        vector<cint16, 16> w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        vector<cint16, 16> d0;
        d0 = ::set_v16cint16(0, data.lo);
        d0 = ::insert(d0, 1, data.hi);

        vector<cint16, 16> c;
        c = ::shuffle(d0, zeros<cint16, 16>::run(), T32_16x2_hi);
        a = ::extract_v8cint16(::shuffle(d0, zeros<cint16, 16>::run(), T32_16x2_lo), 0);

        w = ::set_v16cint16(0, data.tw);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);
        o0 = ::mac_elem_8_2_conf(c, w, o0, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, o1, 0, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 2, cint16, Output, cint32> : public fft_dit_common<Vectorization, 0, 2, cint16, Output, cint32>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw;
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
            ptr_begin_((const v8cint16 __aie_dm_resource_a *)(ptr)),
            ptw_((const cint32 *)ptw),
            r_(r),
            cnt_(0),
            cnt_tw_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 1+r_ /8, r_ /8-1, cnt_, 1);
            ptw_        = ::add_2d_ptr(ptw_, 1, r_ /8-1, cnt_tw_, 0);

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
            return { *ptr_begin_, *(ptr_begin_+ r_ / 8), *ptw_ };
        }

    private:
        const v8cint16 __aie_dm_resource_a * __restrict ptr_begin_; //TODO: this DM_bankA annotation seems to be ignored
        const cint32 * __restrict ptw_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw, Vectorization);
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

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_X : OP_TERM_NEG_COMPLEX);

        accum<cacc64, 8> o0, o1;

        vector c = vector<input_type, 8>(data.hi).grow<16>();
        vector w = broadcast<twiddle_type, 8>::run(data.tw);

        o0 = o1 = accum<cacc64, 8>(data.lo, shift_tw_);

        o0 = ::mac_elem_8_conf(w, c, o0, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_conf(w, c, o1, 0, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};


template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 0, 2, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 0, 2, cint32, Output, Twiddle>
{
    static_assert(utils::is_one_of_v<Twiddle, cint16, cint32>, "Unsupported Twiddle parameter type");
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> dat0;
        vector<input_type, 4> dat1;
        vector<input_type, 4> dat2;
        vector<input_type, 4> dat3;
        const twiddle_type   *tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw, unsigned r) :
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw_(ptw),
            r_(r),
            cnt_(0),
            cnt_tw_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, 2 * r_ / 8 + 2, r_ / 8 - 1, cnt_, 2);
            ptw_        = ::add_2d_ptr(ptw_, 1, r_ / 8 - 1, cnt_tw_, 0);

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
            auto *p = chess_dont_chain(ptr_begin_);
            return { *ptr_begin_, p[1], p[2 * r_ / 8], p[(2 * r_ / 8) + 1], ptw_ };
        }

    private:
        const v4cint32 * ptr_begin_;
        const twiddle_type * ptw_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw, Vectorization);
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

        accum<cacc64, 8> o0, o1;
        vector<cint32, 8>  a, c;
        vector<twiddle_type, 8> w;

        const twiddle_type *__restrict tw = data.tw;

        // a = concat(data.dat0, data.dat1),
        c = concat(data.dat2, data.dat3);
        w = broadcast<twiddle_type, 8>::run(tw[0]);

        const int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0;

        // Perform upshift conversion to accumulator using load unit granularity
        // This ensures proper bundling to VLD.UPS instructions
        o0 = o1 = concat(accum<cacc64, 4>(data.dat0, shift_tw_),
                         accum<cacc64, 4>(data.dat1, shift_tw_));

        if constexpr (std::is_same_v<twiddle_type, cint16>) {
            o0 = ::mac_elem_8_conf(c, w.template grow<16>(), o0, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
            o1 = ::msc_elem_8_conf(c, w.template grow<16>(), o1, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
        }
        else {
            // Note: Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug
            // See CRVO-11861
            o0 = ::mac_elem_8_conf(c, w,                     o0 , zero_acc,          cmplx_mask, sub_mul, sub_acc);
            o1 = ::mac_elem_8_conf(c, w,                     o1 , zero_acc,          cmplx_mask,       1, sub_acc);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 1, 2, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 1, 2, cint32, Output, Twiddle>
{
    static_assert(utils::is_one_of_v<Twiddle, cint16, cint32>);
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> dat0;
        vector<input_type, 4> dat1;
        vector<input_type, 4> dat2;
        vector<input_type, 4> dat3;
        const twiddle_type   *tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw_(ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 4;
            ptw_       += 2;
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
            auto *p = chess_dont_chain(ptr_begin_);
            return { *ptr_begin_, p[1], p[2], p[3], ptw_ };
        }

    private:
        const v4cint32 * ptr_begin_;
        const twiddle_type *  ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<cacc64, 8> o0, o1;

        vector<cint32, 8>  a, c;

        // a = concat(data.dat0, data.dat2);
        c = concat(data.dat1, data.dat3);

        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0;
        const int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        // concat(accum, accum) instead of accum(concat(vector, vector))
        // Latter does not take advantage of VLD.UPS and instead performs VLD and then VUPS of the two components
        o0 = o1 = concat(accum<cacc64, 4>(data.dat0, shift_tw_),
                         accum<cacc64, 4>(data.dat2, shift_tw_));

        // Create a vector with twiddles repeated in the following pattern: [tw0] * 4, [tw1] * 4
        if constexpr (std::is_same_v<twiddle_type, cint16>) {
            vector<twiddle_type, 16> w = transpose<twiddle_type, 16>::run(broadcast<twiddle_type, 16>::run({data.tw[0], data.tw[1]}),
                                                                          4, 4);
            o0 = ::mac_elem_8_conf(c, w, o0, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
            o1 = ::msc_elem_8_conf(c, w, o1, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
        }
        else { // cint32
            vector<twiddle_type, 8> w;
            std::tie(w, std::ignore) = interleave_zip<twiddle_type, 8>::run(broadcast<twiddle_type, 8>::run(data.tw[0]),
                                                                            broadcast<twiddle_type, 8>::run(data.tw[1]),
                                                                            4);

            // Note: Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug
            // See CRVO-11861
            o0 = ::mac_elem_8_conf(c, w, o0, zero_acc, cmplx_mask, sub_mul, sub_acc);
            o1 = ::mac_elem_8_conf(c, w, o1, zero_acc, cmplx_mask,       1, sub_acc);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 2, 2, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 2, 2, cint32, Output, Twiddle>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
        vector<input_type, 4>   dat3;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_begin_(reinterpret_cast<data_ptr_t>(ptr)),
            ptw_(reinterpret_cast<twiddle_ptr_t>(ptw))
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 4;
            ptw_++;
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
            auto *p = chess_dont_chain(ptr_begin_);
            return { *ptr_begin_, p[1], p[2], p[3], ptw_[0] };
        }

    private:
        using data_ptr_t    = const typename decltype(input_data::dat0)::storage_t *;
        using twiddle_ptr_t = const typename decltype(input_data::tw)::storage_t *;

        data_ptr_t    __restrict ptr_begin_;
        twiddle_ptr_t __restrict ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw);
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

        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0;
        const int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<cacc64, 8> o0, o1;
        vector<cint32, 8>  a, c;

        vector<cint32, 8> d0 = concat(data.dat0, data.dat1);
        vector<cint32, 8> d1 = concat(data.dat2, data.dat3);

        c = ::shuffle(d0, d1, T128_4x2_hi);
        a = ::shuffle(d0, d1, T128_4x2_lo);

        // Replicate each twiddle vector element:
        // [a, b, c, d] -> [a, a, b, b, c, c, d, d]
        vector<twiddle_type, 8> w = data.tw.template grow<8>();
        std::tie(w, std::ignore) = interleave_zip<twiddle_type, 8>::run(w, w, 1);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);

        if constexpr (std::is_same_v<twiddle_type, cint16>) {
            vector w16 = w.template grow<16>();
            o0 = ::mac_elem_8_conf(c, w16, o0, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
            o1 = ::msc_elem_8_conf(c, w16, o1, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
        }
        else {
            // Note: Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug
            // See CRVO-11861
            o0 = ::mac_elem_8_conf(c, w, o0, zero_acc, cmplx_mask, sub_mul, sub_acc);
            o1 = ::mac_elem_8_conf(c, w, o1, zero_acc, cmplx_mask,       1, sub_acc);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

//TODO: Can probably refactor with specialization above for radix 2
template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 3, 2, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 3, 2, cint32, Output, Twiddle>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 3, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4>   dat0;
        vector<input_type, 4>   dat1;
        vector<input_type, 4>   dat2;
        vector<input_type, 4>   dat3;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw) :
            ptr_begin_(reinterpret_cast<const data_ptr_t>(ptr)),
            ptw_(reinterpret_cast<const twiddle_ptr_t>(ptw))
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 4;
            ptw_++;
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
            auto *p = chess_dont_chain(ptr_begin_);
            return { *ptr_begin_, p[1], p[2], p[3], ptw_[0] };
        }

    private:
        using data_ptr_t    = const typename decltype(input_data::dat0)::storage_t *;
        using twiddle_ptr_t = const typename decltype(input_data::tw)::storage_t *;

        data_ptr_t    __restrict ptr_begin_;
        twiddle_ptr_t __restrict ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw)
    {
        return stage_iterator(data, ptw);
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

        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0;
        const int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<cacc64, 8> o0, o1;

        vector<cint32, 8>  a, c;

        vector<cint32, 8> d0 = concat(data.dat0, data.dat1);
        vector<cint32, 8> d1 = concat(data.dat2, data.dat3);

        c = ::shuffle(d0, d1, T64_8x2_hi);
        a = ::shuffle(d0, d1, T64_8x2_lo);

        o0 = o1 = accum<cacc64, 8>(a, shift_tw_);

        if constexpr (std::is_same_v<twiddle_type, cint16>) {
            vector<twiddle_type, 16> w = data.tw.template grow<16>();

            o0 = ::mac_elem_8_conf(c, w, o0, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
            o1 = ::msc_elem_8_conf(c, w, o1, zero_acc, shift16, cmplx_mask, sub_mul, sub_acc);
        }
        else { // cint32
            vector<twiddle_type, 8> w = data.tw;

            // Note: Use mac with sub_mul=1 instead of msc due to cint32 x cint32 emulation bug
            // See CRVO-11861
            o0 = ::mac_elem_8_conf(c, w, o0, zero_acc, cmplx_mask, sub_mul, sub_acc);
            o1 = ::mac_elem_8_conf(c, w, o1, zero_acc, cmplx_mask,       1, sub_acc);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};


#if __AIE_API_CBF16_SUPPORT__

template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 2, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 0, 2, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw, unsigned r) :
            ptr_begin_((const v8cbfloat16 *)(ptr)),
            ptw_((const cbfloat16 *)ptw),
            r_(r),
            cnt_(0),
            cnt_tw_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, r_ / 8 + 1, r_ / 8 - 1, cnt_, 1);
            ptw_        = ::add_2d_ptr(ptw_, 1, r_ / 8 - 1, cnt_tw_, 0);

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
            auto *p = chess_dont_chain(ptr_begin_);
            return { *ptr_begin_,  p[ r_ / 8], ptw_[0] };
        }

    private:
        const v8cbfloat16 * ptr_begin_;
        const cbfloat16   * ptw_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw, Vectorization);
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

        accum<caccfloat, 8> o0, o1;
        accum<caccfloat, 8> a(data.lo);
        vector<input_type, 16> c;

        c.insert(0, data.hi);
        c.insert(1, zero_);

        vector w = broadcast<twiddle_type, 16>::run(data.tw);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
#if __AIE_API_CBF16_NO_SHIFT16__
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
#else
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit()
        : zero_(zeros<cbfloat16, 8>::run())
    {}

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv),
          zero_(zeros<cbfloat16, 8>::run())
    {}
private:
    bool inv_;
    v8cbfloat16 zero_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 1, 2, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 1, 2, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw1;
        twiddle_type tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cbfloat16 *)(ptr)),
            ptw_((const cbfloat16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_       += 2;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_, *(ptw_ + 1) };
        }

    private:
        const v8cbfloat16 * ptr_begin_;
        const cbfloat16   * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<caccfloat, 8> o0, o1;

        vector<input_type, 16> c = ::shuffle(data.lo.template grow<16>(),
                                             data.hi.template grow<16>(),
                                             T128_2x4_lo);

        accum<caccfloat, 8> a(c.template extract<8>(0));

        vector<twiddle_type, 16> w = transpose<twiddle_type, 16>::run(broadcast<twiddle_type, 16>::run({data.tw1, data.tw2}), 4, 4);
        w.insert(0, zero_);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
#if __AIE_API_CBF16_NO_SHIFT16__
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
#else
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit()
        : zero_(zeros<cbfloat16, 8>::run())
    {}

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv),
          zero_(zeros<cbfloat16, 8>::run())
    {}
private:
    bool inv_;
    v8cbfloat16 zero_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 2, 2, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 2, 2, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cbfloat16 *)(ptr)),
            ptw_((const v4cbfloat16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_++;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_ };
        }

    private:
        const v8cbfloat16 * ptr_begin_;
        const v4cbfloat16 * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<caccfloat, 8> o0, o1;

        vector<input_type, 16> d0;
        d0.insert(0, data.lo);
        d0.insert(1, data.hi);

        vector<input_type, 16> c = ::shuffle(d0, zeros<input_type, 16>::run(), T64_8x2_hi);

        vector<input_type, 8> a_tmp = ::extract_v8cbfloat16(::shuffle(d0, zeros<input_type, 16>::run(), T64_8x2_lo), 0);
        accum<caccfloat, 8> a(a_tmp);

        vector w = data.tw.template grow<16>();
        w = ::shuffle(w, w, T32_2x16_lo);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
#if __AIE_API_CBF16_NO_SHIFT16__
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
#else
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 3, 2, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 3, 2, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 3, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cbfloat16 *)(ptr)),
            ptw_((const v8cbfloat16 *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_++;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_ };
        }

    private:
        const v8cbfloat16 * ptr_begin_;
        const v8cbfloat16 * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        accum<caccfloat, 8> o0, o1;

        vector<input_type, 16> d0;
        d0.insert(0, data.lo);
        d0.insert(1, data.hi);

        vector<input_type, 16> c = ::shuffle(d0, zeros<input_type, 16>::run(), T32_16x2_hi);

        vector<input_type, 8> a_tmp = ::extract_v8cbfloat16(::shuffle(d0, zeros<input_type, 16>::run(), T32_16x2_lo), 0);
        accum<caccfloat, 8> a(a_tmp);

        vector w = data.tw.template grow<16>();

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
#if __AIE_API_CBF16_NO_SHIFT16__
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, cmplx_mask, 0, 0);
#else
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};
#endif // __AIE_API_CBF16_SUPPORT__

#if __AIE_API_COMPLEX_FP32_EMULATION__ && __AIE_API_COMPLEX_FP32_CONF__
template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw, unsigned r) :
            ptr_begin_((const v8cfloat *)(ptr)),
            ptw_((const cfloat *)ptw),
            r_(r),
            cnt_(0),
            cnt_tw_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_  = ::add_2d_ptr(ptr_begin_, r_ / 8 + 1, r_ / 8 - 1, cnt_, 1);
            ptw_        = ::add_2d_ptr(ptw_, 1, r_ / 8 - 1, cnt_tw_, 0);

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
            return { *ptr_begin_,  *(ptr_begin_+ r_ / 8), *ptw_ };
        }

    private:
        const v8cfloat * ptr_begin_;
        const cfloat   * ptw_;
        unsigned r_;
        addr_t cnt_;
        addr_t cnt_tw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw, Vectorization);
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

        accum<caccfloat, 8> a(data.lo);

        vector w = broadcast<twiddle_type, 8>::run(data.tw);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<caccfloat, 8> o0 = ::mac_elem_8_conf(data.hi, w, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> o1 = ::msc_elem_8_conf(data.hi, w, a, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 1, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 1, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
        twiddle_type tw1;
        twiddle_type tw2;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cfloat *)(ptr)),
            ptw_((const cfloat *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_       += 2;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_, *(ptw_ + 1) };
        }

    private:
        const v8cfloat * ptr_begin_;
        const cfloat   * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        vector<input_type, 8> c0 = ::shuffle(data.lo, data.hi, T256_2x2_lo);
        vector<input_type, 8> c1 = ::shuffle(data.lo, data.hi, T256_2x2_hi);

        accum<caccfloat, 8> a(c0);

        vector<twiddle_type, 8> w = ::shuffle(broadcast<cfloat,8>::run(data.tw1),
                                              broadcast<cfloat,8>::run(data.tw2),
                                              T256_2x2_lo);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<caccfloat, 8> o0 = ::mac_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> o1 = ::msc_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 2, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 2, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cfloat *)(ptr)),
            ptw_((const v4cfloat *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_++;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_ };
        }

    private:
        const v8cfloat * ptr_begin_;
        const v4cfloat * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        vector<input_type, 8> c0 = ::shuffle(data.lo, data.hi, T128_4x2_lo);
        vector<input_type, 8> c1 = ::shuffle(data.lo, data.hi, T128_4x2_hi);

        accum<caccfloat, 8> a(c0);

        vector<twiddle_type, 8> w = ::shuffle(data.tw.template grow<8>(), data.tw.template grow<8>(), T64_2x8_lo);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<caccfloat, 8> o0 = ::mac_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> o1 = ::msc_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 3, 2, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 3, 2, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 3, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 8> lo;
        vector<input_type, 8> hi;
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
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw) :
            ptr_begin_((const v8cfloat *)(ptr)),
            ptw_((const v8cfloat *)ptw)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
            ptr_begin_ += 2;
            ptw_++;

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
            return { *ptr_begin_,  *(ptr_begin_+ 1), *ptw_ };
        }

    private:
        const v8cfloat * ptr_begin_;
        const v8cfloat * ptw_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw)
    {
        return stage_iterator(data, ptw);
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

        vector<input_type, 8> c0 = ::shuffle(data.lo, data.hi, T64_8x2_lo);
        vector<input_type, 8> c1 = ::shuffle(data.lo, data.hi, T64_8x2_hi);

        accum<caccfloat, 8> a(c0);

        vector<twiddle_type, 8> w = data.tw;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<caccfloat, 8> o0 = ::mac_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> o1 = ::msc_elem_8_conf(c1, w, a, 0, cmplx_mask, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1});
    }

    __aie_inline
    fft_dit() = default;

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv)
    {}
private:
    bool inv_;
};

#endif // __AIE_API_COMPLEX_FP32_EMULATION__
}

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif // __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX2_HPP__
