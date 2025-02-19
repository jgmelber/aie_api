// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Operator overloading for AIE API types, currently expects "using namespace aie::operators"
 * in the user code to make use of them directly.
 */

#pragma once

#ifndef __AIE_API_OPERATORS__HPP__
#define __AIE_API_OPERATORS__HPP__

namespace aie::operators {

/**
 * @ingroup group_operators
 *
 * Negation operator. It is equivalent to aie::neg.
 *
 */
template <ArithmeticType T>
constexpr auto operator-(const T &a)
{
    return aie::neg(a);
}

/**
 * @ingroup group_operators
 *
 * Subtraction operator. It is equivalent to aie::sub.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator-(const T1 &a, const T2 &b)
{
    return aie::sub(a, b);
}

/**
 * @ingroup group_operators
 *
 * Addition operator. It is equivalent to aie::add.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator+(const T1 &a, const T2 &b)
{
    return aie::add(a, b);
}

/**
 * @ingroup group_operators
 *
 * Less than comparison operator. It is equivalent to aie::lt.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator<(const T1 &a, const T2 &b)
{
    return aie::lt(a, b);
}

/**
 * @ingroup group_operators
 *
 * Less than or equal comparison operator. It is equivalent to aie::le.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator<=(const T1 &a, const T2 &b)
{
    return aie::le(a, b);
}

/**
 * @ingroup group_operators
 *
 * Greater than comparison operator. It is equivalent to aie::gt.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator>(const T1 &a, const T2 &b)
{
    return aie::gt(a, b);
}

/**
 * @ingroup group_operators
 *
 * Greater than or equal comparison operator. It is equivalent to aie::ge.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator>=(const T1 &a, const T2 &b)
{
    return aie::ge(a, b);
}

/**
 * @ingroup group_operators
 *
 * Equal to comparison operator. It is equivalent to aie::eq.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator==(const T1 &a, const T2 &b)
{
    return aie::eq(a, b);
}

/**
 * @ingroup group_operators
 *
 * Not equal to comparison operator. It is equivalent to aie::neq.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator!=(const T1 &a, const T2 &b)
{
    return aie::neq(a, b);
}

/**
 * @ingroup group_operators
 *
 * Bitwise left shift operator. It is equivalent to aie::upshift.
 *
 */
template <Vector Vec>
constexpr auto operator<<(const Vec &a, unsigned shift)
{
    return aie::upshift(a, shift);
}

/**
 * @ingroup group_operators
 *
 * Bitwise right shift operator. It is equivalent to aie::downshift.
 *
 */
template <Vector Vec>
constexpr auto operator>>(const Vec &a, unsigned shift)
{
    return aie::downshift(a, shift);
}

/**
 * @ingroup group_operators
 *
 * Addition assignment operator.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr T1& operator+=(T1 &lhs, const T2 &rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

/**
 * @ingroup group_operators
 *
 * Subtraction assignment operator.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr T1& operator-=(T1 &lhs, const T2 &rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

/**
 * @ingroup group_operators
 *
 * Bitwise OR operation. It is equivalent to aie::bit_or.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator|(const T1 &a, const T2 &b)
{
    return aie::bit_or(a, b);
}

/**
 * @ingroup group_operators
 *
 * Bitwise XOR operation. It is equivalent to aie::bit_xor.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator^(const T1 &a, const T2 &b)
{
    return aie::bit_xor(a, b);
}

/**
 * @ingroup group_operators
 *
 * Bitwise AND operation. It is equivalent to aie::bit_and.
 *
 */
template <ArithmeticType T1, ArithmeticType T2>
constexpr auto operator&(const T1 &a, const T2 &b)
{
    return aie::bit_and(a, b);
}

/**
 * @ingroup group_operators
 *
 * Bitwise NOT operation. It is equivalent to aie::bit_not.
 *
 */
template <ArithmeticType T>
constexpr auto operator~(const T &a)
{
    return aie::bit_not(a);
}

// TODO Support XOR (emulated)

} // namespace aie::operators

#endif
