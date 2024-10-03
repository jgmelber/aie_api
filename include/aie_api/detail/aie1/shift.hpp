// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SHIFT__HPP__
#define __AIE_API_DETAIL_AIE1_SHIFT__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
static constexpr auto get_ups_op()
{
    if      constexpr (std::is_same_v<T, uint8>)                { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
    else if constexpr (std::is_same_v<T, int8>)                 { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
    else if constexpr (std::is_same_v<T, int16>)                { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
    else if constexpr (std::is_same_v<T, int32> && Elems == 4)  { return [](auto &&... args) __aie_inline { return ::lups(args...); }; }
    else if constexpr (std::is_same_v<T, int32>)                { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
    else if constexpr (std::is_same_v<T, cint16>)               { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
    else if constexpr (std::is_same_v<T, cint32> && Elems == 2) { return [](auto &&... args) __aie_inline { return ::lups(args...); }; }
    else if constexpr (std::is_same_v<T, cint32>)               { return [](auto &&... args) __aie_inline { return ::ups(args...);  }; }
}

template <typename T, unsigned Elems>
static constexpr auto get_srs_op()
{
    if      constexpr (std::is_same_v<T, uint8>)                { return [](auto &&... args) __aie_inline { return ::ubsrs(args...); }; }
    else if constexpr (std::is_same_v<T, int8>)                 { return [](auto &&... args) __aie_inline { return ::bsrs(args...);  }; }
    else if constexpr (std::is_same_v<T, int16>)                { return [](auto &&... args) __aie_inline { return ::srs(args...);   }; }
    else if constexpr (std::is_same_v<T, int32>  && Elems == 4) { return [](auto &&... args) __aie_inline { return ::srs(args...);   }; }
    else if constexpr (std::is_same_v<T, int32>)                { return [](auto &&... args) __aie_inline { return ::lsrs(args...);  }; }
    else if constexpr (std::is_same_v<T, cint16>)               { return [](auto &&... args) __aie_inline { return ::srs(args...);   }; }
    else if constexpr (std::is_same_v<T, cint32> && Elems == 2) { return [](auto &&... args) __aie_inline { return ::srs(args...);   }; }
    else if constexpr (std::is_same_v<T, cint32> )              { return [](auto &&... args) __aie_inline { return ::lsrs(args...);  }; }

}

template<typename T, unsigned Elems>
struct shift_bits_impl<T, 8, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        constexpr auto srs_op = get_srs_op<T, Elems>();
        constexpr auto ups_op = get_ups_op<T, Elems>();

        utils::unroll_times<Elems / 16>([&](auto idx) __aie_inline {
            auto tmp = srs_op(ups_op(v.template extract<16>(idx), upshift), downshift);
            ret.template insert<16>(idx, tmp);
        });

        return ret;
    }
};

template<unsigned Elems>
struct shift_bits_impl<int16, 16, Elems>
{
    using vector_type = vector<int16, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        constexpr auto srs_op = get_srs_op<int16, Elems>();
        constexpr auto ups_op = get_ups_op<int16, Elems>();

        constexpr unsigned iter = std::max(Elems / 16u, 1u);
        constexpr unsigned iter_elems = std::min(Elems, 16u);

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            auto tmp = srs_op(ups_op(v.template extract<iter_elems>(idx), upshift), downshift);
            ret.template insert<iter_elems>(idx, tmp);
        });

        return ret;
    }
};

template<typename T, unsigned Elems>
struct shift_bits_impl<T, 32, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        constexpr auto srs_op = get_srs_op<T, Elems>();
        constexpr auto ups_op = get_ups_op<T, Elems>();

        constexpr unsigned iter = std::max(Elems / 8u, 1u);
        constexpr unsigned iter_elems = std::min(Elems, 8u);

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            auto tmp = srs_op(ups_op(v.template extract<iter_elems>(idx), upshift), downshift);
            ret.template insert<iter_elems>(idx, tmp);
        });

        return ret;
    }
};

template<unsigned Elems>
struct shift_bits_impl<cint32, 64, Elems>
{
    using vector_type = vector<cint32, Elems>;

    __aie_inline
    static vector_type run(const vector_type &v, unsigned upshift, unsigned downshift)
    {
        vector_type ret;

        constexpr auto srs_op = get_srs_op<cint32, Elems>();
        constexpr auto ups_op = get_ups_op<cint32, Elems>();

        constexpr unsigned iter = std::max(Elems / 4u, 1u);
        constexpr unsigned iter_elems = std::min(Elems, 4u);

        utils::unroll_times<iter>([&](auto idx) __aie_inline {
            auto tmp = srs_op(ups_op(v.template extract<iter_elems>(idx), upshift), downshift);
            ret.template insert<iter_elems>(idx, tmp);
        });

        return ret;
    }
};

}

#endif
