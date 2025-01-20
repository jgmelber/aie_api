// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_LINEAR_APPROX_HPP__
#define __AIE_API_DETAIL_LINEAR_APPROX_HPP__

namespace aie::detail {

template <typename T, typename MyLUT>
struct linear_approx;

}

//TODO: Support for linear approx in AIE1

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "aie2/linear_approx.hpp"

#endif

#endif
