// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_CONFIG__HPP__
#define __AIE_API_DETAIL_AIE1_CONFIG__HPP__

#define AIE_API_PLATFORM_VERSION   100
#define AIE_API_ML_VERSION         100
#define AIE_API_MATH_VERSION       100

// >= 900000 is used to detect the master development branch

#define __AIE_API_REGISTER_ATTR_DEFINED__ ((__AIE_MODEL_VERSION__ >= 900000)                                                                   || \
                                           (__AIE_MODEL_VERSION__ >= 20000 && __AIE_MODEL_VERSION__ < 30000 && __AIE_MODEL_VERSION__ >= 21800) || \
                                           (__AIE_MODEL_VERSION__ >= 10000 && __AIE_MODEL_VERSION__ < 20000 && __AIE_MODEL_VERSION__ >= 11800))

#define __AIE_API_SCALAR_TYPES_CONSTEXPR__ ((__AIE_MODEL_VERSION__ >= 900000)                                                                   || \
                                            (__AIE_MODEL_VERSION__ >= 20000 && __AIE_MODEL_VERSION__ < 30000 && __AIE_MODEL_VERSION__ >= 21800) || \
                                            (__AIE_MODEL_VERSION__ >= 10000 && __AIE_MODEL_VERSION__ < 20000 && __AIE_MODEL_VERSION__ >= 11800))

#define __AIE_API_COMPOUND_DM_RESOURCE__   ((__AIE_MODEL_VERSION__ >= 900000)                                                                   || \
                                            (__AIE_MODEL_VERSION__ >= 20000 && __AIE_MODEL_VERSION__ < 30000 && __AIE_MODEL_VERSION__ >= 22400) || \
                                            (__AIE_MODEL_VERSION__ >= 10000 && __AIE_MODEL_VERSION__ < 20000 && __AIE_MODEL_VERSION__ >= 12400))

#define __AIE_API_FLOAT_CONVERSION_VECTORIZED__   ((__AIE_MODEL_VERSION__ >= 900000)                                                                   || \
                                                   (__AIE_MODEL_VERSION__ >= 20000 && __AIE_MODEL_VERSION__ < 30000 && __AIE_MODEL_VERSION__ >= 22400) || \
                                                   (__AIE_MODEL_VERSION__ >= 10000 && __AIE_MODEL_VERSION__ < 20000 && __AIE_MODEL_VERSION__ >= 12400))

#define __AIE_API_SUPPORTED_FRIEND_CONCEPTS__     ((__AIE_MODEL_VERSION__ <= 900000)                                  && \
                                                   ((__AIE_MODEL_VERSION__ >= 20000 && __AIE_MODEL_VERSION__ < 22500) || \
                                                    (__AIE_MODEL_VERSION__ >= 10000 && __AIE_MODEL_VERSION__ < 12500)))

#define __AIE_API_COMPLEX_FP32_EMULATION__    0

#define __AIE_API_BUILTIN_CLZ__               (__AIE_MODEL_VERSION__ >= 22700 && __AIE_CORE_BUILTIN_CLZ__)

#endif
