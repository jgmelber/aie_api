// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>

//![Example]
template <typename Value>
void parallel_lookup(const int8* pIn, Value* pOut, const aie::lut<4, Value>& my_lut,
                     int samples, int step_bits, int bias, int LUT_elems)
{
    aie::parallel_lookup<int8, aie::lut<4, Value>> lookup(my_lut, step_bits, bias);

    auto it_in  = aie::begin_vector<32>(pIn);
    auto it_out = aie::begin_vector<32>(pOut);

    for (unsigned l = 0; l < samples / 32; ++l)
        *it_out++ = lookup.fetch(*it_in++);
}

template <typename OffsetType, typename SlopeType>
void linear_approx(const int8* pIn, OffsetType* pOut, const aie::lut<4, OffsetType, SlopeType>& my_lut,
                   int samples, int step_bits, int bias, int LUT_elems, int shift_offset, int shift_out)
{
    using lut_t = aie::lut<4, OffsetType, SlopeType>;
    aie::linear_approx<int8, lut_t> lin_approx(my_lut, step_bits, bias, shift_offset);

    auto it_in  = aie::begin_vector<32>(pIn);
    auto it_out = aie::begin_vector<32>(pOut);

    for (unsigned l = 0; l < samples / 32; ++l)
        *it_out++ = lin_approx.compute(*it_in++).template to_vector<lut_t>(shift_out);
}
//![Example]
