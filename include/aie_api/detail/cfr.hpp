// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_CFR_HPP__
#define __AIE_API_DETAIL_CFR_HPP__

namespace aie::detail {

template <typename T>
struct cfr;

}

#if __AIE_ARCH__ == 10

#include "aie1/cfr.hpp"

#elif __AIE_ARCH__ == 20

// TODO: implement CFR support on AIE2

#endif

#endif
