// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__

#include "../accum.hpp"

namespace aie::detail {

template <typename T>
struct is_valid_block_type;

template <> struct is_valid_block_type<bfp16ebs8>  { static constexpr bool value = true; };
template <> struct is_valid_block_type<bfp16ebs16> { static constexpr bool value = true; };

template <typename T, unsigned Elems>
struct block_vector_storage;

template <typename T, unsigned Elems>
using block_vector_storage_t = typename block_vector_storage<T, Elems>::type;

template <> struct block_vector_storage<bfp16ebs8,   64> { using type =  v64bfp16ebs8;  static type undef() { return ::undef_v64bfp16ebs8();   } using pointer_type =  v64bfp16ebs8_unaligned; };
template <> struct block_vector_storage<bfp16ebs8,  128> { using type = v128bfp16ebs8;  static type undef() { return ::undef_v128bfp16ebs8();  } using pointer_type = v128bfp16ebs8_unaligned; };

template <> struct block_vector_storage<bfp16ebs16,  64> { using type =  v64bfp16ebs16; static type undef() { return ::undef_v64bfp16ebs16();  } using pointer_type =  v64bfp16ebs16_unaligned; };
template <> struct block_vector_storage<bfp16ebs16, 128> { using type = v128bfp16ebs16; static type undef() { return ::undef_v128bfp16ebs16(); } using pointer_type = v128bfp16ebs16_unaligned; };

template <typename T, unsigned Elems>
struct native_block_vector_type;

template <> struct native_block_vector_type< bfp16ebs8,  64> { using type =  v64bfp16ebs8; };
template <> struct native_block_vector_type< bfp16ebs8, 128> { using type = v128bfp16ebs8; };

template <> struct native_block_vector_type<bfp16ebs16,  64> { using type =  v64bfp16ebs16; };
template <> struct native_block_vector_type<bfp16ebs16, 128> { using type = v128bfp16ebs16; };

template <typename T, unsigned Elems>
using native_block_vector_type_t = typename native_block_vector_type<T, Elems>::type;

#if __AIE_API_HAS_EXTRACT_V64BFP16__

template <unsigned Elems, typename T> static auto vector_extract(const T &v, unsigned idx);

template <> inline auto vector_extract<64, v128bfp16ebs8> (const v128bfp16ebs8  &v, unsigned idx) { return ::extract_v64bfp16ebs8 (v, idx); };
template <> inline auto vector_extract<64, v128bfp16ebs16>(const v128bfp16ebs16 &v, unsigned idx) { return ::extract_v64bfp16ebs16(v, idx); };

#endif

template <typename T> struct native_vector_traits;

template <> struct native_vector_traits<  v64bfp16ebs8  > { using value_type = bfp16ebs8;  static constexpr unsigned size =  64; };
template <> struct native_vector_traits< v128bfp16ebs8  > { using value_type = bfp16ebs8;  static constexpr unsigned size = 128; };

template <> struct native_vector_traits< v64bfp16ebs16  > { using value_type = bfp16ebs16; static constexpr unsigned size =  64; };
template <> struct native_vector_traits<v128bfp16ebs16  > { using value_type = bfp16ebs16; static constexpr unsigned size = 128; };

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__
