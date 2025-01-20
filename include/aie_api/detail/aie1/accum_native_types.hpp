// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ACCUM_NATIVE_TYPES__HPP__
#define __AIE_API_DETAIL_AIE1_ACCUM_NATIVE_TYPES__HPP__

namespace aie::detail {

template <typename AccumTag>
struct accum_native_type;

template <> struct accum_native_type<acc16>    { using type = acc48; };
template <> struct accum_native_type<acc24>    { using type = acc48; };
template <> struct accum_native_type<acc32>    { using type = acc48; };
template <> struct accum_native_type<acc40>    { using type = acc48; };
template <> struct accum_native_type<acc48>    { using type = acc48; };

template <> struct accum_native_type<acc56>    { using type = acc80; };
template <> struct accum_native_type<acc64>    { using type = acc80; };
template <> struct accum_native_type<acc72>    { using type = acc80; };
template <> struct accum_native_type<acc80>    { using type = acc80; };

template <> struct accum_native_type<cacc16>   { using type = cacc48; };
template <> struct accum_native_type<cacc24>   { using type = cacc48; };
template <> struct accum_native_type<cacc32>   { using type = cacc48; };
template <> struct accum_native_type<cacc40>   { using type = cacc48; };
template <> struct accum_native_type<cacc48>   { using type = cacc48; };

template <> struct accum_native_type<cacc56>   { using type = cacc80; };
template <> struct accum_native_type<cacc64>   { using type = cacc80; };
template <> struct accum_native_type<cacc72>   { using type = cacc80; };
template <> struct accum_native_type<cacc80>   { using type = cacc80; };

template <> struct accum_native_type<accfloat>  { using type = accfloat; };
template <> struct accum_native_type<caccfloat> { using type = caccfloat; };

template <typename AccumTag>
struct accum_class_for_tag;

template <> struct accum_class_for_tag<exact_acc48>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<exact_acc80>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<exact_cacc48> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<exact_cacc80> { static constexpr AccumClass value() { return AccumClass::CInt; } };

template <typename AccumTag>
struct accum_bits_for_tag;

template <> struct accum_bits_for_tag<exact_acc48>   { static constexpr unsigned value() { return 48; } };
template <> struct accum_bits_for_tag<exact_acc80>   { static constexpr unsigned value() { return 80; } };
template <> struct accum_bits_for_tag<exact_cacc48>  { static constexpr unsigned value() { return 48; } };
template <> struct accum_bits_for_tag<exact_cacc80>  { static constexpr unsigned value() { return 80; } };

// TODO: remove accum_class_for_type as it is mainly required in the context of multiplications,
// which is already handled by accum_class_for_mul_types
template <typename AccumTag>
struct accum_class_for_type;

template <> struct accum_class_for_type<int8>   { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_type<uint8>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_type<int16>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_type<int32>  { static constexpr AccumClass value() { return AccumClass::Int;  } };

template <> struct accum_class_for_type<cint16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_type<cint32> { static constexpr AccumClass value() { return AccumClass::CInt; } };

template <> struct accum_class_for_type<float>  { static constexpr AccumClass value() { return AccumClass::FP;   } };
template <> struct accum_class_for_type<cfloat> { static constexpr AccumClass value() { return AccumClass::CFP;  } };

template <typename T, typename U>
struct accum_class_for_mul_types;

template <typename T> struct accum_class_for_mul_types<T,      int8>   { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <typename T> struct accum_class_for_mul_types<T,      uint8>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<int8,   int16>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<uint8,  int16>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<int16,  int16>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<int16,  int32>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<int32,  int16>  { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <>           struct accum_class_for_mul_types<int32,  int32>  { static constexpr AccumClass value() { return AccumClass::Int;  } };

template <>           struct accum_class_for_mul_types<cint16, cint16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types<cint16,  int16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types< int16, cint16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types<cint16,  int32> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types< int32, cint16> { static constexpr AccumClass value() { return AccumClass::CInt; } };

template <>           struct accum_class_for_mul_types<cint32, cint32> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types<cint32,  int16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types< int16, cint32> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types<cint32,  int32> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types< int32, cint32> { static constexpr AccumClass value() { return AccumClass::CInt; } };

template <>           struct accum_class_for_mul_types<cint32, cint16> { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <>           struct accum_class_for_mul_types<cint16, cint32> { static constexpr AccumClass value() { return AccumClass::CInt; } };

template <>           struct accum_class_for_mul_types<float,  float>  { static constexpr AccumClass value() { return AccumClass::FP;   } };

template <>           struct accum_class_for_mul_types<cfloat, cfloat> { static constexpr AccumClass value() { return AccumClass::CFP;  } };
template <>           struct accum_class_for_mul_types< float, cfloat> { static constexpr AccumClass value() { return AccumClass::CFP;  } };
template <>           struct accum_class_for_mul_types<cfloat,  float> { static constexpr AccumClass value() { return AccumClass::CFP;  } };

template <AccumClass Class, unsigned Bits, unsigned Elems>
struct accum_storage;

template <AccumClass Class, unsigned Bits, unsigned Elems>
using accum_storage_t = typename accum_storage<Class, Bits, Elems>::type;

template <> struct accum_storage<AccumClass::Int,  48,   8> { using type =                 v8acc48; static type undef() { return undef_v8acc48();                                                            } };
template <> struct accum_storage<AccumClass::Int,  48,  16> { using type =                v16acc48; static type undef() { return undef_v16acc48();                                                           } };
template <> struct accum_storage<AccumClass::Int,  48,  32> { using type = std::array<v16acc48, 2>; static type undef() { return { undef_v16acc48(), undef_v16acc48()                                     }; } };
template <> struct accum_storage<AccumClass::Int,  48,  64> { using type = std::array<v16acc48, 4>; static type undef() { return { undef_v16acc48(), undef_v16acc48(), undef_v16acc48(), undef_v16acc48() }; } };
template <> struct accum_storage<AccumClass::Int,  48, 128> { using type = std::array<v16acc48, 8>; static type undef() { return { undef_v16acc48(), undef_v16acc48(), undef_v16acc48(), undef_v16acc48(),
                                                                                                                                   undef_v16acc48(), undef_v16acc48(), undef_v16acc48(), undef_v16acc48() }; } };

template <> struct accum_storage<AccumClass::Int,  80,   4> { using type =                 v4acc80; static type undef() { return undef_v4acc80();                                                            } };
template <> struct accum_storage<AccumClass::Int,  80,   8> { using type =                 v8acc80; static type undef() { return undef_v8acc80();                                                            } };
template <> struct accum_storage<AccumClass::Int,  80,  16> { using type =  std::array<v8acc80, 2>; static type undef() { return { undef_v8acc80(),  undef_v8acc80()                                      }; } };
template <> struct accum_storage<AccumClass::Int,  80,  32> { using type =  std::array<v8acc80, 4>; static type undef() { return { undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80()  }; } };
template <> struct accum_storage<AccumClass::Int,  80,  64> { using type =  std::array<v8acc80, 8>; static type undef() { return { undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80(),
                                                                                                                                   undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80(),  undef_v8acc80()  }; } };

template <> struct accum_storage<AccumClass::CInt, 48,   4> { using type =                v4cacc48; static type undef() { return undef_v4cacc48();                                                           } };
template <> struct accum_storage<AccumClass::CInt, 48,   8> { using type =                v8cacc48; static type undef() { return undef_v8cacc48();                                                           } };
template <> struct accum_storage<AccumClass::CInt, 48,  16> { using type = std::array<v8cacc48, 2>; static type undef() { return { undef_v8cacc48(), undef_v8cacc48()                                     }; } };
template <> struct accum_storage<AccumClass::CInt, 48,  32> { using type = std::array<v8cacc48, 4>; static type undef() { return { undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48() }; } };
template <> struct accum_storage<AccumClass::CInt, 48,  64> { using type = std::array<v8cacc48, 8>; static type undef() { return { undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48(),
                                                                                                                                   undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48(), undef_v8cacc48() }; } };

template <> struct accum_storage<AccumClass::CInt, 80,   2> { using type =                v2cacc80; static type undef() { return undef_v2cacc80();                                                           } };
template <> struct accum_storage<AccumClass::CInt, 80,   4> { using type =                v4cacc80; static type undef() { return undef_v4cacc80();                                                           } };
template <> struct accum_storage<AccumClass::CInt, 80,   8> { using type = std::array<v4cacc80, 2>; static type undef() { return { undef_v4cacc80(), undef_v4cacc80()                                     }; } };
template <> struct accum_storage<AccumClass::CInt, 80,  16> { using type = std::array<v4cacc80, 4>; static type undef() { return { undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80() }; } };
template <> struct accum_storage<AccumClass::CInt, 80,  32> { using type = std::array<v4cacc80, 8>; static type undef() { return { undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80(),
                                                                                                                                   undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80(), undef_v4cacc80() }; } };

template <> struct accum_storage<AccumClass::FP,   32,   4> { using type =                 v4accfloat; static type undef() { return undef_v4float();                                                         } };
template <> struct accum_storage<AccumClass::FP,   32,   8> { using type =                 v8accfloat; static type undef() { return undef_v8float();                                                         } };
template <> struct accum_storage<AccumClass::FP,   32,  16> { using type =                v16accfloat; static type undef() { return undef_v16float();                                                        } };
template <> struct accum_storage<AccumClass::FP,   32,  32> { using type = std::array<v16accfloat, 2>; static type undef() { return { undef_v16float(), undef_v16float()                                  }; } };

template <> struct accum_storage<AccumClass::CFP,  32,   2> { using type =                v2caccfloat; static type undef() { return undef_v2cfloat();                                                        } };
template <> struct accum_storage<AccumClass::CFP,  32,   4> { using type =                v4caccfloat; static type undef() { return undef_v4cfloat();                                                        } };
template <> struct accum_storage<AccumClass::CFP,  32,   8> { using type =                v8caccfloat; static type undef() { return undef_v8cfloat();                                                        } };
template <> struct accum_storage<AccumClass::CFP,  32,  16> { using type = std::array<v8caccfloat, 2>; static type undef() { return { undef_v8cfloat(), undef_v8cfloat()                                  }; } };


template <typename T> struct native_accum_traits;

template <> struct native_accum_traits<    v8acc48> { using value_type =     acc48; static constexpr unsigned size =  8; };
template <> struct native_accum_traits<   v16acc48> { using value_type =     acc48; static constexpr unsigned size = 16; };

template <> struct native_accum_traits<   v4cacc48> { using value_type =    cacc48; static constexpr unsigned size =  4; };
template <> struct native_accum_traits<   v8cacc48> { using value_type =    cacc48; static constexpr unsigned size =  8; };

template <> struct native_accum_traits<    v4acc80> { using value_type =     acc80; static constexpr unsigned size =  4; };
template <> struct native_accum_traits<    v8acc80> { using value_type =     acc80; static constexpr unsigned size =  8; };

template <> struct native_accum_traits<   v2cacc80> { using value_type =    cacc80; static constexpr unsigned size =  2; };
template <> struct native_accum_traits<   v4cacc80> { using value_type =    cacc80; static constexpr unsigned size =  4; };

template <> struct native_accum_traits< v4accfloat> { using value_type =  accfloat; static constexpr unsigned size =  4; };
template <> struct native_accum_traits< v8accfloat> { using value_type =  accfloat; static constexpr unsigned size =  8; };
template <> struct native_accum_traits<v16accfloat> { using value_type =  accfloat; static constexpr unsigned size = 16; };

template <> struct native_accum_traits<v2caccfloat> { using value_type = caccfloat; static constexpr unsigned size =  2; };
template <> struct native_accum_traits<v4caccfloat> { using value_type = caccfloat; static constexpr unsigned size =  4; };
template <> struct native_accum_traits<v8caccfloat> { using value_type = caccfloat; static constexpr unsigned size =  8; };

template <AccumClass Class, unsigned Bits>
struct native_accum_length;

template <> struct native_accum_length<AccumClass::Int,  48> { static constexpr unsigned value = 16; };
template <> struct native_accum_length<AccumClass::Int,  80> { static constexpr unsigned value =  8; };
template <> struct native_accum_length<AccumClass::CInt, 48> { static constexpr unsigned value =  8; };
template <> struct native_accum_length<AccumClass::CInt, 80> { static constexpr unsigned value =  4; };
template <> struct native_accum_length<AccumClass::FP,   32> { static constexpr unsigned value = 16; };
template <> struct native_accum_length<AccumClass::CFP,  32> { static constexpr unsigned value =  8; };

template <AccumClass Class, unsigned Bits>
static constexpr unsigned native_accum_length_v = native_accum_length<Class, Bits>::value;

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE1_ACCUM_NATIVE_TYPES__HPP__

