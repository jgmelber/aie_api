// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_LOOKUP_TABLE_HPP__
#define __AIE_API_DETAIL_LOOKUP_TABLE_HPP__

#if AIE_API_REMOVE_PAR_LOOKUP_ALL_SPACE_USED_MANIFEST
  #define PAR_LOOKUP_ALL_SPACE_MANIFEST(cond) false
#else
  #define PAR_LOOKUP_ALL_SPACE_MANIFEST(cond) chess_manifest(cond)
#endif

#if AIE_API_REMOVE_PAR_LOOKUP_ZERO_BIAS_MANIFEST
  #define PAR_LOOKUP_ZERO_BIAS_MANIFEST(cond) false
#else
  #define PAR_LOOKUP_ZERO_BIAS_MANIFEST(cond) chess_manifest(cond)
#endif

namespace aie::detail {

template <typename T, typename MyLUT, lut_oor_policy oor_policy>
struct parallel_lookup;

}

//TODO: Support for parallel_lookup in AIE1?

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/parallel_lookup.hpp"

#endif

#endif
