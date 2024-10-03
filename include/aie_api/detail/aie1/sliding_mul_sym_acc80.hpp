// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_ACC80_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_ACC80_HPP__

#include "sliding_mul_acc80.hpp"

#include "../accum.hpp"
#include "../vector.hpp"

//TODO: Test rdata/ldata variant, confirm center tap increment by idx_y is needed there

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType, unsigned Cols, bool CT>
static constexpr auto sliding_mul_sym_acc80_get_mul_op()
{
    constexpr unsigned num_complex = get_num_complex<CoeffType, DataType>();

    if constexpr (CT) {
        if constexpr (Cols == 1) {
            if constexpr (Lanes == 2) {
                if constexpr (MulOp == MulMacroOp::MulSym)                              return [](auto &&... args) __aie_inline { return ::lmul2(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul2_cn(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul2_nc(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul2_cc(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSym)                           return [](auto &&... args) __aie_inline { return ::lnegmul2(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul2_cn(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul2_nc(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul2_cc(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmac2(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac2_cn(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac2_nc(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac2_cc(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmsc2(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc2_cn(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc2_nc(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc2_cc(args...); };
            }
            else if constexpr (Lanes == 4) {
                if constexpr (MulOp == MulMacroOp::MulSym)                              return [](auto &&... args) __aie_inline { return ::lmul4(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_cn(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_c(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmul4_nc(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmul4_cc(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSym)                           return [](auto &&... args) __aie_inline { return ::lnegmul4(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_cn(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::lnegmul4_nc(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::lnegmul4_cc(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmac4(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_cn(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmac4_nc(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmac4_cc(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmsc4(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_cn(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::lmsc4_nc(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::lmsc4_cc(args...); };
            }
        }
        else {
            if constexpr (Lanes == 2) {
                if constexpr (MulOp == MulMacroOp::MulSym)                              return [](auto &&... args) __aie_inline { return ::lmul2_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSym)                           return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmac2_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmsc2_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc2_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc2_sym_ct_c(args...); };
            }
            else if constexpr (Lanes == 4) {
                if constexpr (MulOp == MulMacroOp::MulSym)                              return [](auto &&... args) __aie_inline { return ::lmul4_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmul4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSym)                           return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmac4_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmac4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmsc4_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_sym_ct_c(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::lmsc4_sym_ct_c(args...); };
            }
            else if constexpr (Lanes == 8) {
                if constexpr (MulOp == MulMacroOp::MulSym)                              return [](auto &&... args) __aie_inline { return ::lmul8_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::NegMulSym)                           return [](auto &&... args) __aie_inline { return ::lnegmul8_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Add_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmac8_sym_ct(args...); };
                if constexpr (MulOp == MulMacroOp::Sub_MulSym)                          return [](auto &&... args) __aie_inline { return ::lmsc8_sym_ct(args...); };
            }
        }
    }
    else {
        if constexpr (Lanes == 2) {
            if constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::lmul2_sym(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::lmul2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::lmul2_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::lmul2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::lmul2_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::lmul2_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::lnegmul2_sym(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::lnegmul2_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmac2_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmac2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmac2_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmac2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmac2_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmac2_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmsc2_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmsc2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmsc2_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmsc2_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmsc2_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmsc2_sym_cc(args...); };
        }
        else if constexpr (Lanes == 4) {
            if constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::lmul4_sym(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::lmul4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::lmul4_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::lmul4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::lmul4_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::lmul4_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::lnegmul4_sym(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::lnegmul4_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmac4_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmac4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmac4_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmac4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmac4_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmac4_sym_cc(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmsc4_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmsc4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmsc4_sym_cn(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::lmsc4_sym_c(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::lmsc4_sym_nc(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::lmsc4_sym_cc(args...); };
        }
        else if constexpr (Lanes == 8) {
            if constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::lmul8_sym(args...); };
            if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::lnegmul8_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmac8_sym(args...); };
            if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::lmsc8_sym(args...); };
        }
    }
}

template <typename Coeff, typename Data>
constexpr unsigned lanes_per_mul_ct(unsigned lanes, unsigned points)
{
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int16>) { return 8; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data,  int32>) { return 8; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int16>) { return ((((points + 1) / 2) % 4 == 0) || lanes == 4)? 4 : 8; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint16>) { return ((((points + 1) / 2) % 4 == 0) || lanes == 4)? 4 : 8; }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data,  int32>) { return 4; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint16>) { return 4; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data,  int32>) { return 4; }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint16>) { return 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint16>) { return 4; }
    if constexpr (std::is_same_v<Coeff, cint16> && std::is_same_v<Data, cint32>) { return 4; }
    if constexpr (std::is_same_v<Coeff,  int32> && std::is_same_v<Data, cint32>) { return ((((points + 1) / 2) % 2 == 0) || lanes == 2)? 2 : 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int32>) { return ((((points + 1) / 2) % 2 == 0) || lanes == 2)? 2 : 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data,  int16>) { return 4; }
    if constexpr (std::is_same_v<Coeff,  int16> && std::is_same_v<Data, cint32>) { return 4; }
    if constexpr (std::is_same_v<Coeff, cint32> && std::is_same_v<Data, cint32>) { return 2; }
}

template <unsigned Lanes, unsigned DataStepY, unsigned N_Data>
constexpr unsigned grow_offsets_for_sym_acc80(unsigned offset)
{
    unsigned ret = 0;

    for (unsigned i = 0; i < Lanes; ++i) {
        const unsigned val = ((offset & 0xf) * DataStepY) % 16;

        ret |= val << (i * 4);

        offset >>= 4;
    }

    return ret;
}

template <unsigned Lanes, unsigned DataStepY, unsigned N_Data>
constexpr auto offsets_for_sym_acc80()
{
    if      constexpr (Lanes == 2) return grow_offsets_for_sym_acc80<Lanes, DataStepY, N_Data>(0x10);
    else if constexpr (Lanes == 4) return grow_offsets_for_sym_acc80<Lanes, DataStepY, N_Data>(0x3210);
    else if constexpr (Lanes == 8) return grow_offsets_for_sym_acc80<Lanes, DataStepY, N_Data>(0x76543210);
}

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl<Lanes, Points, true, CoeffStep, DataStepX, DataStepY, 80, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 80>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc80<coeff_type, data_type>();
    static constexpr unsigned   lanes_per_mul = lanes_per_mul_ct<coeff_type, data_type>(Lanes, Points);
    static constexpr unsigned columns_per_mul = muls_per_mul / lanes_per_mul;
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_acc80<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op_noct = sliding_mul_sym_acc80_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op_ct   = sliding_mul_sym_acc80_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, true>();

        constexpr unsigned data_elems  = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = data_start                                    + columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_y = data_start + (Points - DataStepX) * DataStepX - columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_z = coeff_start                                   + columns_per_mul * (num_mul - 1) * CoeffStep;

        // Center is relative to start_x
        constexpr unsigned center = (Points / 2 - 1) * DataStepX + 1 - columns_per_mul * (num_mul - 1) * DataStepX;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, Lanes / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY,  offsets,
                                                                              start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z, 0x0000, CoeffStep);
            }
            else {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + center,  offsets,
                                coeff.template grow_replicate<coeff_elems>(), start_z, 0x0000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx_x) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op_noct(  tmp,
                                     data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets, DataStepX,
                                                                                  start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                    coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000, CoeffStep);
                }
                else {
                    tmp = mac_op_noct(  tmp,
                                     data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets,
                                                                                  start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                    coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000);
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned start_ldata,
                          const vector<data_type, N_Data> &rdata,
                          unsigned start_rdata,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_acc80<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op_noct = sliding_mul_sym_acc80_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op_ct   = sliding_mul_sym_acc80_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, true>();

        constexpr unsigned data_elems  = 512 / type_bits_v<data_type>;
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = start_ldata + columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_y = start_rdata - columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        // Center is relative to start_x
        constexpr unsigned center = (Points / 2 - 1) * DataStepX + 1 - columns_per_mul * (num_mul - 1) * DataStepX;

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, Lanes / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                ldata.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets,
                                rdata.template grow_replicate<data_elems>(),  start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                ldata.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets,
                                rdata.template grow_replicate<data_elems>(),  start_y + lanes_per_mul * idx_y * DataStepY,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx_x) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op_noct(  tmp,
                                     ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets, DataStepX,
                                     rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                    coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000,  CoeffStep);
                }
                else {
                    tmp = mac_op_noct(  tmp,
                                     ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets,
                                     rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                    coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000);
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl<Lanes, Points, false, CoeffStep, DataStepX, DataStepY, 80, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 80>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc80<coeff_type, data_type>();
    static constexpr unsigned   lanes_per_mul = lanes_per_mul_acc80<coeff_type, data_type>(Lanes, Points / 2);
    static constexpr unsigned columns_per_mul = muls_per_mul / lanes_per_mul;
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned ldata_start,
                          unsigned rdata_start,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_acc80<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_sym_acc80_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc80_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = ldata_start;
        const unsigned start_y = rdata_start;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, Lanes / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets, DataStepX,
                                                                           start_y + lanes_per_mul * idx_y * DataStepY,
                             coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets,
                                                                           start_y + lanes_per_mul * idx_y * DataStepY,
                             coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx_x) __aie_inline {
                if constexpr(columns_per_mul > 1) {
                    tmp = mac_op(  tmp,
                                data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets, DataStepX,
                                                                             start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                               coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000, CoeffStep);
                }
                else {
                    tmp = mac_op(  tmp,
                                data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets,
                                                                             start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                               coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000);
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &ldata,
                          unsigned start_ldata,
                          const vector<data_type, N_Data> &rdata,
                          unsigned start_rdata,
                          const Acc &... acc)
    {
        constexpr unsigned offsets = offsets_for_sym_acc80<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_sym_acc80_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc80_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = 512 / type_bits_v<data_type>;
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = start_ldata;
        const unsigned start_y = start_rdata;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, Lanes / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                            ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY, offsets, DataStepX,
                            rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY,
                           coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                            ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY, offsets,
                            rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY,
                           coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx_x) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op(  tmp,
                                ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets, DataStepX,
                                rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                               coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000,  CoeffStep);
                }
                else {
                    tmp = mac_op(  tmp,
                                ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets,
                                rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                               coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000);
                }
            });

            ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif
