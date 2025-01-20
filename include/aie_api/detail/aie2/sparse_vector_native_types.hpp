// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SPARSE_VECTOR_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE2_SPARSE_VECTOR_NATIVE_TYPES__HPP__

namespace aie::detail {

template <typename T, unsigned Elems>
struct sparse_vector_storage;

// No undef?
#if __AIE_ARCH__ == 20
template <> struct sparse_vector_storage< int4,    256> { using type =                 v256int4_sparse; using pointer_type =    v256int4_sparse_compress; };
template <> struct sparse_vector_storage<uint4,    256> { using type =                v256uint4_sparse; using pointer_type =   v256uint4_sparse_compress; };

template <> struct sparse_vector_storage< int8,    128> { using type =                 v128int8_sparse; using pointer_type =    v128int8_sparse_compress; };
template <> struct sparse_vector_storage<uint8,    128> { using type =                v128uint8_sparse; using pointer_type =   v128uint8_sparse_compress; };

template <> struct sparse_vector_storage< int8,    256> { using type = std::array< v128int8_sparse, 2>; using pointer_type =    v128int8_sparse_compress; };
template <> struct sparse_vector_storage<uint8,    256> { using type = std::array<v128uint8_sparse, 2>; using pointer_type =   v128uint8_sparse_compress; };

template <> struct sparse_vector_storage< int16,    64> { using type =                 v64int16_sparse; using pointer_type =    v64int16_sparse_compress; };
template <> struct sparse_vector_storage<uint16,    64> { using type =                v64uint16_sparse; using pointer_type =   v64uint16_sparse_compress; };

template <> struct sparse_vector_storage< int16,   128> { using type = std::array< v64int16_sparse, 2>; using pointer_type =    v64int16_sparse_compress; };
template <> struct sparse_vector_storage<uint16,   128> { using type = std::array<v64uint16_sparse, 2>; using pointer_type =   v64uint16_sparse_compress; };

template <> struct sparse_vector_storage<bfloat16,  64> { using type =                v64bfloat16_sparse; using pointer_type = v64bfloat16_sparse_compress; };
template <> struct sparse_vector_storage<bfloat16, 128> { using type = std::array<v64bfloat16_sparse, 2>; using pointer_type = v64bfloat16_sparse_compress; };
#else
template <> struct sparse_vector_storage<  int4,   256> { using type =                 v256int4_sparse; using pointer_type =    v256int4_sparse_unaligned; };
template <> struct sparse_vector_storage< uint4,   256> { using type =                v256uint4_sparse; using pointer_type =   v256uint4_sparse_unaligned; };

template <> struct sparse_vector_storage<  int8,   128> { using type =                 v128int8_sparse; using pointer_type =    v128int8_sparse_unaligned; };
template <> struct sparse_vector_storage< uint8,   128> { using type =                v128uint8_sparse; using pointer_type =   v128uint8_sparse_unaligned; };

template <> struct sparse_vector_storage<  int8,   256> { using type =                 v256int8_sparse; using pointer_type =    v256int8_sparse_unaligned; };
template <> struct sparse_vector_storage< uint8,   256> { using type =                v256uint8_sparse; using pointer_type =   v256uint8_sparse_unaligned; };

template <> struct sparse_vector_storage< int16,    64> { using type =                 v64int16_sparse; using pointer_type =    v64int16_sparse_unaligned; };
template <> struct sparse_vector_storage<uint16,    64> { using type =                v64uint16_sparse; using pointer_type =   v64uint16_sparse_unaligned; };

template <> struct sparse_vector_storage< int16,   128> { using type =                v128int16_sparse; using pointer_type =   v128int16_sparse_unaligned; };
template <> struct sparse_vector_storage<uint16,   128> { using type =               v128uint16_sparse; using pointer_type =  v128uint16_sparse_unaligned; };

#endif

template <typename T>
struct is_valid_sparse_element_type
{
#if __AIE_ARCH__ == 20
    static constexpr bool value = utils::is_one_of_v<T, int8, uint8, int16, uint16, int4, uint4, bfloat16>;
#else
    static constexpr bool value = utils::is_one_of_v<T, int8, uint8, int16, uint16, int4, uint4>;
#endif
};

template <typename T, unsigned Elems>
struct native_sparse_vector_type;

template <> struct native_sparse_vector_type< int4,    256> { using type =  v256int4_sparse; };
template <> struct native_sparse_vector_type<uint4,    256> { using type = v256uint4_sparse; };

template <> struct native_sparse_vector_type< int8,    128> { using type =  v128int8_sparse; };
template <> struct native_sparse_vector_type<uint8,    128> { using type = v128uint8_sparse; };

template <> struct native_sparse_vector_type< int16,    64> { using type =  v64int16_sparse; };
template <> struct native_sparse_vector_type<uint16,    64> { using type = v64uint16_sparse; };

#if __AIE_ARCH__ == 20
template <> struct native_sparse_vector_type<bfloat16,  64> { using type =  v64bfloat16_sparse; };
#endif

template <typename T> struct native_vector_traits;
#if __AIE_ARCH__ == 20
template <> struct native_vector_traits<    v256int4_sparse> { using value_type =     int4; static constexpr unsigned size = 256; };
template <> struct native_vector_traits<   v256uint4_sparse> { using value_type =    uint4; static constexpr unsigned size = 256; };

template <> struct native_vector_traits<    v128int8_sparse> { using value_type =     int8; static constexpr unsigned size = 128; };
template <> struct native_vector_traits<   v128uint8_sparse> { using value_type =    uint8; static constexpr unsigned size = 128; };

template <> struct native_vector_traits<    v64int16_sparse> { using value_type =    int16; static constexpr unsigned size =  64; };
template <> struct native_vector_traits<   v64uint16_sparse> { using value_type =   uint16; static constexpr unsigned size =  64; };

template <> struct native_vector_traits< v64bfloat16_sparse> { using value_type = bfloat16; static constexpr unsigned size =  64; };
#else
template <> struct native_vector_traits<    v256int4_sparse> { using value_type =     int4; static constexpr unsigned size = 256; };
template <> struct native_vector_traits<   v256uint4_sparse> { using value_type =    uint4; static constexpr unsigned size = 256; };

template <> struct native_vector_traits<    v128int8_sparse> { using value_type =     int8; static constexpr unsigned size = 128; };
template <> struct native_vector_traits<   v128uint8_sparse> { using value_type =    uint8; static constexpr unsigned size = 128; };

template <> struct native_vector_traits<    v256int8_sparse> { using value_type =     int8; static constexpr unsigned size = 256; };
template <> struct native_vector_traits<   v256uint8_sparse> { using value_type =    uint8; static constexpr unsigned size = 256; };

template <> struct native_vector_traits<    v64int16_sparse> { using value_type =    int16; static constexpr unsigned size =  64; };
template <> struct native_vector_traits<   v64uint16_sparse> { using value_type =   uint16; static constexpr unsigned size =  64; };

template <> struct native_vector_traits<   v128int16_sparse> { using value_type =    int16; static constexpr unsigned size = 128; };
template <> struct native_vector_traits<  v128uint16_sparse> { using value_type =   uint16; static constexpr unsigned size = 128; };

#endif

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2_SPARSE_VECTOR_NATIVE_TYPES__HPP__

