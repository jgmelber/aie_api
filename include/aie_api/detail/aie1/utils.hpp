// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_UTILS__HPP__
#define __AIE_API_DETAIL_AIE1_UTILS__HPP__

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
    else if constexpr(vector_type::bits() == 128)
    {
        if      constexpr(Reg == 0 ) { PIN(vrl0, v); }
        else if constexpr(Reg == 1 ) { PIN(vrh0, v); }
        else if constexpr(Reg == 2 ) { PIN(vrl1, v); }
        else if constexpr(Reg == 3 ) { PIN(vrh1, v); }
        else if constexpr(Reg == 4 ) { PIN(vrl2, v); }
        else if constexpr(Reg == 5 ) { PIN(vrh2, v); }
        else if constexpr(Reg == 6 ) { PIN(vrl3, v); }
        else if constexpr(Reg == 7 ) { PIN(vrh3, v); }
        else if constexpr(Reg == 8 ) { PIN(vcl0, v); }
        else if constexpr(Reg == 9 ) { PIN(vch0, v); }
        else if constexpr(Reg == 10) { PIN(vcl1, v); }
        else if constexpr(Reg == 11) { PIN(vch1, v); }
        else if constexpr(Reg == 12) { PIN(vdl0, v); }
        else if constexpr(Reg == 13) { PIN(vdh0, v); }
        else if constexpr(Reg == 14) { PIN(vdl1, v); }
        else if constexpr(Reg == 15) { PIN(vdh1, v); }
        else                         { auto __aie_register_keep() tmp = v; v = __aie_copy(tmp); }
    }
    else if constexpr(vector_type::bits() == 256)
    {
        if      constexpr(Reg == 0) { PIN(wr0, v); }
        else if constexpr(Reg == 1) { PIN(wr1, v); }
        else if constexpr(Reg == 2) { PIN(wr2, v); }
        else if constexpr(Reg == 3) { PIN(wr3, v); }
        else if constexpr(Reg == 4) { PIN(wc0, v); }
        else if constexpr(Reg == 5) { PIN(wc1, v); }
        else if constexpr(Reg == 6) { PIN(wd0, v); }
        else if constexpr(Reg == 7) { PIN(wd1, v); }
        else                        { auto __aie_register_keep() tmp = v; v = __aie_copy(tmp); }
    }
    else if constexpr(vector_type::bits() == 512)
    {
        if      constexpr(Reg == 0) { PIN(xa, v); }
        else if constexpr(Reg == 1) { PIN(xb, v); }
        else if constexpr(Reg == 2) { PIN(xc, v); }
        else if constexpr(Reg == 3) { PIN(xd, v); }
        else                        { auto __aie_register_keep() tmp = v; v = __aie_copy(tmp); }
    }
    else if constexpr(vector_type::bits() == 1024)
    {
        if      constexpr(Reg == 0) { PIN(ya, v); }
        else if constexpr(Reg == 1) { PIN(yd, v); }
        else                        { auto __aie_register_keep() tmp = v; v = __aie_copy(tmp); }
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
    else if constexpr(accum_type::bits() == 384 && accum_type::accum_bits() == 48)
    {
        if      constexpr(Reg == 0) { PIN(aml0, v); }
        else if constexpr(Reg == 1) { PIN(amh0, v); }
        else if constexpr(Reg == 2) { PIN(aml1, v); }
        else if constexpr(Reg == 3) { PIN(amh1, v); }
        else if constexpr(Reg == 4) { PIN(aml2, v); }
        else if constexpr(Reg == 5) { PIN(amh2, v); }
        else if constexpr(Reg == 6) { PIN(aml3, v); }
        else if constexpr(Reg == 7) { PIN(amh3, v); }
        else                        { PIN(  AM, v); }
    }
    // Double accumulator (768 bits).
    // Note that accum<acc80, 8>::bits() does not return 768 but 640 
    else if constexpr(accum_type::bits() == 768 ||
                      (accum_type::accum_bits() == 80 && accum_type::size() ==  8))
    {
        if      constexpr(Reg == 0) { PIN(bm0, v); }
        else if constexpr(Reg == 1) { PIN(bm1, v); }
        else if constexpr(Reg == 2) { PIN(bm2, v); }
        else if constexpr(Reg == 3) { PIN(bm3, v); }
        else                        { PIN( BM, v); }
    }
    else
    {
        std::integral_constant<unsigned, accum_type::bits()>::hello();
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
        static_assert(RegFile == AIE_RegFile::M,
                      "locate_in_register not yet implemented for this type and register file");

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
}

#undef PIN

} // namespace detail::utils
} // namespace aie

#endif // __AIE_API_DETAIL_AIE2_UTILS__HPP__

