// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC48_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC48_HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <typename CoeffType, typename DataType>
static constexpr auto get_num_complex()
{
    unsigned num_complex = 0;

    if constexpr (is_complex_v<CoeffType>)
        ++num_complex;
    if constexpr (is_complex_v<DataType>)
        ++num_complex;

    return num_complex;
}

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_op()
{
    constexpr unsigned num_complex = get_num_complex<CoeffType, DataType>();

    if constexpr (Lanes == 4) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul4(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::mul4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::mul4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul4(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::negmul4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::negmul4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac4(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::mac4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::mac4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc4(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::msc4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::msc4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc4_cc(args...); };
    }
    else if constexpr (Lanes == 8) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul8(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::mul8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::mul8_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_cn(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_nc(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul8_cc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul8(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::negmul8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::negmul8_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_cn(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_nc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul8_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac8(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::mac8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::mac8_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac8_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc8(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::msc8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::msc8_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc8_cc(args...); };
    }
    else if constexpr (Lanes == 16) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::mul16(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::mul16_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::negmul16(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::negmul16_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::mac16(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::mac16_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::msc16(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::msc16_abs(args...); };
    }
}

template <typename Data, typename Coeff>
constexpr unsigned columns_per_mul_acc48(unsigned lanes)
{
    if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int16>) return 32 / lanes;
    if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff, cint16>) return 16 / lanes;
    if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    if constexpr (std::is_same_v<Data,  int32> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int32>) return 16 / lanes;
    if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint16>) return  8 / lanes;
    if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int32>) return  8 / lanes;
    if constexpr (std::is_same_v<Data,  int32> && std::is_same_v<Coeff, cint16>) return  8 / lanes;
    if constexpr (std::is_same_v<Data, cint32> && std::is_same_v<Coeff,  int16>) return  8 / lanes;
    if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff, cint32>) return  8 / lanes;
    if constexpr (std::is_same_v<Data, cint32> && std::is_same_v<Coeff, cint16>) return  4 / lanes;
    if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint32>) return  4 / lanes;
}

template <typename Coeff, typename Data>
constexpr unsigned muls_per_mul_acc48()
{
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int16>) { return 16; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data,  int32>) { return 16; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint16>) { return 16; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int16>) { return 16; }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint16>) { return 8;  }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int32>) { return 8;  }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint16>) { return 8;  }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int16>) { return 8;  }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint32>) { return 8;  }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint16>) { return 4;  }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint32>) { return 4;  }
}

template <typename Coeff, typename Data>
constexpr unsigned lanes_per_mul_acc48(unsigned lanes, unsigned points)
{
    constexpr unsigned muls_per_mul = muls_per_mul_acc48<Coeff, Data>();

    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int16>) { return ((points % (muls_per_mul / 8)) == 0) || lanes == 8? 8 : 16; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data,  int32>) { return ((points % (muls_per_mul / 8)) == 0) || lanes == 8? 8 : 16; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint16>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int16>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint16>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int32>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint16>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int16>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint32>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 :  8; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint16>) { return 4;                                    }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint32>) { return 4;                                    }
}


template <unsigned Lanes, int DataStepY, unsigned N_Data>
constexpr unsigned grow_offsets_for_acc48(unsigned offset)
{
    unsigned ret = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = ((offset & 0xf) * DataStepY) % 16;

        ret |= val << (i * 4);

        offset >>= 4;
    }

    return ret;
}

template <unsigned Lanes, int DataStepY, unsigned N_Data>
constexpr auto offsets_for_acc48()
{
    if constexpr (Lanes == 4) {
        return grow_offsets_for_acc48<Lanes, DataStepY, N_Data>(0x3210);
    }
    else if constexpr (Lanes == 8) {
        return grow_offsets_for_acc48<Lanes, DataStepY, N_Data>(0x76543210);
    }
    else if constexpr (Lanes == 16) {
        return std::make_pair(grow_offsets_for_acc48<8, DataStepY, N_Data>(0x76543210),
                              grow_offsets_for_acc48<8, DataStepY, N_Data>(0xfedcba98));
    }
}

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, CoeffStep, DataStepX, DataStepY, 48, 16, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned      offsets_lo = 0x03020100;
    static constexpr unsigned      offsets_hi = 0x07060504;
    static constexpr unsigned columns_per_mul = columns_per_mul_acc48<data_type, coeff_type>(Lanes);
    static constexpr unsigned   lanes_per_mul = Lanes;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(DataStepX <= 1, "DataStepX > 1 is not supported at the moment");
    static_assert(DataStepY <= 1, "DataStepY > 1 is not supported at the moment");

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector<coeff_type, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<data_type, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), Lanes, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<             MulOp, Lanes, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 512u / type_bits_v<data_type>);

        accum_type ret;

        const unsigned odd = data_start & 0x1;
        const int start = data_start - odd;
        const int square = 0x2110 + odd*(0x1111);

        if constexpr (columns_per_mul > 2) {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),       start, offsets_lo,         2, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, CoeffStep);
        }
        else {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),       start, offsets_lo, offsets_hi, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, 0x00000000, CoeffStep);
        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            if constexpr (columns_per_mul > 2) {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),       start + columns_per_mul * idx,             offsets_lo, 2, square,
                             coeff.template grow_replicate<16>(),         coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
            }
            else {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),      start  + columns_per_mul * idx,             offsets_lo, offsets_hi, square,
                             coeff.template grow_replicate<16>(),         coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, 0x00000000, CoeffStep);
            }
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, typename CoeffType, typename DataType> requires(DataStepX > 1)
struct sliding_mul_bits_impl<Lanes, Points, CoeffStep, DataStepX, 1, 48, 16, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned      offsets_lo = 0x03020100 | (0x10101010u * unsigned(DataStepX / 2 - 1));
    static constexpr unsigned      offsets_hi = 0x07060504 | (0x10101010u * unsigned(DataStepX / 2 - 1));
    static constexpr unsigned columns_per_mul = columns_per_mul_acc48<data_type, coeff_type>(Lanes);
    static constexpr unsigned   lanes_per_mul = Lanes;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(DataStepX % 2 == 0);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector<coeff_type, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<data_type, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), Lanes, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<             MulOp, Lanes, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 512u / type_bits_v<data_type>);

        REQUIRES_MSG(data_start % 2 == 0, "data_start must be divisible by 2\n");

        accum_type ret;

        const int square = 0x3120;

        if constexpr (columns_per_mul > 2) {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  data_start, offsets_lo, 2 * DataStepX, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, CoeffStep);
        }
        else {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  data_start, offsets_lo, offsets_hi, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, 0x00000000, CoeffStep);
        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            if constexpr (columns_per_mul > 2) {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  data_start + columns_per_mul * idx * DataStepX, offsets_lo, 2 * DataStepX, square,
                             coeff.template grow_replicate<16>(),         coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
            }
            else {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  data_start + columns_per_mul * idx * DataStepX, offsets_lo, offsets_hi, square,
                             coeff.template grow_replicate<16>(),         coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, 0x00000000, CoeffStep);
            }
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, CoeffStep, 1, 2, 48, 16, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned      offsets_lo = 0x06040200;
    static constexpr unsigned      offsets_hi = 0x0e0c0a08;
    static constexpr unsigned columns_per_mul = columns_per_mul_acc48<data_type, coeff_type>(Lanes);
    static constexpr unsigned   lanes_per_mul = Lanes;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector<coeff_type, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<data_type, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), Lanes, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<             MulOp, Lanes, coeff_type, data_type>();

        constexpr unsigned data_elems = std::max(N_Data, 512u / type_bits_v<data_type>);

        REQUIRES_MSG(data_start % 2 == 0, "data_start must be divisible by 2\n");

        accum_type ret;

        const int square = 0x3210;

        if constexpr (columns_per_mul > 2) {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  data_start, offsets_lo, 2, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, CoeffStep);
        }
        else {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  data_start, offsets_lo, offsets_hi, square,
                         coeff.template grow_replicate<16>(),         coeff_start, 0x00000000, 0x00000000, CoeffStep);
        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            if constexpr (columns_per_mul > 2) {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  data_start + columns_per_mul * idx,             offsets_lo, 2, square,
                                 coeff.template grow_replicate<16>(),     coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
            }
            else {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  data_start + columns_per_mul * idx,             offsets_lo, offsets_hi, square,
                             coeff.template grow_replicate<16>(),         coeff_start + columns_per_mul * idx * CoeffStep, 0x00000000, 0x00000000, CoeffStep);
            }
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits,
    unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, CoeffStep, DataStepX, DataStepY, 48, CoeffTypeBits, DataTypeBits, CoeffType,
    DataType>
{
    using coeff_type = CoeffType;
    using  data_type = DataType;
    using  accum_tag = accum_tag_for_mul_types<coeff_type, data_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc48<coeff_type, data_type>();
    static constexpr unsigned   lanes_per_mul = lanes_per_mul_acc48<coeff_type, data_type>(Lanes, Points);
    static constexpr unsigned columns_per_mul = muls_per_mul / lanes_per_mul;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static auto run(const vector<coeff_type, N_Coeff> &coeff,
                    unsigned coeff_start,
                    bool coeff_sign,
                    const vector<data_type, N_Data> &data,
                    unsigned data_start,
                    bool data_sign,
                    const Acc &... acc)
    {
        static_assert(Points * Lanes % lanes_per_mul == 0);

        constexpr auto offsets = offsets_for_acc48<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType>();

        constexpr unsigned  data_elems = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, Lanes / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY, offsets,    DataStepX,
                             coeff.template grow_replicate<coeff_elems>(), coeff_start,                                     0x00000000, CoeffStep);
            }
            else {
                if constexpr (lanes_per_mul == 16) {
                    tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                  data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY, offsets.first, offsets.second,
                                 coeff.template grow_replicate<coeff_elems>(), coeff_start,                                     0x00000000,    0x00000000);
                }
                else {
                    tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                  data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY, offsets,
                                 coeff.template grow_replicate<coeff_elems>(), coeff_start,                                     0x00000000);
                }
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op(  tmp,
                                  data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx * DataStepX, offsets,    DataStepX,
                                 coeff.template grow_replicate<coeff_elems>(), coeff_start +                                     columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
                }
                else {
                    if constexpr (lanes_per_mul == 16) {
                        tmp = mac_op(  tmp,
                                      data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx * DataStepX, offsets.first, offsets.second,
                                     coeff.template grow_replicate<coeff_elems>(), coeff_start +                                     columns_per_mul * idx * CoeffStep, 0x00000000,    0x00000000);
                    }
                    else {
                        tmp = mac_op(  tmp,
                                      data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx * DataStepX, offsets,
                                     coeff.template grow_replicate<coeff_elems>(), coeff_start +                                     columns_per_mul * idx * CoeffStep, 0x00000000);
                    }
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif
