// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_VECTOR_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE2_VECTOR_NATIVE_TYPES__HPP__

namespace aie::detail {

template <typename T, unsigned Elems>
struct native_vector_type
{
    using type = int;
};

template <> struct native_vector_type<int8,      16> { using type =      v16int8; };
template <> struct native_vector_type<int8,      32> { using type =      v32int8; };
template <> struct native_vector_type<int8,      64> { using type =      v64int8; };
template <> struct native_vector_type<int8,     128> { using type =     v128int8; };

template <> struct native_vector_type<uint8,     16> { using type =     v16uint8; };
template <> struct native_vector_type<uint8,     32> { using type =     v32uint8; };
template <> struct native_vector_type<uint8,     64> { using type =     v64uint8; };
template <> struct native_vector_type<uint8,    128> { using type =    v128uint8; };

template <> struct native_vector_type<int16,      8> { using type =      v8int16; };
template <> struct native_vector_type<int16,     16> { using type =     v16int16; };
template <> struct native_vector_type<int16,     32> { using type =     v32int16; };
template <> struct native_vector_type<int16,     64> { using type =     v64int16; };

template <> struct native_vector_type<uint16,     8> { using type =     v8uint16; };
template <> struct native_vector_type<uint16,    16> { using type =    v16uint16; };
template <> struct native_vector_type<uint16,    32> { using type =    v32uint16; };
template <> struct native_vector_type<uint16,    64> { using type =    v64uint16; };

template <> struct native_vector_type<int32,      4> { using type =      v4int32; };
template <> struct native_vector_type<int32,      8> { using type =      v8int32; };
template <> struct native_vector_type<int32,     16> { using type =     v16int32; };
template <> struct native_vector_type<int32,     32> { using type =     v32int32; };

template <> struct native_vector_type<uint32,     4> { using type =     v4uint32; };
template <> struct native_vector_type<uint32,     8> { using type =     v8uint32; };
template <> struct native_vector_type<uint32,    16> { using type =    v16uint32; };
template <> struct native_vector_type<uint32,    32> { using type =    v32uint32; };

template <> struct native_vector_type<cint16,     4> { using type =     v4cint16; };
template <> struct native_vector_type<cint16,     8> { using type =     v8cint16; };
template <> struct native_vector_type<cint16,    16> { using type =    v16cint16; };
template <> struct native_vector_type<cint16,    32> { using type =    v32cint16; };

template <> struct native_vector_type<cint32,     2> { using type =     v2cint32; };
template <> struct native_vector_type<cint32,     4> { using type =     v4cint32; };
template <> struct native_vector_type<cint32,     8> { using type =     v8cint32; };
template <> struct native_vector_type<cint32,    16> { using type =    v16cint32; };

template <> struct native_vector_type<int4,      32> { using type =      v32int4; };
template <> struct native_vector_type<int4,      64> { using type =      v64int4; };
template <> struct native_vector_type<int4,     128> { using type =     v128int4; };
template <> struct native_vector_type<int4,     256> { using type =     v256int4; };

template <> struct native_vector_type<uint4,     32> { using type =     v32uint4; };
template <> struct native_vector_type<uint4,     64> { using type =     v64uint4; };
template <> struct native_vector_type<uint4,    128> { using type =    v128uint4; };
template <> struct native_vector_type<uint4,    256> { using type =    v256uint4; };

template <> struct native_vector_type<bfloat16,   8> { using type =   v8bfloat16; };
template <> struct native_vector_type<bfloat16,  16> { using type =  v16bfloat16; };
template <> struct native_vector_type<bfloat16,  32> { using type =  v32bfloat16; };
template <> struct native_vector_type<bfloat16,  64> { using type =  v64bfloat16; };

#if __AIE_API_FP32_EMULATION__
template <> struct native_vector_type<float,      4> { using type =     v4float; };
template <> struct native_vector_type<float,      8> { using type =     v8float; };
template <> struct native_vector_type<float,     16> { using type =    v16float; };
template <> struct native_vector_type<float,     32> { using type =    v32float; };
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <> struct native_vector_type<cbfloat16,  4> { using type =  v4cbfloat16; };
template <> struct native_vector_type<cbfloat16,  8> { using type =  v8cbfloat16; };
template <> struct native_vector_type<cbfloat16, 16> { using type = v16cbfloat16; };
template <> struct native_vector_type<cbfloat16, 32> { using type = v32cbfloat16; };
#endif

template <> struct native_vector_type<cfloat,     2> { using type =    v2cfloat; };
template <> struct native_vector_type<cfloat,     4> { using type =    v4cfloat; };
template <> struct native_vector_type<cfloat,     8> { using type =    v8cfloat; };
template <> struct native_vector_type<cfloat,    16> { using type =   v16cfloat; };
#endif

template <typename T, unsigned Elems>
using native_vector_type_t = typename native_vector_type<T, Elems>::type;


template <typename T> struct native_vector_traits;

template <> struct native_vector_traits<    v16int8> { using value_type =     int8; static constexpr unsigned size =  16; };
template <> struct native_vector_traits<    v32int8> { using value_type =     int8; static constexpr unsigned size =  32; };
template <> struct native_vector_traits<    v64int8> { using value_type =     int8; static constexpr unsigned size =  64; };

template <> struct native_vector_traits<   v16uint8> { using value_type =    uint8; static constexpr unsigned size =  16; };
template <> struct native_vector_traits<   v32uint8> { using value_type =    uint8; static constexpr unsigned size =  32; };
template <> struct native_vector_traits<   v64uint8> { using value_type =    uint8; static constexpr unsigned size =  64; };

template <> struct native_vector_traits<    v8int16> { using value_type =    int16; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<   v16int16> { using value_type =    int16; static constexpr unsigned size =  16; };
template <> struct native_vector_traits<   v32int16> { using value_type =    int16; static constexpr unsigned size =  32; };

template <> struct native_vector_traits<   v8uint16> { using value_type =   uint16; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<  v16uint16> { using value_type =   uint16; static constexpr unsigned size =  16; };
template <> struct native_vector_traits<  v32uint16> { using value_type =   uint16; static constexpr unsigned size =  32; };

template <> struct native_vector_traits<    v4int32> { using value_type =    int32; static constexpr unsigned size =   4; };
template <> struct native_vector_traits<    v8int32> { using value_type =    int32; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<   v16int32> { using value_type =    int32; static constexpr unsigned size =  16; };

template <> struct native_vector_traits<   v4uint32> { using value_type =   uint32; static constexpr unsigned size =   4; };
template <> struct native_vector_traits<   v8uint32> { using value_type =   uint32; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<  v16uint32> { using value_type =   uint32; static constexpr unsigned size =  16; };

template <> struct native_vector_traits<   v4cint16> { using value_type =   cint16; static constexpr unsigned size =   4; };
template <> struct native_vector_traits<   v8cint16> { using value_type =   cint16; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<  v16cint16> { using value_type =   cint16; static constexpr unsigned size =  16; };

template <> struct native_vector_traits<   v2cint32> { using value_type =   cint32; static constexpr unsigned size =   2; };
template <> struct native_vector_traits<   v4cint32> { using value_type =   cint32; static constexpr unsigned size =   4; };
template <> struct native_vector_traits<   v8cint32> { using value_type =   cint32; static constexpr unsigned size =   8; };

template <> struct native_vector_traits<    v32int4> { using value_type =     int4; static constexpr unsigned size =  32; };
template <> struct native_vector_traits<    v64int4> { using value_type =     int4; static constexpr unsigned size =  64; };
template <> struct native_vector_traits<   v128int4> { using value_type =     int4; static constexpr unsigned size = 128; };

template <> struct native_vector_traits<   v32uint4> { using value_type =    uint4; static constexpr unsigned size =  32; };
template <> struct native_vector_traits<   v64uint4> { using value_type =    uint4; static constexpr unsigned size =  64; };
template <> struct native_vector_traits<  v128uint4> { using value_type =    uint4; static constexpr unsigned size = 128; };

template <> struct native_vector_traits< v8bfloat16> { using value_type = bfloat16; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<v16bfloat16> { using value_type = bfloat16; static constexpr unsigned size =  16; };
template <> struct native_vector_traits<v32bfloat16> { using value_type = bfloat16; static constexpr unsigned size =  32; };

#if __AIE_API_FP32_EMULATION__
template <> struct native_vector_traits<    v4float> { using value_type =    float; static constexpr unsigned size =   4; };
template <> struct native_vector_traits<    v8float> { using value_type =    float; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<   v16float> { using value_type =    float; static constexpr unsigned size =  16; };
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <> struct native_vector_traits< v4cbfloat16> { using value_type = cbfloat16; static constexpr unsigned size =   4; };
template <> struct native_vector_traits< v8cbfloat16> { using value_type = cbfloat16; static constexpr unsigned size =   8; };
template <> struct native_vector_traits<v16cbfloat16> { using value_type = cbfloat16; static constexpr unsigned size =  16; };
#endif

template <> struct native_vector_traits<   v2cfloat> { using value_type =   cfloat; static constexpr unsigned size =  2; };
template <> struct native_vector_traits<   v4cfloat> { using value_type =   cfloat; static constexpr unsigned size =  4; };
template <> struct native_vector_traits<   v8cfloat> { using value_type =   cfloat; static constexpr unsigned size =  8; };
#endif

template <typename T, unsigned Elems>
struct vector_storage;

template <typename T, unsigned Elems>
using vector_storage_t = typename vector_storage<T, Elems>::type;

template <unsigned Elems, typename NativeStorage>
struct compound_vector_storage
{
    static_assert(utils::is_powerof2(Elems), "Vector sizes are required to be powers of two");

    static constexpr unsigned native_elems = native_vector_traits<typename NativeStorage::type>::size;
    static constexpr unsigned N = Elems / native_elems;
    using native_type = typename NativeStorage::type;

    using type = std::array<native_type, N>;
    static type undef() { return utils::make_array<N>(NativeStorage::undef); }
};

template <>                               struct vector_storage<     int8,  16> { using type =  v16int8;     static type undef() { return ::undef_v16int8();     } };
template <>                               struct vector_storage<     int8,  32> { using type =  v32int8;     static type undef() { return ::undef_v32int8();     } };
template <>                               struct vector_storage<     int8,  64> { using type =  v64int8;     static type undef() { return ::undef_v64int8();     } };
template <unsigned N> requires (N >= 128) struct vector_storage<     int8,   N> : compound_vector_storage<N, vector_storage<int8,  64>> {};

template <>                               struct vector_storage<    uint8,  16> { using type =  v16uint8;    static type undef() { return ::undef_v16uint8();    } };
template <>                               struct vector_storage<    uint8,  32> { using type =  v32uint8;    static type undef() { return ::undef_v32uint8();    } };
template <>                               struct vector_storage<    uint8,  64> { using type =  v64uint8;    static type undef() { return ::undef_v64uint8();    } };
template <unsigned N> requires (N >= 128) struct vector_storage<    uint8,   N> : compound_vector_storage<N, vector_storage<uint8, 64>> {};

template <>                               struct vector_storage<    int16,   8> { using type =  v8int16;    static type undef() { return ::undef_v8int16();     } };
template <>                               struct vector_storage<    int16,  16> { using type = v16int16;    static type undef() { return ::undef_v16int16();    } };
template <>                               struct vector_storage<    int16,  32> { using type = v32int16;    static type undef() { return ::undef_v32int16();    } };
template <unsigned N> requires (N >=  64) struct vector_storage<    int16,   N> : compound_vector_storage<N, vector_storage<int16, 32>> {};

template <>                               struct vector_storage<   uint16,   8> { using type =  v8uint16;   static type undef() { return ::undef_v8uint16();    } };
template <>                               struct vector_storage<   uint16,  16> { using type = v16uint16;   static type undef() { return ::undef_v16uint16();   } };
template <>                               struct vector_storage<   uint16,  32> { using type = v32uint16;   static type undef() { return ::undef_v32uint16();   } };
template <unsigned N> requires (N >=  64) struct vector_storage<   uint16,   N> : compound_vector_storage<N, vector_storage<uint16, 32>> {};

template <>                               struct vector_storage<    int32,   4> { using type =  v4int32;    static type undef() { return ::undef_v4int32();     } };
template <>                               struct vector_storage<    int32,   8> { using type =  v8int32;    static type undef() { return ::undef_v8int32();     } };
template <>                               struct vector_storage<    int32,  16> { using type = v16int32;    static type undef() { return ::undef_v16int32();    } };
template <unsigned N> requires (N >=  32) struct vector_storage<    int32,   N> : compound_vector_storage<N, vector_storage<int32, 16>> {};

template <>                               struct vector_storage<   uint32,   4> { using type =  v4uint32;   static type undef() { return ::undef_v4uint32();    } };
template <>                               struct vector_storage<   uint32,   8> { using type =  v8uint32;   static type undef() { return ::undef_v8uint32();    } };
template <>                               struct vector_storage<   uint32,  16> { using type = v16uint32;   static type undef() { return ::undef_v16uint32();   } };
template <unsigned N> requires (N >=  32) struct vector_storage<   uint32,   N> : compound_vector_storage<N, vector_storage<uint32, 16>> {};

template <>                               struct vector_storage<   cint16,   4> { using type =  v4cint16;   static type undef() { return ::undef_v4cint16();    } };
template <>                               struct vector_storage<   cint16,   8> { using type =  v8cint16;   static type undef() { return ::undef_v8cint16();    } };
template <>                               struct vector_storage<   cint16,  16> { using type = v16cint16;   static type undef() { return ::undef_v16cint16();   } };
template <unsigned N> requires (N >=  32) struct vector_storage<   cint16,   N> : compound_vector_storage<N, vector_storage<cint16, 16>> {};

template <>                               struct vector_storage<   cint32,   2> { using type =  v2cint32;    static type undef() { return ::undef_v2cint32();    } };
template <>                               struct vector_storage<   cint32,   4> { using type =  v4cint32;    static type undef() { return ::undef_v4cint32();    } };
template <>                               struct vector_storage<   cint32,   8> { using type =  v8cint32;    static type undef() { return ::undef_v8cint32();    } };
template <unsigned N> requires (N >=  16) struct vector_storage<   cint32,   N> : compound_vector_storage<N, vector_storage<cint32,  8>> {};

template <>                               struct vector_storage<     int4,  32> { using type =  v32int4;    static type undef() { return ::undef_v32int4();     } };
template <>                               struct vector_storage<     int4,  64> { using type =  v64int4;    static type undef() { return ::undef_v64int4();     } };
template <>                               struct vector_storage<     int4, 128> { using type = v128int4;    static type undef() { return ::undef_v128int4();    } };
template <unsigned N> requires (N >= 256) struct vector_storage<     int4,   N> : compound_vector_storage<N, vector_storage<int4, 128>> {};

template <>                               struct vector_storage<    uint4,  32> { using type =  v32uint4;   static type undef() { return ::undef_v32uint4();    } };
template <>                               struct vector_storage<    uint4,  64> { using type =  v64uint4;   static type undef() { return ::undef_v64uint4();    } };
template <>                               struct vector_storage<    uint4, 128> { using type = v128uint4;   static type undef() { return ::undef_v128uint4();   } };
template <unsigned N> requires (N >= 256) struct vector_storage<    uint4,   N> : compound_vector_storage<N, vector_storage<uint4, 128>> {};

template <>                               struct vector_storage< bfloat16,   8> { using type =  v8bfloat16; static type undef() { return ::undef_v8bfloat16();  } };
template <>                               struct vector_storage< bfloat16,  16> { using type = v16bfloat16; static type undef() { return ::undef_v16bfloat16(); } };
template <>                               struct vector_storage< bfloat16,  32> { using type = v32bfloat16; static type undef() { return ::undef_v32bfloat16(); } };
template <unsigned N> requires (N >=  64) struct vector_storage< bfloat16,   N> : compound_vector_storage<N, vector_storage<bfloat16, 32>> {};

#if __AIE_API_FP32_EMULATION__
template <>                               struct vector_storage<   float,    4> { using type =  v4float;    static type undef() { return ::undef_v4float();     } };
template <>                               struct vector_storage<   float,    8> { using type =  v8float;    static type undef() { return ::undef_v8float();     } };
template <>                               struct vector_storage<   float,   16> { using type = v16float;    static type undef() { return ::undef_v16float();    } };
template <unsigned N> requires (N >=  32) struct vector_storage<   float,    N> : compound_vector_storage<N, vector_storage<float, 16>> {};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <>                               struct vector_storage<  cfloat,    2> { using type =  v2cfloat;    static type undef() { return ::undef_v2cfloat();    } };
template <>                               struct vector_storage<  cfloat,    4> { using type =  v4cfloat;    static type undef() { return ::undef_v4cfloat();    } };
template <>                               struct vector_storage<  cfloat,    8> { using type =  v8cfloat;    static type undef() { return ::undef_v8cfloat();    } };
template <unsigned N> requires (N >=  16) struct vector_storage<  cfloat,    N> : compound_vector_storage<N, vector_storage<cfloat,  8>> {};
#endif

#if __AIE_API_CBF16_SUPPORT__
template <>                               struct vector_storage< cbfloat16,  4> { using type =  v4cbfloat16; static type undef() { return undef_v4cbfloat16();  } };
template <>                               struct vector_storage< cbfloat16,  8> { using type =  v8cbfloat16; static type undef() { return undef_v8cbfloat16();  } };
template <>                               struct vector_storage< cbfloat16, 16> { using type = v16cbfloat16; static type undef() { return undef_v16cbfloat16(); } };
template <unsigned N> requires (N >=  32) struct vector_storage< cbfloat16,  N> : compound_vector_storage<N, vector_storage<cbfloat16, 16>> {};
#endif

template <typename T>
struct is_valid_element_type
{
    static constexpr bool value = utils::is_one_of_v<T,
                                                     int8,
                                                     uint8,
                                                     int16,
                                                     int32,
                                                     uint16,
                                                     uint32,
                                                     cint16,
                                                     cint32,
                                                     int4,
                                                     uint4,
                                                     bfloat16
#if __AIE_API_FP32_EMULATION__
                                                     , float
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
                                                     , cbfloat16
#endif
                                                     , cfloat
#endif
                                                     >;
};

template <typename T>
struct is_complex
{
    static constexpr bool value = utils::is_one_of_v<T,
                                                     cint16,
                                                     cint32
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
                                                     , cbfloat16
#endif
                                                     , cfloat
#endif
                                                     >;
};

template <typename T>
struct is_integral
{
    static constexpr bool value = utils::is_one_of_v<T, int4, uint4, int8, uint8, int16, int32, uint16, uint32, cint16, cint32>;
};

template <typename T>
struct is_floating_point
{
    static constexpr bool value = utils::is_one_of_v<T,
                                                     bfloat16,
                                                     float
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
                                                     , cbfloat16
#endif
                                                     , cfloat
#endif
#if __AIE_ARCH__ == 21
                                                     , bfp16ebs8, bfp16ebs16
#endif
                                                         >;
};

#if __AIE_ARCH__ == 21

template <typename T> struct is_block_floating_point
{
    static constexpr bool value = detail::utils::is_one_of_v<T,
                                                             bfp16ebs8,
                                                             bfp16ebs16>;
};

#endif

template <typename T>
struct is_signed
{
    static constexpr bool value = utils::is_one_of_v<T,
                                                     int4,
                                                     int8,
                                                     int16,
                                                     int32,
                                                     cint16,
                                                     cint32,
                                                     bfloat16,
                                                     float
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
                                                     , cbfloat16
#endif
                                                     , cfloat
#endif
                                                         >;
};

template <>
struct type_bits<int4>
{
    static constexpr unsigned value = 4;
};

template <>
struct type_bits<uint4>
{
    static constexpr unsigned value = 4;
};

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2_VECTOR_NATIVE_TYPES__HPP__
