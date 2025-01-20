// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief AIE API types declaration. This is useful in graph compilation scenarios where the aie.hpp has been included
 * in kernel headers but the compiler doesn't support C++20.
 */

#pragma once

#ifndef __AIE_API_AIE_DECLARATION_HPP__
#define __AIE_API_AIE_DECLARATION_HPP__

#include <cstddef>
#include <limits>

enum class aie_dm_resource {
    none
};

namespace aie {

template <typename T, unsigned Elems>
class vector {};

template <typename AccumTag, unsigned Elems>
class accum {};

template <unsigned Elems>
class mask {};

struct accauto {};

template <unsigned M, unsigned K, unsigned N,
          typename TypeA, typename TypeB = TypeA,
          typename AccumTag = accauto>
struct mmul {};


template <unsigned Stage, unsigned Radix, typename T1, typename T2=T1>
struct fft_dit {};

template <typename T>
struct cfr {};

static constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

template <typename T, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct              circular_iterator {};

template <typename T, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct        const_circular_iterator {};

template <typename T, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct       random_circular_iterator {};

template <typename T, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct const_random_circular_iterator {};

template <typename T, unsigned Steps>
struct               pattern_iterator {};

template <typename T, unsigned Steps>
struct         const_pattern_iterator {};

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
struct                vector_iterator {};

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
struct       restrict_vector_iterator {};

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
struct          const_vector_iterator {};

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
struct const_restrict_vector_iterator {};

template <typename T, unsigned N, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct       vector_circular_iterator {};

template <typename T, unsigned N, size_t Elems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct const_vector_circular_iterator {};

template <typename T, unsigned Elems, size_t ArrayElems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct       vector_random_circular_iterator {};

template <typename T, unsigned Elems, size_t ArrayElems = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
struct const_vector_random_circular_iterator {};

// These two are useful for global variable declaration
static constexpr unsigned vector_decl_align   = 4;

template <typename T, unsigned Elems>
static constexpr unsigned vector_ldst_align_v = 4;

}

#endif
