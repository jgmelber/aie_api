// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_REVERSE__HPP__
#define __AIE_API_DETAIL_AIE2_REVERSE__HPP__

#include "../../vector.hpp"
#include "../../accum.hpp"

namespace aie::detail {

alignas(vector_decl_align) static constexpr uint8 reverse_matrix_8x8[64] = { 0, 0, 0, 0, 0, 0, 0, 1,
                                                                             0, 0, 0, 0, 0, 0, 1, 0,
                                                                             0, 0, 0, 0, 0, 1, 0, 0,
                                                                             0, 0, 0, 0, 1, 0, 0, 0,
                                                                             0, 0, 0, 1, 0, 0, 0, 0,
                                                                             0, 0, 1, 0, 0, 0, 0, 0,
                                                                             0, 1, 0, 0, 0, 0, 0, 0,
                                                                             1, 0, 0, 0, 0, 0, 0, 0 };

template <typename T, unsigned Elems>
struct reverse_impl<4, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        vector_type ret;

        if constexpr (vector_type::bits() == 1024) {
            ret.template insert(1, reverse_impl<8, next_type, Elems / 2>::run(v.template extract<Elems / 2>(0).unpack()).pack());
            ret.template insert(0, reverse_impl<8, next_type, Elems / 2>::run(v.template extract<Elems / 2>(1).unpack()).pack());
        }
        else {
            return reverse_impl<8, next_type, Elems>::run(v.unpack()).pack();
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        const vector<uint8, 64> v_reverse = *(v64uint8 *)reverse_matrix_8x8;

        const vector<uint16, Elems / 2> v_uint16 = v.template cast_to<uint16>();

        vector_type ret;

        if constexpr (Elems == 16) {
            accum<acc64, 16> acc;
            vector<T, 32> v2;
            vector<T, 64> v3;
            vector<T, 64> tmp_lo, tmp_hi;

            acc = ::mul_2x8_8x8(v_uint16.template grow<32>(), v_reverse);
            v2 = acc.template to_vector<uint16>().template cast_to<T>();

            // Swap neighboring elements
            tmp_lo = ::shuffle(v2.template grow<64>(), v2.template grow<64>(), DINTLV_lo_8o16);
            tmp_hi = ::shuffle(v2.template grow<64>(), v2.template grow<64>(), DINTLV_hi_8o16);

            v3 = shuffle(tmp_hi, tmp_lo, INTLV_lo_8o16);

            ret = v3.template extract<Elems>(0);
        }
        else {
            utils::unroll_times<Elems / 32>([&](auto idx) __aie_inline {
                accum<acc64, 16> acc;
                vector<T, 32> v2;
                vector<T, 64> v3;
                vector<T, 64> tmp_lo, tmp_hi;

                vector<uint16, 32> tmp;

                if constexpr (idx == 0)
                    tmp = v_uint16.template grow_extract<32>(idx);
                else
                    tmp = v_uint16.template extract<16>(idx).template grow<32>();

                // Swap the two 8-element subvectors
                tmp = ::shiftx(tmp, tmp, 3, 48);

                acc = ::mul_2x8_8x8(tmp, v_reverse);

                v2 = acc.template to_vector<uint16>().template cast_to<T>();

                // Swap neighboring elements
                tmp_lo = ::shuffle(v2.template grow<64>(), v2.template grow<64>(), DINTLV_lo_8o16);
                tmp_hi = ::shuffle(v2.template grow<64>(), v2.template grow<64>(), DINTLV_hi_8o16);

                v3 = shuffle(tmp_hi, tmp_lo, INTLV_lo_8o16);

                ret.insert(Elems / 32 - idx - 1, v3.template extract<32>(0));
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        const vector<uint8, 64> v_reverse = *(v64uint8 *)reverse_matrix_8x8;

        const vector<uint16, Elems> v_uint16 = v.template cast_to<uint16>();

        vector_type ret;

        if constexpr (Elems == 8) {
            const accum<acc64, 16> acc = ::mul_2x8_8x8(v_uint16.template grow<32>(), v_reverse);

            ret = acc.template to_vector<uint16>().template cast_to<T>().template extract<Elems>(0);
        }
        else {
            utils::unroll_times<Elems / 16>([&](auto idx) __aie_inline {
                accum<acc64, 16> acc;
                vector<uint16, 16> v2;

                vector<uint16, 32> tmp;

                if constexpr (idx == 0)
                    tmp = v_uint16.template grow_extract<32>(idx);
                else
                    tmp = v_uint16.template extract<16>(idx).template grow<32>();

                // Swap the two 8-element subvectors
                tmp = ::shiftx(tmp, tmp, 3, 48);

                acc = ::mul_2x8_8x8(tmp, v_reverse);

                v2 = acc.template to_vector<uint16>();

                ret.insert(Elems / 16 - idx - 1, v2.template cast_to<T>().template extract<16>(0));
            });
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        if constexpr (Elems == 4) {
            return reverse<T, 16>::run(v.template grow<16>()).template extract<Elems>(3);
        }
        else if constexpr (Elems == 8) {
            return reverse<T, 16>::run(v.template grow<16>()).template extract<Elems>(1);
        }
        else if constexpr (Elems == 16) {
            vector<uint32, Elems> v1_uint32 = v.template cast_to<uint32>();
            vector<uint32, Elems> v2_uint32;

            v2_uint32 = ::shiftx(v1_uint32, v1_uint32, 3, 32);
            v1_uint32 = ::shift_bytes(v1_uint32, v2_uint32, 48);
            v1_uint32 = ::shuffle(v1_uint32, v2_uint32, T32_4x8_lo);

            v2_uint32 = ::shiftx(v1_uint32, v1_uint32, 3, 32);
            v1_uint32 = ::shift_bytes(v1_uint32, v2_uint32, 48);
            v2_uint32 = ::shuffle(v1_uint32, v2_uint32, T32_4x8_lo);

            return v2_uint32.template cast_to<T>();
        }
        else if constexpr (Elems == 32) {
            vector_type ret;

            ret.insert(0, reverse<T, 16>::run(v.template extract<16>(1)));
            ret.insert(1, reverse<T, 16>::run(v.template extract<16>(0)));

            return ret;
        }
    }
};

template <typename T, unsigned Elems>
struct reverse_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v)
    {
        if constexpr (Elems == 2) {
            vector<T, 8> ret;
            ret[0] = v.template grow<8>()[1];
            ret[1] = v.template grow<8>()[0];

            return ret.template extract<2>(0);
        }
        else if constexpr (Elems == 4) {
            return reverse<T, 8>::run(v.template grow<8>()).template extract<Elems>(1);
        }
        else if constexpr (Elems == 8) {
            vector<cint32, Elems> v1_cint32 = v.template cast_to<cint32>();
            vector<cint32, Elems> v2_cint32;

            v2_cint32 = ::shiftx(v1_cint32, v1_cint32, 3, 32);
            v1_cint32 = ::shift_bytes( v1_cint32, v2_cint32, 48 );
            v2_cint32 = ::shuffle(v1_cint32, v1_cint32, DINTLV_lo_128o256);
            v1_cint32 = ::shuffle(v1_cint32, v1_cint32, DINTLV_hi_128o256);
            v1_cint32 = ::shuffle(v2_cint32, v1_cint32, INTLV_hi_256o512);
            v2_cint32 = ::shuffle(v1_cint32, v1_cint32, DINTLV_lo_64o128);
            v1_cint32 = ::shuffle(v1_cint32, v1_cint32, DINTLV_hi_64o128);
            v1_cint32 = ::shuffle(v1_cint32, v2_cint32, INTLV_lo_64o128);

            return v1_cint32.template cast_to<T>();
        }
        else if constexpr (Elems == 16) {
            vector_type ret;

            ret.insert(0, reverse<T, 8>::run(v.template extract<8>(1)));
            ret.insert(1, reverse<T, 8>::run(v.template extract<8>(0)));

            return ret;
        }
    }
};

}

#endif
