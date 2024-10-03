// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC80_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_ACC80_HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc80_get_mul_op()
{
    constexpr unsigned num_complex = get_num_complex<CoeffType, DataType>();

    if constexpr (Lanes == 2) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul2(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::lmul2_abs(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::lmul2_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul2_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul2_cn(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul2_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul2_nc(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul2_cc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul2(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::lnegmul2_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::lnegmul2_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul2_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul2_cn(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul2_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul2_nc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul2_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac2(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac2_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmac2_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac2_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac2_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac2_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac2_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac2_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc2(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc2_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmsc2_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc2_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc2_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc2_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc2_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc2_cc(args...); };
    }
    else if constexpr (Lanes == 4) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul4(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::lmul4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1Conj2)                     return [](auto &&... args) __aie_inline { return ::lmul4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::MulConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul4(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::lnegmul4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1Conj2)                  return [](auto &&... args) __aie_inline { return ::lnegmul4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac4(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmac4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac4_cc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc4(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc4_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1Conj2)                 return [](auto &&... args) __aie_inline { return ::lmsc4_abs_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_cn(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_nc(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc4_cc(args...); };
    }
    else if constexpr (Lanes == 8) {
        if constexpr (MulOp == MulMacroOp::Mul)                              return [](auto &&... args) __aie_inline { return ::lmul8(args...); };
        if constexpr (MulOp == MulMacroOp::MulAbs1)                          return [](auto &&... args) __aie_inline { return ::lmul8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)                           return [](auto &&... args) __aie_inline { return ::lnegmul8(args...); };
        if constexpr (MulOp == MulMacroOp::NegMulAbs1)                       return [](auto &&... args) __aie_inline { return ::lnegmul8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul)                          return [](auto &&... args) __aie_inline { return ::lmac8(args...); };
        if constexpr (MulOp == MulMacroOp::Add_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmac8_abs(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul)                          return [](auto &&... args) __aie_inline { return ::lmsc8(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_MulAbs1)                      return [](auto &&... args) __aie_inline { return ::lmsc8_abs(args...); };
    }
}

template <typename Coeff, typename Data>
constexpr unsigned muls_per_mul_acc80()
{
    if constexpr (type_bits_v<Coeff> == 16 && type_bits_v<Data> == 32) { return 16; }
    if constexpr (type_bits_v<Coeff> == 32 && type_bits_v<Data> == 16) { return 16; }
    if constexpr (type_bits_v<Coeff> == 32 && type_bits_v<Data> == 32) { return  8; }
    if constexpr (type_bits_v<Coeff> == 64 && type_bits_v<Data> == 32) { return  4; }
    if constexpr (type_bits_v<Coeff> == 32 && type_bits_v<Data> == 64) { return  4; }
    if constexpr (type_bits_v<Coeff> == 16 && type_bits_v<Data> == 64) { return  8; }
    if constexpr (type_bits_v<Coeff> == 64 && type_bits_v<Data> == 16) { return  8; }
    if constexpr (type_bits_v<Coeff> == 64 && type_bits_v<Data> == 64) { return  2; }
}

template <typename Coeff, typename Data>
constexpr unsigned lanes_per_mul_acc80(unsigned lanes, unsigned points)
{
    constexpr unsigned muls_per_mul = muls_per_mul_acc80<Coeff, Data>();

    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int16>) { return 8;                                                         }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data,  int32>) { return 8;                                                         }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int32>) { return ((points % (muls_per_mul / 4)) == 0) || lanes == 4? 4 : 8; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int32>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint16>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint16>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint32>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint32>) { return ((points % (muls_per_mul / 2)) == 0) || lanes == 2? 2 : 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int32>) { return ((points % (muls_per_mul / 2)) == 0) || lanes == 2? 2 : 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int16>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint32>) { return 4;                                                         }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint32>) { return 2;                                                         }
}

template <unsigned Lanes, int DataStepY, unsigned N_Data>
constexpr unsigned grow_offsets_for_acc80(unsigned offset)
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
constexpr auto offsets_for_acc80()
{
    if      constexpr (Lanes == 2) return grow_offsets_for_acc80<Lanes, DataStepY, N_Data>(0x10);
    else if constexpr (Lanes == 4) return grow_offsets_for_acc80<Lanes, DataStepY, N_Data>(0x3210);
    else if constexpr (Lanes == 8) return grow_offsets_for_acc80<Lanes, DataStepY, N_Data>(0x76543210);
}

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits,
    unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, CoeffStep, DataStepX, DataStepY, 80, CoeffTypeBits, DataTypeBits, CoeffType,
    DataType>
{
    using coeff_type = CoeffType;
    using  data_type = DataType;
    using  accum_tag = accum_tag_for_mul_types<coeff_type, data_type, 80>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc80<coeff_type, data_type>();
    static constexpr unsigned   lanes_per_mul = lanes_per_mul_acc80<coeff_type, data_type>(Lanes, Points);
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
        constexpr unsigned offsets = offsets_for_acc80<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_acc80_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType>();
        constexpr auto mul_op = sliding_mul_acc80_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType>();

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
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY, offsets,
                             coeff.template grow_replicate<coeff_elems>(), coeff_start,                                     0x00000000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op(  tmp,
                                  data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx * DataStepX, offsets,    DataStepX,
                                 coeff.template grow_replicate<coeff_elems>(), coeff_start +                                     columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
                }
                else {
                    tmp = mac_op(  tmp,
                                  data.template grow_replicate<data_elems>(),   data_start + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx * DataStepX, offsets,
                                 coeff.template grow_replicate<coeff_elems>(), coeff_start +                                     columns_per_mul * idx * CoeffStep, 0x00000000);
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif
