// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_CONFIG__HPP__
#define __AIE_API_DETAIL_AIE2P_CONFIG__HPP__

#define AIE_API_PLATFORM_VERSION 200
#define AIE_API_ML_VERSION       210
#define AIE_API_MATH_VERSION     0

#define __AIE_API_REGISTER_ATTR_DEFINED__     0

#define __AIE_API_SCALAR_TYPES_CONSTEXPR__    0

#define __AIE_API_COMPR_CONST_PTR__           0

#define __AIE_API_EMULATED_MUL_INTRINSICS__   0

#define __AIE_API_MUL_CONJUGATE_INTRINSICS__  0

#define __AIE_API_MUL_CONJUGATE_32BIT_INTRINSICS__ (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_SHIFT_BYTES__               (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_FP32_EMULATION__            (__AIE_MODEL_VERSION__ >= 10600)

#define __AIE_API_HAS_EXTRACT_V64BFP16__      (__AIE_MODEL_VERSION__ >= 10000)

#define __AIE_API_TERM_NEG_COMPLEX_DEFINES__  (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_HAS_32B_MUL__               (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_8_LANE_MUL_ELEM_8__         (__AIE_MODEL_VERSION__ < 200)

#define __AIE_API_128_BIT_INSERT_CONCAT__     (__AIE_MODEL_VERSION__ >= 10100)

#define __AIE_API_HAS_128_FIFO_POP_ADDR___    (__AIE_MODEL_VERSION__ >= 10300)

#define __AIE_API_HAS_COMPLEX_BFLOAT16_FIFO__ (__AIE_MODEL_VERSION__ >= 10500)

#define __AIE_API_COMPOUND_DM_RESOURCE__      (__AIE_MODEL_VERSION__ >= 10500)

#define __AIE_API_NATIVE_FIFO__               (__AIE_MODEL_VERSION__ >= 10500)

#define __AIE_API_ACC_BROADCAST_NAME__        (__AIE_MODEL_VERSION__ >= 10800)

#define __AIE_API_CONSTEXPR_BFLOAT16__        (__AIE_MODEL_VERSION__ >= 10900)

#define __AIE_API_DIMS_STRUCTS__              (__AIE_MODEL_VERSION__ >= 10400)

#define __AIE_API_SUPPORTED_FRIEND_CONCEPTS__ (__AIE_MODEL_VERSION__ <= 10700)

#define __AIE_API_COMPLEX_FP32_EMULATION__    0

#define __AIE_API_HAS_32x16_MMUL_INTRINSICS__ (__AIE_MODEL_VERSION__ >= 10700)

#define __AIE_API_EMULATED_FP32_ZEROIZATION__ (__AIE_MODEL_VERSION__ >= 10900)

#define __AIE_API_SCALAR_BFP_TYPES__          (__AIE_MODEL_VERSION__ >= 11000)

#define __AIE_API_CBF16_SUPPORT__             0

#define __AIE_API_BUILTIN_CLZ__               (__AIE_MODEL_VERSION__ >= 11100 && __AIE_CORE_BUILTIN_CLZ__)

// Workaround disabled by default. Define to 1 to enable workaround.
// If enabled, replaces ::concat with ::insert for aie::concat(accum...) and accum::upd_all(accum...)
#ifndef __AIE_API_WORKAROUND_CR_1223259__
#define __AIE_API_WORKAROUND_CR_1223259__     0
#endif

#endif
