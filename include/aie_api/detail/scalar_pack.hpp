// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_SCALAR_PACK__HPP__
#define __AIE_API_DETAIL_SCALAR_PACK__HPP__

namespace aie::detail {

// Creates a scalar value from multiple narrower scalar values
// Example: create 32bit scalar from 4 int8 values or from 8 int4 values
template <typename T, unsigned N>
auto pack_to_scalar(const T (&a)[N]) {
    auto init = [](T v) __aie_inline {
        if constexpr (std::is_same_v<T, int4>) {
            if constexpr (N == 2)
                return ::set_v2int4(0, v);
            if constexpr (N == 4)
                return ::set_v4int4(0, v);
            if constexpr (N == 8)
                return ::set_v8int4(0, v);
            if constexpr (N == 16)
                return ::set_v16int4(0, v);
        }
        else if constexpr (std::is_same_v<T, uint4>) {
            if constexpr (N == 2)
                return ::set_v2uint4(0, v);
            if constexpr (N == 4)
                return ::set_v4uint4(0, v);
            if constexpr (N == 8)
                return ::set_v8uint4(0, v);
            if constexpr (N == 16)
                return ::set_v16uint4(0, v);
        }
        else
        if constexpr (std::is_same_v<T, int8>) {
            if constexpr (N == 2)
                return ::set_v2int8(0, v);
            if constexpr (N == 4)
                return ::set_v4int8(0, v);
            if constexpr (N == 8)
                return ::set_v8int8(0, v);
        }
        else if constexpr (std::is_same_v<T, uint8>) {
            if constexpr (N == 2)
                return ::set_v2uint8(0, v);
            if constexpr (N == 4)
                return ::set_v4uint8(0, v);
            if constexpr (N == 8)
                return ::set_v8uint8(0, v);
        }
        else if constexpr (std::is_same_v<T, int16>) {
            if constexpr (N == 2)
                return ::set_v2int16(0, v);
            if constexpr (N == 4)
                return ::set_v4int16(0, v);
        }
        else if constexpr (std::is_same_v<T, uint16>) {
            if constexpr (N == 2)
                return ::set_v2uint16(0, v);
            if constexpr (N == 4)
                return ::set_v4uint16(0, v);
        }
        else if constexpr (std::is_same_v<T, int32>) {
            if constexpr (N == 2)
                return ::set_v2int32(0, v);
        }
        else if constexpr (std::is_same_v<T, uint32>) {
            if constexpr (N == 2)
                return ::set_v2uint32(0, v);
        }
    };

    auto value = init(a[0]);

    // If we don't insert 0 twice, result may not always be correct.
    // See CRVO-11943
    #pragma unroll
    for (unsigned i = 0; i < N; ++i) {
        value = ::insert(value, i, a[i]);
    }
    return value;
}

} // namespace aie::detail

#endif // __AIE_API_DETAIL_SCALAR_PACK__HPP__
