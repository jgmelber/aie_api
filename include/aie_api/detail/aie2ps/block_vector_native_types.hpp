// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2PS_BLOCK_VECTOR_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE2PS_BLOCK_VECTOR_NATIVE_TYPES__HPP__

#include "../accum.hpp"

namespace aie::detail {

template <> struct is_valid_block_type<mx4> : std::true_type {};
template <> struct is_valid_block_type<mx6> : std::true_type {};
template <> struct is_valid_block_type<mx9> : std::true_type {};

template <typename T>
struct native_block_vector_length
{
    static constexpr unsigned value = std::is_same_v<T, mx9> ? 64 : 128;
};

template <typename T> struct block_vector_fill_frequency;
template <>           struct block_vector_fill_frequency<mx4> { static constexpr unsigned value = 0; };
template <>           struct block_vector_fill_frequency<mx6> { static constexpr unsigned value = 2; };
template <>           struct block_vector_fill_frequency<mx9> { static constexpr unsigned value = 8; };

template <typename T>
static constexpr unsigned block_vector_fill_frequency_v = block_vector_fill_frequency<T>::value;

template <> struct block_vector_storage<mx4,  64> { using type =  v64mx4;  static type undef() { return ::undef_v64mx4();   } using pointer_type =  v64mx4_unaligned;/*FIXME*/ };
template <> struct block_vector_storage<mx4, 128> { using type = v128mx4;  static type undef() { return ::undef_v128mx4();  } using pointer_type = v128mx4_unaligned;          };
template <> struct block_vector_storage<mx4, 256> { using type = v256mx4;  static type undef() { return ::undef_v256mx4();  } using pointer_type = v128mx4_unaligned;          };

template <> struct block_vector_storage<mx6,  64> { using type =  v64mx6;  static type undef() { return ::undef_v64mx6();   } using pointer_type =  v64mx6_unaligned;/*FIXME*/ };
template <> struct block_vector_storage<mx6, 128> { using type = v128mx6;  static type undef() { return ::undef_v128mx6();  } using pointer_type = v128mx6_unaligned;          };
template <> struct block_vector_storage<mx6, 256> { using type = v256mx6;  static type undef() { return ::undef_v256mx6();  } using pointer_type = v128mx6_unaligned;          };

template <> struct block_vector_storage<mx9,  64> { using type =  v64mx9;  static type undef() { return ::undef_v64mx9();   } using pointer_type =  v64mx9_unaligned;          };
template <> struct block_vector_storage<mx9, 128> { using type = v128mx9;  static type undef() { return ::undef_v128mx9();  } using pointer_type =  v64mx9_unaligned;/*FIXME*/ };
template <> struct block_vector_storage<mx9, 256> { using type = v256mx9;  static type undef() { return ::undef_v256mx9();  } using pointer_type =  v64mx9_unaligned;/*FIXME*/ };

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

template <unsigned N> requires (N >= 512) struct block_vector_storage<mx4, N> : compound_block_vector_storage<N, block_vector_storage<mx4, 256>> {};
template <unsigned N> requires (N >= 512) struct block_vector_storage<mx6, N> : compound_block_vector_storage<N, block_vector_storage<mx6, 256>> {};
template <unsigned N> requires (N >= 512) struct block_vector_storage<mx9, N> : compound_block_vector_storage<N, block_vector_storage<mx9, 256>> {};


template <>               struct native_block_vector_type<mx4,    64> { using type =  v64mx4; };
template <>               struct native_block_vector_type<mx4,   128> { using type = v128mx4; };
template <unsigned Elems> struct native_block_vector_type<mx4, Elems> { using type = v256mx4; };

template <>               struct native_block_vector_type<mx6,    64> { using type =  v64mx6; };
template <>               struct native_block_vector_type<mx6,   128> { using type = v128mx6; };
template <unsigned Elems> struct native_block_vector_type<mx6, Elems> { using type = v256mx6; };

template <>               struct native_block_vector_type<mx9,    32> { using type =  v32mx9; };
template <>               struct native_block_vector_type<mx9,    64> { using type =  v64mx9; };
template <>               struct native_block_vector_type<mx9,   128> { using type = v128mx9; };
template <unsigned Elems> struct native_block_vector_type<mx9, Elems> { using type = v256mx9; };

template <typename T, unsigned Elems>
using native_block_vector_type_t = typename native_block_vector_type<T, Elems>::type;


template <typename T, unsigned Elems> struct block_vector_set;

template <> struct block_vector_set<mx4,        128> { static v128mx4         run(const v64mx4        &v, unsigned idx) { return ::set_v128mx4(idx, v);        } };
template <> struct block_vector_set<mx4,        256> { static v256mx4         run(const v128mx4       &v, unsigned idx) { return ::set_v256mx4(idx, v);        } };
template <> struct block_vector_set<mx6,        128> { static v128mx6         run(const v64mx6        &v, unsigned idx) { return ::set_v128mx6(idx, v);        } };
template <> struct block_vector_set<mx6,        256> { static v256mx6         run(const v128mx6       &v, unsigned idx) { return ::set_v256mx6(idx, v);        } };
template <> struct block_vector_set<mx9,        128> { static v128mx9         run(const v64mx9        &v, unsigned idx) { return ::set_v128mx9(idx, v);        } };

template <unsigned Elems, typename T> static auto vector_extract(const T &v, unsigned idx);

template <> inline auto vector_extract< 64, v128mx4> (const v128mx4 &v, unsigned idx) { return ::extract_v64mx4 (v, idx); };
template <> inline auto vector_extract< 64, v256mx4> (const v256mx4 &v, unsigned idx) { return ::extract_v64mx4 (v, idx); };
template <> inline auto vector_extract<128, v256mx4> (const v256mx4 &v, unsigned idx) { return ::extract_v128mx4(v, idx); };

template <> inline auto vector_extract< 64, v128mx6> (const v128mx6 &v, unsigned idx) { return ::extract_v64mx6 (v, idx); };
template <> inline auto vector_extract< 64, v256mx6> (const v256mx6 &v, unsigned idx) { return ::extract_v64mx6 (v, idx); };
template <> inline auto vector_extract<128, v256mx6> (const v256mx6 &v, unsigned idx) { return ::extract_v128mx6(v, idx); };

template <> inline auto vector_extract< 64, v128mx9> (const v128mx9 &v, unsigned idx) { return ::extract_v64mx9 (v, idx); };
template <> inline auto vector_extract< 64, v256mx9> (const v256mx9 &v, unsigned idx) { return ::extract_v64mx9 (::extract_v128mx9(v, idx / 2), idx % 2); };
template <> inline auto vector_extract<128, v256mx9> (const v256mx9 &v, unsigned idx) { return ::extract_v128mx9(v, idx); };


template <typename T> struct native_vector_traits;

template <> struct native_vector_traits< v64mx4> { using value_type = mx4;  static constexpr unsigned size =  64; };
template <> struct native_vector_traits<v128mx4> { using value_type = mx4;  static constexpr unsigned size = 128; };
template <> struct native_vector_traits<v256mx4> { using value_type = mx4;  static constexpr unsigned size = 256; };

template <> struct native_vector_traits< v64mx6> { using value_type = mx6;  static constexpr unsigned size =  64; };
template <> struct native_vector_traits<v128mx6> { using value_type = mx6;  static constexpr unsigned size = 128; };
template <> struct native_vector_traits<v256mx6> { using value_type = mx6;  static constexpr unsigned size = 256; };

template <> struct native_vector_traits< v64mx9> { using value_type = mx9;  static constexpr unsigned size =  64; };
template <> struct native_vector_traits<v128mx9> { using value_type = mx9;  static constexpr unsigned size = 128; };
template <> struct native_vector_traits<v256mx9> { using value_type = mx9;  static constexpr unsigned size = 256; };

template <> struct type_bits<mx4>        { static constexpr unsigned value = 3; };
template <> struct type_bits<mx6>        { static constexpr unsigned value = 5; };
template <> struct type_bits<mx9>        { static constexpr unsigned value = 8; };

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2PS_BLOCK_VECTOR_NATIVE_TYPES__HPP__

