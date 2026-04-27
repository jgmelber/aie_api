// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>
#include <aie_api/sliding_mul.hpp>
#include <aie_api/adf/stream.hpp>

//![Example mul and mac]
constexpr unsigned Lanes  = 16;
constexpr unsigned Points = 32;
using sliding_mul_t = aie::partial_sliding_mul<Lanes, Points / 2, 1, 1, 1, cint16, cint16, cacc64>;

sliding_mul_t kernel(const cint16 *coeff, const cint16 *data) {

    aie::vector coeffs = aie::load_v<32>(coeff);
    aie::vector_iterator data_it = aie::begin_vector<32>(data);

    unsigned c_start = 0, d_start = 0;
    sliding_mul_t result;
    result.mul(coeffs, c_start, *data_it++, d_start);
    result.mac(coeffs, c_start, *data_it++, d_start);

    return result;
}
//![Example mul and mac]

void kernel(input_cascade<cacc64> &in, output_cascade<cacc64> &out, const cint16 *coeff, const cint16 *data) {
    aie::vector coeffs = aie::load_v<32>(coeff);
    aie::vector_iterator data_it = aie::begin_vector<32>(data);

    unsigned c_start = 0, d_start = 0;
    sliding_mul_t result;

    in >> result;
    result.mac(coeffs, c_start, *data_it++, d_start);

    out << result;
}
