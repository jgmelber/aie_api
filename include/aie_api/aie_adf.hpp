// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Wrappers for ADF graph types
 */

#pragma once

#ifndef __AIE_API_AIE_ADF_HPP__
#define __AIE_API_AIE_ADF_HPP__

// AIE API currently requires C++20
#define AIE_API_CXX_VERSION 202002L

// TODO: When host/graph compiler is updated to C++20 we will need to change this logic
#if __cplusplus < AIE_API_CXX_VERSION

#ifdef __chess__

#error "C++20 or greater is required to compile AIE API kernels"

#endif

#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "adf/stream.hpp"
#ifndef __AIECC__
#include "adf/window.hpp"
#endif
#include "adf/io_buffer.hpp"

#pragma GCC diagnostic pop

#endif

#endif
