// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_TILE__HPP__
#define __AIE_API_TILE__HPP__

#include "aie_types.hpp"

#if __AIE_ARCH__ == 10

#include "detail/aie1/tile.hpp"

#elif __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21

#include "detail/aie2/tile.hpp"

#endif


namespace aie {

class tile : private detail::tile
{
private:
    using base_type = detail::tile;

    tile(const base_type &t) : detail::tile(t) {}
public:
    /** \brief Return the global tile id */
    __aie_inline
    tile_id global_id() const { return base_type::global_id(); }

    /** \brief Return the tile id */
    __aie_inline
    tile_id id() const { return base_type::id(); }

    /** \brief Return the elapsed number of cycles */
    __aie_inline
    uint64_t cycles() const { return base_type::cycles(); }

    /** \brief Return an instance of \ref tile representing the current tile. */
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
    void set_rounding(rounding_mode mode) { base_type::set_rounding(mode);
    }

    /** \brief Returns current rounding mode */
    __aie_inline
    rounding_mode get_rounding() const { return base_type::get_rounding(); }
};

}

#endif // __AIE_API_TILE__HPP__
