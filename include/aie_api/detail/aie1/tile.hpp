// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_TILE__HPP__
#define __AIE_API_DETAIL_AIE1_TILE__HPP__

#include "../../aie_types.hpp"

namespace aie::detail {

struct tile_id_hw
{
    union
    {
        int data;

        struct
        {
            unsigned row       : 5;
            unsigned reserved1 : 11;
            unsigned col       : 7;
            unsigned reserved2 : 9;
        } coord;
    };
};

class tile
{
private:
    static constexpr uint16_t compute_row_offset = 1;

    __aie_inline
    constexpr tile() {}

public:
    __aie_inline
    tile_id global_id() const
    {
        const tile_id_hw id_hw = { get_coreid() };
        const tile_id ret      = { uint16_t(id_hw.coord.row), uint16_t(id_hw.coord.col) };

        return ret;
    }

    __aie_inline
    tile_id id() const
    {
        const tile_id gid = global_id();
        const tile_id ret = { uint16_t(gid.row - compute_row_offset), gid.col };

        return ret;
    }

    __aie_inline
    uint64_t cycles() const
    {
        return ::get_cycles();
    }

    __aie_inline
    static tile current()
    {
        return tile();
    }

    __aie_inline
    void set_saturation(saturation_mode mode)
    {
        // CRVO-2774: create combined get_sat get_symsat intrinsic
        if (mode == saturation_mode::none) {
            ::clr_sat();
            ::clr_symsat();
        }
        else if (mode == saturation_mode::saturate) {
            ::set_sat();
        }
        else {
            ::set_sat();
            ::set_symsat();
        }
    }

    __aie_inline
    saturation_mode get_saturation() const
    {
        // CRVO-2774: create combined get_sat get_symsat intrinsic
        return (saturation_mode)((::get_symsat() << 1) | ::get_sat());
    }

    __aie_inline
    void set_rounding(rounding_mode mode)
    {
        ::set_rnd((unsigned)mode);
    }

    __aie_inline
    rounding_mode get_rounding() const
    {
        return (rounding_mode)::get_rnd();
    }
};

} // namespace aie::detail

#endif
