// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX2_HPP__
#define __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX2_HPP__

#include "../array_helpers.hpp"
#include "../broadcast.hpp"

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

        v8cacc64 o0, o1;

        v8cint16  a;
        v16cint16 w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        a = data.lo;
        v16cint16 c = ::concat(data.hi, c_);

        w = broadcast<cint16, 16>::run(data.tw);

        o0 = ::mac_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        if      constexpr (std::is_same_v<Output, cint16>)
        {
            ret[0] = ::ssrs(o0, shift_);
            ret[1] = ::ssrs(o1, shift_);
        }
        else if constexpr (std::is_same_v<Output, cint32>)
        {
            ret[0] = ::lsrs(o0, shift_);
            ret[1] = ::lsrs(o1, shift_);
        }

        return ret;
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
inline v16cint16 broadcast_2c16_T32_4x4 (cint16 a, cint16 b) {
    return ::shuffle(broadcast_2c16(a, b), ::undef_v16cint16(), T32_4x4);
}
#if __AIE_API_CBF16_SUPPORT__
inline v16cbfloat16 broadcast_2c16 (cbfloat16 a, cbfloat16 b) {
    return (v16cbfloat16)broadcast_2c16(__builtin_bit_cast(cint16, a),
                                        __builtin_bit_cast(cint16, b));
}
inline v16cbfloat16 broadcast_2c16_T32_4x4 (cbfloat16 a, cbfloat16 b) {
    return (v16cbfloat16)broadcast_2c16_T32_4x4(__builtin_bit_cast(cint16, a),
                                                __builtin_bit_cast(cint16, b));
}
#endif

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
        v8cint16 a;
        v16cint16 w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        v16cint16 d0 = ::set_v16cint16(0, data.lo);
        v16cint16 d1 = ::set_v16cint16(0, data.hi);

        v16cint16 c = ::shuffle(d0, d1, T128_2x4_lo);
        a = ::extract_v8cint16(c, 0);

        w = broadcast_2c16_T32_4x4(data.tw1 , data.tw2);
        w = ::insert(w, 0, ::ssrs(zeros_, 0));       zeros_ = chess_copy(zeros_);

        o0 = ::mac_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
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

        v8cint16 a;
        v16cint16 w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        v16cint16 d0 = ::set_v16cint16(0, data.lo);
        d0           = ::insert(d0, 1, data.hi);

        v16cint16 c = ::shuffle(d0, zeros<cint16, 16>::run(), T64_8x2_hi);
        a = ::extract_v8cint16(::shuffle(d0, zeros<cint16, 16>::run(), T64_8x2_lo), 0);

        w = ::set_v16cint16(0, ::set_v8cint16(0, data.tw));
        w = ::shuffle(w, w, T32_2x16_lo);

        o0 = ::mac_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
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

        v8cint16  a;
        v16cint16 w;

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        v16cint16 d0 = ::set_v16cint16(0, data.lo);
        d0           = ::insert(d0, 1, data.hi);

        v16cint16 c = ::shuffle(d0, zeros<cint16, 16>::run(), T32_16x2_hi);
        a = ::extract_v8cint16(::shuffle(d0, zeros<cint16, 16>::run(), T32_16x2_lo), 0);

        w = ::set_v16cint16(0, data.tw);

        o0 = ::mac_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};


template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 2, cint32, Output, cint16> : public fft_dit_common<Vectorization, 0, 2, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> dat0;
        vector<input_type, 4> dat1;
        vector<input_type, 4> dat2;
        vector<input_type, 4> dat3;
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
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw_((const cint16 *)ptw),
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
            return { *ptr_begin_,  *(ptr_begin_ + 1), *(ptr_begin_+ 2 * r_ / 8), *(ptr_begin_+ (2 * r_ / 8) + 1), *ptw_ };
        }

    private:
        const v4cint32 * ptr_begin_;
        const cint16   * ptw_;
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
        v8cint32  a, c;
        v16cint16 w;

        a = ::set_v8cint32(0, data.dat0);
        a = ::insert(a,    1, data.dat1);

        c = ::set_v8cint32(0, data.dat2);
        c = ::insert(c,    1, data.dat3);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        w = (v16cint16) broadcast<cint16, 16>::run(data.tw);

        o0 = ::mac_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 1, 2, cint32, Output, cint16> : public fft_dit_common<Vectorization, 1, 2, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> dat0;
        vector<input_type, 4> dat1;
        vector<input_type, 4> dat2;
        vector<input_type, 4> dat3;
        twiddle_type tw0;
        twiddle_type tw1;
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
            ptw_((const cint16 *)ptw)
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
            return { *ptr_begin_,  *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw_, *(ptw_ + 1) };
        }

    private:
        const v4cint32 * ptr_begin_;
        const cint16 *  ptw_;
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

        v8cint32  a, c;
        v16cint16 w;
        a = ::set_v8cint32(0, data.dat0);
        c = ::set_v8cint32(0, data.dat1);

        a = ::insert(a,    1, data.dat2);
        c = ::insert(c,    1, data.dat3);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        w = broadcast_2c16_T32_4x4(data.tw0, data.tw1);

        o0 = ::mac_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 2, cint32, Output, cint16> : public fft_dit_common<Vectorization, 2, 2, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 2, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        vector<input_type, 4> dat0;
        vector<input_type, 4> dat1;
        vector<input_type, 4> dat2;
        vector<input_type, 4> dat3;
        v4cint16 tw;
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
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw_((const v4cint16 *)ptw)
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
            return { *ptr_begin_,  *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw_ };
        }

    private:
        const v4cint32 * __restrict ptr_begin_;
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

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<cacc64, 8> o0, o1;

        v8cint32  a, c;
        v16cint16 w;

        v8cint32 d0 = ::set_v8cint32(0, data.dat0);
        d0 = ::insert(d0, 1, data.dat1);

        v8cint32 d1 = ::set_v8cint32(0, data.dat2);
        d1 = ::insert(d1, 1, data.dat3);

        c = ::shuffle(d0, d1, T128_4x2_hi);
        a = ::shuffle(d0, d1, T128_4x2_lo);

        w = ::set_v16cint16(0, ::set_v8cint16(0, data.tw));
        w = ::shuffle(w, w, T32_2x16_lo);

        o0 = ::mac_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

//TODO: Can probably refactor with specialization above for radix 2
template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 3, 2, cint32, Output, cint16> : public fft_dit_common<Vectorization, 3, 2, cint32, Output, cint16>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = cint16;
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
            ptr_begin_((const v4cint32 *)(ptr)),
            ptw_((const v8cint16 *)ptw)
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
            return { *ptr_begin_,  *(ptr_begin_ + 1), *(ptr_begin_ + 2), *(ptr_begin_ + 3), *ptw_ };
        }

    private:
        const v4cint32 * __restrict ptr_begin_;
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

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);

        accum<cacc64, 8> o0, o1;

        v8cint32  a, c;
        v16cint16 w;

        v8cint32 d0 = ::set_v8cint32(0, data.dat0);
        d0 = ::insert(d0, 1, data.dat1);

        v8cint32 d1 = ::set_v8cint32(0, data.dat2);
        d1 = ::insert(d1, 1, data.dat3);

        c = ::shuffle(d0, d1, T64_8x2_hi);
        a = ::shuffle(d0, d1, T64_8x2_lo);

        w = ::set_v16cint16(0, data.tw);

        o0 = ::mac_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_conf(c, w, ::lups(a, shift_tw_), 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>(shift_);
        ret[1] = o1.to_vector<output_type>(shift_);

        return ret;
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
            return { *ptr_begin_,  *(ptr_begin_+ r_ / 8), *ptw_ };
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
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();

        return ret;
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

        vector<twiddle_type, 16> w = broadcast_2c16_T32_4x4(data.tw1, data.tw2);
        w.insert(0, zero_);

        int cmplx_mask = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();

        return ret;
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
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();

        return ret;
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
        o0 = ::mac_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);
        o1 = ::msc_elem_8_2_conf(c, w, a, 0, 0, cmplx_mask, 0, 0);

        ret[0] = o0.to_vector<output_type>();
        ret[1] = o1.to_vector<output_type>();

        return ret;
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

}

#endif
