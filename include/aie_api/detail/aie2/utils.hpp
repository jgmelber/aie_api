// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_UTILS__HPP__
#define __AIE_API_DETAIL_AIE2_UTILS__HPP__

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>

#include "../../vector.hpp"

namespace aie {

template <ElemBaseType, unsigned Elems>
class vector;

template <AccumElemBaseType MinAccumTag, unsigned Elems>
class accum;

namespace detail::utils {

enum class AIE_RegFile {
    Default,
    R,
    P,
    M,
    DC,
    DJ,
    DN,
    Vector,
    Accum,
};

#define PIN(reg, val)                                              \
    do {                                                           \
        auto __aie_register(reg) tmp = val; val = __aie_copy(tmp); \
    } while(0)

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default, NativeVectorType T>
__aie_inline
void locate_in_register_impl(T& v)
{
    static_assert(RegFile == AIE_RegFile::Default || RegFile == AIE_RegFile::Vector,
                  "locate_in_register not yet implemented for this type and register file");

    using vector_type = ::aie::vector<typename native_vector_traits<T>::value_type, native_vector_traits<T>::size>;

    if constexpr(Reg == ~0u)
    {
        auto __aie_register_keep() tmp = v; v = __aie_copy(tmp);
    }
    else if constexpr(vector_type::bits() == 256)
    {
        if      constexpr(Reg == 0 ) { PIN(wl0,  v); }
        else if constexpr(Reg == 1 ) { PIN(wh0,  v); }
        else if constexpr(Reg == 2 ) { PIN(wl1,  v); }
        else if constexpr(Reg == 3 ) { PIN(wh1,  v); }
        else if constexpr(Reg == 4 ) { PIN(wl2,  v); }
        else if constexpr(Reg == 5 ) { PIN(wh2,  v); }
        else if constexpr(Reg == 6 ) { PIN(wl3,  v); }
        else if constexpr(Reg == 7 ) { PIN(wh3,  v); }
        else if constexpr(Reg == 8 ) { PIN(wl4,  v); }
        else if constexpr(Reg == 9 ) { PIN(wh4,  v); }
        else if constexpr(Reg == 10) { PIN(wl5,  v); }
        else if constexpr(Reg == 11) { PIN(wh5,  v); }
        else if constexpr(Reg == 12) { PIN(wl6,  v); }
        else if constexpr(Reg == 13) { PIN(wh6,  v); }
        else if constexpr(Reg == 14) { PIN(wl7,  v); }
        else if constexpr(Reg == 15) { PIN(wh7,  v); }
        else if constexpr(Reg == 16) { PIN(wl8,  v); }
        else if constexpr(Reg == 17) { PIN(wh8,  v); }
        else if constexpr(Reg == 18) { PIN(wl9,  v); }
        else if constexpr(Reg == 19) { PIN(wh9,  v); }
        else if constexpr(Reg == 20) { PIN(wl10, v); }
        else if constexpr(Reg == 21) { PIN(wh10, v); }
        else if constexpr(Reg == 22) { PIN(wl11, v); }
        else if constexpr(Reg == 23) { PIN(wh11, v); }
        else                         { PIN(   W, v); }
    }
    else if constexpr(vector_type::bits() == 512)
    {
        if      constexpr(Reg == 0 ) { PIN(x0,  v); }
        else if constexpr(Reg == 1 ) { PIN(x1,  v); }
        else if constexpr(Reg == 2 ) { PIN(x2,  v); }
        else if constexpr(Reg == 3 ) { PIN(x3,  v); }
        else if constexpr(Reg == 4 ) { PIN(x4,  v); }
        else if constexpr(Reg == 5 ) { PIN(x5,  v); }
        else if constexpr(Reg == 6 ) { PIN(x6,  v); }
        else if constexpr(Reg == 7 ) { PIN(x7,  v); }
        else if constexpr(Reg == 8 ) { PIN(x8,  v); }
        else if constexpr(Reg == 9 ) { PIN(x9,  v); }
        else if constexpr(Reg == 10) { PIN(x10, v); }
        else if constexpr(Reg == 11) { PIN(x11, v); }
        else                         { PIN(  X, v); }
    }
    else if constexpr(vector_type::bits() == 1024)
    {
        if      constexpr(Reg == 0) { PIN(y0, v); }
        else if constexpr(Reg == 1) { PIN(y1, v); }
        else if constexpr(Reg == 2) { PIN(y2, v); }
        else if constexpr(Reg == 3) { PIN(y3, v); }
        else if constexpr(Reg == 4) { PIN(y4, v); }
        else if constexpr(Reg == 5) { PIN(y5, v); }
        else                        { PIN( Y, v); }
    }
    else
    {
        chess_error("locate_in_register not yet implemented for this type");
    }
}

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default, NativeAccumType Acc>
__aie_inline
void locate_in_register_impl(Acc& v)
{
    static_assert(RegFile == AIE_RegFile::Default || RegFile == AIE_RegFile::Accum,
                  "locate_in_register not yet implemented for this type and register file");

    using accum_type  = ::aie::accum<typename detail::native_accum_traits<Acc>::value_type, detail::native_accum_traits<Acc>::size>;

    if constexpr(Reg == ~0u)
    {
        auto __aie_register_keep() tmp = v; v = __aie_copy(tmp);
    }
#if __AIE_ARCH__ == 20
    else if constexpr(accum_type::bits() == 256)
    {
        if      constexpr(Reg == 0 ) { PIN(amll0, v); }
        else if constexpr(Reg == 1 ) { PIN(amlh0, v); }
        else if constexpr(Reg == 2 ) { PIN(amhl0, v); }
        else if constexpr(Reg == 3 ) { PIN(amhh0, v); }
        else if constexpr(Reg == 4 ) { PIN(amll1, v); }
        else if constexpr(Reg == 5 ) { PIN(amlh1, v); }
        else if constexpr(Reg == 6 ) { PIN(amhl1, v); }
        else if constexpr(Reg == 7 ) { PIN(amhh1, v); }
        else if constexpr(Reg == 8 ) { PIN(amll2, v); }
        else if constexpr(Reg == 9 ) { PIN(amlh2, v); }
        else if constexpr(Reg == 10) { PIN(amhl2, v); }
        else if constexpr(Reg == 11) { PIN(amhh2, v); }
        else if constexpr(Reg == 12) { PIN(amll3, v); }
        else if constexpr(Reg == 13) { PIN(amlh3, v); }
        else if constexpr(Reg == 14) { PIN(amhl3, v); }
        else if constexpr(Reg == 15) { PIN(amhh3, v); }
        else if constexpr(Reg == 16) { PIN(amll4, v); }
        else if constexpr(Reg == 17) { PIN(amlh4, v); }
        else if constexpr(Reg == 18) { PIN(amhl4, v); }
        else if constexpr(Reg == 19) { PIN(amhh4, v); }
        else if constexpr(Reg == 20) { PIN(amll5, v); }
        else if constexpr(Reg == 21) { PIN(amlh5, v); }
        else if constexpr(Reg == 22) { PIN(amhl5, v); }
        else if constexpr(Reg == 23) { PIN(amhh5, v); }
        else if constexpr(Reg == 24) { PIN(amll6, v); }
        else if constexpr(Reg == 25) { PIN(amlh6, v); }
        else if constexpr(Reg == 26) { PIN(amhl6, v); }
        else if constexpr(Reg == 27) { PIN(amhh6, v); }
        else if constexpr(Reg == 28) { PIN(amll7, v); }
        else if constexpr(Reg == 29) { PIN(amlh7, v); }
        else if constexpr(Reg == 30) { PIN(amhl7, v); }
        else if constexpr(Reg == 31) { PIN(amhh7, v); }
        else if constexpr(Reg == 32) { PIN(amll8, v); }
        else if constexpr(Reg == 33) { PIN(amlh8, v); }
        else if constexpr(Reg == 34) { PIN(amhl8, v); }
        else if constexpr(Reg == 35) { PIN(amhh8, v); }
        else                         { PIN(   AM, v); }
    }
    else if constexpr(accum_type::bits() == 512)
    {
        if      constexpr(Reg == 0 ) { PIN(bml0, v); }
        else if constexpr(Reg == 1 ) { PIN(bmh0, v); }
        else if constexpr(Reg == 2 ) { PIN(bml1, v); }
        else if constexpr(Reg == 3 ) { PIN(bmh1, v); }
        else if constexpr(Reg == 4 ) { PIN(bml2, v); }
        else if constexpr(Reg == 5 ) { PIN(bmh2, v); }
        else if constexpr(Reg == 6 ) { PIN(bml3, v); }
        else if constexpr(Reg == 7 ) { PIN(bmh3, v); }
        else if constexpr(Reg == 8 ) { PIN(bml4, v); }
        else if constexpr(Reg == 9 ) { PIN(bmh4, v); }
        else if constexpr(Reg == 10) { PIN(bml5, v); }
        else if constexpr(Reg == 11) { PIN(bmh5, v); }
        else if constexpr(Reg == 12) { PIN(bml6, v); }
        else if constexpr(Reg == 13) { PIN(bmh6, v); }
        else if constexpr(Reg == 14) { PIN(bml7, v); }
        else if constexpr(Reg == 15) { PIN(bmh7, v); }
        else if constexpr(Reg == 16) { PIN(bml8, v); }
        else if constexpr(Reg == 17) { PIN(bmh8, v); }
        else                         { PIN(  BM, v); }
    }
    else if constexpr(accum_type::bits() == 1024)
    {
        if      constexpr(Reg == 0 ) { PIN(cm0,  v); }
        else if constexpr(Reg == 1 ) { PIN(cm1,  v); }
        else if constexpr(Reg == 2 ) { PIN(cm2,  v); }
        else if constexpr(Reg == 3 ) { PIN(cm3,  v); }
        else if constexpr(Reg == 4 ) { PIN(cm4,  v); }
        else if constexpr(Reg == 5 ) { PIN(cm5,  v); }
        else if constexpr(Reg == 6 ) { PIN(cm6,  v); }
        else if constexpr(Reg == 7 ) { PIN(cm7,  v); }
        else if constexpr(Reg == 8 ) { PIN(cm8,  v); }
        else                         { PIN( CM, v); }
    }
#elif __AIE_ARCH__ == 21
    else if constexpr(accum_type::bits() == 512)
    {
        if      constexpr(Reg == 0 ) { PIN(bmll0, v); }
        else if constexpr(Reg == 1 ) { PIN(bmlh0, v); }
        else if constexpr(Reg == 2 ) { PIN(bmhl0, v); }
        else if constexpr(Reg == 3 ) { PIN(bmhh0, v); }
        else if constexpr(Reg == 4 ) { PIN(bmll1, v); }
        else if constexpr(Reg == 5 ) { PIN(bmlh1, v); }
        else if constexpr(Reg == 6 ) { PIN(bmhl1, v); }
        else if constexpr(Reg == 7 ) { PIN(bmhh1, v); }
        else if constexpr(Reg == 8 ) { PIN(bmll2, v); }
        else if constexpr(Reg == 9 ) { PIN(bmlh2, v); }
        else if constexpr(Reg == 10) { PIN(bmhl2, v); }
        else if constexpr(Reg == 11) { PIN(bmhh2, v); }
        else if constexpr(Reg == 12) { PIN(bmll3, v); }
        else if constexpr(Reg == 13) { PIN(bmlh3, v); }
        else if constexpr(Reg == 14) { PIN(bmhl3, v); }
        else if constexpr(Reg == 15) { PIN(bmhh3, v); }
        else if constexpr(Reg == 16) { PIN(bmll4, v); }
        else if constexpr(Reg == 17) { PIN(bmlh4, v); }
        else if constexpr(Reg == 18) { PIN(bmhl4, v); }
        else if constexpr(Reg == 19) { PIN(bmhh4, v); }
        else                         { PIN(   BM, v); }
    }
    else if constexpr(accum_type::bits() == 1024)
    {
        if      constexpr(Reg == 0 ) { PIN(cml0, v); }
        else if constexpr(Reg == 1 ) { PIN(cmh0, v); }
        else if constexpr(Reg == 2 ) { PIN(cml1, v); }
        else if constexpr(Reg == 3 ) { PIN(cmh1, v); }
        else if constexpr(Reg == 4 ) { PIN(cml2, v); }
        else if constexpr(Reg == 5 ) { PIN(cmh2, v); }
        else if constexpr(Reg == 6 ) { PIN(cml3, v); }
        else if constexpr(Reg == 7 ) { PIN(cmh3, v); }
        else if constexpr(Reg == 8 ) { PIN(cml4, v); }
        else if constexpr(Reg == 9 ) { PIN(cmh4, v); }
        else                         { PIN(  CM, v); }
    }
    else if constexpr(accum_type::bits() == 2048)
    {
        if      constexpr(Reg == 0) { PIN(dm0, v); }
        else if constexpr(Reg == 1) { PIN(dm1, v); }
        else if constexpr(Reg == 2) { PIN(dm2, v); }
        else if constexpr(Reg == 3) { PIN(dm3, v); }
        else if constexpr(Reg == 4) { PIN(dm4, v); }
        else                        { PIN( DM, v); }
    }
#endif
    else
    {
        chess_error("locate_in_register not yet implemented for this type");
    }
}

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
    requires(Vector<T> || Accum<T>)
__aie_inline
void locate_in_register_impl(T& val)
{
    using type      = T;
    using storage_t = typename type::storage_t;

    constexpr unsigned num_chunks = utils::num_elems_v<storage_t>;
    unroll_times<num_chunks>([&]<unsigned I>(std::integral_constant<unsigned, I>) __aie_inline {
        constexpr unsigned R = Reg == ~0u? Reg : Reg * num_chunks + I;
        auto native = val.template extract<type::size() / num_chunks>(I).to_native();
        locate_in_register_impl<R, RegFile>(native);
        val.insert(I, native);
    });
}

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
    requires (is_one_of_v<T, int, unsigned, bool>)
__aie_inline
void locate_in_register_impl(T& val)
{
    if constexpr(Reg == ~0u)
    {
        auto __aie_register_keep() tmp = val; val = __aie_copy(tmp);
    }
    else
    {
        static_assert(RegFile == AIE_RegFile::Default || RegFile == AIE_RegFile::R,
                      "locate_in_register not yet implemented for this type and register file");

        if      constexpr(Reg == 0 ) { PIN(r0,  val); }
        else if constexpr(Reg == 1 ) { PIN(r1,  val); }
        else if constexpr(Reg == 2 ) { PIN(r2,  val); }
        else if constexpr(Reg == 3 ) { PIN(r3,  val); }
        else if constexpr(Reg == 4 ) { PIN(r4,  val); }
        else if constexpr(Reg == 5 ) { PIN(r5,  val); }
        else if constexpr(Reg == 6 ) { PIN(r6,  val); }
        else if constexpr(Reg == 7 ) { PIN(r7,  val); }
        else if constexpr(Reg == 8 ) { PIN(r8,  val); }
        else if constexpr(Reg == 9 ) { PIN(r9,  val); }
        else if constexpr(Reg == 10) { PIN(r10, val); }
        else if constexpr(Reg == 11) { PIN(r11, val); }
        else if constexpr(Reg == 12) { PIN(r12, val); }
        else if constexpr(Reg == 13) { PIN(r13, val); }
        else if constexpr(Reg == 14) { PIN(r14, val); }
        else if constexpr(Reg == 15) { PIN(r15, val); }
        else if constexpr(Reg == 16) { PIN(r16, val); }
        else if constexpr(Reg == 17) { PIN(r17, val); }
        else if constexpr(Reg == 18) { PIN(r18, val); }
        else if constexpr(Reg == 19) { PIN(r19, val); }
        else if constexpr(Reg == 20) { PIN(r20, val); }
        else if constexpr(Reg == 21) { PIN(r21, val); }
        else if constexpr(Reg == 22) { PIN(r22, val); }
        else if constexpr(Reg == 23) { PIN(r23, val); }
        else if constexpr(Reg == 24) { PIN(r24, val); }
        else if constexpr(Reg == 25) { PIN(r25, val); }
        else if constexpr(Reg == 26) { PIN(r26, val); }
        else if constexpr(Reg == 27) { PIN(r27, val); }
        else if constexpr(Reg == 28) { PIN(r28, val); }
        else if constexpr(Reg == 29) { PIN(r29, val); }
        else if constexpr(Reg == 30) { PIN(r30, val); }
        else if constexpr(Reg == 31) { PIN(r31, val); }
        else                         { PIN(  R, val); }
    }
}

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default, typename T>
__aie_inline
void locate_in_register_impl(T*& val)
{
    if constexpr(Reg == ~0u)
    {
        auto __aie_register_keep() tmp = val; val = __aie_copy(tmp);
    }
    else
    {
        static_assert(RegFile == AIE_RegFile::Default || RegFile == AIE_RegFile::P,
                      "locate_in_register not yet implemented for this type and register file");

        if      constexpr(Reg == 0) { PIN(p0, val); }
        else if constexpr(Reg == 1) { PIN(p1, val); }
        else if constexpr(Reg == 2) { PIN(p2, val); }
        else if constexpr(Reg == 3) { PIN(p3, val); }
        else if constexpr(Reg == 4) { PIN(p4, val); }
        else if constexpr(Reg == 5) { PIN(p5, val); }
        else if constexpr(Reg == 6) { PIN(p6, val); }
        else if constexpr(Reg == 7) { PIN(p7, val); }
        else                        { PIN( P, val); }
    }
}

template<unsigned Reg = ~0u, AIE_RegFile RegFile = AIE_RegFile::Default>
__aie_inline
void locate_in_register_impl(addr_t& val)
{
    if constexpr(Reg == ~0u)
    {
        auto __aie_register_keep() tmp = val; val = __aie_copy(tmp);
    }
    else
    {
        static_assert(RegFile == AIE_RegFile::M  ||
                      RegFile == AIE_RegFile::DC ||
                      RegFile == AIE_RegFile::DJ ||
                      RegFile == AIE_RegFile::DN,
                      "locate_in_register not yet implemented for this type and register file");

        if constexpr (RegFile == AIE_RegFile::M)
        {
            if      constexpr(Reg == 0) { PIN(m0, val); }
            else if constexpr(Reg == 1) { PIN(m1, val); }
            else if constexpr(Reg == 2) { PIN(m2, val); }
            else if constexpr(Reg == 3) { PIN(m3, val); }
            else if constexpr(Reg == 4) { PIN(m4, val); }
            else if constexpr(Reg == 5) { PIN(m5, val); }
            else if constexpr(Reg == 6) { PIN(m6, val); }
            else if constexpr(Reg == 7) { PIN(m7, val); }
            else                        { PIN( M, val); }
        }
        else if constexpr (RegFile == AIE_RegFile::DC)
        {
            if      constexpr(Reg == 0) { PIN(dc0, val); }
            else if constexpr(Reg == 1) { PIN(dc1, val); }
            else if constexpr(Reg == 2) { PIN(dc2, val); }
            else if constexpr(Reg == 3) { PIN(dc3, val); }
            else if constexpr(Reg == 4) { PIN(dc4, val); }
            else if constexpr(Reg == 5) { PIN(dc5, val); }
            else if constexpr(Reg == 6) { PIN(dc6, val); }
            else if constexpr(Reg == 7) { PIN(dc7, val); }
            else                        { PIN( DC, val); }
        }
        else if constexpr (RegFile == AIE_RegFile::DJ)
        {
            if      constexpr(Reg == 0) { PIN(dj0, val); }
            else if constexpr(Reg == 1) { PIN(dj1, val); }
            else if constexpr(Reg == 2) { PIN(dj2, val); }
            else if constexpr(Reg == 3) { PIN(dj3, val); }
            else if constexpr(Reg == 4) { PIN(dj4, val); }
            else if constexpr(Reg == 5) { PIN(dj5, val); }
            else if constexpr(Reg == 6) { PIN(dj6, val); }
            else if constexpr(Reg == 7) { PIN(dj7, val); }
            else                        { PIN( DJ, val); }
        }
        else if constexpr (RegFile == AIE_RegFile::DN)
        {
            if      constexpr(Reg == 0) { PIN(dn0, val); }
            else if constexpr(Reg == 1) { PIN(dn1, val); }
            else if constexpr(Reg == 2) { PIN(dn2, val); }
            else if constexpr(Reg == 3) { PIN(dn3, val); }
            else if constexpr(Reg == 4) { PIN(dn4, val); }
            else if constexpr(Reg == 5) { PIN(dn5, val); }
            else if constexpr(Reg == 6) { PIN(dn6, val); }
            else if constexpr(Reg == 7) { PIN(dn7, val); }
            else                        { PIN( DN, val); }
        }
    }
}

#undef PIN

} // namespace detail::utils
} // namespace aie

#endif // __AIE_API_DETAIL_AIE2_UTILS__HPP__
