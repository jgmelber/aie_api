// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ACCUM__HPP__
#define __AIE_API_DETAIL_ACCUM__HPP__

#include <type_traits>

#include "../concepts.hpp"
#include "../types.hpp"

#include "utils.hpp"

namespace aie {

template <AccumElemBaseType Tag, unsigned Elems>
class accum;

namespace detail {

enum class AccumClass
{
    Int,
    CInt,
    FP,
    CFP
};

static constexpr bool is_floating_point_class(AccumClass c) {
    return c == AccumClass::FP || c == AccumClass::CFP;
}

static constexpr bool is_complex_class(AccumClass c) {
    return c == AccumClass::CInt || c == AccumClass::CFP;
}

template <typename T>
struct accum_native_type;

template <typename T>
using accum_native_type_t = typename accum_native_type<T>::type;

template <typename T>
struct accum_class_for_tag;

template <> struct accum_class_for_tag<acc16>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc24>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc32>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc40>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc48>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc56>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc64>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
#if __AIE_ARCH__ == 10
template <> struct accum_class_for_tag<acc72>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
template <> struct accum_class_for_tag<acc80>        { static constexpr AccumClass value() { return AccumClass::Int;  } };
#endif

template <> struct accum_class_for_tag<cacc16>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc24>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc32>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc40>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc48>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc56>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc64>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
#if __AIE_ARCH__ == 10
template <> struct accum_class_for_tag<cacc72>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
template <> struct accum_class_for_tag<cacc80>       { static constexpr AccumClass value() { return AccumClass::CInt; } };
#endif

template <> struct accum_class_for_tag<accfloat>     { static constexpr AccumClass value() { return AccumClass::FP;   } };
#if __AIE_ARCH__ == 10 || __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct accum_class_for_tag<caccfloat>    { static constexpr AccumClass value() { return AccumClass::CFP;  } };
#endif

template <typename T>
static constexpr AccumClass accum_class_for_tag_v = accum_class_for_tag<T>::value();

template <typename T>
struct accum_bits_for_tag;

template <> struct accum_bits_for_tag<acc16>         { static constexpr unsigned value() { return 16; } };
template <> struct accum_bits_for_tag<acc24>         { static constexpr unsigned value() { return 24; } };
template <> struct accum_bits_for_tag<acc32>         { static constexpr unsigned value() { return 32; } };
template <> struct accum_bits_for_tag<acc40>         { static constexpr unsigned value() { return 40; } };
template <> struct accum_bits_for_tag<acc48>         { static constexpr unsigned value() { return 48; } };
template <> struct accum_bits_for_tag<acc56>         { static constexpr unsigned value() { return 56; } };
template <> struct accum_bits_for_tag<acc64>         { static constexpr unsigned value() { return 64; } };
#if __AIE_ARCH__ == 10
template <> struct accum_bits_for_tag<acc72>         { static constexpr unsigned value() { return 72; } };
template <> struct accum_bits_for_tag<acc80>         { static constexpr unsigned value() { return 80; } };
#endif

template <> struct accum_bits_for_tag<cacc16>        { static constexpr unsigned value() { return 16; } };
template <> struct accum_bits_for_tag<cacc24>        { static constexpr unsigned value() { return 24; } };
template <> struct accum_bits_for_tag<cacc32>        { static constexpr unsigned value() { return 32; } };
template <> struct accum_bits_for_tag<cacc40>        { static constexpr unsigned value() { return 40; } };
template <> struct accum_bits_for_tag<cacc48>        { static constexpr unsigned value() { return 48; } };
template <> struct accum_bits_for_tag<cacc56>        { static constexpr unsigned value() { return 56; } };
template <> struct accum_bits_for_tag<cacc64>        { static constexpr unsigned value() { return 64; } };
#if __AIE_ARCH__ == 10
template <> struct accum_bits_for_tag<cacc72>        { static constexpr unsigned value() { return 72; } };
template <> struct accum_bits_for_tag<cacc80>        { static constexpr unsigned value() { return 80; } };
#endif

template <> struct accum_bits_for_tag< accfloat>     { static constexpr unsigned value() { return 32; } };
#if __AIE_ARCH__ == 10 || __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct accum_bits_for_tag<caccfloat>     { static constexpr unsigned value() { return 32; } };
#endif

template <typename T>
static constexpr unsigned accum_bits_for_tag_v = accum_bits_for_tag<T>::value();

template <typename T>
struct accum_class_for_type;

template <typename T>
static constexpr AccumClass accum_class_for_type_v = accum_class_for_type<T>::value();

template <typename T1, typename T2>
struct accum_class_for_mul_types;

template <typename T1, typename T2>
static constexpr AccumClass accum_class_for_mul_types_v = accum_class_for_mul_types<T1, T2>::value();

template <typename T>
struct is_valid_accum_type
{
#if __AIE_ARCH__ == 20
    static constexpr bool value = utils::is_one_of_v<T,  acc16,  acc24,  acc32,  acc40,  acc48,  acc56,  acc64, accfloat> ||
                                  utils::is_one_of_v<T, cacc16, cacc24, cacc32, cacc40, cacc48, cacc56, cacc64
#if __AIE_API_COMPLEX_FP32_EMULATION__
                                                     , caccfloat
#endif
                                                     >;
#elif __AIE_ARCH__ == 10
    static constexpr bool value = utils::is_one_of_v<T,  acc16,  acc24,  acc32,  acc40,  acc48,  acc56,  acc64,  acc72,  acc80> ||
                                  utils::is_one_of_v<T, cacc16, cacc24, cacc32, cacc40, cacc48, cacc56, cacc64, cacc72, cacc80> ||
                                  utils::is_one_of_v<T, accfloat, caccfloat>;
#endif
};

template <AccumClass Class, unsigned Bits>
struct accum_tag;

#if __AIE_ARCH__ == 10 || __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct accum_tag<AccumClass::CFP,  32> { using type = caccfloat; };
#endif
template <> struct accum_tag<AccumClass::FP,   32> { using type = accfloat;  };
template <> struct accum_tag<AccumClass::Int,  16> { using type = acc16;     };
template <> struct accum_tag<AccumClass::Int,  24> { using type = acc24;     };
template <> struct accum_tag<AccumClass::Int,  32> { using type = acc32;     };
template <> struct accum_tag<AccumClass::Int,  40> { using type = acc40;     };
template <> struct accum_tag<AccumClass::Int,  48> { using type = acc48;     };
template <> struct accum_tag<AccumClass::Int,  56> { using type = acc56;     };
template <> struct accum_tag<AccumClass::Int,  64> { using type = acc64;     };
#if __AIE_ARCH__ == 10
template <> struct accum_tag<AccumClass::Int,  72> { using type = acc72;     };
template <> struct accum_tag<AccumClass::Int,  80> { using type = acc80;     };
#endif
template <> struct accum_tag<AccumClass::CInt, 16> { using type = cacc16;    };
template <> struct accum_tag<AccumClass::CInt, 24> { using type = cacc24;    };
template <> struct accum_tag<AccumClass::CInt, 32> { using type = cacc32;    };
template <> struct accum_tag<AccumClass::CInt, 40> { using type = cacc40;    };
template <> struct accum_tag<AccumClass::CInt, 48> { using type = cacc48;    };
template <> struct accum_tag<AccumClass::CInt, 56> { using type = cacc56;    };
template <> struct accum_tag<AccumClass::CInt, 64> { using type = cacc64;    };
#if __AIE_ARCH__ == 10
template <> struct accum_tag<AccumClass::CInt, 72> { using type = cacc72;    };
template <> struct accum_tag<AccumClass::CInt, 80> { using type = cacc80;    };
#endif

template <AccumClass Class, unsigned Bits>
using accum_tag_t = typename accum_tag<Class, Bits>::type;

template <typename DstTag, typename Acc>
auto accum_cast(const Acc &acc)
{
    return acc.template cast_to<DstTag>();
}

template <typename Accum>
struct concat_accum_helper
{
    template <typename... Accums>
        requires(sizeof...(Accums) > 0 && (std::is_same_v<Accum, Accums> && ...))
    __aie_inline
    static auto run(const Accum &acc, const Accums & ...accums)
    {
        constexpr unsigned elements = Accum::size() + (Accums::size() + ...);
        using out_accum_type = accum<typename Accum::value_type, elements>;

        out_accum_type ret;
        ret.upd_all(acc, accums...);

        return ret;
    }
};

template <typename Accum, typename... Accums>
__aie_inline
auto concat_accum(const Accum & acc, const Accum & acc2, const Accums & ...accums)
{
    using accum_type = utils::remove_all_t<Accum>;
    return concat_accum_helper<accum_type>::run(acc, acc2, accums...);
}

template <typename T>
struct is_accum
{
    static constexpr bool value = false;
};

template <AccumElemBaseType Tag, unsigned Elems>
struct is_accum<accum<Tag, Elems>>
{
    static constexpr bool value = true;
};

template <typename A, typename B>
    // Either both or neither types are floating point
    requires(is_floating_point_v<A> == is_floating_point_v<B>)
static constexpr unsigned default_accum_bits()
{
    if constexpr (type_bits_v<A> < type_bits_v<B>)
        return default_accum_bits<B, A>();

    if constexpr (is_floating_point_v<A>)
        return 32;

// TODO: replace with AIE_API_MATH_VERSION when it is supported
#if __AIE_ARCH__ == 10
    else if constexpr (is_complex_v<A> && is_complex_v<B>) {
        if      constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, cint16>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, cint16>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, cint32>) return 64;
    }
    else if constexpr (is_complex_v<A>) {
        if      constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, int16>) return 32;
        else if constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, int32>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, int16>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, int32>) return 64;
    }
#elif __AIE_ARCH__ == 20
    else if constexpr (is_complex_v<A> && is_complex_v<B>) {
        if      constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, cint16>) return 48; // TODO: check discrepancy with AIE1
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, cint16>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, cint32>) return 64;
    }
    else if constexpr (is_complex_v<A>) {
        if      constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, int16>) return 48;
        else if constexpr (std::is_same_v<A, cint16> && std::is_same_v<B, int32>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, int16>) return 48;
        else if constexpr (std::is_same_v<A, cint32> && std::is_same_v<B, int32>) return 64;
    }
#endif
    else if constexpr (is_complex_v<B>) {
        return default_accum_bits<B, A>();
    }
    else {
        if      constexpr (type_bits_v<A> ==  8 && type_bits_v<B> ==  8) return 32;
        else if constexpr (type_bits_v<A> ==  8 && type_bits_v<B> == 16) return 32;
        else if constexpr (type_bits_v<A> == 16 && type_bits_v<B> ==  8) return 32;
        else if constexpr (type_bits_v<A> == 16 && type_bits_v<B> == 16) return 32;
        else if constexpr (type_bits_v<A> == 32 && type_bits_v<B> == 16) return 48;
        else if constexpr (type_bits_v<A> == 32 && type_bits_v<B> == 32) return 64;
#if AIE_API_PLATFORM_VERSION >= 200
        else if constexpr (type_bits_v<A> ==  4 && type_bits_v<B> ==  4) return 32;
        else if constexpr (type_bits_v<A> ==  8 && type_bits_v<B> ==  4) return 32;
#endif
    }
}

template <unsigned SumBits, bool is_float = false> struct default_accum_tag_helper;

// TODO: replace with AIE_API_MATH_VERSION when it is supported
#if __AIE_ARCH__ == 10
template <> struct default_accum_tag_helper<32, true>  { using type = accfloat; using ctype = caccfloat; };
#elif __AIE_ARCH__ == 20
#if __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct default_accum_tag_helper<32, true>  { using type = accfloat; using ctype = caccfloat; };
#else
template <> struct default_accum_tag_helper<32, true>  { using type = accfloat; using ctype = void; };
#endif
#endif

template <> struct default_accum_tag_helper<32, false> { using type = acc32;    using ctype = cacc32; };

template <> struct default_accum_tag_helper<48>        { using type = acc48;    using ctype = cacc48; };

template <> struct default_accum_tag_helper<64>        { using type = acc64;    using ctype = cacc64; };

template <typename A, typename B> struct default_accum_tag
{
    using type = std::conditional_t<is_complex_v<A> || is_complex_v<B>,
                                    typename default_accum_tag_helper<default_accum_bits<A, B>(), is_floating_point_v<A>>::ctype,
                                    typename default_accum_tag_helper<default_accum_bits<A, B>(), is_floating_point_v<A>>::type>;
};

template <typename A, typename B>
using default_accum_tag_t = typename default_accum_tag<A, B>::type;

template <typename T1, typename T2, unsigned Bits = default_accum_bits<T1, T2>()>
using accum_tag_for_mul_types = accum_tag_t<accum_class_for_mul_types_v<T1, T2>, Bits>;

template <typename T, unsigned Bits = default_accum_bits<T, T>()>
using accum_tag_for_type = accum_tag_t<accum_class_for_type<T>::value(), Bits>;

template <AccumClass Class, unsigned MinBits>
static constexpr unsigned to_native_accum_bits();

template <typename AccumTag>
static constexpr unsigned to_native_accum_bits();

template <typename A, typename B, unsigned Bits = default_accum_bits<A, B>()>
static constexpr unsigned to_native_accum_bits_for_mul_types()
{
    constexpr AccumClass accum_class = accum_class_for_mul_types_v<A, B>;
    constexpr unsigned accum_bits = Bits == 0? default_accum_bits<A, B>() : Bits;

    return to_native_accum_bits<accum_class, accum_bits>();
}

template <typename A, typename B, typename AccumTag>
static constexpr unsigned to_native_accum_bits_for_mul_types_tag()
{
    if constexpr (std::is_same_v<AccumTag, accauto>)
        return to_native_accum_bits_for_mul_types<A, B, 0>();
    else
        return to_native_accum_bits<AccumTag>();
}

template <typename ...Tags> struct deduce_accauto_helper;

template <ElemBaseType Tag>
struct deduce_accauto_helper<Tag>        { using type = detail::default_accum_tag_t<Tag, Tag>; };

template <ElemBaseType Tag1, ElemBaseType Tag2>
struct deduce_accauto_helper<Tag1, Tag2> { using type = detail::default_accum_tag_t<Tag1, Tag2>; };

template <typename AccumTag, typename ...Tags>
using accum_tag_or_default_t = std::conditional_t<std::is_same_v<AccumTag, accauto>,
                                                  typename deduce_accauto_helper<Tags...>::type,
                                                  AccumTag>;

} // namespace detail
} // namespace aie

#endif
