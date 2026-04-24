// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_EMULATED_MUL_INTRINSICS__HPP__
#define __AIE_API_DETAIL_AIE2P_EMULATED_MUL_INTRINSICS__HPP__

#include "../accum.hpp"
#include "../interleave.hpp"
#include "../vector.hpp"

#if __AIE_API_HAS_32B_MUL__ == 0 || __AIE_API_MUL_CONJUGATE_32BIT_INTRINSICS__ == 0

// TODO: These are temporary implementations for the emulation intrinsics that will come in the future

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> mul_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, aie::vector<T2, 32> b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );
    aie::accum<acc64, 32> acc = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::mac_elem_32_conf( lo, false, b, b_sign, acc, 0, 1, 0, 0 );

    return acc;
}

#if __AIE_ARCH__ == 22

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> mul_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, T2 b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::mac_elem_32_conf( lo, false, b, b_sign, acc, 0, 1, 0, 0 );

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline aie::accum<acc64, 32> mul_elem_32_t(aie::vector<T1, 32> a, bool a_sign, T2 b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    uint16 b_lo = 0xFFFFU & b;
    hi_type b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    aie::accum<acc64, 32> acc = ::mul_elem_32( a, a_sign, b_hi, b_sign );
    acc = ::mac_elem_32_conf( a, a_sign, b_lo, false, acc, 0, 1, 0, 0 );

    return acc;
}

#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> negmul_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, aie::vector<T2, 32> b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::mac_elem_32_conf( lo, false, b, b_sign, acc, 0, 1, 1, 1 );

    return acc;
}

#if __AIE_ARCH__ == 22

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> negmul_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, T2 b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::mac_elem_32_conf( lo, false, b, b_sign, acc, 0, 1, 1, 1 );

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline aie::accum<acc64, 32> negmul_elem_32_t(aie::vector<T1, 32> a, bool a_sign, T2 b, bool b_sign )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    uint16 b_lo = 0xFFFFU & b;
    hi_type b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    aie::accum<acc64, 32> acc = ::mul_elem_32( a, a_sign, b_hi, b_sign );
    acc = ::mac_elem_32_conf( a, a_sign, b_lo, false, acc, 0, 1, 1, 1 );

    return acc;
}
#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> mac_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, aie::vector<T2, 32> b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc_hi = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::addmac_elem_32_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 0, 0, 0 );

    return acc;
}

#if __AIE_ARCH__ == 22

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> mac_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, T2 b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc_hi = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign );
    acc = ::addmac_elem_32_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 0, 0, 0 );

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline aie::accum<acc64, 32> mac_elem_32_t(aie::vector<T1, 32> a, bool a_sign, T2 b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    uint16 b_lo = 0xFFFFU & b;
    hi_type b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    aie::accum<acc64, 32> acc_hi = ::mul_elem_32( a, a_sign, b_hi, b_sign );
    acc = ::addmac_elem_32_conf( a, a_sign, b_lo, false, acc_hi, acc, 0, 1, 0, 0, 0 );

    return acc;
}
#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> msc_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, aie::vector<T2, 32> b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc_hi = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign);
    acc = ::addmac_elem_32_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 1, 1, 0 );

    return acc;
}

#if __AIE_ARCH__ == 22

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int16, uint16>)
inline __aie_inline aie::accum<acc64, 32> msc_elem_32_t(aie::vector<T1, 16> a1, aie::vector<T1, 16> a2, bool a_sign, T2 b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [lo, hi] = interleave_unzip<uint16, 32>::run( a1.template cast_to<uint16>(),
                                                       a2.template cast_to<uint16>(), 1 );

    aie::accum<acc64, 32> acc_hi;
    acc_hi = ::mul_elem_32( hi.template cast_to<hi_type>(), a_sign, b, b_sign);
    acc = ::addmac_elem_32_conf( lo, false, b, b_sign, acc_hi, acc, 0, 1, 1, 1, 0 );

    return acc;
}

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int16, uint16> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline aie::accum<acc64, 32> msc_elem_32_t(aie::vector<T1, 32> a, bool a_sign, T2 b, bool b_sign, aie::accum<acc64, 32> acc )
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    uint16 b_lo = 0xFFFFU & b;
    hi_type b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    aie::accum<acc64, 32> acc_hi;
    acc_hi = ::mul_elem_32( a, a_sign, b_hi, b_sign);
    acc = ::addmac_elem_32_conf( a, a_sign, b_lo, false, acc_hi, acc, 0, 1, 1, 1, 0 );

    return acc;
}

#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 mul_elem_32(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, aie::vector<T2, 16> b0, aie::vector<T2, 16> b1, int sgn_y)
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    auto [b_lo, b_hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                           b1.template cast_to<uint16>(), 1 );

    v32acc64 acc;
    acc = mul_elem_32(a_hi.template cast_to<hi_type>(), sgn_x,
                      b_hi.template cast_to<hi_type>(), sgn_y);
    acc = mac_elem_32_conf(a_hi.template cast_to<hi_type>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_hi.template cast_to<hi_type>(), sgn_y,
                           acc, 0, 0, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_lo, false, acc, 0, 1, 0, 0);
    return acc;
}

#if __AIE_ARCH__ == 22
template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 mul_elem_32_t(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, T2 b, int sgn_y)
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    uint16 b_lo = 0xFFFFU & b;
    hi_type b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    v32acc64 acc;
    acc = mul_elem_32(a_hi.template cast_to<hi_type>(), sgn_x,
                      b_hi, sgn_y);
    acc = mac_elem_32_conf(a_hi.template cast_to<hi_type>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_hi, sgn_y,
                           acc, 0, 0, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_lo, false, acc, 0, 1, 0, 0);
    return acc;
}
#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 negmul_elem_32(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, aie::vector<T2, 16> b0, aie::vector<T2, 16> b1, int sgn_y)
{
    using aie::detail::interleave_unzip;
    using hi_type = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    auto [b_lo, b_hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                           b1.template cast_to<uint16>(), 1 );
    v32acc64 acc;
    acc = negmul_elem_32(a_hi.template cast_to<hi_type>(), sgn_x,
                         b_hi.template cast_to<hi_type>(), sgn_y);
    acc = msc_elem_32_conf(a_hi.template cast_to<hi_type>(),
                           sgn_x, b_lo, false, acc, 0, 1, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_hi.template cast_to<hi_type>(), sgn_y,
                           acc, 0, 0, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_lo, false, acc, 0, 1, 0, 0);
    return acc;
}

#if __AIE_ARCH__ == 22
template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 negmul_elem_32_t(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, T2 b, int sgn_y)
{
    using aie::detail::interleave_unzip;
    using a_hi_t = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;
    using b_hi_t = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    uint16 b_lo = 0xFFFFU & b;
    b_hi_t b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    v32acc64 acc;
    acc = negmul_elem_32(a_hi.template cast_to<a_hi_t>(), sgn_x,
                         b_hi, sgn_y);
    acc = msc_elem_32_conf(a_hi.template cast_to<a_hi_t>(),
                           sgn_x, b_lo, false, acc, 0, 1, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_hi, sgn_y,
                           acc, 0, 0, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_lo, false, acc, 0, 1, 0, 0);
    return acc;
}
#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 mac_elem_32(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, aie::vector<T2, 16> b0, aie::vector<T2, 16> b1, int sgn_y, v32acc64 acc1)
{
    using aie::detail::interleave_unzip;
    using a_hi_t = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;
    using b_hi_t = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    auto [b_lo, b_hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                           b1.template cast_to<uint16>(), 1 );
    v32acc64 acc;
    acc = mul_elem_32(a_hi.template cast_to<a_hi_t>(), sgn_x,
                      b_hi.template cast_to<b_hi_t>(), sgn_y);
    acc = mac_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_hi.template cast_to<a_hi_t>(), sgn_y,
                           acc, 0, 0, 0, 0);
    acc = addmac_elem_32_conf(a_lo, false, b_lo, false, acc, acc1, 0, 1, 0, 0, 0);
    return acc;
}

#if __AIE_ARCH__ == 22
template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 mac_elem_32_t(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, T2 b, int sgn_y, v32acc64 acc1)
{
    using aie::detail::interleave_unzip;
    using a_hi_t = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;
    using b_hi_t = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    uint16 b_lo = 0xFFFFU & b;
    b_hi_t b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    v32acc64 acc;
    acc = mul_elem_32(a_hi.template cast_to<a_hi_t>(), sgn_x,
                      b_hi, sgn_y);
    acc = mac_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = mac_elem_32_conf(a_lo, false, b_hi, sgn_y,
                           acc, 0, 0, 0, 0);
    acc = addmac_elem_32_conf(a_lo, false, b_lo, false, acc, acc1, 0, 1, 0, 0, 0);
    return acc;
}
#endif

template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 msc_elem_32(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, aie::vector<T2, 16> b0, aie::vector<T2, 16> b1, int sgn_y, v32acc64 acc1)
{
    using aie::detail::interleave_unzip;
    using a_hi_t = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;
    using b_hi_t = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    auto [b_lo, b_hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                           b1.template cast_to<uint16>(), 1 );
    v32acc64 acc;
    acc = negmul_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x,
                              b_hi.template cast_to<b_hi_t>(), sgn_y, 0);
    acc = msc_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_hi.template cast_to<b_hi_t>(), sgn_y,
                           acc, 0, 0, 0, 0);
    acc = addmsc_elem_32_conf(a_lo, false, b_lo, false, acc, acc1, 0, 1, 0, 0, 0);
    return acc;
}

#if __AIE_ARCH__ == 22
template <typename T1, typename T2> requires(aie::detail::utils::is_one_of_v<T1, int32, uint32> && aie::detail::utils::is_one_of_v<T2, int32, uint32>)
inline __aie_inline v32acc64 msc_elem_32_t(aie::vector<T1, 16> a0, aie::vector<T1, 16> a1, int sgn_x, T2 b, int sgn_y, v32acc64 acc1)
{
    using aie::detail::interleave_unzip;
    using a_hi_t = std::conditional_t<aie::detail::is_signed_v<T1>, int16, uint16>;
    using b_hi_t = std::conditional_t<aie::detail::is_signed_v<T2>, int16, uint16>;

    auto [a_lo, a_hi] = interleave_unzip<uint16, 32>::run( a0.template cast_to<uint16>(),
                                                           a1.template cast_to<uint16>(), 1 );

    uint16 b_lo = 0xFFFFU & b;
    b_hi_t b_hi = b >> 16; // TODO: we should probably take b_sign into account for sign extension here

    v32acc64 acc;
    acc = negmul_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x,
                              b_hi, sgn_y, 0);
    acc = msc_elem_32_conf(a_hi.template cast_to<a_hi_t>(), sgn_x, b_lo, false,
                           acc, 0, 1, 0, 0);
    acc = msc_elem_32_conf(a_lo, false, b_hi, sgn_y,
                           acc, 0, 0, 0, 0);
    acc = addmsc_elem_32_conf(a_lo, false, b_lo, false, acc, acc1, 0, 1, 0, 0, 0);
    return acc;
}
#endif

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <aie::detail::MulMacroOp MulOp>
inline __aie_inline aie::accum<cacc64, 16> mul_elem_16_t( aie::vector<cint32, 16> a, aie::vector<cint32, 8> b0, aie::vector<cint32, 8> b1  )
{
    constexpr auto op_term = [&] {
        if (aie::detail::has_conj1<MulOp>() && aie::detail::has_conj2<MulOp>()) return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y);
        if (aie::detail::has_conj1<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X,   OP_TERM_NEG_COMPLEX_CONJUGATE_X);
        if (aie::detail::has_conj2<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_Y,   OP_TERM_NEG_COMPLEX_CONJUGATE_Y);
        else                                                                    return std::pair(OP_TERM_NEG_COMPLEX,               OP_TERM_NEG_COMPLEX);
    }();

    aie::vector<cint16, 16> lo = (v16cint16) ::shuffle( b0, b1, T16_32x2_lo);
    aie::vector<cint16, 16> hi = (v16cint16) ::shuffle( b0, b1, T16_32x2_hi);
    aie::accum<cacc64, 16> acc;
    acc = ::mul_elem_16_conf( a, hi.template cast_to<cint16>(), op_term.first, 0);
    acc = ::mac_elem_16_conf( a, 1, lo.template cast_to<cint16>(), 0, acc, 0, 1, op_term.second, 0, 0); //OP_TERM_NEG_COMPLEX is mask for complex multiplication
    return acc;
}

template <aie::detail::MulMacroOp MulOp>
inline __aie_inline aie::accum<cacc64, 16> negmul_elem_16_t( aie::vector<cint32, 16> a, aie::vector<cint32, 8> b0, aie::vector<cint32, 8> b1 )
{
    constexpr auto op_term = [&] {
        if (aie::detail::has_conj1<MulOp>() && aie::detail::has_conj2<MulOp>()) return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y);
        if (aie::detail::has_conj1<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X,   OP_TERM_NEG_COMPLEX_CONJUGATE_X);
        if (aie::detail::has_conj2<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_Y,   OP_TERM_NEG_COMPLEX_CONJUGATE_Y);
        else                                                                    return std::pair(OP_TERM_NEG_COMPLEX,               OP_TERM_NEG_COMPLEX);
    }();

    using aie::detail::interleave_unzip;
    auto [lo, hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                       b1.template cast_to<uint16>(), 1 );

    aie::accum<cacc64, 16> acc;
    acc = ::negmul_elem_16_conf( a, hi.template cast_to<cint16>(), op_term.first, 0 );
    acc = ::msc_elem_16_conf( a, 1, lo.template cast_to<cint16>(), 0, acc, 0, 1, op_term.second, 0, 0); //OP_TERM_NEG_COMPLEX is mask for complex multiplication
    return acc;
}

template <aie::detail::MulMacroOp MulOp>
inline __aie_inline aie::accum<cacc64, 16> mac_elem_16_t( aie::vector<cint32, 16> a, aie::vector<cint32, 8> b0, aie::vector<cint32, 8> b1,
                                                          aie::accum<cacc64, 16> acc )
{
    constexpr auto op_term = [&] {
        if (aie::detail::has_conj1<MulOp>() && aie::detail::has_conj2<MulOp>()) return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y);
        if (aie::detail::has_conj1<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X,   OP_TERM_NEG_COMPLEX_CONJUGATE_X);
        if (aie::detail::has_conj2<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_Y,   OP_TERM_NEG_COMPLEX_CONJUGATE_Y);
        else                                                                    return std::pair(OP_TERM_NEG_COMPLEX,               OP_TERM_NEG_COMPLEX);
    }();

    using aie::detail::interleave_unzip;
    auto [lo, hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                       b1.template cast_to<uint16>(), 1 );
    aie::accum<cacc64, 16> acc_hi;
    acc_hi = ::mul_elem_16_conf( a, hi.template cast_to<cint16>(), op_term.first, 0 );
    acc    = ::addmac_elem_16_conf( a, 1, lo.template cast_to<cint16>(), 0, acc_hi, acc, 0, 1, op_term.second, 0, 0, 0);  //OP_TERM_NEG_COMPLEX is mask for complex multiplication
    return acc;
}

template <aie::detail::MulMacroOp MulOp>
inline __aie_inline aie::accum<cacc64, 16> msc_elem_16_t( aie::vector<cint32, 16> a, aie::vector<cint32, 8> b0, aie::vector<cint32, 8> b1,
                                                          aie::accum<cacc64, 16> acc )
{
    constexpr auto op_term = [&] {
        if (aie::detail::has_conj1<MulOp>() && aie::detail::has_conj2<MulOp>()) return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X_Y, OP_TERM_NEG_COMPLEX);
        if (aie::detail::has_conj1<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_X,   OP_TERM_NEG_COMPLEX);
        if (aie::detail::has_conj2<MulOp>())                                    return std::pair(OP_TERM_NEG_COMPLEX_CONJUGATE_Y,   OP_TERM_NEG_COMPLEX);
        else                                                                    return std::pair(OP_TERM_NEG_COMPLEX,               OP_TERM_NEG_COMPLEX);
    }();

    using aie::detail::interleave_unzip;
    auto [lo, hi] = interleave_unzip<uint16, 32>::run( b0.template cast_to<uint16>(),
                                                       b1.template cast_to<uint16>(), 1 );
    aie::accum<cacc64, 16> acc_hi;
    acc_hi = ::negmul_elem_16_conf( a, hi.template cast_to<cint16>(), op_term.first, 0 );
    acc    = ::addmsc_elem_16_conf( a, 1, lo.template cast_to<cint16>(), 0, acc_hi, acc, 0, 1, op_term.second, 0, 0, 0);  //OP_TERM_NEG_COMPLEX is mask for complex multiplication
    return acc;
}
#endif

#endif

#endif
