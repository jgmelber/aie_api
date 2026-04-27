// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>

//! [Vector add]
void add(int32 * __restrict out,
         const int32 * __restrict in1, const int32 * __restrict in2, unsigned count)
{
    for (unsigned i = 8; i < count; i += 8) {
        aie::vector<int32, 8> vec = aie::add(aie::load_v<8>(in1 + i),
                                             aie::load_v<8>(in2 + i));

        aie::store_v(out, vec);
    }
}
//! [Vector add]
