// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>
//![Example operators]
#include <aie_api/operators.hpp>

using namespace aie::operators;

aie::mask<16> less_than_add(aie::vector<int32, 16> a, aie::vector<int32, 16> b, aie::vector<int32, 16> c)
{
    return c < (a + b);
}
//![Example operators]
