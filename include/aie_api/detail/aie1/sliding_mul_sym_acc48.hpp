// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_ACC48_HPP__
#define __AIE_API_DETAIL_AIE1_SLIDING_MUL_SYM_ACC48_HPP__

#include "sliding_mul_sym_acc80.hpp"

#include "../accum.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, typename CoeffType, typename DataType, unsigned Cols, bool CT>
static constexpr auto sliding_mul_sym_acc48_get_mul_op()
{
    constexpr unsigned num_complex = get_num_complex<CoeffType, DataType>();

    if constexpr (CT) {
        if constexpr (Cols == 1) {
            if constexpr (Lanes == 4) {
                if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul4(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul4(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac4(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc4(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul4(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul4(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac4(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc4(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc4_cc(args...); };
            }
            else if constexpr (Lanes == 8) {
                if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul8(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul8(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac8(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc8(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul8(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul8(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac8(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac8_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc8(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc8_cc(args...); };
            }
        }
        else {
            if constexpr (Lanes == 4) {
                if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul4_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul4_sym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul4_sym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac4_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac4_sym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc4_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc4_sym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_antisym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_antisym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_antisym_ct_cc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct_cn(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct_nc(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc4_antisym_ct_cc(args...); };
            }
            else if constexpr (Lanes == 8) {
                if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul8_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul8_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac8_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc8_sym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_sym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul8_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul8_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac8_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc8_antisym_ct(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_antisym_ct_c(args...); };
                else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_antisym_ct_c(args...); };
            }
        }
    }
    else {
        if constexpr (Lanes == 4) {
            if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul4_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul4_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul4_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul4_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul4_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul4_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac4_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac4_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac4_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc4_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc4_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc4_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc4_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul4_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul4_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul4_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul4_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul4_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul4_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac4_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac4_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac4_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc4_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc4_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc4_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc4_antisym_cc(args...); };
        }
        else if constexpr (Lanes == 8) {
            if      constexpr (MulOp == MulMacroOp::MulSym)                                  return [](auto &&... args) __aie_inline { return ::mul8_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul8_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 1)         return [](auto &&... args) __aie_inline { return ::mul8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj2 && num_complex == 2)         return [](auto &&... args) __aie_inline { return ::mul8_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulSymConj1Conj2)                        return [](auto &&... args) __aie_inline { return ::mul8_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSym)                               return [](auto &&... args) __aie_inline { return ::negmul8_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 1)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj2 && num_complex == 2)      return [](auto &&... args) __aie_inline { return ::negmul8_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSymConj1Conj2)                     return [](auto &&... args) __aie_inline { return ::negmul8_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSym)                              return [](auto &&... args) __aie_inline { return ::mac8_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac8_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mac8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mac8_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mac8_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSym)                              return [](auto &&... args) __aie_inline { return ::msc8_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc8_sym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::msc8_sym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::msc8_sym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::msc8_sym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisym)                              return [](auto &&... args) __aie_inline { return ::mul8_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 1)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj2 && num_complex == 2)     return [](auto &&... args) __aie_inline { return ::mul8_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisymConj1Conj2)                    return [](auto &&... args) __aie_inline { return ::mul8_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisym)                           return [](auto &&... args) __aie_inline { return ::negmul8_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 1)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj2 && num_complex == 2)  return [](auto &&... args) __aie_inline { return ::negmul8_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisymConj1Conj2)                 return [](auto &&... args) __aie_inline { return ::negmul8_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::mac8_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::mac8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::mac8_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::mac8_antisym_cc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)                          return [](auto &&... args) __aie_inline { return ::msc8_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_antisym_cn(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 1) return [](auto &&... args) __aie_inline { return ::msc8_antisym_c(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj2 && num_complex == 2) return [](auto &&... args) __aie_inline { return ::msc8_antisym_nc(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisymConj1Conj2)                return [](auto &&... args) __aie_inline { return ::msc8_antisym_cc(args...); };
        }
        else if constexpr (Lanes == 16) {
            if      constexpr (MulOp == MulMacroOp::MulSym)          return [](auto &&... args) __aie_inline { return ::mul16_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulSym)       return [](auto &&... args) __aie_inline { return ::negmul16_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulSym)      return [](auto &&... args) __aie_inline { return ::mac16_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulSym)      return [](auto &&... args) __aie_inline { return ::msc16_sym(args...); };
            else if constexpr (MulOp == MulMacroOp::MulAntisym)      return [](auto &&... args) __aie_inline { return ::mul16_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::NegMulAntisym)   return [](auto &&... args) __aie_inline { return ::negmul16_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Add_MulAntisym)  return [](auto &&... args) __aie_inline { return ::mac16_antisym(args...); };
            else if constexpr (MulOp == MulMacroOp::Sub_MulAntisym)  return [](auto &&... args) __aie_inline { return ::msc16_antisym(args...); };
        }
    }
}

template <unsigned Lanes, unsigned DataStepY, unsigned N_Data>
constexpr unsigned grow_offsets_for_sym_acc48(unsigned offset)
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
constexpr auto offsets_for_sym_acc48()
{
    if constexpr (Lanes == 4) {
        return grow_offsets_for_sym_acc48<Lanes, DataStepY, N_Data>(0x3210);
    }
    else if constexpr (Lanes == 8) {
        return grow_offsets_for_sym_acc48<Lanes, DataStepY, N_Data>(0x76543210);
    }
    else if constexpr (Lanes == 16) {
        return std::make_pair(grow_offsets_for_sym_acc48<8, DataStepY, N_Data>(0x76543210),
                              grow_offsets_for_sym_acc48<8, DataStepY, N_Data>(0xfedcba98));
    }
}

template <typename Data, typename Coeff>
constexpr unsigned columns_per_sym_mul_acc48(unsigned lanes)
{
    if      constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int16>) return 32 / lanes;
    else if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff, cint16>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff,  int32>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data,  int32> && std::is_same_v<Coeff,  int16>) return 16 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint16>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff,  int32>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data,  int32> && std::is_same_v<Coeff, cint16>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data, cint32> && std::is_same_v<Coeff,  int16>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data,  int16> && std::is_same_v<Coeff, cint32>) return  8 / lanes;
    else if constexpr (std::is_same_v<Data, cint32> && std::is_same_v<Coeff, cint16>) return  4 / lanes;
    else if constexpr (std::is_same_v<Data, cint16> && std::is_same_v<Coeff, cint32>) return  4 / lanes;
}

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl<Lanes, Points, false, CoeffStep, DataStepX, DataStepY, 48, 16, 16, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    static constexpr unsigned      offsets_lo = 0x03020100;
    static constexpr unsigned      offsets_hi = 0x07060504;
    static constexpr unsigned columns_per_mul = columns_per_sym_mul_acc48<data_type, coeff_type>(Lanes);
    static constexpr unsigned   lanes_per_mul = Lanes;
    static constexpr unsigned         num_mul = ((Points / 2) + columns_per_mul - 1) / columns_per_mul;
    static_assert(DataStepX <= 1, "DataStepX > 1 is not supported at the moment");
    static_assert(DataStepY <= 1, "DataStepY > 1 is not supported at the moment");

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires((is_accum_v<Acc> && ...))
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          const vector<data_type, N_Data> &data,
                          unsigned ldata_start,
                          unsigned rdata_start,
                          const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), Lanes, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc48_get_mul_op<             MulOp, Lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        // static_assert(N_Coeff >= (Points / 2) * CoeffStep);

        unsigned       start_x = ldata_start;
        unsigned       start_y = rdata_start - DataStepX;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        const unsigned lodd = ldata_start & 0x1;
        start_x -= lodd;
        const int square_x = 0x2110 + lodd*(0x1111);

        const unsigned rodd = start_y & 0x1;
        start_y -= rodd;
        const int square_y = 0x2312 - (!rodd)*(0x1111);

        // x indices must be adjusted by "data_start"
        //
        // c0 * (x0s * x15s) + c1 * (x1s * x14s) + c2 * (x2s * x13s) + c3 * (x3s * x12s) + c4 * (x4s * x11s) + c5 * (x5s * x10s) + c6 * (x6s * x9s ) + c7 * (x7s * x8s )
        if constexpr (columns_per_mul > 2) {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  start_x, offsets_lo,          2, square_x,
                                                                       start_y,                         square_y,
                         coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);
        }
        else {
            ret = mul_op(  acc...,
                          data.template grow_replicate<data_elems>(),  start_x, offsets_lo, offsets_hi, square_x,
                                                                       start_y,                         square_y,
                         coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, 0x00000000, CoeffStep);

        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            if constexpr (columns_per_mul > 2) {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  start_x + columns_per_mul * idx * DataStepX, offsets_lo,         2, square_x,
                                                                           start_y - columns_per_mul * idx * DataStepX,                        square_y,
                             coeff.template grow_replicate<coeff_elems>(), start_z + columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
            }
            else  {
                ret = mac_op(  ret,
                              data.template grow_replicate<data_elems>(),  start_x + columns_per_mul * idx * DataStepX, offsets_lo, offsets_hi, square_x,
                                                                           start_y - columns_per_mul * idx * DataStepX,                         square_y,
                             coeff.template grow_replicate<coeff_elems>(), start_z + columns_per_mul * idx * CoeffStep, 0x00000000, 0x00000000, CoeffStep);
            }
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
                          const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), Lanes, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc48_get_mul_op<             MulOp, Lanes, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = 512 / type_bits_v<data_type>;
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        // static_assert(N_Coeff >= (Points / 2) * CoeffStep);

        unsigned  start_x = ldata_start;
        unsigned  start_y = rdata_start - DataStepX;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret;

        const unsigned lodd = ldata_start & 0x1;
        start_x -= lodd;
        const int square_x = 0x2110 + lodd*(0x1111);

        const unsigned rodd = start_y & 0x1;
        start_y -= rodd;
        const int square_y = 0x2312 - (!rodd)*(0x1111);

        // x indices must be adjusted by "data_start"
        //
        // c0 * (x0s * x15s) + c1 * (x1s * x14s) + c2 * (x2s * x13s) + c3 * (x3s * x12s) + c4 * (x4s * x11s) + c5 * (x5s * x10s) + c6 * (x6s * x9s ) + c7 * (x7s * x8s )
        if constexpr (columns_per_mul > 2) {
            ret = mul_op(  acc...,
                         ldata.template grow_replicate<data_elems>(),  start_x, offsets_lo,          2, square_x,
                         rdata.template grow_replicate<data_elems>(),  start_y,                         square_y,
                         coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, CoeffStep);
        }
        else {
            ret = mul_op(  acc...,
                         ldata.template grow_replicate<data_elems>(),  start_x, offsets_lo, offsets_hi, square_x,
                         rdata.template grow_replicate<data_elems>(),  start_y,                         square_y,
                         coeff.template grow_replicate<coeff_elems>(), start_z, 0x00000000, 0x00000000, CoeffStep);

        }

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            if constexpr (columns_per_mul > 2) {
                ret = mac_op(  ret,
                             ldata.template grow_replicate<data_elems>(),  start_x + columns_per_mul * idx * DataStepX, offsets_lo,         2, square_x,
                             rdata.template grow_replicate<data_elems>(),  start_y - columns_per_mul * idx * DataStepX,                        square_y,
                             coeff.template grow_replicate<coeff_elems>(), start_z + columns_per_mul * idx * CoeffStep, 0x00000000, CoeffStep);
            }
            else  {
                ret = mac_op(  ret,
                             ldata.template grow_replicate<data_elems>(),  start_x + columns_per_mul * idx * DataStepX, offsets_lo, offsets_hi, square_x,
                             rdata.template grow_replicate<data_elems>(),  start_y - columns_per_mul * idx * DataStepX,                         square_y,
                             coeff.template grow_replicate<coeff_elems>(), start_z + columns_per_mul * idx * CoeffStep, 0x00000000, 0x00000000, CoeffStep);
            }
        });

        return ret;
    }
};

template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl<Lanes, Points, true, CoeffStep, DataStepX, DataStepY, 48, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc48<coeff_type, data_type>();
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
        constexpr auto offsets = offsets_for_sym_acc48<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op_noct = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op_ct   = sliding_mul_sym_acc48_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, true>();

        constexpr unsigned data_elems  = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = data_start                                    + columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_y = data_start + (Points - DataStepX) * DataStepX - columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_z = coeff_start                                   + columns_per_mul * (num_mul - 1) * CoeffStep;

        // Center is relative to start_x
        constexpr unsigned center = (Points / 2 - 1) * DataStepX + 1           - columns_per_mul * (num_mul - 1) * DataStepX;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, (Lanes + lanes_per_mul - 1) / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 2) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY,  offsets, DataStepX,
                                                                              start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                      0x0000, CoeffStep);
            }
            else if constexpr (columns_per_mul > 1) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY,  offsets,
                                                                              start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                      0x0000, CoeffStep);
            }
            else if constexpr (lanes_per_mul == 16) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + center, offsets.first, offsets.second,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                                     0x0000,         0x0000);
            }
            else {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                 data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + center, offsets,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                              0x0000);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx_x) __aie_inline {
                if constexpr (columns_per_mul > 1) {
                    tmp = mac_op_noct(  tmp,
                                       data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets, DataStepX,
                                                                                    start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                      coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000, CoeffStep);
                }
                else if constexpr (lanes_per_mul == 16) {
                    tmp = mac_op_noct(  tmp,
                                       data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets.first, offsets.second,
                                                                                    start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                      coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000,        0x0000);
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
        constexpr auto offsets = offsets_for_sym_acc48<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op_noct = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op_ct   = sliding_mul_sym_acc48_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, true>();

        constexpr unsigned data_elems  = 512 / type_bits_v<data_type>;
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = start_ldata + columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_y = start_rdata - columns_per_mul * (num_mul - 1) * DataStepX;
        const unsigned start_z = coeff_start + columns_per_mul * (num_mul - 1) * CoeffStep;

        // Center is relative to start_x
        constexpr unsigned center = (Points / 2 - 1) * DataStepX + 1 - columns_per_mul * (num_mul - 1) * DataStepX;

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, (Lanes + lanes_per_mul - 1) / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 2) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                ldata.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets, DataStepX,
                                rdata.template grow_replicate<data_elems>(),  start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else if constexpr (columns_per_mul > 1) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                ldata.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets,
                                rdata.template grow_replicate<data_elems>(),  start_y + lanes_per_mul * idx_y * DataStepY,
                                                                              center,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else if constexpr (lanes_per_mul == 16) {
                tmp = mul_op_ct(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                                ldata.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets.first, offsets.second,
                                rdata.template grow_replicate<data_elems>(),  start_y + lanes_per_mul * idx_y * DataStepY,
                                coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,        0x0000);
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
                else if constexpr (lanes_per_mul == 16) {
                    tmp = mac_op_noct(  tmp,
                                      ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX, offsets.first, offsets.second,
                                      rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX,
                                     coeff.template grow_replicate<coeff_elems>(), start_z                                     - columns_per_mul * idx_x * CoeffStep, 0x0000,        0x0000);
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

// Generic implementation when no central point is used. This works for all type combinations but
// 16b x 16b.
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, unsigned CoeffTypeBits, unsigned DataTypeBits, typename CoeffType, typename DataType>
struct sliding_mul_sym_bits_impl<Lanes, Points, false, CoeffStep, DataStepX, DataStepY, 48, CoeffTypeBits, DataTypeBits, CoeffType, DataType>
{
    using  data_type = DataType;
    using coeff_type = CoeffType;
    using  accum_tag = accum_tag_for_mul_types<data_type, coeff_type, 48>;
    using accum_type = accum<accum_tag, Lanes>;

    template <unsigned Lanes2 = Lanes>
    using accum_internal_type = accum<accum_tag, Lanes2>;

    static constexpr unsigned    muls_per_mul = muls_per_mul_acc48<coeff_type, data_type>();
    static constexpr unsigned   lanes_per_mul = lanes_per_mul_acc48<coeff_type, data_type>(Lanes, Points / 2);
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
        constexpr auto offsets = offsets_for_sym_acc48<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc48_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = std::max(N_Data, 512u / type_bits_v<data_type>);
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = ldata_start;
        const unsigned start_y = rdata_start;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, (Lanes + lanes_per_mul - 1) / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets, DataStepX,
                                                                           start_y + lanes_per_mul * idx_y * DataStepY,
                             coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else if constexpr (lanes_per_mul == 16) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                              data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY, offsets.first, offsets.second,
                                                                           start_y + lanes_per_mul * idx_y * DataStepY,
                             coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,        0x0000);
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
                else if constexpr(lanes_per_mul == 16) {
                    tmp = mac_op(  tmp,
                                  data.template grow_replicate<data_elems>(),  start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets.first, offsets.second,
                                                                               start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                                 coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000,        0x0000);
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
        constexpr auto offsets = offsets_for_sym_acc48<lanes_per_mul, DataStepY, N_Data>();

        constexpr auto mac_op = sliding_mul_sym_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();
        constexpr auto mul_op = sliding_mul_sym_acc48_get_mul_op<             MulOp, lanes_per_mul, CoeffType, DataType, columns_per_mul, false>();

        constexpr unsigned data_elems  = 512 / type_bits_v<data_type>;
        constexpr unsigned coeff_elems = 256 / type_bits_v<coeff_type>;

        const unsigned start_x = start_ldata;
        const unsigned start_y = start_rdata;
        const unsigned start_z = coeff_start;

        REQUIRES_CONSTANT(start_z);

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, (Lanes + lanes_per_mul - 1) / lanes_per_mul>([&](auto idx_y) __aie_inline {
            accum_internal_type<lanes_per_mul> tmp;

            if constexpr (columns_per_mul > 1) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                            ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY, offsets, DataStepX,
                            rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY,
                           coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,  CoeffStep);
            }
            else if constexpr (lanes_per_mul == 16) {
                tmp = mul_op(utils::get_nth<0>(ret.template extract<lanes_per_mul>(idx_y), acc)...,
                            ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY, offsets.first, offsets.second,
                            rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY,
                           coeff.template grow_replicate<coeff_elems>(), start_z,                                     0x0000,        0x0000);
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
                else if constexpr (lanes_per_mul == 16) {
                    tmp = mac_op(  tmp,
                                ldata.template grow_replicate<data_elems>(), start_x + lanes_per_mul * idx_y * DataStepY + columns_per_mul * idx_x * DataStepX, offsets.first, offsets.second,
                                rdata.template grow_replicate<data_elems>(), start_y + lanes_per_mul * idx_y * DataStepY - columns_per_mul * idx_x * DataStepX,
                               coeff.template grow_replicate<coeff_elems>(), start_z                                     + columns_per_mul * idx_x * CoeffStep, 0x0000,        0x0000);
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
