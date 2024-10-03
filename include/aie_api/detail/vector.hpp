// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_VECTOR__HPP__
#define __AIE_API_DETAIL_VECTOR__HPP__

#include "config.hpp"
#include "utils.hpp"

#include <array>
#include <tuple>

namespace aie::detail {

template <typename T>
struct native_vector_length;

template <typename T>
static constexpr unsigned native_vector_length_v = native_vector_length<T>::value;

template <typename T>
struct type_bits
{
    static constexpr unsigned value = sizeof(T) * 8;
};

template <aie_dm_resource Resource, typename T> struct add_memory_bank { using type = T; };

template <typename T> struct add_memory_bank<aie_dm_resource::a,     T> { using type = T __aie_dm_resource_a; };
template <typename T> struct add_memory_bank<aie_dm_resource::b,     T> { using type = T __aie_dm_resource_b; };
template <typename T> struct add_memory_bank<aie_dm_resource::c,     T> { using type = T __aie_dm_resource_c; };
template <typename T> struct add_memory_bank<aie_dm_resource::d,     T> { using type = T __aie_dm_resource_d; };
#if __AIE_API_COMPOUND_DM_RESOURCE__
template <typename T> struct add_memory_bank<aie_dm_resource::ab,    T> { using type = T __aie_dm_resource_ab; };
template <typename T> struct add_memory_bank<aie_dm_resource::ac,    T> { using type = T __aie_dm_resource_ac; };
template <typename T> struct add_memory_bank<aie_dm_resource::ad,    T> { using type = T __aie_dm_resource_ad; };
template <typename T> struct add_memory_bank<aie_dm_resource::bc,    T> { using type = T __aie_dm_resource_bc; };
template <typename T> struct add_memory_bank<aie_dm_resource::bd,    T> { using type = T __aie_dm_resource_bd; };
template <typename T> struct add_memory_bank<aie_dm_resource::cd,    T> { using type = T __aie_dm_resource_cd; };
#endif

template <typename T> struct add_memory_bank<aie_dm_resource::stack, T> { using type = T __aie_dm_resource_stack; };

template <aie_dm_resource Resource, typename T>
using add_memory_bank_t = typename add_memory_bank<Resource, T>::type;

template <typename T, unsigned Elems>
struct vector_ldst_align;

} // namespace aie::detail

#endif
