// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

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

template <typename T> struct internal_type_bits;

template <typename T>
static constexpr unsigned internal_type_bits_v = internal_type_bits<T>::value;

template <typename T>
struct internal_type_bits 
{ 
    static constexpr unsigned value =  type_bits_v<T>;
};

template <aie_dm_resource Resource, typename T> struct add_memory_bank { using type = T; };

// If the dm resource is explicitly set via template parameter it will override the resource set by
// the pointer type and therefore we remove the resource set on T in the following specializations.
template <typename T> struct add_memory_bank<aie_dm_resource::a,  T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_a; };
template <typename T> struct add_memory_bank<aie_dm_resource::b,  T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_b; };
template <typename T> struct add_memory_bank<aie_dm_resource::c,  T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_c; };
template <typename T> struct add_memory_bank<aie_dm_resource::d,  T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_d; };
template <typename T> struct add_memory_bank<aie_dm_resource::ab, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_ab; };
template <typename T> struct add_memory_bank<aie_dm_resource::ac, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_ac; };
template <typename T> struct add_memory_bank<aie_dm_resource::ad, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_ad; };
template <typename T> struct add_memory_bank<aie_dm_resource::bc, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_bc; };
template <typename T> struct add_memory_bank<aie_dm_resource::bd, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_bd; };
template <typename T> struct add_memory_bank<aie_dm_resource::cd, T> { using type = aie_dm_resource_remove_t<T> __aie_dm_resource_cd; };

template <typename T> struct add_memory_bank<aie_dm_resource::stack, T> { using type = T __aie_dm_resource_stack; };

template <aie_dm_resource Resource, typename T>
using add_memory_bank_t = typename add_memory_bank<Resource, T>::type;

template <typename T, unsigned Elems>
struct vector_ldst_align;

} // namespace aie::detail

#endif
