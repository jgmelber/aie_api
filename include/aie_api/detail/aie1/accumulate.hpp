// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ACCUMULATE__HPP__
#define __AIE_API_DETAIL_AIE1_ACCUMULATE__HPP__

#include "../accum.hpp"
#include "../vector.hpp"
#include "../broadcast.hpp"
#include "../mul.hpp"

#include <type_traits>

namespace aie::detail {

template <bool WithInputAccum, Operation Op, typename Acc>
    requires(is_accum_v<Acc>)
constexpr auto get_mul_op()
{
    if constexpr (Acc::value_class() == AccumClass::FP || Acc::value_class() == AccumClass::CFP) {
        if      constexpr (!WithInputAccum)              return [](auto &&... params)  { return ::fpmul(params...); };
        else if constexpr (Op == Operation::Acc_Add)     return [](auto &&... params)  { return ::fpmac(params...); };
        else if constexpr (Op == Operation::Acc_Sub)     return [](auto &&... params)  { return ::fpmsc(params...); };
    }
    else if constexpr (Acc::accum_bits() == 80) {
        if constexpr (Acc::size() == 8) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::lmul8(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::lmac8(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::lmsc8(params...); };
        }
        else if constexpr (Acc::size() == 4) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::lmul4(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::lmac4(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::lmsc4(params...); };
        }
        else if constexpr (Acc::size() == 2) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::lmul2(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::lmac2(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::lmsc2(params...); };
        }
    }
    else if constexpr (Acc::accum_bits() == 48) {
        if constexpr (Acc::size() == 16) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul16(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac16(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc16(params...); };
        }
        else if constexpr (Acc::size() == 8) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul8(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac8(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc8(params...); };
        }
        else if constexpr (Acc::size() == 4) {
            if      constexpr (!WithInputAccum)          return [](auto &&... params)  { return ::mul4(params...); };
            else if constexpr (Op == Operation::Acc_Add) return [](auto &&... params)  { return ::mac4(params...); };
            else if constexpr (Op == Operation::Acc_Sub) return [](auto &&... params)  { return ::msc4(params...); };
        }
    }
}

template <unsigned Lanes, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<48, Lanes, 16, int16, N_Coeff, 16, int16, Op, Step>
{
    static constexpr unsigned AccumBits = 48;

    using           T_Coeff = int16;
    using            T_Data = int16;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    static constexpr std::pair<unsigned, unsigned> get_coeff_start_offset(unsigned coeff_start)
    {
        if constexpr (N_Coeff <= 8)  {
            return std::make_pair(coeff_start, 0x00000000);
        }
        else if constexpr (N_Coeff == 16) {
            unsigned offset = 0;

            for (unsigned i = 0; i < 8; ++i)
                offset = (offset << 4) | coeff_start;

            return std::make_pair(0, offset);
        }
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        std::pair<unsigned, unsigned> ctrl = get_coeff_start_offset(coeff_start);

        accum_type<Lanes> ret;

        constexpr unsigned num_mul = Lanes < 32? 1 : Lanes / 16;
        constexpr unsigned off_lo  = Lanes < 32?  0x73727170 : 0xf3f2f1f0;
        constexpr unsigned off_hi  = Lanes < 32?  0x77767574 : 0xf7f6f5f4;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                        concat_vector(x.template grow_extract<16>(idx), y.template grow_extract<16>(idx)), 0,          off_lo,      off_hi,      0x3120,
                                        coeff,                                                                             ctrl.first, ctrl.second, ctrl.second, Step);
            ret.insert(idx, tmp.template extract<(Lanes < 16? Lanes : 16)>(0));
        });

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        std::pair<unsigned, unsigned> ctrl = get_coeff_start_offset(coeff_start);

        accum_type<Lanes> ret;

        constexpr unsigned num_mul = Lanes < 32? 1 : Lanes / 16;
        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                        ::concat(v.template grow_extract<16>(idx), *(v16int16 *)ZERO), 0,          0x73727170,  0x77767574,  0x3120, 
                                        coeff,                                                         ctrl.first, ctrl.second, ctrl.second, Step);
            ret.insert(idx, tmp.template extract<(Lanes < 16? Lanes : 16)>(0));
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<48, Lanes, 32, int32, N_Coeff, 16, int16, Op, Step>
{
    static constexpr unsigned AccumBits = 48;

    using           T_Coeff = int32;
    using            T_Data = int16;
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul8_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<8>>();
        constexpr auto mul_op  = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        constexpr auto mac_op  = get_mul_op<               true, Op, accum_type<16>>();

        accum_type<Lanes> ret;

        if constexpr (Lanes == 8) {
            REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

            ret = mul8_op( acc...,
                              concat_vector(x.template grow<16>(), y.template grow<16>()), 0,           0x76543210, 16, 
                              coeff,                                                       coeff_start, 0x00000000, Step);
        }
        else if constexpr (Lanes == 16) {
            if (!chess_const(coeff_start)) {
                ret = mul_op(acc...,
                             coeff.template grow<32>(), coeff_start, 0x00000000, 0x00000000,
                             x,                         0,           0x76543210, 0xfedcba98);
                ret = mac_op(ret,
                             coeff.template grow<32>(), coeff_start + Step, 0x00000000, 0x00000000,
                             y,                         0,                  0x76543210, 0xfedcba98);
            }
            else { 
                ret = mul_op(acc...,
                             x.template grow<32>(),                               0,                  0x76543210, 0xfedcba98,
                             coeff.template extract<8>(coeff_start / 8),          coeff_start,        0x00000000, 0x00000000);
                ret = mac_op(ret,
                             y.template grow<32>(),                               0,                  0x76543210, 0xfedcba98,
                             coeff.template extract<8>((coeff_start + Step) / 8), coeff_start + Step, 0x00000000, 0x00000000);
            }
        }
        else if constexpr (Lanes == 32) {
            REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

            accum_type<16> acc1, acc2;

            acc1 = mul_op(acc.template extract<16>(0)...,
                          x,     0,                  0x76543210, 0xfedcba98,
                          coeff, coeff_start,        0x00000000, 0x00000000);

            acc2 = mul_op(acc.template extract<16>(1)...,
                          x,     16,                 0x76543210, 0xfedcba98,
                          coeff, coeff_start,        0x00000000, 0x00000000);

            acc1 = mac_op(acc1,
                          y,     0,                  0x76543210, 0xfedcba98,
                          coeff, coeff_start + Step, 0x00000000, 0x00000000);

            acc2 = mac_op(acc2,
                          y,     16,                 0x76543210, 0xfedcba98,
                          coeff, coeff_start + Step, 0x00000000, 0x00000000);

            ret.insert(0, acc1);
            ret.insert(1, acc2);
        }

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;

        constexpr unsigned num_mul = Lanes < 32? 1 : Lanes / 16;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<16> tmp = mul_op(acc.template grow_extract<16>(idx)...,
                                        v.template grow<32>(), 16 * idx,    0x76543210, 0xfedcba98,
                                        coeff,                 coeff_start, 0x00000000, 0x00000000);
            ret.insert(idx, tmp.template extract<(Lanes < 16? Lanes : 16)>(0));
        });
        
        return ret;
    }
};

template <unsigned Lanes, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<48, Lanes, 16, int16, N_Coeff, 32, int32, Op, Step>
{
    static constexpr unsigned AccumBits = 48;

    using           T_Coeff = int16;
    using            T_Data = int32;

    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul8_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<8>>();
        constexpr auto  mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        constexpr auto  mac_op = get_mul_op<               true, Op, accum_type<16>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;

        if constexpr (Lanes == 8) {
            ret = mul8_op(acc...,
                          concat_vector(x, y), 0,           0x76543210, 0xfedcba98,
                          coeff,               coeff_start, 0x00000000, 1);
        }
        else if constexpr (Lanes == 16 || Lanes == 32) {
            constexpr unsigned num_mul = Lanes < 32? 1 : Lanes / 16;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<16> 
                tmp = mul_op(acc.template extract<16>(idx)...,
                             x.template extract<16>(idx), 0,                  0x76543210, 0xfedcba98,
                             coeff,                       coeff_start,        0x00000000, 0x00000000);
                tmp = mac_op(tmp,
                             y.template extract<16>(idx), 0,                  0x76543210, 0xfedcba98,
                             coeff,                       coeff_start + Step, 0x00000000, 0x00000000);
                ret.insert(idx, tmp);
            });
        }
        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul8_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<8>>();
        constexpr auto  mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<16>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");
        
        accum_type<Lanes> ret;

        if constexpr (Lanes == 8) {
            accum<accum_tag, 8> acc1;

            acc1 = mul8_op(acc...,
                         ::concat(v, *(v8int32 *)ZERO), 0,           0x76543210, 0xfedcba98,
                           coeff,                       coeff_start, 0x00000000, 0x00000000);

            ret = acc1.template extract<Lanes>(0);
        }
        else if constexpr (Lanes == 16 || Lanes == 32) {
            constexpr unsigned num_mul = Lanes < 32? 1 : Lanes / 16;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<16> tmp = mul_op(acc.template extract<16>(idx)...,
                                            v.template extract<16>(idx), 16 * idx,    0x76543210, 0xfedcba98,
                                            coeff,                       coeff_start, 0x00000000, 0x00000000);
                ret.insert(idx, tmp);
            });
        }

        return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, Operation Op, typename T_Data, int Step>
struct consume_terms<AccumBits, Lanes, 32, T_Coeff, N_Coeff, 32, T_Data, Op, Step>
{
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto  mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<8>>();
        constexpr auto  mac_op = get_mul_op<               true, Op, accum_type<8>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;

        if constexpr (Lanes == 4) {
            if constexpr(std::is_same_v<T_Coeff, float>) {
                accum_type<8> tmp;

                tmp = mul_op(acc...,
                             concat_vector(x, y).template grow<16>(), 0,                0x76543210,
                             coeff,                                   coeff_start,      0x00000000);
                tmp = mac_op(tmp,
                             concat_vector(x, y).template grow<16>(), 4,                0x76543210,
                             coeff,                                   coeff_start+Step, 0x00000000);

                ret = tmp.template extract<4>(0);
            }
            else {
                constexpr auto mul4_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();

                ret = mul4_op(acc...,
                              concat_vector(x.template grow<8>(), y.template grow<8>()), 0,           0x3210, 8,
                              coeff,                                                     coeff_start, 0x0000, Step);
            }
        }
        else if constexpr (Lanes == 8) {
            ret = mul_op(std::forward<Accum>(acc)...,
                         concat_vector(x, y), 0,                  0x76543210,
                         coeff,               coeff_start,        0x00000000);
            ret = mac_op(ret,
                         concat_vector(x, y), 8,                  0x76543210,
                         coeff,               coeff_start + Step, 0x00000000);
        }
        else if constexpr (Lanes == 16 || Lanes == 32) {
            constexpr unsigned num_mul = Lanes / 8;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<8> tmp;
                tmp = mul_op(acc.template extract<8>(idx)...,
                             x.template extract<16>(idx / 2), (idx % 2) * 8,      0x76543210,
                             coeff,                           coeff_start,        0x00000000);
                tmp = mac_op(tmp,
                             y.template extract<16>(idx / 2), (idx % 2) * 8,      0x76543210, 
                             coeff,                           coeff_start + Step, 0x00000000);
                ret.insert(idx, tmp);
            });
        }

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<8>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned num_mul = Lanes < 8? 1 : Lanes / 8;

        accum_type<Lanes> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<8> tmp = mul_op(acc.template grow_extract<8>(idx)...,
                                       v.template grow_extract<16>(idx / 2), (idx % 2) * 8,  0x76543210,  
                                       coeff,                                coeff_start,    0x00000000);
            ret.insert(idx, tmp.template extract<(Lanes < 8? Lanes : 8)>(0));
        });

        return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<AccumBits, Lanes, 64, T_Coeff, N_Coeff, 32, T_Data, Op, Step>
{
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();
        constexpr auto mac_op = get_mul_op<               true, Op, accum_type<4>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        if constexpr (Lanes == 4) {
            accum_type<Lanes> ret;

            ret = mul_op(acc...,
                         concat_vector(x, y).template grow<16>(), 0,                 0x3210,
                         coeff,                                   coeff_start,       0x0000);
            ret = mac_op(ret,
                         concat_vector(x, y).template grow<16>(), 4,                  0x3210,
                         coeff,                                   coeff_start + Step, 0x0000);

            return ret;
        }
        else if constexpr (Lanes == 8) {
            accum_type<4> acc1, acc2;

            acc1 = mul_op(acc.template extract<4>(0)...,
                          concat_vector(x, y),     0, 0x3210,
                          coeff,         coeff_start, 0x0000);
            acc2 = mul_op(acc.template extract<4>(1)...,
                          concat_vector(x, y),     4, 0x3210,
                          coeff,         coeff_start, 0x0000);
            acc1 = mac_op(acc1,
                          concat_vector(x, y),     8, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);
            acc2 = mac_op(acc2,
                          concat_vector(x, y),    12, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);

            return concat_accum(acc1, acc2);
        }
        else if constexpr (Lanes == 16) {
            accum_type<4> acc1, acc2, acc3, acc4;

            acc1 = mul_op(acc.template extract<4>(0)...,
                          x,                       0, 0x3210,
                          coeff,         coeff_start, 0x0000);
            acc2 = mul_op(acc.template extract<4>(1)...,
                          x,                       4, 0x3210,
                          coeff,         coeff_start, 0x0000);
            acc3 = mul_op(acc.template extract<4>(2)...,
                          x,                       8, 0x3210,
                          coeff,         coeff_start, 0x0000);
            acc4 = mul_op(acc.template extract<4>(3)...,
                          x,                      12, 0x3210,
                          coeff,         coeff_start, 0x0000);

            acc1 = mac_op(acc1,
                          y,                       0, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);
            acc2 = mac_op(acc2,
                          y,                       4, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);
            acc3 = mac_op(acc3,
                          y,                       8, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);
            acc4 = mac_op(acc4,
                          y,                      12, 0x3210,
                          coeff,  coeff_start + Step, 0x0000);

            return concat_accum(acc1, acc2, acc3, acc4);
        }
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");
        constexpr unsigned num_mul = Lanes < 8? 1 : Lanes / 4;

        accum_type<Lanes> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template extract<4>(idx)...,
                                       v.template grow<16>(), 4 * idx,     0x3210,  
                                       coeff,                 coeff_start, 0x0000);
            ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, Operation Op, typename T_Data, int Step>
struct consume_terms<AccumBits, Lanes, 32, T_Coeff, N_Coeff, 64, T_Data, Op, Step>
{
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();
        constexpr auto mac_op = get_mul_op<               true, Op, accum_type<4>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;
        if constexpr (Lanes == 4) {
            ret = mul_op(acc...,
                         concat_vector(x, y), 0,                  0x3210,
                         coeff,               coeff_start,        0x0000);
            ret = mac_op(ret,
                         concat_vector(x, y), 4,                  0x3210,
                         coeff,               coeff_start + Step, 0x0000);

            return ret;
        }
        else if constexpr (Lanes == 8 || Lanes == 16) {
            constexpr unsigned num_mul = Lanes / 4;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<4> tmp;

                tmp = mul_op(acc.template extract<4>(idx)...,
                             x.template grow<16>(), 4 * idx,            0x3210,
                             coeff,                 coeff_start,        0x0000);
                tmp = mac_op(tmp,
                             y.template grow<16>(), 4 * idx,            0x3210, 
                             coeff,                 coeff_start + Step, 0x0000);

                ret.insert(idx, tmp);
            });
        }

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned num_mul = Lanes < 8? 1 : Lanes / 4;

        accum_type<Lanes> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template extract<4>(idx)...,
                                       v.template grow<16>(), 4 * idx,     0x3210,
                                       coeff,                 coeff_start, 0x0000);
            ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, typename T_Coeff, unsigned N_Coeff, typename T_Data, Operation Op, int Step>
struct consume_terms<AccumBits, Lanes, 64, T_Coeff, N_Coeff, 64, T_Data, Op, Step>
{
    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<2>>();
        constexpr auto mac_op = get_mul_op<               true, Op, accum_type<2>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;

        if constexpr (Lanes == 2) {
            ret = mul_op(std::forward<Accum>(acc)...,
                         concat_vector(x, y).template grow<16>(), 0,                  0x10,
                         coeff,                                   coeff_start,        0x00);
            ret = mac_op(ret,
                         concat_vector(x, y).template grow<16>(), 2,                  0x10,
                         coeff,                                   coeff_start + Step, 0x00);
        }
        else if constexpr (Lanes == 4) {
            accum_type<2> acc1, acc2;

            acc1 = mul_op(std::forward<Accum>(acc.template extract<2>(0))...,
                          concat_vector(x, y), 0,                  0x10,
                          coeff,               coeff_start,        0x00);
            acc2 = mul_op(std::forward<Accum>(acc.template extract<2>(1))...,
                          concat_vector(x, y), 2,                  0x10,
                          coeff,               coeff_start,        0x00);
            acc1 = mac_op(acc1,
                          concat_vector(x, y), 4,                  0x10,
                          coeff,               coeff_start + Step, 0x00);
            acc2 = mac_op(acc2,
                          concat_vector(x, y), 6,                  0x10,
                          coeff,               coeff_start + Step, 0x00);

            ret.insert(0, acc1);
            ret.insert(1, acc2);
        }
        else if constexpr (Lanes == 8 || Lanes == 16) {
            constexpr unsigned num_mul = Lanes / 2;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<2> tmp;

                tmp = mul_op(acc.template extract<2>(idx)...,
                             x.template grow_extract<8>(idx / 4), (idx % 4) * 2,      0x10,
                             coeff,                               coeff_start,        0x00);
                tmp = mac_op(tmp,
                             y.template grow_extract<8>(idx / 4), (idx % 4) * 2,      0x10,
                             coeff,                               coeff_start + Step, 0x00);

                ret.insert(idx, tmp);
            });
        }

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<2>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned num_mul = Lanes < 4? 1 : Lanes / 2;

        accum_type<Lanes> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<2> tmp = mul_op(acc.template grow_extract<2>(idx)...,
                                       v.template grow_extract<8>(idx / 4), (idx % 4) * 2, 0x10,
                                       coeff,                               coeff_start,   0x00);
            ret.insert(idx, tmp);
        });

        return ret;
    }
};

template <unsigned AccumBits, unsigned Lanes, unsigned N_Coeff, Operation Op, int Step>
struct consume_terms<AccumBits, Lanes, 64, cfloat, N_Coeff, 64, cfloat, Op, Step>
{
    using           T_Coeff = cfloat;
    using            T_Data = cfloat;

    using coeff_vector_type = vector<T_Coeff, N_Coeff>;
    using  data_vector_type = vector<T_Data, Lanes>;
    using         accum_tag = accum_tag_for_mul_types<T_Coeff, T_Data, AccumBits>;
    template <unsigned Elems>
    using        accum_type = accum<accum_tag, Elems>;

    template <typename... Accum>
    static accum_type<Lanes> consume_2(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &x, const data_vector_type &y, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<2>>();
        constexpr auto mac_op = get_mul_op<               true, Op, accum_type<2>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        accum_type<Lanes> ret;

        if constexpr (Lanes == 2) {
            accum_type<4> tmp;

            tmp = mul_op(acc.template grow<4>()..., 
                         x.template grow<8>(), 0,           0x10,
                         coeff,                coeff_start, 0x00);
            tmp = mac_op(tmp,
                         y.template grow<8>(), 0,                  0x10,
                         coeff,                coeff_start + Step, 0x00);

            ret = tmp.template extract<2>(0);
        }
        else if constexpr (Lanes == 4) { 
            ret = mul_op(acc...,
                         concat_vector(x, y), 0,                  0x3210,
                         coeff,               coeff_start,        0x0000);
            ret = mac_op(ret,
                         concat_vector(x, y), 4, 0x3210,
                         coeff,               coeff_start + Step, 0x0000);
        }
        else if constexpr (Lanes == 8 || Lanes == 16) {
            constexpr unsigned num_mul = Lanes / 4;

            utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
                accum_type<4> tmp;

                tmp = mul_op(acc.template grow_extract<4>(idx)...,
                             x,     4 * idx,            0x3210,
                             coeff, coeff_start,        0x0000);
                tmp = mac_op(tmp,
                             y,     4 * idx,            0x3210,
                             coeff, coeff_start + Step, 0x0000);

                ret.insert(idx, tmp);
            });
        }

        return ret;
    }

    template <typename... Accum>
    static accum_type<Lanes> consume_1(const coeff_vector_type &coeff, unsigned coeff_start, const data_vector_type &v, Accum &&... acc)
    {
        constexpr auto mul_op = get_mul_op<sizeof...(acc) == 1, Op, accum_type<4>>();
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        constexpr unsigned num_mul = Lanes < 8? 1 : Lanes / 4;

        accum_type<Lanes> ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            accum_type<4> tmp = mul_op(acc.template grow_extract<4>(idx)...,
                                       v.template grow_extract<8>(idx / 2), (idx % 2) * 4, 0x3210,
                                       coeff,                               coeff_start,   0x0000);
            ret.insert(idx, tmp.template extract<(Lanes < 4? Lanes : 4)>(0));
        });

        return ret;
    }
};

}

#endif
