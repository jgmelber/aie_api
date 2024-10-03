// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_UCT_ACC48_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_UCT_ACC48_HPP__

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType, unsigned Cols>
static constexpr auto sliding_mul_sym_uct_acc48_get_mul_op()
{
    constexpr unsigned num_complex = get_num_complex<CoeffType, DataType>();

    if constexpr (Lanes == 4) {
        if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul4_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul4_sym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul4_sym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac4_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac4_sym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc4_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc4_sym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_antisym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_antisym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_antisym_uct_cc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct_cn(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct_c(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct_nc(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc4_antisym_uct_cc(args...); };
    }
    else if constexpr (Lanes == 8) {
        if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul8_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul8_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac8_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc8_sym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul8_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul8_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac8_antisym_uct(args...); };
        else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc8_antisym_uct(args...); };
    }
}

template <unsigned Lanes, unsigned DataStep, unsigned N_Data>
constexpr unsigned grow_offsets_for_sym_uct_acc48(unsigned offset)
{
    unsigned ret = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = ((offset & 0xf) * DataStep) % 16;

        ret |= val << (i * 4);

        offset >>= 4;
    }

    return ret;
}

template <unsigned Lanes, unsigned DataStep, unsigned N_Data>
constexpr auto offsets_for_sym_uct_acc48()
{
    if constexpr (Lanes == 4) {
        return grow_offsets_for_sym_uct_acc48<Lanes, DataStep, N_Data>(0x3210);
    }
    else if constexpr (Lanes == 8) {
        return grow_offsets_for_sym_uct_acc48<Lanes, DataStep, N_Data>(0x76543210);
    }
    else if constexpr (Lanes == 16) {
        return std::make_pair(grow_offsets_for_sym_uct_acc48<8, DataStep, N_Data>(0x76543210),
                              grow_offsets_for_sym_uct_acc48<8, DataStep, N_Data>(0xfedcba98));
    }
}

template <typename Data, typename Coeff>
constexpr unsigned columns_per_sym_mul_uct_acc48(unsigned lanes)
{
    if      constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int32>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int32>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint16>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data, cint32> && std::is_same_v<Coeff, cint16>) return  4 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint32>) return  4 / lanes;
}

// This uses 16c*32 and 16c*16c intrinsics
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl<Lanes, Points, CoeffStep, DataStep, 48, 32, 32, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned           lanes = Lanes / 2;
    static constexpr unsigned   lanes_per_mul = lanes;
    static constexpr unsigned columns_per_mul = columns_per_sym_mul_uct_acc48<data_type, coeff_type>(lanes);
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 8;

        const unsigned start_x = data_start                           + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = data_start + (Points - 1) * DataStep - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start                          + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                      data.template grow_replicate<data_elems>(),  start_x, offsets, DataStep,
                                                                   start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               data.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,     offsets, DataStep,
                                                                                            start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<data_type, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 8;

        const unsigned start_x = ldata_start + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = rdata_start - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                     ldata.template grow_replicate<data_elems>(),  start_x, offsets, DataStep,
                     rdata.template grow_replicate<data_elems>(),  start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                              ldata.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,     offsets, DataStep,
                                              rdata.template grow_replicate<data_elems>(),  start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }
};

// This uses 16*16 intrinsics
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl<Lanes, Points, CoeffStep, DataStep, 48, 16, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned           lanes = Lanes / 2;
    static constexpr unsigned   lanes_per_mul = lanes;
    static constexpr unsigned         offsets = 0x03020100;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 64u);
        constexpr unsigned coeff_elems = 16;

        unsigned       start_x = data_start                           + columns_per_mul * (num_mul - 1) * DataStep;
        unsigned       start_y = data_start + (Points - 2) * DataStep - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start                          + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        const unsigned odd = data_start & 0x1;
        const int square_x = 0x2110 + odd*(0x1111);
        start_x -= odd;
        const int square_y = 0x2312 - (!odd)*(0x1111);
        start_y -= odd;

        accum_type ret;

        ret = mul_op(   acc...,
                      data.template grow_replicate<data_elems>(),  start_x, offsets,    2, square_x,
                                                                   start_y,                square_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               data.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets,    2, square_x,
                                                                                            start_y + columns_per_mul * idx * DataStep,                 square_y,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<data_type, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 32u);
        constexpr unsigned coeff_elems = 16;

        unsigned       start_x = ldata_start + columns_per_mul * (num_mul - 1) * DataStep;
        unsigned       start_y = rdata_start - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        const unsigned odd_x = ldata_start & 0x1;
        const int square_x = 0x2110 + odd_x*(0x1111);
        start_x -= odd_x;
        const unsigned odd_y = rdata_start & 0x1;
        const int square_y = 0x2312 - odd_y*(0x1111);
        start_y -= 2 - odd_y;

        accum_type ret;

        ret = mul_op(   acc...,
                     ldata.template grow_replicate<data_elems>(),  start_x, offsets,    2, square_x,
                     rdata.template grow_replicate<data_elems>(),  start_y,                square_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                              ldata.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets,    2, square_x,
                                              rdata.template grow_replicate<data_elems>(),  start_y + columns_per_mul * idx * DataStep,                 square_y,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }
};

// This uses 16c*16 intrinsics
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl<Lanes, Points, CoeffStep, DataStep, 48, 16, 32, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned           lanes = Lanes / 2;
    static constexpr unsigned   lanes_per_mul = lanes;
    static constexpr unsigned columns_per_mul = columns_per_sym_mul_uct_acc48<data_type, coeff_type>(lanes);
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 16;

        const unsigned start_x = data_start                           + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = data_start + (Points - 1) * DataStep - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start                          + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                      data.template grow_replicate<data_elems>(),  start_x, offsets, DataStep,
                                                                   start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               data.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets, DataStep,
                                                                                            start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<data_type, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 16;

        const unsigned start_x = ldata_start + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = rdata_start - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;
        ret = mul_op(   acc...,
                      ldata.template grow_replicate<data_elems>(), start_x, offsets, DataStep,
                      rdata.template grow_replicate<data_elems>(), start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               ldata.template grow_replicate<data_elems>(), start_x - columns_per_mul * idx * DataStep,  offsets, DataStep,
                                               rdata.template grow_replicate<data_elems>(), start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }
};

// This uses 16b*32b intrinsics
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl<Lanes, Points, CoeffStep, DataStep, 48, 32, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned           lanes = Lanes / 2;
    static constexpr unsigned   lanes_per_mul = lanes;
    static constexpr unsigned columns_per_mul = columns_per_sym_mul_uct_acc48<data_type, coeff_type>(lanes);
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 64u);
        constexpr unsigned coeff_elems = 8;

        const unsigned start_x = data_start                           + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = data_start + (Points - 1) * DataStep - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start                          + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                      data.template grow_replicate<data_elems>(),  start_x, offsets, DataStep,
                                                                   start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               data.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets, DataStep,
                                                                                            start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<data_type, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 32u);
        constexpr unsigned coeff_elems = 8;

        const unsigned start_x = ldata_start + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = rdata_start - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        const unsigned uct_col = (Points / 2 - 1) % columns_per_mul;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                     ldata.template grow_replicate<data_elems>(),  start_x, offsets, DataStep,
                     rdata.template grow_replicate<data_elems>(),  start_y,
                                                                   uct_col, uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                              ldata.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets, DataStep,
                                              rdata.template grow_replicate<data_elems>(),  start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep));
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, typename CoeffType, typename DataType>
struct sliding_mul_sym_uct_bits_impl<Lanes, Points, CoeffStep, DataStep, 48, 64, 32, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned           lanes = Lanes / 2;
    static constexpr unsigned   lanes_per_mul = lanes;
    static constexpr unsigned columns_per_mul = columns_per_sym_mul_uct_acc48<data_type, coeff_type>(lanes);
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 4;

        const unsigned start_x = data_start                           + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = data_start + (Points - 1) * DataStep - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start                          + columns_per_mul * (num_mul - 1) * CoeffStep;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                      data.template grow_replicate<data_elems>(),  start_x, offsets,
                                                                   start_y,
                                                                   uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                               data.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets,
                                                                                            start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000));
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned ldata_start,
                          const vector<data_type, N_Data> &rdata,
                          unsigned rdata_start,
                          unsigned uct_shift,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_uct_acc48<lanes_per_mul, DataStep, N_Data>();

        constexpr auto mul_op = sliding_mul_sym_uct_acc48_get_mul_op<             MulOp, lanes, CoeffType, DataType, columns_per_mul>();
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op    <add_to_op<MulOp>(), lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = std::max(N_Data, 16u);
        constexpr unsigned coeff_elems = 4;

        const unsigned start_x = ldata_start + columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_y = rdata_start - columns_per_mul * (num_mul - 1) * DataStep;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        ret = mul_op(   acc...,
                     ldata.template grow_replicate<data_elems>(),  start_x, offsets,
                     rdata.template grow_replicate<data_elems>(),  start_y,
                                                                   uct_shift,
                     coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000);

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            ret.template insert<lanes> (0,
                                        mac_op(ret.template extract<lanes>(0),
                                              ldata.template grow_replicate<data_elems>(),  start_x - columns_per_mul * idx * DataStep,  offsets,
                                              rdata.template grow_replicate<data_elems>(),  start_y + columns_per_mul * idx * DataStep,
                                              coeff.template grow_replicate<coeff_elems>(), start_z - columns_per_mul * idx * CoeffStep, 0x00000000));
        });

        return ret;
    }
};

}

#endif
