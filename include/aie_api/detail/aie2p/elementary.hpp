// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_ELEMENTARY__HPP__
#define __AIE_API_DETAIL_AIE2P_ELEMENTARY__HPP__

#include "../add.hpp"
#include "../broadcast.hpp"
#include "../filter.hpp"
#include "../neg.hpp"

#include "../aie2/tile.hpp"

namespace aie::detail {

template <unsigned Bits, typename TR, typename T, unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Fix2Float, Bits, TR, T, N>
{
    using vector_ret_type = vector<TR, N>;
    using     vector_type = vector<T, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign = vector_type::is_signed())
    {
        constexpr unsigned N_Op = std::max(N, 32u);

        using acc_fp_t  = aie::accum<accfloat, N_Op>;
        using acc_int_t = aie::accum<acc32,    N_Op>;

        const acc_fp_t magic_h(broadcast<int16, N_Op>::run(0x5301 - 128 * shift).template cast_to<bfloat16>());
        const acc_fp_t magic_l(broadcast<int16, N_Op>::run(0x4b01 - 128 * shift).template cast_to<bfloat16>());

        acc_fp_t vfp;
        if constexpr (sizeof(T) > 2) {
            acc_fp_t tmp;
            acc_int_t vint_h, vint_l;
            vint_h.from_vector_sign(filter<uint16, N_Op * 2, FilterOp::Odd >::run(v.template cast_to<uint16>().template grow<N_Op * 2>(), 1), sign);
            vint_l.from_vector_sign(filter<uint16, N_Op * 2, FilterOp::Even>::run(v.template cast_to<uint16>().template grow<N_Op * 2>(), 1), false);

            vint_h = add_accum<acc32, N_Op>::run(vint_h, false, magic_h.template cast_to<acc32>());
            vint_l = add_accum<acc32, N_Op>::run(vint_l, false, magic_l.template cast_to<acc32>());

            tmp = sub_accum<accfloat, N_Op>::run(vint_h.template cast_to<accfloat>(), false, magic_h);
            vfp = sub_accum<accfloat, N_Op>::run(vint_l.template cast_to<accfloat>(), false, magic_l);

            vfp = add_accum<accfloat, N_Op>::run(vfp, false, tmp);
        }
        else {
            accum<acc32, N> tmp;
                            tmp.from_vector_sign(v, sign);
            acc_int_t vint = tmp.template grow<N_Op>();

            vint = add_accum<acc32,    N_Op>::run(vint, false, magic_l.template cast_to<acc32>());
            vfp  = sub_accum<accfloat, N_Op>::run(vint.template cast_to<accfloat>(), false, magic_l);
        }

        return vfp.template extract<N>(0).template to_vector_sign<TR>(sign);
    }
};

template <unsigned Bits, typename TR, typename T, unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Float2Fix, Bits, TR, T, N>
{
    using vector_ret_type = vector<TR, N>;
    using     vector_type = vector<T, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned N_Op = std::max(N, 32u);

        using upper_t = std::conditional_t<std::is_signed_v<T>, int16, uint16>;

        using acc_fp_t  = aie::accum<accfloat, N_Op>;
        using acc_int_t = aie::accum<acc32, N_Op>;

        const acc_fp_t magic_h(broadcast<int16, N_Op>::run( 0x5301 - 128 * shift ).template cast_to<bfloat16>( ));
        const acc_fp_t magic_l(broadcast<int16, N_Op>::run( 0x4b01 - 128 * shift ).template cast_to<bfloat16>( ));

        const saturation_mode sat = tile::current().get_saturation();
        tile::current().set_saturation(saturation_mode::saturate);

        const acc_fp_t acc_input(v.template grow<N_Op>());
        acc_fp_t vfp = acc_input;
        acc_int_t vint;
        vector<upper_t, N_Op> out_h;
        vector<TR, N> output;

        if constexpr (sizeof(TR) > 2 ) {
            acc_fp_t tmp;
            vfp  = add_accum<accfloat, N_Op>::run(vfp, false, magic_h);
            vint = sub_accum<acc32,    N_Op>::run(vfp.template cast_to<acc32>(),
                                                  false,
                                                  magic_h.template cast_to<acc32>());

            out_h = vint.template to_vector<upper_t>();

            vint = add_accum<acc32, N_Op>::run(acc_int_t(out_h),
                                               false,
                                               magic_h.template cast_to<acc32>());
            tmp  = sub_accum<accfloat, N_Op>::run(vint.template cast_to<accfloat>(), false, magic_h);
            vfp  = sub_accum<accfloat, N_Op>::run(acc_input, false, tmp);
        }

        vfp  = add_accum<accfloat, N_Op>::run(vfp, false, magic_l);
        vint = sub_accum<acc32,    N_Op>::run(vfp.template cast_to<acc32>(),
                                              false,
                                              magic_l.template cast_to<acc32>());

        if constexpr (sizeof(TR) > 2) {
            auto out_lp = vint.template to_vector<uint16>();
            aie::accum<acc32, N_Op> vint_neg = neg_acc<32, acc32, N_Op>::run(vint.template grow_extract<N_Op>(0));
            auto out_ln = vint_neg.template to_vector<uint16>();

            using acc_t = aie::accum<acc64, N_Op>;
            acc_t tmp;
            tmp = add_accum<acc64, N_Op>::run(acc_t(out_h, 16), false, acc_t(out_lp));
            auto acc = sub_accum<acc64, N_Op>::run(tmp, false, acc_t(out_ln));
            output = acc.template to_vector<TR>().template extract<N>(0);
        }
        else {
            output = vint.template to_vector<TR>().template extract<N>(0);
        }

        tile::current().set_saturation(sat);

        return output;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Float2Fix, 16, int16, bfloat16, N>
{
    using vector_ret_type = vector<int16, N>;
    using     vector_type = vector<bfloat16, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned num_op = N < 32? 1 : N / 32;

        vector_ret_type ret;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            vector<int32, 16> tmp1, tmp2;

            tmp1 = ::bfloat16_to_int(v.template grow_extract<16>(idx * 2), shift);

            if constexpr (N > 16)
                tmp2 = ::bfloat16_to_int(v.template grow_extract<16>(idx * 2 + 1), shift);

            tmp1 = ::shuffle(tmp1, tmp2, DINTLV_lo_16o32);

            if constexpr (N <= 16)
                ret = tmp1.cast_to<int16>().extract<N>(0);
            else
                ret.insert(idx, tmp1.cast_to<int16>());
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Float2Fix, 16, int32, bfloat16, N>
{
    using vector_ret_type = vector<int32, N>;
    using     vector_type = vector<bfloat16, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned num_op = N < 16? 1 : N / 16;

        vector_ret_type ret;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            vector<int32, 16> tmp;

            tmp = ::bfloat16_to_int(v.template grow_extract<16>(idx), shift);

            if constexpr (N < 16)
                ret = tmp.extract<N>(0);
            else
                ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Inv, 16, bfloat16, bfloat16, N>
{
    using vector_ret_type = vector<bfloat16, N>;
    using     vector_type = vector<bfloat16, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned native_lanes = N <= 32? 32 : 64;
        constexpr unsigned num_op       = N <  64?  1 : N / 64;
        BFLOAT16_CONSTEXPR bfloat16 K1 = 1.9395974f;

        const vector<int16, native_lanes> magic = broadcast<int16, native_lanes>::run(0x7eb5);

        accum<accfloat, native_lanes> k2;

        utils::unroll_times<native_lanes / 16>([&](auto idx) __aie_inline {
            k2.template insert<16>(idx, ::broadcast_to_v16accfloat(1.436142f));
        });

        accum<accfloat, native_lanes> acc_twos;

        utils::unroll_times<native_lanes / 16>([&](auto idx) __aie_inline {
            acc_twos.template insert<16>(idx, ::broadcast_to_v16accfloat(2.0f));
        });

        vector_ret_type ret;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            vector<bfloat16, native_lanes> current = v.template grow_extract<native_lanes>(idx);

            // int16 i = *(int16*)&v;
            // i = 0x7eb5 - i;
            // bfloat16 y = *(bfloat16*)&i;
            vector<bfloat16, native_lanes> y = sub<int16, native_lanes>::run(magic, current.template cast_to<int16>()).template cast_to<bfloat16>();

            // y = k1*y*((-v)*y + k2);
            accum<accfloat, native_lanes> acc;
            if constexpr (native_lanes == 32)
                acc = ::negmul_elem_32(current, y);
            else
                acc = ::negmul_elem_64(current, y);

            acc = ::add(acc, k2);

            if constexpr (native_lanes == 32)
                acc = ::mul_elem_32(acc.template to_vector<bfloat16>(), y);
            else
                acc = ::mul_elem_64(acc.template to_vector<bfloat16>(), y);

            // This computes -y
            if constexpr (native_lanes == 32)
                acc = ::negmul_elem_32(acc.template to_vector<bfloat16>(), mul_vector_or_scalar<native_lanes>(K1));
            else
                acc = ::negmul_elem_64(acc.template to_vector<bfloat16>(), mul_vector_or_scalar<native_lanes>(K1));

            y = acc.template to_vector<bfloat16>();

            // bfloat16 r = -y * v + 2.0f;
            if constexpr (native_lanes == 32)
                acc = ::mac_elem_32(y, current, acc_twos);
            else
                acc = ::mac_elem_64(y, current, acc_twos);

            // y = y*r
            if constexpr (native_lanes == 32)
                acc = ::negmul_elem_32( y, acc.template to_vector<bfloat16>());
            else
                acc = ::negmul_elem_64( y, acc.template to_vector<bfloat16>());

            if constexpr (N < native_lanes)
                ret = acc.template to_vector<bfloat16>().template extract<N>(0);
            else
                ret.insert(idx, acc.template to_vector<bfloat16>());
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::InvSqrt, 16, bfloat16, bfloat16, N>
{
    using vector_ret_type = vector<bfloat16, N>;
    using     vector_type = vector<bfloat16, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        // Implementation based on http://rrrola.wz.cz/inv_sqrt.html
        constexpr int   C1 = 0x5F1FFFF9;
        constexpr float C2 = 0.703952253f;
        constexpr float C3 = 2.38924456f;

        BFLOAT16_CONSTEXPR bfloat16 C2_bf16 = C2;

        // TODO: some operations support 64 lanes. Try to optimize that case
        constexpr unsigned native_lanes = 32;                   // (N <= 32? 32 : 64);
        constexpr unsigned num_op       = (N < 32? 1 : N / 32); // (N < 32? 1 : (N < 64? 2 : N / 64));

        vector_ret_type ret;

        const vector<int32, native_lanes> c1 = broadcast<int32, native_lanes>::run(C1);

        accum<accfloat, native_lanes> c2c3;

        const auto c2c3_tmp = ::broadcast_to_v16accfloat(C2 * C3);
        c2c3.template insert<16>(0, c2c3_tmp);
        if constexpr (N > 16) c2c3.template insert<16>(1, c2c3_tmp);

        vector<bfloat16, native_lanes>   x2;
        vector<bfloat16, native_lanes>    y;
        vector<bfloat16, native_lanes>   yy;
        vector<bfloat16, native_lanes> x2yy;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            const vector<bfloat16, native_lanes> current = v.template grow_extract<native_lanes>(idx);

            x2 = mul<MulMacroOp::Mul, 32, bfloat16, bfloat16>::run(current,
                                                                   false,
                                                                   mul_vector_or_scalar<native_lanes>(C2_bf16),
                                                                   false).template to_vector<bfloat16>();

            accum<accfloat, native_lanes> current_acc(current);
            accum<acc64, native_lanes> current_int_acc(current_acc.template cast_to<acc32>().template to_vector<int32>());

            vector<int32, native_lanes> i = sub<int32, native_lanes>::run(c1, current_int_acc.template to_vector<int32>(1));
            accum<accfloat, native_lanes> ii = (v32accfloat)i;

            y = ii.template to_vector<bfloat16>();

            yy   = mul<MulMacroOp::Mul,     32, bfloat16, bfloat16>::run(y,  false, y,  false).template to_vector<bfloat16>();
            x2yy = mul<MulMacroOp::Sub_Mul, 32, bfloat16, bfloat16>::run(x2, false, yy, false, c2c3).template to_vector<bfloat16>();

            const accum<accfloat, native_lanes> acc = mul<MulMacroOp::Mul, 32, bfloat16, bfloat16>::run(y, false, x2yy, false);

            if constexpr (N < native_lanes)
                ret = acc.template to_vector<bfloat16>().template extract<N>(0);
            else
                ret.insert(idx, acc.template to_vector<bfloat16>());
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Tanh, 32, bfloat16, float, N>
{
    using vector_ret_type = vector<bfloat16, N>;
    using     vector_type = vector<float, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned native_lanes = 16;
        constexpr unsigned num_op = N < native_lanes ? 1 : N / native_lanes;

        vector_ret_type ret;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            const accum<accfloat, native_lanes> acc(v.template grow_extract<native_lanes>(idx));
            const vector<bfloat16, native_lanes> tmp = ::tanh(acc);

            if constexpr (N < native_lanes)
                ret = tmp.extract<N>(0);
            else
                ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned N>
struct elementary_vector_bits_impl<ElementaryOp::Exp2, 32, bfloat16, float, N>
{
    using vector_ret_type = vector<bfloat16, N>;
    using     vector_type = vector<float, N>;

    __aie_inline
    static vector_ret_type run(const vector_type &v, int shift = 0, bool sign_dummy = false)
    {
        constexpr unsigned native_lanes = 16;
        constexpr unsigned num_op = N < native_lanes? 1 : N / native_lanes;

        vector_ret_type ret;

        utils::unroll_times<num_op>([&](auto idx) __aie_inline {
            const accum<accfloat, native_lanes> acc(v.template grow_extract<native_lanes>(idx));
            const vector<bfloat16, native_lanes> tmp = ::exp2(acc);

            if constexpr (N < native_lanes)
                ret = tmp.extract<N>(0);
            else
                ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned Bits, typename TR, typename T, unsigned N>
struct elementary_acc_bits_impl<ElementaryOp::Fix2Float, Bits, TR, T, N>
{
    using vector_ret_type = vector<TR, N>;
    using      accum_type = accum<T, N>;

    static constexpr unsigned native_elems = native_vector_length_v<TR>;

    using           add = add<TR, native_elems>;
    template <typename U>
    using      to_float = elementary_vector_bits_impl<ElementaryOp::Fix2Float, 16, TR, U, native_elems>;
    using   signed_type = vector<int16,  native_elems>;
    using unsigned_type = vector<uint16, native_elems>;

    template <typename U>
    __aie_inline
    static auto srs_helper(auto acc, int shift)
    {
        constexpr auto sat_mode = (unsigned)::aie::saturation_mode::none;
        constexpr auto rnd_mode = (unsigned)::aie::rounding_mode::floor;
        if constexpr (native_elems == 16) {
            if      constexpr (std::is_same_v<U, int16>)  return   signed_type(::srs_to_v16int16_conf (acc, shift, sat_mode, rnd_mode));
            else if constexpr (std::is_same_v<U, uint16>) return unsigned_type(::srs_to_v16uint16_conf(acc, shift, sat_mode, rnd_mode));
        }
        else {
            if      constexpr (std::is_same_v<U, int16>)  return   signed_type(::srs_to_v32int16_conf (acc, shift, sat_mode, rnd_mode));
            else if constexpr (std::is_same_v<U, uint16>) return unsigned_type(::srs_to_v32uint16_conf(acc, shift, sat_mode, rnd_mode));
        }
    }

    __aie_inline
    static vector_ret_type run(const accum_type &acc, int shift = 0)
    {
        vector_ret_type ret;

        utils::unroll_times<std::max(1u, N / native_elems)>([&](unsigned idx) {
            vector<TR, native_elems> v;

            if      constexpr (Bits == 32) {
                v =             to_float< int16>::run(srs_helper< int16>(acc.template grow_extract<native_elems>(idx), 16), shift-16);
                v = add::run(v, to_float<uint16>::run(srs_helper<uint16>(acc.template grow_extract<native_elems>(idx),  0), shift   ));
            }
            else if constexpr (Bits == 64) {
                v =             to_float< int16>::run(srs_helper< int16>(acc.template grow_extract<native_elems>(idx), 48), shift-48);
                v = add::run(v, to_float<uint16>::run(srs_helper<uint16>(acc.template grow_extract<native_elems>(idx), 32), shift-32));
                v = add::run(v, to_float<uint16>::run(srs_helper<uint16>(acc.template grow_extract<native_elems>(idx), 16), shift-16));
                v = add::run(v, to_float<uint16>::run(srs_helper<uint16>(acc.template grow_extract<native_elems>(idx),  0), shift   ));
            }

            ret.insert(idx, v.template extract<std::min(N, native_elems)>(0));
        });

        return ret;
    }
};

template <unsigned TypeBits, typename TR> requires(utils::is_one_of_v<TR, int16, int32>)
struct elementary_bits_impl<ElementaryOp::Float2Fix, TypeBits, TR, float>
{
    using  T = float;

    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        return ::float2fix(a, shift);
    }
};

template <unsigned TypeBits, typename T> requires(utils::is_one_of_v<T, int16, int32>)
struct elementary_bits_impl<ElementaryOp::Fix2Float, TypeBits, float, T>
{
    using  TR = float;

    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        return ::fix2float(a, shift);
    }
};

template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Inv, TypeBits, float, float>
{
    using  T = float;
    using TR = float;

    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        return ::inv(a);
    }
};

template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::InvSqrt, TypeBits, float, float>
{
    using  T = float;
    using TR = float;

    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        return ::invsqrt(a);
    }
};


template <ElementaryOp Op, unsigned TypeBits, typename TR, typename T>
struct elementary_bits_common_impl
{
    static constexpr unsigned native_elems = 512 / TypeBits;

    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        vector<T, native_elems> v(a);

        return elementary_vector<Op, TR, T, native_elems>::run(v, shift)[0];
    }
};

template <unsigned TypeBits, typename TR, typename T>
    requires(is_floating_point_v<TR> && is_floating_point_v<T>)
struct elementary_bits_common_impl<ElementaryOp::Sqrt, TypeBits, TR, T>
{
    __aie_inline
    static TR run(const T &a, int shift = 0)
    {
        return TR(::_sqrtf(a));
    }
};

template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Fix2Float, TypeBits, bfloat16, int16> : elementary_bits_common_impl<ElementaryOp::Fix2Float, TypeBits, bfloat16, int16> {};
template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Fix2Float, TypeBits, bfloat16, int32> : elementary_bits_common_impl<ElementaryOp::Fix2Float, TypeBits, bfloat16, int32> {};

template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Inv,     TypeBits, bfloat16, bfloat16> : elementary_bits_common_impl<ElementaryOp::Inv,      TypeBits, bfloat16, bfloat16> {};
template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::InvSqrt, TypeBits, bfloat16, bfloat16> : elementary_bits_common_impl<ElementaryOp::InvSqrt,  TypeBits, bfloat16, bfloat16> {};

template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Sqrt, TypeBits, float, float>       : elementary_bits_common_impl<ElementaryOp::Sqrt,  TypeBits, float, float> {};
template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Sqrt, TypeBits, bfloat16, bfloat16> : elementary_bits_common_impl<ElementaryOp::Sqrt,  TypeBits, bfloat16, bfloat16> {};
template <unsigned TypeBits>
struct elementary_bits_impl<ElementaryOp::Sqrt, TypeBits, float, bfloat16>    : elementary_bits_common_impl<ElementaryOp::Sqrt,  TypeBits, float, bfloat16> {};

}

#endif

