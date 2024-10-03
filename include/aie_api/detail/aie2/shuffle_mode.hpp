// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SHUFFLE_MODE__HPP__
#define __AIE_API_DETAIL_AIE2_SHUFFLE_MODE__HPP__

#include "../utils.hpp"
#include "../filter.hpp"
#include "../interleave.hpp"

namespace aie::detail {

// Compute mode to be used by doing some operations on the step, given that modes are sequentially defined.
// This is with the exception of bypass (512b) modes, whose ids are defined 12 numbers later.
template <unsigned TypeBits, unsigned Elems>
class shuffle_mode {
private:
    static constexpr unsigned native_shuffle_bits = 1024;

    template <unsigned Bits2, unsigned Elems2>
    friend class shuffle_mode;

    __aie_inline
    static constexpr bool is_bypass(unsigned step);

    // Special case for 1024b vectors where each of their halves are swapped
    __aie_inline
    static constexpr bool is_swap(unsigned step);

    __aie_inline
    static constexpr unsigned zip_mode_low(unsigned step);

    __aie_inline
    static constexpr unsigned unzip_mode_low(unsigned step);

    // 1024b vectors use same shuffle mode for both bypass and swap
    // (step == 1024b vs step == 512b)
    // We need to disambiguate this somehow
    static constexpr unsigned bypass_sentinel = TypeBits * Elems >= native_shuffle_bits ? ~INTLV_lo_512o1024
                                                                                        : INTLV_lo_512o1024;

    unsigned mode;
public:
    unsigned step;

    __aie_inline
    constexpr shuffle_mode(unsigned step, shuffle_zip_tag) noexcept
        : mode(zip_mode_low(step)), step(step)
    {
    }

    __aie_inline
    constexpr shuffle_mode(unsigned step, shuffle_unzip_tag) noexcept
        : mode(unzip_mode_low(step)), step(step)
    {
    }

    // Allow casting provided the bypass special value does not change
    template <unsigned Elems2>
    __aie_inline constexpr shuffle_mode(const shuffle_mode<TypeBits, Elems2> &other) noexcept
        : mode(other.low())
    {
        REQUIRES_MSG(!other.is_bypass() ||
                     bypass_sentinel == other.bypass_sentinel,
                     "Casting bypass shuffle but vector sizes are not compatible");
    }

    __aie_inline
    constexpr unsigned low() const { return mode; }

    __aie_inline
    constexpr unsigned high() const { return mode + 1; }

    __aie_inline
    constexpr bool operator==(const shuffle_mode &other) const { return mode == other.mode; }

    __attribute__((const)) __aie_inline
    constexpr bool is_bypass() const { return mode == bypass_sentinel; }

    // Special case for 1024b vectors where each of their halves are swapped
    __aie_inline
    constexpr bool is_swap() const
    {
        return TypeBits * Elems < native_shuffle_bits ? false : mode == INTLV_lo_512o1024;
    }
};

template <unsigned TypeBits, unsigned Elems>
class filter_mode {
public:
    __aie_inline
    constexpr filter_mode(unsigned step, filter_even_tag) noexcept
        : mode(shuffle_mode<TypeBits, Elems>(step, shuffle_unzip_tag{}).low()), step(step), is_even(true)
    {
    }

    __aie_inline
    constexpr filter_mode(unsigned step, filter_odd_tag) noexcept
        : mode(shuffle_mode<TypeBits, Elems>(step, shuffle_unzip_tag{}).high()), step(step), is_even(false)
    {
    }

    // Allow casting.
    // TODO: It would be great to verify the source mode is compatible at compile time.
    template <unsigned TypeBits2, unsigned Elems2>
    __aie_inline
    constexpr filter_mode(const filter_mode<TypeBits2, Elems2> &other) noexcept
        : mode(other.mode)
    {
    }

    __aie_inline
    constexpr bool operator==(const filter_mode &other) const { return mode == other.mode; }

    unsigned mode;
    unsigned step;
    bool is_even;
};

template <unsigned TypeBits, unsigned Elems>
__aie_inline
inline constexpr bool shuffle_mode<TypeBits, Elems>::is_bypass(unsigned step)
{
    return TypeBits * Elems >= (native_shuffle_bits / 2) ? step == Elems : false;
}

template <unsigned TypeBits, unsigned Elems>
__aie_inline
inline constexpr bool shuffle_mode<TypeBits, Elems>::is_swap(unsigned step)
{
    if constexpr (TypeBits * Elems < native_shuffle_bits) {
        return false;
    }
    else {
        constexpr unsigned swap_shuffle_step = 512 / TypeBits;
        return step == swap_shuffle_step;
    }
}

template <unsigned TypeBits, unsigned Elems>
__aie_inline
inline constexpr unsigned shuffle_mode<TypeBits, Elems>::zip_mode_low(unsigned step)
{
    constexpr unsigned base_shuffle_mode = INTLV_lo_8o16 - (utils::fls(TypeBits / 8)) * 2;
    return is_bypass(step) ? bypass_sentinel
           : is_swap(step) ? INTLV_lo_512o1024
           : base_shuffle_mode - utils::fls(step) * 2;
}

template <unsigned TypeBits, unsigned Elems>
__aie_inline
inline constexpr unsigned shuffle_mode<TypeBits, Elems>::unzip_mode_low(unsigned step)
{
    constexpr unsigned base_shuffle_mode = DINTLV_lo_8o16 + (utils::fls(TypeBits / 8)) * 2;
    return is_bypass(step) ? bypass_sentinel
           : is_swap(step) ? INTLV_lo_512o1024
           : base_shuffle_mode + utils::fls(step) * 2;
}

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2_SHUFFLE_MODE__HPP__
