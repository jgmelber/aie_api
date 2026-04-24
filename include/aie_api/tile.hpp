// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_TILE__HPP__
#define __AIE_API_TILE__HPP__

#include "aie_types.hpp"

#if __AIE_ARCH__ == 10

#include "detail/aie1/tile.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22

#include "detail/aie2/tile.hpp"

#endif


namespace aie {

class tile : private detail::tile
{
private:
    using base_type = detail::tile;

    tile(const base_type &t) : detail::tile(t) {}
public:
    /** @ingroup group_config
      * \brief Return the global tile id */
    __aie_inline
    tile_id global_id() const { return base_type::global_id(); }

    /** @ingroup group_config
      * \brief Return the tile id */
    __aie_inline
    tile_id id() const { return base_type::id(); }

    /** @ingroup group_config
      * \brief Return the elapsed number of cycles */
    __aie_inline
    uint64_t cycles() const { return base_type::cycles(); }

    /** @ingroup group_config
      * \brief Return an instance of \ref tile representing the current tile. */
    __aie_inline
    static tile current() { return base_type::current(); }

    /** \brief Changes saturation mode */
    __aie_inline
    void set_saturation(saturation_mode mode) { base_type::set_saturation(mode); }

    /** \brief Returns current saturation mode */
    __aie_inline
    saturation_mode get_saturation() const { return base_type::get_saturation(); }

    /** \brief Changes rounding mode */
    __aie_inline
    void set_rounding(rounding_mode mode) { base_type::set_rounding(mode); }

    /** \brief Returns current rounding mode */
    __aie_inline
    rounding_mode get_rounding() const { return base_type::get_rounding(); }
};

/**
 * Sets saturation/rounding mode in the current scope, restoring the existing mode upon exit.
 */
template <typename T>
    requires(std::is_same_v<T, rounding_mode> || std::is_same_v<T, saturation_mode>)
class scoped_mode {
public:
    __aie_inline
    explicit scoped_mode(const T &desired) noexcept
        : saved(current())
    {
        set(desired);
    }

    scoped_mode(const scoped_mode &) = delete; // non copyable
    scoped_mode(scoped_mode &&) = delete; // non moveable

    scoped_mode& operator=(const scoped_mode &) = delete; // non copy assignable
    scoped_mode& operator=(scoped_mode &&) = delete; // non move assignable

    __aie_inline
    ~scoped_mode() noexcept { set(saved); }

private:
    __aie_inline
    static T current() {
        if constexpr (std::is_same_v<T, saturation_mode>)
            return tile::current().get_saturation();
        if constexpr (std::is_same_v<T, rounding_mode>)
            return tile::current().get_rounding();
    }

    __aie_inline
    static void set(const T& mode) {
        if constexpr (std::is_same_v<T, saturation_mode>)
            tile::current().set_saturation(mode);
        if constexpr (std::is_same_v<T, rounding_mode>)
            tile::current().set_rounding(mode);
    }

    T saved;
};

// Template deduction guidelines for class scoped_mode
scoped_mode(saturation_mode) -> scoped_mode<saturation_mode>;
scoped_mode(rounding_mode) -> scoped_mode<rounding_mode>;

} // namespace aie

#endif // __AIE_API_TILE__HPP__
