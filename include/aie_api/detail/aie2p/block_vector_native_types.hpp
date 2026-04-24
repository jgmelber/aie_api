// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__

#include "../accum.hpp"

namespace aie::detail {

template <> struct is_valid_block_type<bfp16ebs8>  : std::true_type {};
template <> struct is_valid_block_type<bfp16ebs16> : std::true_type {};

template <typename T>
struct native_block_vector_length
{
    static constexpr unsigned value = 64;
};

template <typename T> struct block_vector_fill_frequency;
template <>           struct block_vector_fill_frequency<bfp16ebs8 > { static constexpr unsigned value = 8;  };
template <>           struct block_vector_fill_frequency<bfp16ebs16> { static constexpr unsigned value = 16; };

template <typename T>
static constexpr unsigned block_vector_fill_frequency_v = block_vector_fill_frequency<T>::value;

template <> struct block_vector_storage<bfp16ebs8,   64> { using type =  v64bfp16ebs8;  static type undef() { return ::undef_v64bfp16ebs8();   } using pointer_type =  v64bfp16ebs8_unaligned; };
template <> struct block_vector_storage<bfp16ebs8,  128> { using type = v128bfp16ebs8;  static type undef() { return ::undef_v128bfp16ebs8();  } using pointer_type = v128bfp16ebs8_unaligned; };

template <> struct block_vector_storage<bfp16ebs16,  64> { using type =  v64bfp16ebs16; static type undef() { return ::undef_v64bfp16ebs16();  } using pointer_type =  v64bfp16ebs16_unaligned; };
template <> struct block_vector_storage<bfp16ebs16, 128> { using type = v128bfp16ebs16; static type undef() { return ::undef_v128bfp16ebs16(); } using pointer_type = v128bfp16ebs16_unaligned; };

template <unsigned Elems, typename NativeStorage>
struct compound_block_vector_storage
{
    static_assert(utils::is_powerof2(Elems), "Vector sizes are required to be powers of two");

    static constexpr unsigned native_elems = native_vector_traits<typename NativeStorage::type>::size;
    static constexpr unsigned N = Elems / native_elems;
    using native_type = typename NativeStorage::type;

    using type = std::array<native_type, N>;
    static type undef() { return utils::make_array<N>(NativeStorage::undef); }

    using pointer_type = typename NativeStorage::pointer_type;
};

template <unsigned N> requires (N >= 256) struct block_vector_storage<bfp16ebs8,     N> : compound_block_vector_storage<N, block_vector_storage<bfp16ebs8,  128>> {};
template <unsigned N> requires (N >= 256) struct block_vector_storage<bfp16ebs16,    N> : compound_block_vector_storage<N, block_vector_storage<bfp16ebs16, 128>> {};

template <>               struct native_block_vector_type<bfp16ebs8,     64> { using type =  v64bfp16ebs8; };
template <unsigned Elems> struct native_block_vector_type<bfp16ebs8,  Elems> { using type = v128bfp16ebs8; };

template <>               struct native_block_vector_type<bfp16ebs16,    64> { using type =  v64bfp16ebs16; };
template <unsigned Elems> struct native_block_vector_type<bfp16ebs16, Elems> { using type = v128bfp16ebs16; };

template <typename T, unsigned Elems> struct block_vector_set;

template <> struct block_vector_set<bfp16ebs8,  128> { static v128bfp16ebs8   run(const v64bfp16ebs8  &v, unsigned idx) { return ::set_v128bfp16ebs8(idx, v);  } };
template <> struct block_vector_set<bfp16ebs16, 128> { static v128bfp16ebs16  run(const v64bfp16ebs16 &v, unsigned idx) { return ::set_v128bfp16ebs16(idx, v); } };

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

template <> struct type_bits<bfp16ebs8>        { static constexpr unsigned value = 8; };
template <> struct type_bits<bfp16ebs16>       { static constexpr unsigned value = 8; };

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR_NATIVE_TYPES__HPP__
