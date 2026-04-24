// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX4_HPP__
#define __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX4_HPP__

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__

#include "../array_helpers.hpp"
#include "../broadcast.hpp"
#include "../filter.hpp"
#include "../interleave.hpp"

namespace aie::detail {

// Swaps real and imaginary components in a vector of complex values
// z  = a + bi
// z' = b + ai
__aie_inline
inline vector<cint16, 16> flip_complex(vector<cint16, 16> in) {
    return ::shuffle(in, ::undef_v16cint16(), T16_1x2_flip);
}

__aie_inline
inline vector<cint32, 8> flip_complex(vector<cint32, 8> in) {
    auto [real, imag] = interleave_unzip<int32, 16>::run(in.template cast_to<int32>(), {}, 1);
    auto [a, b] = interleave_zip<int32, 16>::run(imag, real, 1);
    return a.template cast_to<cint32>();
}

#if __AIE_API_CBF16_SUPPORT__
__aie_inline
inline vector<cbfloat16, 16> flip_complex(vector<cbfloat16, 16> in) {
    return flip_complex(in.cast_to<cint16>()).cast_to<cbfloat16>();
}
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
__aie_inline
inline vector<cfloat, 8> flip_complex(vector<cfloat, 8> in) {
    return flip_complex(in.cast_to<cint32>()).cast_to<cfloat>();
}
#endif

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 4, cint16, Output, cint16> : public fft_dit_common<Vectorization, 0, 4, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cint16* dat0;
        const v8cint16* dat1;
        unsigned off;
        twiddle_type tw1;
        twiddle_type tw2;
        twiddle_type tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3, unsigned r) :
            ptr_begin0_((const v8cint16  __aie_dm_resource_a *)(ptr)),
            ptr_begin1_((const v8cint16 __aie_dm_resource_a *)(ptr + r)),
            ptw1_((const cint16 *)ptw2),
            ptw2_((const cint16 *)ptw1),
            ptw3_((const cint16 *)ptw3),
            r_(r),
            off_(r/4),
            cnt0_(0),
            cnt1_(0),
            cnt_tw1_(0),
            cnt_tw2_(0),
            cnt_tw3_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin0_ = ::add_2d_ptr(ptr_begin0_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt0_, 1);
           ptr_begin1_ = ::add_2d_ptr(ptr_begin1_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt1_, 1);

           ptw1_ = ::add_2d_ptr(ptw1_, 1, r_ / 8 - 1, cnt_tw1_, 0);
           ptw2_ = ::add_2d_ptr(ptw2_, 1, r_ / 8 - 1, cnt_tw2_, 0);
           ptw3_ = ::add_2d_ptr(ptw3_, 1, r_ / 8 - 1, cnt_tw3_, 0);
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
            return {  ptr_begin0_, ptr_begin1_, off_,
                     *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        const v8cint16 * __restrict ptr_begin0_; //TODO: this will need DM_bankA
        const v8cint16 * __restrict ptr_begin1_; //TODO: this will need DM_bankA
        const cint16 * __restrict ptw1_;
        const cint16 * __restrict ptw2_;
        const cint16 * __restrict ptw3_;
        unsigned r_;
        unsigned off_;
        addr_t cnt0_;
        addr_t cnt1_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
        addr_t cnt_tw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<cacc64, 8> a, g, h, o0, o1, o2, o3;
        vector<cint16, 16> b, c;
        vector<cint16, 16> w1, w2, w3;

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

        a = accum<cacc64, 8>(vector(data.dat0[0]), shift_tw_);

        b = ::set_v16cint16(0, data.dat1[0]);
        b = ::insert(b, 1, data.dat1[data.off]);

        c = ::concat(data.dat0[data.off], c_);

        w1 = broadcast<cint16, 16>::run(data.tw1);
        w2 = broadcast<cint16, 16>::run(data.tw2);
        w3 = broadcast<cint16, 16>::run(data.tw3);

        g = ::mac_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask,    0, 0);
        h = ::msc_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask,    0, 0);

        w1 = ::shuffle(w1, w3, T256_2x2_lo);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
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

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 1, 4, cint16, Output, cint16> : public fft_dit_common<Vectorization, 1, 4, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cint16* dat;
        const twiddle_type* tw1;
        const twiddle_type* tw2;
        const twiddle_type* tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3) :
            ptr_begin_((const v8cint16 __aie_dm_resource_a *)(ptr)),
            ptw1_((const cint16 *)ptw2),
            ptw2_((const cint16 *)ptw1),
            ptw3_((const cint16 *)ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 4;

           ptw1_ += 2;
           ptw2_ += 2;
           ptw3_ += 2;
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
            return { ptr_begin_, ptw1_, ptw2_, ptw3_};
        }

    private:
        const v8cint16 * __restrict ptr_begin_; //TODO: this will need DM_bankA
        const cint16 * __restrict ptw1_;
        const cint16 * __restrict ptw2_;
        const cint16 * __restrict ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        v8cacc64 g, h;
        accum<cacc64, 8> o0, o1, o2, o3;
        vector<cint16, 16> i0, i1;
        vector<cint16, 16> b, c;
        vector<cint16, 16> w1, w2, w3;

        int cmplx_mask    = (this->inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (this->inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

        i0 = ::set_v16cint16(0, data.dat[0]);
        i1 = ::set_v16cint16(0, data.dat[1]);
        i0 = ::insert(i0,    1, data.dat[2]);
        i1 = ::insert(i1,    1, data.dat[3]);

        b = ::shuffle(i1, i0, T128_4x2_hi);
        c = ::shuffle(i1, i0, T128_4x2_lo);

        using broadcast_fn = broadcast<twiddle_type, 16>;
        using transpose_fn = transpose<twiddle_type, 16>;
        w2 = transpose_fn::run(broadcast_fn::run({data.tw2[0] , data.tw2[1]}), 4, 4);
        w2 = ::insert(w2, 1, ::ssrs(ones_, 0));       ones_ = chess_copy(ones_);

        g = ::mul_elem_8_2_conf(c, w2, (this->inv_ ? 0xD4 : OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY), 0);
        h = ::mul_elem_8_2_conf(c, w2, (this->inv_ ? 0xE7 : 0xF5), 0);

        w1 = broadcast_fn::run({data.tw3[0], data.tw1[0]});
        w3 = broadcast_fn::run({data.tw3[1], data.tw1[1]});

        w1 = ::shuffle(w1, w3, T32_8x4_lo);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {
        cint16 one  = {int16_t(std::numeric_limits<int16_t>::min() >> (std::numeric_limits<int16_t>::digits - this->shift_tw_)), 0};
        ones_       = accum<cacc64, 8>(broadcast<cint16, 8>::run(one), 0);
    }
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    v8cacc64 ones_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 2, 4, cint16, Output, cint16> : public fft_dit_common<Vectorization, 2, 4, cint16, Output, cint16>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cint16* dat;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        vector<twiddle_type, 8> tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3) :
            ptr_begin_((const v8cint16 __aie_dm_resource_a *)(ptr)),
            ptw1_((const v8cint16 *)ptw2),
            ptw2_((const v8cint16 *)ptw1),
            ptw3_((const v8cint16 *)ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 4;

           ptw1_++;
           ptw2_++;
           ptw3_++;
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
            return { ptr_begin_, *ptw1_, *ptw2_, *ptw3_};
        }

    private:
        const v8cint16 * __restrict ptr_begin_; //TODO: this will need DM_bankA to avoid memory conflicts
        const v8cint16 * __restrict ptw1_;
        const v8cint16 * __restrict ptw2_;
        const v8cint16 * __restrict ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        accum<cacc64, 8> a, g, h;
        accum<cacc64, 8> o0, o1, o2, o3;
        vector<cint16, 16> b, c;
        vector<cint16, 16> i0, i1;
        vector<cint16, 16> w1, w2, w3;

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

        i0 = ::set_v16cint16(0, data.dat[0]);
        i0 = ::insert(i0,    1, data.dat[1]);
        i1 = ::set_v16cint16(0, data.dat[2]);
        i1 = ::insert(i1,    1, data.dat[3]);

        b = ::shuffle(i0, i1, T32_8x4_lo);
        c = ::shuffle(i0, i1, T32_8x4_hi);

        a = accum<cacc64, 8>(b.extract<8>(0), shift_tw_);
        b = ::shuffle(b,  c,  T256_2x2_hi);

        w1 = ::set_v16cint16(0, data.tw1);
        w2 = ::concat(data.tw2, w2_);
        w3 = ::set_v16cint16(0, data.tw3);

        g = ::mac_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask,    0, 0);
        h = ::msc_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask,    0, 0);

        w1 = ::shuffle(w1, w3, T256_2x2_lo);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
    }

    __aie_inline
    fft_dit()
        : w2_(zeros<cint16, 8>::run())
    {}

    __aie_inline
    fft_dit(unsigned shift_tw, unsigned shift, bool inv)
        : shift_tw_(shift_tw),
          shift_(shift),
          inv_(inv),
          w2_(zeros<cint16, 8>::run())
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
    v8cint16 w2_;
};

template<unsigned Vectorization, typename Output>
struct fft_dit<Vectorization, 0, 4, cint16, Output, cint32> : public fft_dit_common<Vectorization, 0, 4, cint16, Output, cint32>
{
    using   input_type = cint16;
    using  output_type = Output;
    using twiddle_type = cint32;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cint16* dat0;
        const v8cint16* dat1;
        unsigned off;
        twiddle_type tw1;
        twiddle_type tw2;
        twiddle_type tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3, unsigned r) :
            ptr_begin0_((const v8cint16  __aie_dm_resource_a *)(ptr)),
            ptr_begin1_((const v8cint16 __aie_dm_resource_a *)(ptr + 2 * r)),
            ptw1_((const cint32 *)ptw2),
            ptw2_((const cint32 *)ptw1),
            ptw3_((const cint32 *)ptw3),
            r_(r),
            off_(r/8),
            cnt0_(0),
            cnt1_(0),
            cnt_tw1_(0),
            cnt_tw2_(0),
            cnt_tw3_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin0_ = ::add_2d_ptr(ptr_begin0_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt0_, 1);
           ptr_begin1_ = ::add_2d_ptr(ptr_begin1_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt1_, 1);

           ptw1_ = ::add_2d_ptr(ptw1_, 1, r_ / 8 - 1, cnt_tw1_, 0);
           ptw2_ = ::add_2d_ptr(ptw2_, 1, r_ / 8 - 1, cnt_tw2_, 0);
           ptw3_ = ::add_2d_ptr(ptw3_, 1, r_ / 8 - 1, cnt_tw3_, 0);
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
            return {  ptr_begin0_, ptr_begin1_, off_,
                     *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        const v8cint16 * __restrict ptr_begin0_; //TODO: this will need DM_bankA
        const v8cint16 * __restrict ptr_begin1_; //TODO: this will need DM_bankA
        const cint32 * __restrict ptw1_;
        const cint32 * __restrict ptw2_;
        const cint32 * __restrict ptw3_;
        unsigned r_;
        unsigned off_;
        addr_t cnt0_;
        addr_t cnt1_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
        addr_t cnt_tw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_X : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_X);

        accum<cacc64, 8>   a = accum<cacc64, 8>(vector(data.dat0[0]), shift_tw_);
        vector<cint16, 16> b = vector<cint16, 8>(data.dat0[data.off]).grow<16>();
        vector<cint16, 16> c = vector<cint16, 8>(data.dat1[0]).grow<16>();
        vector<cint16, 16> d = vector<cint16, 8>(data.dat1[data.off]).grow<16>();

        const int zero_acc = 0, sub_mul = 0, sub_acc = 0;
        const int shift16 = 0;

        using broadcast_fn = broadcast<twiddle_type, 8>;
        vector w1 = broadcast_fn::run(data.tw1);
        vector w2 = broadcast_fn::run(data.tw2);
        vector w3 = broadcast_fn::run(data.tw3);

        accum<cacc64, 8> g, h, k, l;
        g = ::mac_elem_8_conf(             w2,  c, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
        h = ::msc_elem_8_conf(             w2,  c, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
        k = ::mul_elem_8_conf(             w1,  b,                       cmplx_mask,    sub_mul);
        l = ::mul_elem_8_conf(flip_complex(w1), b,                       cmplx_mask_mj, sub_mul);

        accum<cacc64, 8> o0, o1, o2, o3;
        o0 = ::addmac_elem_8_conf(             w3,  d, g, k, zero_acc, shift16, cmplx_mask,    0, 0, 0);
        o1 = ::addmsc_elem_8_conf(flip_complex(w3), d, h, l, zero_acc, shift16, cmplx_mask_mj, 0, 0, 0);
        o2 = ::submsc_elem_8_conf(             w3,  d, g, k, zero_acc,          cmplx_mask,    0, 0, 0);
        o3 = ::submac_elem_8_conf(flip_complex(w3), d, h, l, zero_acc,          cmplx_mask_mj, 0, 0, 0);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
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
struct fft_dit<Vectorization, 0, 4, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 0, 4, cint32, Output, Twiddle>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cint32* dat0;
        const v4cint32* dat1;
        const int off;
        twiddle_type tw1;
        twiddle_type tw2;
        twiddle_type tw3;
    };

    class stage_iterator
    {
        using data_ptr_t = const v4cint32 *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3, unsigned r) :
            ptr_begin0_((data_ptr_t)(2*r + ptr)),
            ptr_begin1_((data_ptr_t)(2*r + ptr+r)),
            ptw1_(ptw2),
            ptw2_(ptw1),
            ptw3_(ptw3),
            r_(r),
            off_(-2*(int)r/4),
            cnt0_(0),
            cnt1_(0),
            cnt_tw1_(0),
            cnt_tw2_(0),
            cnt_tw3_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin0_ = ::add_2d_ptr(ptr_begin0_, 6*r_/8+2, r_/8-1, cnt0_, 2);
           ptr_begin1_ = ::add_2d_ptr(ptr_begin1_, 6*r_/8+2, r_/8-1, cnt1_, 2);

           ptw1_ = ::add_2d_ptr(ptw1_, 1, r_ / 8 - 1, cnt_tw1_, 0);
           ptw2_ = ::add_2d_ptr(ptw2_, 1, r_ / 8 - 1, cnt_tw2_, 0);
           ptw3_ = ::add_2d_ptr(ptw3_, 1, r_ / 8 - 1, cnt_tw3_, 0);
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
            return {  ptr_begin0_, ptr_begin1_, off_,
                     *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        data_ptr_t           ptr_begin0_; //TODO: this will need DM_bankA to avoid memory conflicts
        data_ptr_t           ptr_begin1_; //TODO: this will need DM_bankA to avoid memory conflicts
        const twiddle_type * ptw1_;
        const twiddle_type * ptw2_;
        const twiddle_type * ptw3_;
        unsigned r_;
        int off_;
        addr_t cnt0_;
        addr_t cnt1_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
        addr_t cnt_tw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<cacc64, 8> a, g, h, k, l, o0, o1, o2, o3;
        vector<cint32, 8> b, c, d;

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        // Make sure we use unique dj* register for each of the two accesses
        // The offset value may not fit in load unit B as immediate even if the value is known at compile time
        int tmp0 = chess_copy(data.off);
        int tmp1 = chess_copy(data.off+1);

        // The order of loads has an impact on performance
        a = concat(accum<cacc64, 4>(vector(data.dat0[tmp0]), shift_tw_),
                   accum<cacc64, 4>(vector(data.dat0[tmp1]), shift_tw_));

        c = concat(data.dat0[0],    data.dat0[1]);
        b = concat(data.dat1[tmp0], data.dat1[tmp1]);
        d = concat(data.dat1[0],    data.dat1[1]);

        const int zero_acc = 0, sub_mul = 0, sub_acc = 0, sub_acc2 = 0;
        if constexpr (std::is_same_v<twiddle_type, cint16>) {
            const int shift16 = 0;

            using broadcast_fn = broadcast<twiddle_type, 16>;
            vector w1 = broadcast_fn::run(data.tw1);
            vector w2 = broadcast_fn::run(data.tw2);
            vector w3 = broadcast_fn::run(data.tw3);

            g = ::mac_elem_8_conf(c, w2, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            h = ::msc_elem_8_conf(c, w2, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            k = ::mul_elem_8_conf(b, w1,                       cmplx_mask,    sub_mul);
            l = ::mul_elem_8_conf(b, flip_complex(w1),         cmplx_mask_mj, sub_mul);

            o0 = ::addmac_elem_8_conf(d,               w3, g, k, zero_acc, shift16, cmplx_mask,    0, 0, 0);
            o1 = ::addmsc_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, shift16, cmplx_mask_mj, 0, 0, 0);
            o2 = ::submsc_elem_8_conf(d,               w3, g, k, zero_acc,          cmplx_mask,    0, 0, 0);
            o3 = ::submac_elem_8_conf(d, flip_complex(w3), h, l, zero_acc,          cmplx_mask_mj, 0, 0, 0);
        } else { // cint32
            using broadcast_fn = broadcast<twiddle_type, 8>;
            vector w1 = broadcast_fn::run(data.tw1);
            vector w2 = broadcast_fn::run(data.tw2);
            vector w3 = broadcast_fn::run(data.tw3);

            // msc_elem_8_conf to use mac due to an emulation bug
            // mul_elem_8_conf intrinsic does not support sub_mask for complex int32 factors
            namespace emu = aie::detail;
            g = emu::mac_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
            h = emu::msc_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
            k = emu::mul_elem_8_conf(b, w1,               cmplx_mask,    sub_mul);
            l = emu::mul_elem_8_conf(b, flip_complex(w1), cmplx_mask_mj, sub_mul);

            o0 = emu::addmac_elem_8_conf(d,               w3, g, k, zero_acc,    cmplx_mask, sub_mul, sub_acc, sub_acc2);
            o1 = emu::addmsc_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
            o2 = emu::submsc_elem_8_conf(d,               w3, g, k, zero_acc,    cmplx_mask, sub_mul, sub_acc, sub_acc2);
            o3 = emu::submac_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 1, 4, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 1, 4, cint32, Output, Twiddle>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cint32 * dat;
        twiddle_type tw1_0;
        twiddle_type tw1_1;
        twiddle_type tw2_0;
        twiddle_type tw2_1;
        twiddle_type tw3_0;
        twiddle_type tw3_1;
    };

    class stage_iterator
    {
        using data_ptr_t = const v4cint32 *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3) :
            ptr_begin_((data_ptr_t)ptr),
            ptw1_(ptw2),
            ptw2_(ptw1),
            ptw3_(ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 8;
           ptw1_ += 2;
           ptw2_ += 2;
           ptw3_ += 2;
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
            return {  ptr_begin_,
                     *ptw1_, *(ptw1_ + 1), *ptw2_, *(ptw2_ + 1), *ptw3_, *(ptw3_ + 1)};
        }

    private:
        data_ptr_t           ptr_begin_; //TODO: this will need DM_bankA to avoid memory conflicts
        const twiddle_type * ptw1_;
        const twiddle_type * ptw2_;
        const twiddle_type * ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        accum<cacc64, 8> a, g, h, k, l;
        accum<cacc64, 8> o0, o1, o2, o3;
        vector<cint32, 8> b, c, d;

        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0, sub_acc2 = 0;
        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        a.insert(0, accum<cacc64, 4>(vector(data.dat[0]), shift_tw_));
        a.insert(1, accum<cacc64, 4>(vector(data.dat[4]), shift_tw_));

        c.insert(0, data.dat[2]);
        c.insert(1, data.dat[6]);

        b.insert(0, data.dat[1]);
        b.insert(1, data.dat[5]);

        d.insert(0, data.dat[3]);
        d.insert(1, data.dat[7]);

        // Create a vector with twiddles repeated in the following pattern: [tw0] * 4, [tw1] * 4
        auto transform_fn = [](auto &tw0, auto &tw1) {
            if constexpr (std::is_same_v<Twiddle, cint16>) {
                using broadcast_fn = broadcast<cint16, 16>;
                using transpose_fn = transpose<cint16, 16>;
                return transpose_fn::run(broadcast_fn::run({tw0, tw1}), 4, 4);
            } else {
                using broadcast_fn = broadcast<cint32, 8>;
                using zip_fn = interleave_zip<cint32, 8>;
                vector<cint32, 8> w;
                std::tie(w, std::ignore) = zip_fn::run(broadcast_fn::run(tw0),
                                                       broadcast_fn::run(tw1),
                                                       4);
                return w;
            }
        };

        vector w1 = transform_fn(data.tw1_0, data.tw1_1);
        vector w2 = transform_fn(data.tw2_0, data.tw2_1);
        vector w3 = transform_fn(data.tw3_0, data.tw3_1);

        if constexpr (std::is_same_v<Twiddle, cint16>) {
            g = ::mac_elem_8_conf(c, w2, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            h = ::msc_elem_8_conf(c, w2, a, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            k = ::mul_elem_8_conf(b, w1,                       cmplx_mask,    sub_mul);
            l = ::mul_elem_8_conf(b, flip_complex(w1),         cmplx_mask_mj, sub_mul);

            o0 = ::addmac_elem_8_conf(d,               w3, g, k, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o2 = ::submsc_elem_8_conf(d,               w3, g, k, zero_acc,          cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o3 = ::submac_elem_8_conf(d, flip_complex(w3), h, l, zero_acc,          cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
            o1 = ::addmsc_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, shift16, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
        } else { // cint32
            // msc_elem_8_conf to use mac due to an emulation bug
            // mul_elem_8_conf intrinsic does not support sub_mask for complex int32 factors
            namespace emu = aie::detail;
            g = emu::mac_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
            h = emu::msc_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
            k = emu::mul_elem_8_conf(b, w1,               cmplx_mask,    sub_mul);
            l = emu::mul_elem_8_conf(b, flip_complex(w1), cmplx_mask_mj, sub_mul);

            // cint32 x cint32 does not support post-adding
            o0 = emu::addmac_elem_8_conf(d,               w3, g, k, zero_acc, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o1 = emu::addmsc_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
            o2 = emu::submsc_elem_8_conf(d,               w3, g, k, zero_acc, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o3 = emu::submac_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned shift_tw, unsigned shift, bool inv) : shift_tw_(shift_tw), shift_(shift), inv_(inv)
    {}
private:
    unsigned shift_tw_, shift_;
    bool inv_;
};

template<unsigned Vectorization, typename Output, typename Twiddle>
struct fft_dit<Vectorization, 2, 4, cint32, Output, Twiddle> : public fft_dit_common<Vectorization, 2, 4, cint32, Output, Twiddle>
{
    using   input_type = cint32;
    using  output_type = Output;
    using twiddle_type = Twiddle;
    using  output_data = typename fft_dit_common<Vectorization, 2, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cint32* dat;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        vector<twiddle_type, 8> tw3;
    };

    class stage_iterator
    {
        using data_ptr_t    = const v4cint32 *;
        using twiddle_ptr_t = const typename decltype(input_data::tw1)::storage_t *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3) :
            ptr_begin_((data_ptr_t)ptr),
            ptw1_(reinterpret_cast<twiddle_ptr_t>(ptw2)),
            ptw2_(reinterpret_cast<twiddle_ptr_t>(ptw1)),
            ptw3_(reinterpret_cast<twiddle_ptr_t>(ptw3))
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 8;
           ptw1_++;
           ptw2_++;
           ptw3_++;
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
            return { ptr_begin_, *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        data_ptr_t    ptr_begin_; //TODO: this will need DM_bankA to avoid memory conflicts
        twiddle_ptr_t ptw1_;
        twiddle_ptr_t ptw2_;
        twiddle_ptr_t ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        accum<cacc64, 8> g, h, k, l, o0, o1, o2, o3;
        vector<cint32, 8> a, b, c, d;
        vector<cint32, 8> i0, i1, i2, i3;
        vector<cint32, 8> s0, s1, s2, s3;

        const int zero_acc = 0, shift16 = 0, sub_mul = 0, sub_acc = 0, sub_acc2 = 0;
        const int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        const int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        i0.insert(0, data.dat[0]); i0.insert(1, data.dat[1]);
        i1.insert(0, data.dat[2]); i1.insert(1, data.dat[3]);
        i2.insert(0, data.dat[4]); i2.insert(1, data.dat[5]);
        i3.insert(0, data.dat[6]); i3.insert(1, data.dat[7]);

        std::tie(s0, s1) = interleave_unzip<cint32, 8>::run(i0, i1, 1);
        std::tie(s2, s3) = interleave_unzip<cint32, 8>::run(i2, i3, 1);

        std::tie(a, c) = interleave_unzip<cint32, 8>::run(s0, s2, 1);
        std::tie(b, d) = interleave_unzip<cint32, 8>::run(s1, s3, 1);

        accum<cacc64, 8> acc(a, shift_tw_);

        if constexpr (std::is_same_v<Twiddle, cint16>) {
            vector<cint16, 16> w1, w2, w3;
            w1 = data.tw1.template grow<16>();
            w2 = data.tw2.template grow<16>();
            w3 = data.tw3.template grow<16>();

            g = ::mac_elem_8_conf(c, w2, acc, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            h = ::msc_elem_8_conf(c, w2, acc, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc);
            k = ::mul_elem_8_conf(b, w1,                       cmplx_mask,    sub_mul);
            l = ::mul_elem_8_conf(b, flip_complex(w1),         cmplx_mask_mj, sub_mul);

            // addmac-like instructions reuse their first input accumulator as the output register, overwriting its
            // contents with the result of the operation.
            // Copying g and h accumulators via VMOV produces a bottleneck in the shuffle unit.
            // Since we have spare cycles in the MAC unit, we using __aie_api_duplicate to compute g and h results twice,
            // each time with a different destination accumulator register.
            o0 = __aie_api_duplicate(g);
            o1 = __aie_api_duplicate(h);

            o0 = ::addmac_elem_8_conf(d,               w3, o0, k, zero_acc, shift16, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o1 = ::addmsc_elem_8_conf(d, flip_complex(w3), o1, l, zero_acc, shift16, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
            o2 = ::submsc_elem_8_conf(d,               w3,  g, k, zero_acc,          cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o3 = ::submac_elem_8_conf(d, flip_complex(w3),  h, l, zero_acc,          cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
        } else { // cint32
            vector<cint32, 8> w1 = data.tw1, w2 = data.tw2, w3 = data.tw3;

            // msc_elem_8_conf to use mac due to an emulation bug
            // mul_elem_8_conf intrinsic does not support sub_mask for complex int32 factors
            namespace emu = aie::detail;
            g = emu::mac_elem_8_conf(c, w2, acc, zero_acc, cmplx_mask,    sub_mul, sub_acc);
            h = emu::msc_elem_8_conf(c, w2, acc, zero_acc, cmplx_mask,    sub_mul, sub_acc);
            k = emu::mul_elem_8_conf(b, w1,                cmplx_mask,    sub_mul);
            l = emu::mul_elem_8_conf(b, flip_complex(w1),  cmplx_mask_mj, sub_mul);

            o0 = emu::addmac_elem_8_conf(d,               w3, g, k, zero_acc, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o1 = emu::addmsc_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
            o2 = emu::submsc_elem_8_conf(d,               w3, g, k, zero_acc, cmplx_mask,    sub_mul, sub_acc, sub_acc2);
            o3 = emu::submac_elem_8_conf(d, flip_complex(w3), h, l, zero_acc, cmplx_mask_mj, sub_mul, sub_acc, sub_acc2);
        }

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, shift_);
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
struct fft_dit<Vectorization, 0, 4, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 0, 4, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cbfloat16* dat0;
        const v8cbfloat16* dat1;
        unsigned off;
        twiddle_type tw1;
        twiddle_type tw2;
        twiddle_type tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3, unsigned r) :
            ptr_begin0_((const v8cbfloat16  __aie_dm_resource_a *)(ptr)),
            ptr_begin1_((const v8cbfloat16 __aie_dm_resource_a *)(ptr + r)),
            ptw1_((const cbfloat16 *)ptw2),
            ptw2_((const cbfloat16 *)ptw1),
            ptw3_((const cbfloat16 *)ptw3),
            r_(r),
            off_(r/4),
            cnt0_(0),
            cnt1_(0),
            cnt_tw1_(0),
            cnt_tw2_(0),
            cnt_tw3_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin0_ = ::add_2d_ptr(ptr_begin0_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt0_, 1);
           ptr_begin1_ = ::add_2d_ptr(ptr_begin1_, 3*r_ / 8 + 1, r_ / 8 - 1, cnt1_, 1);

           ptw1_ = ::add_2d_ptr(ptw1_, 1, r_ / 8 - 1, cnt_tw1_, 0);
           ptw2_ = ::add_2d_ptr(ptw2_, 1, r_ / 8 - 1, cnt_tw2_, 0);
           ptw3_ = ::add_2d_ptr(ptw3_, 1, r_ / 8 - 1, cnt_tw3_, 0);
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
            return {  ptr_begin0_, ptr_begin1_, off_,
                     *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        const v8cbfloat16 * __restrict ptr_begin0_; //TODO: this will need DM_bankA
        const v8cbfloat16 * __restrict ptr_begin1_; //TODO: this will need DM_bankA
        const cbfloat16 * __restrict ptw1_;
        const cbfloat16 * __restrict ptw2_;
        const cbfloat16 * __restrict ptw3_;
        unsigned r_;
        unsigned off_;
        addr_t cnt0_;
        addr_t cnt1_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
        addr_t cnt_tw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<caccfloat, 8> o0, o1, o2, o3;
        vector<twiddle_type, 16> w1, w2, w3;

        accum<caccfloat, 8> a(vector<input_type, 8>(data.dat0[0]));
        vector<input_type, 16> b = ::insert(::set_v16cbfloat16(0, data.dat1[0]), 1, data.dat1[data.off]);
        vector<input_type, 16> c = ::concat(data.dat0[data.off], zero_);

        w1 = broadcast<cbfloat16, 16>::run(data.tw1);
        w2 = broadcast<cbfloat16, 16>::run(data.tw2);
        w3 = broadcast<cbfloat16, 16>::run(data.tw3);
        w1 = ::shuffle(w1, w3, T256_2x2_lo);

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

#if __AIE_API_CBF16_NO_SHIFT16__
        accum<caccfloat, 8> g = ::mac_elem_8_2_conf(c, w2, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> h = ::msc_elem_8_2_conf(c, w2, a, 0, cmplx_mask, 0, 0);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
#else
        accum<caccfloat, 8> g = ::mac_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> h = ::msc_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask, 0, 0);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3});
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
struct fft_dit<Vectorization, 1, 4, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 1, 4, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cbfloat16* dat;
        const twiddle_type* tw1;
        const twiddle_type* tw2;
        const twiddle_type* tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3, unsigned r) :
            ptr_begin_((const v8cbfloat16  __aie_dm_resource_a *)(ptr)),
            ptw1_((const cbfloat16 *)ptw2),
            ptw2_((const cbfloat16 *)ptw1),
            ptw3_((const cbfloat16 *)ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 4;
           ptw1_ += 2;
           ptw2_ += 2;
           ptw3_ += 2;

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
            return {  ptr_begin_, ptw1_, ptw2_, ptw3_ };
        }

    private:
        const v8cbfloat16 * __restrict ptr_begin_; //TODO: this will need DM_bankA
        const cbfloat16 * __restrict ptw1_;
        const cbfloat16 * __restrict ptw2_;
        const cbfloat16 * __restrict ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<caccfloat, 8> o0, o1, o2, o3;
        vector<twiddle_type, 16> w1, w2, w3;

        vector<input_type, 16> i02, i13;
        i02.insert<8>(0, data.dat[0]);
        i13.insert<8>(0, data.dat[1]);
        i02.insert<8>(1, data.dat[2]);
        i13.insert<8>(1, data.dat[3]);

        vector<input_type, 16> b = ::shuffle(i13, i02, T128_4x2_hi);
        vector<input_type, 16> c = ::shuffle(i13, i02, T128_4x2_lo);

        using broadcast_fn = broadcast<twiddle_type, 16>;
        using transpose_fn = transpose<twiddle_type, 16>;
        w2 = transpose_fn::run(broadcast_fn::run({data.tw2[0] , data.tw2[1]}), 4, 4);
        w2.insert(1, minus_one_);

        w1 = broadcast_fn::run({data.tw3[0], data.tw1[0]});
        w3 = broadcast_fn::run({data.tw3[1], data.tw1[1]});

        w1 = ::shuffle(w1, w3, T32_8x4_lo);

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

        accum<caccfloat, 8> g = ::mul_elem_8_2_conf(c, w2, (this->inv_ ? 0xD4 : OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY), 0);
        accum<caccfloat, 8> h = ::mul_elem_8_2_conf(c, w2, (this->inv_ ? 0xE7 : 0xF5), 0);

#if __AIE_API_CBF16_NO_SHIFT16__
        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o1 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o3 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
#else
        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3});
    }

    __aie_inline
    fft_dit()
        : minus_one_(broadcast<cbfloat16, 8>::run(cbfloat16(-1.0f, 0.0f)))
    {}

    __aie_inline
    fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv)
        : inv_(inv),
          minus_one_(broadcast<cbfloat16, 8>::run(cbfloat16(-1.0f, 0.0f)))
    {}
private:
    bool inv_;
    v8cbfloat16 minus_one_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 2, 4, cbfloat16, cbfloat16, cbfloat16> : public fft_dit_common<Vectorization, 2, 4, cbfloat16, cbfloat16, cbfloat16>
{
    using   input_type = cbfloat16;
    using  output_type = cbfloat16;
    using twiddle_type = cbfloat16;
    using  output_data = typename fft_dit_common<Vectorization, 2, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v8cbfloat16* dat;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        vector<twiddle_type, 8> tw3;
    };

    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * __restrict ptr, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3, unsigned r) :
            ptr_begin_((const v8cbfloat16  __aie_dm_resource_a *)(ptr)),
            ptw1_((const v8cbfloat16 *)ptw2),
            ptw2_((const v8cbfloat16 *)ptw1),
            ptw3_((const v8cbfloat16 *)ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 4;
           ptw1_++;
           ptw2_++;
           ptw3_++;

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
            return {  ptr_begin_, *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        const v8cbfloat16 * __restrict ptr_begin_; //TODO: this will need DM_bankA
        const v8cbfloat16 * __restrict ptw1_;
        const v8cbfloat16 * __restrict ptw2_;
        const v8cbfloat16 * __restrict ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * __restrict data, const twiddle_type * __restrict ptw1, const twiddle_type * __restrict ptw2, const twiddle_type * __restrict ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<caccfloat, 8> o0, o1, o2, o3;
        vector<twiddle_type, 16> w1, w2, w3;

        vector<input_type, 16> i01, i23;
        i01.insert<8>(0, data.dat[0]);
        i01.insert<8>(1, data.dat[1]);
        i23.insert<8>(0, data.dat[2]);
        i23.insert<8>(1, data.dat[3]);

        accum<caccfloat, 8>    a = ::ups_to_v8caccfloat(::extract_v8cbfloat16(::shuffle(i01, i23, T32_8x4_lo), 0));
        vector<input_type, 16> b = ::shuffle(i01, i23, T32_8x4_lo);
        vector<input_type, 16> c = ::shuffle(i01, i23, T32_8x4_hi);
        b = ::shuffle(b, c, T256_2x2_hi);

        w1 = data.tw1.template grow<16>();
        w2 = ::concat(data.tw2, zero_);
        w3 = data.tw3.template grow<16>();

        w1 = ::shuffle(w1, w3, T256_2x2_lo);

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y         : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_BUTTERFLY : OP_TERM_NEG_COMPLEX_BUTTERFLY);

#if __AIE_API_CBF16_NO_SHIFT16__
        accum<caccfloat, 8> g = ::mac_elem_8_2_conf(c, w2, a, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> h = ::msc_elem_8_2_conf(c, w2, a, 0, cmplx_mask, 0, 0);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, cmplx_mask_mj, 0, 0);
#else
        accum<caccfloat, 8> g = ::mac_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask, 0, 0);
        accum<caccfloat, 8> h = ::msc_elem_8_2_conf(c, w2, a, 0, 0, cmplx_mask, 0, 0);

        o0 = ::mac_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o1 = ::mac_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
        o2 = ::msc_elem_8_2_conf(b,               w1, g, 0, 0, cmplx_mask,    0, 0);
        o3 = ::msc_elem_8_2_conf(b, flip_complex(w1), h, 0, 0, cmplx_mask_mj, 0, 0);
#endif

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3});
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

#endif // __AIE_API_CBF16_SUPPORT__

#if __AIE_API_COMPLEX_FP32_EMULATION__
template<unsigned Vectorization>
struct fft_dit<Vectorization, 0, 4, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 0, 4, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 0, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cfloat* dat0;
        const v4cfloat* dat1;
        const int off;
        twiddle_type tw1;
        twiddle_type tw2;
        twiddle_type tw3;
    };

    class stage_iterator
    {
        using data_ptr_t = const v4cfloat *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3, unsigned r) :
            ptr_begin0_((data_ptr_t)(2*r + ptr)),
            ptr_begin1_((data_ptr_t)(2*r + ptr+r)),
            ptw1_(ptw2),
            ptw2_(ptw1),
            ptw3_(ptw3),
            r_(r),
            off_(-2*(int)r/4),
            cnt0_(0),
            cnt1_(0),
            cnt_tw1_(0),
            cnt_tw2_(0),
            cnt_tw3_(0)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin0_ = ::add_2d_ptr(ptr_begin0_, 6*r_/8+2, r_/8-1, cnt0_, 2);
           ptr_begin1_ = ::add_2d_ptr(ptr_begin1_, 6*r_/8+2, r_/8-1, cnt1_, 2);

           ptw1_ = ::add_2d_ptr(ptw1_, 1, r_ / 8 - 1, cnt_tw1_, 0);
           ptw2_ = ::add_2d_ptr(ptw2_, 1, r_ / 8 - 1, cnt_tw2_, 0);
           ptw3_ = ::add_2d_ptr(ptw3_, 1, r_ / 8 - 1, cnt_tw3_, 0);
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
            return {  ptr_begin0_, ptr_begin1_, off_,
                     *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        data_ptr_t           ptr_begin0_; //TODO: this will need DM_bankA to avoid memory conflicts
        data_ptr_t           ptr_begin1_; //TODO: this will need DM_bankA to avoid memory conflicts
        const twiddle_type * ptw1_;
        const twiddle_type * ptw2_;
        const twiddle_type * ptw3_;
        unsigned r_;
        int off_;
        addr_t cnt0_;
        addr_t cnt1_;
        addr_t cnt_tw1_;
        addr_t cnt_tw2_;
        addr_t cnt_tw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3, Vectorization);
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

        accum<caccfloat, 8> a, g, h, k, l, o0, o1, o2, o3;
        vector<cfloat, 8> b, c, d;

        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        // Make sure we use unique dj* register for each of the two accesses
        // The offset value may not fit in load unit B as immediate even if the value is known at compile time
        int tmp0 = chess_copy(data.off);
        int tmp1 = chess_copy(data.off+1);

        // The order of loads has an impact on performance
        a = concat(accum<caccfloat, 4>(vector(data.dat0[tmp0])),
                   accum<caccfloat, 4>(vector(data.dat0[tmp1])));

        c = concat(data.dat0[0],    data.dat0[1]);
        b = concat(data.dat1[tmp0], data.dat1[tmp1]);
        d = concat(data.dat1[0],    data.dat1[1]);

        const int zero_acc = 0, sub_mul = 0, sub_acc = 0;

        using broadcast_fn = broadcast<twiddle_type, 8>;
        vector w1 = broadcast_fn::run(data.tw1);
        vector w2 = broadcast_fn::run(data.tw2);
        vector w3 = broadcast_fn::run(data.tw3);

        g = ::mac_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
        h = ::msc_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
        k = ::mul_elem_8_conf(b, w1,               cmplx_mask,    sub_mul);
        l = ::mul_elem_8_conf(b, flip_complex(w1), cmplx_mask_mj, sub_mul);

        o0 = ::add(::mac_elem_8_conf(d,               w3, g, zero_acc,    cmplx_mask, sub_mul, sub_acc), k);
        o1 = ::add(::msc_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);
        o2 = ::sub(::msc_elem_8_conf(d,               w3, g, zero_acc,    cmplx_mask, sub_mul, sub_acc), k);
        o3 = ::sub(::mac_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, 0);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 1, 4, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 1, 4, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 1, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cfloat * dat;
        twiddle_type tw1_0;
        twiddle_type tw1_1;
        twiddle_type tw2_0;
        twiddle_type tw2_1;
        twiddle_type tw3_0;
        twiddle_type tw3_1;
    };

    class stage_iterator
    {
        using data_ptr_t = const v4cfloat *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr,
                                              const twiddle_type * ptw1,
                                              const twiddle_type * ptw2,
                                              const twiddle_type * ptw3) :
            ptr_begin_((data_ptr_t)ptr),
            ptw1_(ptw2),
            ptw2_(ptw1),
            ptw3_(ptw3)
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 8;
           ptw1_ += 2;
           ptw2_ += 2;
           ptw3_ += 2;
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
            return {  ptr_begin_,
                     *ptw1_, *(ptw1_ + 1), *ptw2_, *(ptw2_ + 1), *ptw3_, *(ptw3_ + 1)};
        }

    private:
        data_ptr_t           ptr_begin_; //TODO: this will need DM_bankA to avoid memory conflicts
        const twiddle_type * ptw1_;
        const twiddle_type * ptw2_;
        const twiddle_type * ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        accum<caccfloat, 8> a, g, h, k, l;
        accum<caccfloat, 8> o0, o1, o2, o3;
        vector<cfloat, 8> b, c, d;

        const int zero_acc = 0, sub_mul = 0, sub_acc = 0;
        int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        a.insert(0, accum<caccfloat, 4>(vector(data.dat[0])));
        a.insert(1, accum<caccfloat, 4>(vector(data.dat[4])));

        c.insert(0, data.dat[2]);
        c.insert(1, data.dat[6]);

        b.insert(0, data.dat[1]);
        b.insert(1, data.dat[5]);

        d.insert(0, data.dat[3]);
        d.insert(1, data.dat[7]);

        // Create a vector with twiddles repeated in the following pattern: [tw0] * 4, [tw1] * 4
        auto transform_fn = [](auto &tw0, auto &tw1) {
            using broadcast_fn = broadcast<cfloat, 8>;
            using zip_fn = interleave_zip<cfloat, 8>;
            vector<cfloat, 8> w;
            std::tie(w, std::ignore) = zip_fn::run(broadcast_fn::run(tw0),
                                                   broadcast_fn::run(tw1),
                                                   4);
            return w;
        };

        vector w1 = transform_fn(data.tw1_0, data.tw1_1);
        vector w2 = transform_fn(data.tw2_0, data.tw2_1);
        vector w3 = transform_fn(data.tw3_0, data.tw3_1);

        g = ::mac_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
        h = ::msc_elem_8_conf(c, w2, a, zero_acc,  cmplx_mask,    sub_mul, sub_acc);
        k = ::mul_elem_8_conf(b, w1,               cmplx_mask,    sub_mul);
        l = ::mul_elem_8_conf(b, flip_complex(w1), cmplx_mask_mj, sub_mul);

        o0 = ::add(::mac_elem_8_conf(d,               w3, g, zero_acc, cmplx_mask,    sub_mul, sub_acc), k);
        o1 = ::add(::msc_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);
        o2 = ::sub(::msc_elem_8_conf(d,               w3, g, zero_acc, cmplx_mask,    sub_mul, sub_acc), k);
        o3 = ::sub(::mac_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, 0);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

template<unsigned Vectorization>
struct fft_dit<Vectorization, 2, 4, cfloat, cfloat, cfloat> : public fft_dit_common<Vectorization, 2, 4, cfloat, cfloat, cfloat>
{
    using   input_type = cfloat;
    using  output_type = cfloat;
    using twiddle_type = cfloat;
    using  output_data = typename fft_dit_common<Vectorization, 2, 4, input_type, output_type, twiddle_type>::output_data;

    struct input_data
    {
        const v4cfloat* dat;
        vector<twiddle_type, 8> tw1;
        vector<twiddle_type, 8> tw2;
        vector<twiddle_type, 8> tw3;
    };

    class stage_iterator
    {
        using data_ptr_t    = const v4cfloat *;
        using twiddle_ptr_t = const typename decltype(input_data::tw1)::storage_t *;
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        __aie_inline
        SCALAR_TYPES_CONSTEXPR stage_iterator(const input_type * ptr,
                                              const twiddle_type * ptw1,
                                              const twiddle_type * ptw2,
                                              const twiddle_type * ptw3) :
            ptr_begin_((data_ptr_t)ptr),
            ptw1_(reinterpret_cast<twiddle_ptr_t>(ptw2)),
            ptw2_(reinterpret_cast<twiddle_ptr_t>(ptw1)),
            ptw3_(reinterpret_cast<twiddle_ptr_t>(ptw3))
        {}

        __aie_inline
        stage_iterator &operator++()
        {
           ptr_begin_ += 8;
           ptw1_++;
           ptw2_++;
           ptw3_++;
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
            return { ptr_begin_, *ptw1_, *ptw2_, *ptw3_ };
        }

    private:
        data_ptr_t    ptr_begin_; //TODO: this will need DM_bankA to avoid memory conflicts
        twiddle_ptr_t ptw1_;
        twiddle_ptr_t ptw2_;
        twiddle_ptr_t ptw3_;
    };

    __aie_inline
    stage_iterator begin_stage(const input_type * data, const twiddle_type * ptw1, const twiddle_type * ptw2, const twiddle_type * ptw3)
    {
        return stage_iterator(data, ptw1, ptw2, ptw3);
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

        accum<caccfloat, 8> g, h, k, l, o0, o1, o2, o3;
        vector<cfloat, 8> a, b, c, d;
        vector<cfloat, 8> i0, i1, i2, i3;
        vector<cfloat, 8> s0, s1, s2, s3;

        const int zero_acc = 0, sub_mul = 0, sub_acc = 0;
        const int cmplx_mask    = (inv_ ? OP_TERM_NEG_COMPLEX_CONJUGATE_Y : OP_TERM_NEG_COMPLEX);
        const int cmplx_mask_mj = (inv_ ? OP_TERM_NEG_COMPLEX : OP_TERM_NEG_COMPLEX_CONJUGATE_Y);

        i0.insert(0, data.dat[0]); i0.insert(1, data.dat[1]);
        i1.insert(0, data.dat[2]); i1.insert(1, data.dat[3]);
        i2.insert(0, data.dat[4]); i2.insert(1, data.dat[5]);
        i3.insert(0, data.dat[6]); i3.insert(1, data.dat[7]);

        std::tie(s0, s1) = interleave_unzip<cfloat, 8>::run(i0, i1, 1);
        std::tie(s2, s3) = interleave_unzip<cfloat, 8>::run(i2, i3, 1);

        std::tie(a, c) = interleave_unzip<cfloat, 8>::run(s0, s2, 1);
        std::tie(b, d) = interleave_unzip<cfloat, 8>::run(s1, s3, 1);

        accum<caccfloat, 8> acc(a);

        vector<cfloat, 8> w1 = data.tw1, w2 = data.tw2, w3 = data.tw3;

        g = ::mac_elem_8_conf(c, w2, acc, zero_acc, cmplx_mask,    sub_mul, sub_acc);
        h = ::msc_elem_8_conf(c, w2, acc, zero_acc, cmplx_mask,    sub_mul, sub_acc);
        k = ::mul_elem_8_conf(b, w1,                cmplx_mask,    sub_mul);
        l = ::mul_elem_8_conf(b, flip_complex(w1),  cmplx_mask_mj, sub_mul);

        o0 = ::add(::mac_elem_8_conf(d,               w3, g, zero_acc, cmplx_mask,    sub_mul, sub_acc), k);
        o1 = ::add(::msc_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);
        o2 = ::sub(::msc_elem_8_conf(d,               w3, g, zero_acc, cmplx_mask,    sub_mul, sub_acc), k);
        o3 = ::sub(::mac_elem_8_conf(d, flip_complex(w3), h, zero_acc, cmplx_mask_mj, sub_mul, sub_acc), l);

        return fft_dit::fft_dit_common::template to_output<256>(std::array{o0, o1, o2, o3}, 0);
    }

    __aie_inline fft_dit() = default;
    __aie_inline fft_dit(unsigned /*shift_tw*/, unsigned /*shift*/, bool inv) : inv_(inv)
    {}
private:
    bool inv_;
};

#endif // __AIE_API_COMPLEX_FP32_EMULATION__

}

#endif // __AIE_API_COMPLEX_VECTOR_SUPPORT__
#endif //  __AIE_API_DETAIL_AIE2_FFT_DIT_RADIX4_HPP__
