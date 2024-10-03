// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_SHIFT__HPP__
#define __AIE_API_DETAIL_AIE2_SHIFT__HPP__

#include "../vector.hpp"

namespace aie::detail {

template<typename T, unsigned Elems>
struct shift_bits_impl<T, 4, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        vector_type ret;

        if constexpr (Elems < 256) {
            ret = shift_bits_impl<next_type, 8, Elems>::run(v.template unpack(), upshift, downshift).pack();
        }
        else {
            const auto tmp1 = v.template extract<128>(0).unpack();
            const auto tmp2 = v.template extract<128>(1).unpack();

            ret = concat_vector(shift_bits_impl<next_type, 8, Elems / 2>::run(tmp1, upshift, downshift).pack(),
                                shift_bits_impl<next_type, 8, Elems / 2>::run(tmp2, upshift, downshift).pack());
        }

        return ret;
    }
};

template<typename T, unsigned TypeBits, unsigned Elems>
struct shift_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_ups_op()
    {
        if      constexpr (type_bits_v<T> < 32)       { return [](auto &&... args) __aie_inline { return ::sups(args...);  }; }
        else                                          { return [](auto &&... args) __aie_inline { return ::lups(args...);  }; }
    }

    static constexpr auto get_srs_op()
    {
        if      constexpr (std::is_same_v<T, uint8>)  { return [](auto &&... args) __aie_inline { return ::ussrs(args...); }; }
        else if constexpr (std::is_same_v<T, int8>)   { return [](auto &&... args) __aie_inline { return ::ssrs(args...);  }; }
        else if constexpr (std::is_same_v<T, uint16>) { return [](auto &&... args) __aie_inline { return ::ulsrs(args...); }; }
        else if constexpr (std::is_same_v<T, int16>)  { return [](auto &&... args) __aie_inline { return ::lsrs(args...);  }; }
        else if constexpr (std::is_same_v<T, uint32>) { return [](auto &&... args) __aie_inline { return ::ulsrs(args...); }; }
        else if constexpr (std::is_same_v<T, int32>)  { return [](auto &&... args) __aie_inline { return ::lsrs(args...);  }; }
        else if constexpr (std::is_same_v<T, cint16>) { return [](auto &&... args) __aie_inline { return ::ssrs(args...);  }; }
        else if constexpr (std::is_same_v<T, cint32>) { return [](auto &&... args) __aie_inline { return ::lsrs(args...);  }; }
    }

    static constexpr auto get_op_elems()
    {
        if      constexpr (utils::is_one_of_v<T, int8, uint8>)   { return 32; }
        else if constexpr (utils::is_one_of_v<T, int16, uint16>) {
            if     constexpr (Elems <= 16)                       { return 16; }
            else                                                 { return 32; }
        }
        else if constexpr (utils::is_one_of_v<T, int32, uint32>) { return 16; }
        else if constexpr (std::is_same_v<T, cint16>)            { return 8;  }
        else if constexpr (std::is_same_v<T, cint32>) {
            if     constexpr (Elems <= 4)                        { return 4;  }
            else                                                 { return 8;  }
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        constexpr auto srs_op = get_srs_op();
        constexpr auto ups_op = get_ups_op();

        constexpr unsigned op_elems   = get_op_elems(); 
        constexpr unsigned iter       = std::max(1u, Elems / op_elems);
        constexpr unsigned iter_elems = std::min(Elems, op_elems);

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            vector<T, op_elems> tmp = srs_op(ups_op(v.template grow_extract<op_elems>(idx), upshift), downshift);
            ret.template insert<iter_elems>(idx, tmp.template extract<iter_elems>(0));
        });

        return ret;
    }
};

template <typename T, unsigned Elems> struct shift_bits_impl<T,      8,  Elems> : public shift_bits_impl_common<T,      8,  Elems> {};
template <typename T, unsigned Elems> struct shift_bits_impl<T,      16, Elems> : public shift_bits_impl_common<T,      16, Elems> {};
template <typename T, unsigned Elems> struct shift_bits_impl<T,      32, Elems> : public shift_bits_impl_common<T,      32, Elems> {};
template <            unsigned Elems> struct shift_bits_impl<cint16, 32, Elems> : public shift_bits_impl_common<cint16, 32, Elems> {};
template <            unsigned Elems> struct shift_bits_impl<cint32, 64, Elems> : public shift_bits_impl_common<cint32, 64, Elems> {};

}

#endif
