// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>

//![Abs and conj]
aie::accum<cacc48, 16> mul_conj(aie::vector<int16, 16> a, aie::vector<cint16, 16> b)
{
    aie::accum<cacc48, 16> ret;

    ret = aie::mul(aie::op_abs(a), aie::op_conj(b));

    return ret;
}
//![Abs and conj]
