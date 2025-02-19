// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_CONFIG__HPP__
#define __AIE_API_DETAIL_AIE2_CONFIG__HPP__

#define AIE_API_PLATFORM_VERSION 200
#define AIE_API_ML_VERSION       200
#define AIE_API_MATH_VERSION     0

#define __AIE_API_REGISTER_ATTR_DEFINED__           0

#define __AIE_API_SCALAR_TYPES_CONSTEXPR__          (__AIE_MODEL_VERSION__ >= 2500)

#define __AIE_API_COMPR_CONST_PTR__                 0

#define __AIE_API_MUL_CONJUGATE_INTRINSICS__        (__AIE_MODEL_VERSION__ >= 3000)

#define __AIE_API_MUL_CONJUGATE_32BIT_INTRINSICS__  (__AIE_MODEL_VERSION__ >= 10200)

#define __AIE_API_SHIFT_BYTES__                     (__AIE_MODEL_VERSION__ >= 3000)

#define __AIE_API_FP32_EMULATION__                  (__AIE_MODEL_VERSION__ >= 3000)

#define __AIE_API_TERM_NEG_COMPLEX_DEFINES__        (__AIE_MODEL_VERSION__ >= 10200)

#define __AIE_API_128_BIT_INSERT_CONCAT__           (__AIE_MODEL_VERSION__ >= 2700)

#define __AIE_API_32ELEM_FLOAT_SRS_UPS__            (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_COMPOUND_DM_RESOURCE__            (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_CONSTEXPR_BFLOAT16__              0

#define __AIE_API_COMPLEX_FP32_EMULATION__          (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_NATIVE_FIFO__                     1

#define __AIE_API_SUPPORTED_FRIEND_CONCEPTS__       (__AIE_MODEL_VERSION__ <= 10300)

#define __AIE_API_EMULATED_FP32_ZEROIZATION__       (__AIE_MODEL_VERSION__ >= 10400)

#define __AIE_API_CFP_TO_FP_CONVERSIONS__           (__AIE_MODEL_VERSION__ >= 10400)

#define __AIE_API_CBF16_SUPPORT__                   (__AIE_MODEL_VERSION__ >= 10500)

#define __AIE_API_BUILTIN_CLZ__                     (__AIE_MODEL_VERSION__ >= 10600 && __AIE_CORE_BUILTIN_CLZ__)

#endif
