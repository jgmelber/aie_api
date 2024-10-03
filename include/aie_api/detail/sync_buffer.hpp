// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_SYNC_BUFFER__HPP__
#define __AIE_API_DETAIL_SYNC_BUFFER__HPP__

#include "lock.hpp"
#include "vector.hpp"
#include "mdspan.hpp"

namespace aie::detail::sync {

enum class direction
{
    Input,
    Output
};

template <direction Direction,
          typename Span,
          unsigned NumBuffers,
          unsigned NumReaders,
          unsigned NumWriters,
          typename = std::make_index_sequence<NumBuffers>>
class sync_data_impl;

}

#if __AIE_ARCH__ == 10

#include "aie1/sync_buffer.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/sync_buffer.hpp"

#endif

namespace aie::detail::sync {

template <typename Span, unsigned NumBuffers, unsigned NumReaders = 1, unsigned NumWriters = 1>
using input  = sync_data_impl<direction::Input,  Span, NumBuffers, NumReaders, NumWriters, std::make_index_sequence<NumBuffers>>;

template <typename Span, unsigned NumBuffers, unsigned NumReaders = 1, unsigned NumWriters = 1>
using output = sync_data_impl<direction::Output, Span, NumBuffers, NumReaders, NumWriters, std::make_index_sequence<NumBuffers>>;

}

#endif
