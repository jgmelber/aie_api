// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_COMPARE__HPP__
#define __AIE_API_DETAIL_AIE1_COMPARE__HPP__

#include "../../mask.hpp"
#include "../utils.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 8, T, Elems>
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1_1 = v1.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp1_2 = v1.template extract<Elems / 2>(1);

            const vector<T, Elems / 2> tmp2_1 = v2.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_2 = v2.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp1_1, tmp2_1);
            mask<Elems/2> mask2 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp1_2, tmp2_2);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            const vector<int16, Elems> tmp1 = v1.unpack();
            const vector<int16, Elems> tmp2 = v2.unpack();

            return cmp_bits_impl<Op, 16, int16, Elems>::run(tmp1, tmp2);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = cmp_bits_impl<Op, 8, T, Elems / 2>::run((int16)(a.get()), tmp_1);
            mask<Elems/2> mask2 = cmp_bits_impl<Op, 8, T, Elems / 2>::run((int16)(a.get()), tmp_2);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return cmp_bits_impl<Op, 16, int16, Elems>::run((int16)(a.get()), tmp);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp_1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp_2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp_1, (int16)(a.get()));
            mask<Elems/2> mask2 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp_2, (int16)(a.get()));

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return cmp_bits_impl<Op, 16, int16, Elems>::run(tmp, (int16)(a.get()));
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(a, tmp1);
            mask<Elems/2> mask2 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(a, tmp2);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            return cmp_bits_impl<Op, 16, int16, Elems>::run((int16)a, v.unpack());
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp1, a);
            mask<Elems/2> mask2 = cmp_bits_impl<Op, 8, T, Elems / 2>::run(tmp2, a);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            return cmp_bits_impl<Op, 16, int16, Elems>::run(v.unpack(), (int16)a);
        }
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 16, int16, Elems>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;
    using mask_type = mask<Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == CmpOp::LT) return [](auto... args) { return ::lt32(args...); };
        else if constexpr (Op == CmpOp::GE) return [](auto... args) { return ::ge32(args...); };
    }

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems == 64) {
            unsigned result1 = op(v1.template extract<32>(0), v2.template extract<32>(0));
            unsigned result2 = op(v1.template extract<32>(1), v2.template extract<32>(1));

            return mask_type::from_uint32(result1, result2);
        }
        else if constexpr (Elems == 32) {
            unsigned result = op(v1, 0, 0x06040200, 0x0e0c0a08, 0x3210, v2, 0, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 16) {
            const auto tmp = ::concat(v1, v2);

            unsigned result = op(tmp,  0, 0x06040200, 0x0e0c0a08, 0x3210, 16, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2, undef_v8int16(), undef_v8int16());

            unsigned result = op(tmp,  0, 0x06040200, 0x06040200, 0x3210,  0, 0x02000604, 0x02000604, 0x3210);

            return mask_type::from_uint32(result);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<int16, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 32) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 32;
            subv_lane = a.offset % 32;
        }

        const unsigned odd = a.offset & 0x1;
        const int offset = subv_lane - odd;
        const int square = 0x1111 * odd;

        if constexpr (Elems == 64) {
            const unsigned result1 = op(a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square,
                                        v.template extract<32>(0),                         0, 0x06040200, 0x0e0c0a08, 0x3210);

            const unsigned result2 = op(a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square,
                                        v.template extract<32>(1),                         0, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result1, result2);
        }
        else if constexpr (Elems <= 32) {
            const unsigned result = op(a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square,
                                       v.template grow<32>(),                             0, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<int16, Elems2> a)
    {
        constexpr auto op = get_op();

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 32) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 32;
            subv_lane = a.offset % 32;
        }

        const unsigned odd = a.offset & 0x1;
        const int offset = subv_lane - odd;
        const int square = 0x1111 * odd;

        if constexpr (Elems == 64) {
            const unsigned result1 = op(v.template extract<32>(0),                         0, 0x06040200, 0x0e0c0a08, 0x3210,
                                        a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square);

            const unsigned result2 = op(v.template extract<32>(1),                         0, 0x06040200, 0x0e0c0a08, 0x3210,
                                        a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square);

            return mask_type::from_uint32(result1, result2);
        }
        else if constexpr (Elems <= 32) {
            const unsigned result = op(v.template grow<32>(),                             0, 0x06040200, 0x0e0c0a08, 0x3210,
                                       a.parent.template grow_extract<32>(subv_idx), offset, 0x00000000, 0x00000000, square);

            return mask_type::from_uint32(result);
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        const vector<T, 32> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
        const vector<T, 32> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 32, int32, Elems>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == CmpOp::LT) return [](auto... args) { return ::lt16(args...); };
        else if constexpr (Op == CmpOp::GE) return [](auto... args) { return ::ge16(args...); };
    }

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        if constexpr (Elems == 32) {
            unsigned result1 = op(v1.template extract<16>(0), v2.template extract<16>(0));
            unsigned result2 = op(v1.template extract<16>(1), v2.template extract<16>(1));

            return mask_type::from_uint32(result2 << 16 | result1);
        }
        else if constexpr (Elems == 16) {
            unsigned result = op(v1, 0, 0x76543210, 0xfedcba98, v2, 0, 0x76543210, 0xfedcba98);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2);

            unsigned result = op(tmp,  0, 0x76543210, 0x00000000,  8, 0x76543210, 0x00000000);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 4) {
            const auto tmp = ::concat(v1, v2, undef_v4int32(), undef_v4int32());

            unsigned result = op(tmp,  0, 0x00003210, 0x00000000,  4, 0x00003210, 0x00000000);

            return mask_type::from_uint32(result);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<int32, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 16) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 16;
            subv_lane = a.offset % 16;
        }


        if constexpr (Elems == 32) {
            const unsigned result1 = op(a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000,
                                        v.template extract<16>(0),                            0, 0x76543210, 0xfedcba98);

            const unsigned result2 = op(a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000,
                                        v.template extract<16>(1),                            0, 0x76543210, 0xfedcba98);

            return mask_type::from_uint32(result2 << 16 | result1);
        }
        else if constexpr (Elems <= 16) {
            const unsigned result = op(a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000,
                                       v.template grow<16>(),                                0, 0x76543210, 0xfedcba98);

            return mask_type::from_uint32(result);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<int32, Elems2> a)
    {
        constexpr auto op = get_op();

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 16) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 16;
            subv_lane = a.offset % 16;
        }

        if constexpr (Elems == 32) {
            const unsigned result1 = op(v.template extract<16>(0),                            0, 0x76543210, 0xfedcba98,
                                        a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000);

            const unsigned result2 = op(v.template extract<16>(1),                            0, 0x76543210, 0xfedcba98,
                                        a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000);

            return mask_type::from_uint32(result2 << 16 | result1);
        }
        else if constexpr (Elems <= 16) {
            const unsigned result = op(v.template grow<16>(),                                0, 0x76543210, 0xfedcba98,
                                       a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000);

            return mask_type::from_uint32(result);
        }
    }

    __aie_inline
    static mask_type run(const int32 &a, const vector_type &v)
    {
        const vector<int32, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const int32 &a)
    {
        const vector<int32, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 32, cint16, Elems>
{
    using           T = cint16;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        return cmp_bits_impl<Op, 32, int32, Elems>::run(v1.template cast_to<int32>(), v2.template cast_to<int32>());
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return cmp_bits_impl<Op, 32, int32, Elems>::run(__builtin_bit_cast(vector_elem_const_ref<int32, Elems2>, a),
                                                        v.template cast_to<int32>());
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return cmp_bits_impl<Op, 32, int32, Elems>::run(v.template cast_to<int32>(),
                                                        __builtin_bit_cast(vector_elem_const_ref<int32, Elems2>, a));
        return cmp_bits_impl<Op, 32, int32, Elems>::run(v.template cast_to<int32>(),
                                                        __builtin_bit_cast(vector_elem_const_ref<int32, Elems2>, a));
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T&a)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == CmpOp::LT) return [](auto... args) { return ::fplt_safe(args...); };
        else if constexpr (Op == CmpOp::GE) return [](auto... args) { return ::fpge_safe(args...); };
    }

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();

        unsigned result = 0;

        utils::unroll_times<std::max(1u, Elems / 8u)>([&](unsigned idx) __aie_inline {
            result |= op(v1.template grow_extract<8>(idx), v2.template grow_extract<16>(idx / 2), 8 * (idx & 1), 0x76543210) << (idx * 8);
        });

        return mask_type::from_uint32(result);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<float, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();

        unsigned result = 0;

        vector<float, 8> a_vector;

        if constexpr (Elems2 <= 16) {
            a_vector = ::ext_w(::fpshuffle16(a.parent.template grow<16>(), a.offset, 0x00000000, 0x00000000), 0);
        }
        else {
            unsigned subv_idx  = a.offset / 16;
            unsigned subv_lane = a.offset % 16;

            a_vector = ::ext_w(::fpshuffle16(a.parent.template extract<16>(subv_idx), subv_lane, 0x00000000, 0x00000000), 0);
        }

        utils::unroll_times<std::max(1u, Elems / 8u)>([&](unsigned idx) __aie_inline {
            result |= op(a_vector, v.template grow_extract<16>(idx / 2), 8 * (idx & 1), 0x76543210) << (idx * 8);
        });

        return mask_type::from_uint32(result);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<float, Elems2> a)
    {
        constexpr auto op = get_op();

        unsigned result = 0;

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 16) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 16;
            subv_lane = a.offset % 16;
        }

        utils::unroll_times<std::max(1u, Elems / 8u)>([&](unsigned idx) __aie_inline {
            result |= op(v.template grow_extract<8>(idx), a.parent.template grow_extract<16>(subv_idx), subv_lane, 0x00000000) << (idx * 8);
        });

        return mask_type::from_uint32(result);
    }

    __aie_inline
    static mask_type run(const float &a, const vector_type &v)
    {
        const vector<float, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const float &a)
    {
        const vector<float, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <typename T, unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 8, T, Elems>
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1_1 = v1.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp1_2 = v1.template extract<Elems / 2>(1);

            const vector<T, Elems / 2> tmp2_1 = v2.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2_2 = v2.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = neq<T, Elems / 2>::run(tmp1_1, tmp2_1);
            mask<Elems/2> mask2 = neq<T, Elems / 2>::run(tmp1_2, tmp2_2);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            const vector<int16, Elems> tmp1 = v1.unpack();
            const vector<int16, Elems> tmp2 = v2.unpack();

            return neq<int16, Elems>::run(tmp1, tmp2);
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = neq<T, Elems / 2>::run(a, tmp1);
            mask<Elems/2> mask2 = neq<T, Elems / 2>::run(a, tmp2);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            return neq<int16, Elems>::run((int16)a, v.unpack());
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, const int16 &a)
    {
        if constexpr (Elems == 128) {
            const vector<T, Elems / 2> tmp1 = v.template extract<Elems / 2>(0);
            const vector<T, Elems / 2> tmp2 = v.template extract<Elems / 2>(1);

            mask<Elems/2> mask1 = neq<T, Elems / 2>::run(tmp1, a);
            mask<Elems/2> mask2 = neq<T, Elems / 2>::run(tmp2, a);

            return mask_type::from_masks(mask1, mask2);
        }
        else {
            return neq<int16, Elems>::run(v.unpack(), (int16)a);
        }
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 16, int16, Elems>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;
    using mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 64) {
            const auto tmp1 = ::concat(v1.template extract<32>(0), v2.template extract<32>(0));
            const unsigned result1 = ::lt32(tmp1,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210) |
                                     ::lt32(tmp1, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210);

            const auto tmp2 = ::concat(v1.template extract<32>(1), v2.template extract<32>(1));
            const unsigned result2 = ::lt32(tmp2,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210) |
                                     ::lt32(tmp2, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result1, result2);
        }
        else if constexpr (Elems == 32) {
            const auto tmp = ::concat(v1, v2);

            const unsigned result = ::lt32(tmp,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210) |
                                    ::lt32(tmp, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 16) {
            const auto tmp = ::concat(v1, v2);

            const unsigned result = ::lt32(tmp,  0, 0x06040200, 0x0e0c0a08, 0x3210, 0, 0x0e0c0a08, 0x06040200, 0x3210);

            return mask_type::from_uint32(result | (result >> 16));
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2, undef_v8int16(), undef_v8int16());

            const unsigned result = ::lt32(tmp,  0, 0x06040200, 0x00000000, 0x3210, 0, 0x02000604, 0x00000000, 0x3210);

            return mask_type::from_uint32(result | (result >> 8));
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<int16, Elems2> a, const vector_type &v)
    {
        return lt<int16, Elems>::run(a, v) | lt<int16, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<int16, Elems2> a)
    {
        return lt<int16, Elems>::run(a, v) | lt<int16, Elems>::run(v, a);
    }

    __aie_inline
    static mask_type run(const int16 &a, const vector_type &v)
    {
        const vector<int16, 32> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const int16 &a)
    {
        const vector<int16, 32> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 32, int32, Elems>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 32) {
            const auto tmp1 = ::concat(v1.template extract<16>(0), v2.template extract<16>(0));
            const unsigned result1 = ::lt16(tmp1,  0, 0x76543210, 0xfedcba98, 16, 0x76543210, 0xfedcba98) |
                                     ::lt16(tmp1, 16, 0x76543210, 0xfedcba98,  0, 0x76543210, 0xfedcba98);

            const auto tmp2 = ::concat(v1.template extract<16>(1), v2.template extract<16>(1));
            const unsigned result2 = ::lt16(tmp2,  0, 0x76543210, 0xfedcba98, 16, 0x76543210, 0xfedcba98) |
                                     ::lt16(tmp2, 16, 0x76543210, 0xfedcba98,  0, 0x76543210, 0xfedcba98);

            return mask_type::from_uint32((result2 << 16) | result1);
        }
        else if constexpr (Elems == 16) {
            const auto tmp = ::concat(v1, v2);

            const unsigned result = ::lt16(tmp,  0, 0x76543210, 0xfedcba98, 16, 0x76543210, 0xfedcba98) |
                                    ::lt16(tmp, 16, 0x76543210, 0xfedcba98,  0, 0x76543210, 0xfedcba98);

            return mask_type::from_uint32(result);
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2);

            const unsigned result = ::lt16(tmp,  0, 0x76543210, 0xfedcba98,  0, 0xfedcba98, 0x76543210);

            return mask_type::from_uint32(result | (result >> 8));
        }
        else if constexpr (Elems == 4) {
            const auto tmp = ::concat(v1, v2, vector_type(), vector_type());

            const unsigned result = ::lt16(tmp,  0, 0x76543210, 0x00000000,  0, 0x32107654, 0x00000000);

            return mask_type::from_uint32(result | (result >> 4));
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a);
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 64, cint32, Elems>
{
    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 16) {
            const auto tmp1 = ::concat(v1.template extract<8>(0).template cast_to<int32>(),
                                       v2.template extract<8>(0).template cast_to<int32>());
            unsigned result1 = ::lt16(tmp1,  0, 0xeca86420, 0xfdb97531, 16, 0xeca86420, 0xfdb97531) |
                               ::lt16(tmp1, 16, 0xeca86420, 0xfdb97531,  0, 0xeca86420, 0xfdb97531);

            const auto tmp2 = ::concat(v1.template extract<8>(1).template cast_to<int32>(),
                                       v2.template extract<8>(1).template cast_to<int32>());
            unsigned result2 = ::lt16(tmp2,  0, 0xeca86420, 0xfdb97531, 16, 0xeca86420, 0xfdb97531) |
                               ::lt16(tmp2, 16, 0xeca86420, 0xfdb97531,  0, 0xeca86420, 0xfdb97531);

            result1 = (result1 | (result1 >> 8)) & 0xff;
            result2 = (result2 | (result2 >> 8));

            return mask_type::from_uint32((result2 << 8) | result1);
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1.template cast_to<int32>(), v2.template cast_to<int32>());

            const unsigned result = ::lt16(tmp,  0, 0xeca86420, 0xfdb97531, 16, 0xeca86420, 0xfdb97531) |
                                    ::lt16(tmp, 16, 0xeca86420, 0xfdb97531,  0, 0xeca86420, 0xfdb97531);

            return mask_type::from_uint32(result | (result >> 8));
        }
        else if constexpr (Elems <= 4) {
            const auto tmp = ::concat(v1.template cast_to<int32>().template grow<8>(),
                                      v2.template cast_to<int32>().template grow<8>());

            unsigned result = ::lt16(tmp,  0, 0x75316420, 0xfdb9eca8, 0, 0xfdb9eca8, 0x75316420);

            result = result | (result >> 8);

            return mask_type::from_uint32(result | (result >> 4));
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<T, Elems2> _a, const vector_type &_v)
    {
        if constexpr (Elems == 16) {
            vector<int32, 32> v = _v.template cast_to<int32>();
            vector<int32, 16> a = _a.parent.template cast_to<int32>().template grow<16>();

            unsigned result1 = ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(0),             0, 0xeca86420, 0xfdb97531) |
                               ::lt16(v.extract<16>(0),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111);

            unsigned result2 = ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(1),             0, 0xeca86420, 0xfdb97531) |
                               ::lt16(v.extract<16>(1),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111);

            result1 = (result1 | (result1 >> 8)) & 0xff;
            result2 = (result2 | (result2 >> 8));

            return mask_type::from_uint32((result2 << 8) | result1);
        }
        else if constexpr (Elems <= 8) {
            vector<int32, 16> v = _v.template cast_to<int32>().template grow<16>();
            vector<int32, 16> a = _a.parent.template cast_to<int32>().template grow<16>();

            const unsigned result = ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(0),             0, 0xeca86420, 0xfdb97531) |
                                    ::lt16(v.extract<16>(0),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111);

            return mask_type::from_uint32(result | (result >> 8));
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &_v, vector_elem_const_ref<T, Elems2> _a)
    {
        if constexpr (Elems == 16) {
            vector<int32, 32> v = _v.template cast_to<int32>();
            vector<int32, 16> a = _a.parent.template cast_to<int32>().template grow<16>();

            unsigned result1 = ::lt16(v.extract<16>(0),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111) |
                               ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(0),             0, 0xeca86420, 0xfdb97531);

            unsigned result2 = ::lt16(v.extract<16>(1),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111) |
                               ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(1),             0, 0xeca86420, 0xfdb97531);

            result1 = (result1 | (result1 >> 8)) & 0xff;
            result2 = (result2 | (result2 >> 8));

            return mask_type::from_uint32((result2 << 8) | result1);
        }
        else if constexpr (Elems <= 8) {
            vector<int32, 16> v = _v.template cast_to<int32>().template grow<16>();
            vector<int32, 16> a = _a.parent.template cast_to<int32>().template grow<16>();

            const unsigned result = ::lt16(v.extract<16>(0),             0, 0xeca86420, 0xfdb97531, a,                _a.offset * 2, 0x00000000, 0x11111111) |
                                    ::lt16(a,                _a.offset * 2, 0x00000000, 0x11111111, v.extract<16>(0),             0, 0xeca86420, 0xfdb97531);

            return mask_type::from_uint32(result | (result >> 8));
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        const vector<T, 8> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
        const vector<T, 8> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        return lt<float, Elems>::run(v1, v2) | lt<float, Elems>::run(v2, v1);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(vector_elem_const_ref<float, Elems2> a, const vector_type &v)
    {
        return lt<float, Elems>::run(a, v) | lt<float, Elems>::run(v, a);
    }

    template <unsigned Elems2>
    __aie_inline
    static mask_type run(const vector_type &v, vector_elem_const_ref<float, Elems2> a)
    {
        return lt<float, Elems>::run(a, v) | lt<float, Elems>::run(v, a);
    }

    __aie_inline
    static mask_type run(const float &a, const vector_type &v)
    {
        const vector<float, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, const float &a)
    {
        const vector<float, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v1), vector_cast<int16>(v2));
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        int16 val = a.get();
        val |= val << 8;
        return equal_bits_impl<16, int16, Elems / 2>::run(val, vector_cast<int16>(v));
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        int16 val = a;
        val |= val << 8;
        return equal_bits_impl<16, int16, Elems / 2>::run(val, vector_cast<int16>(v));
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        int16 val = a.get();
        val |= val << 8;
        return equal_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), val);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        int16 val = a;
        val |= val << 8;
        return equal_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), val);
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &_v1, const vector_type &_v2)
    {
        vector<int16, Elems> v1 = vector_cast<int16>(_v1);
        vector<int16, Elems> v2 = vector_cast<int16>(_v2);

        if constexpr (Elems == 64) {
            unsigned result;
            const auto tmp1 = ::concat(v1.template extract<32>(0), v2.template extract<32>(0));

            result  = ::lt32(tmp1,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210);
            result |= ::lt32(tmp1, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210);

            const auto tmp2 = ::concat(v1.template extract<32>(1), v2.template extract<32>(1));

            result |= ::lt32(tmp2,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210);
            result |= ::lt32(tmp2, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210);

            return result == 0;
        }
        else if constexpr (Elems == 32) {
            const auto tmp = ::concat(v1, v2);

            return (::lt32(tmp,  0, 0x06040200, 0x0e0c0a08, 0x3210, 32, 0x06040200, 0x0e0c0a08, 0x3210) |
                    ::lt32(tmp, 32, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x06040200, 0x0e0c0a08, 0x3210)) == 0;
        }
        else if constexpr (Elems == 16) {
            const auto tmp = ::concat(v1, v2);

            return ::lt32(tmp,  0, 0x06040200, 0x0e0c0a08, 0x3210,  0, 0x0e0c0a08, 0x06040200, 0x3210) == 0;
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2, undef_v8int16(), undef_v8int16());

            return ::lt32(tmp,  0, 0x06040200, 0x06040200, 0x3210,  0, 0x02000604, 0x02000604, 0x3210) == 0;
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return (lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a)).empty();
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return (lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a)).empty();
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        const vector<T, 32> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        const vector<T, 32> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<16, int16, Elems * 2>::run(vector_cast<int16>(v1),
                                                          vector_cast<int16>(v2));
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return (lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a)).empty();
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return (lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a)).empty();
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        const vector<T, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems>
struct equal_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        unsigned result = 0;

        if constexpr (Elems == 4) {
            const vector<float, 8> tmp_orig = concat_vector(v1, v2);

            result = ::fplt_safe(tmp_orig, tmp_orig.template grow<16>(), 0, 0x32107654);
        }
        else if constexpr (Elems == 8) {
            const auto tmp = ::concat(v1, v2);

            result |= ::fplt_safe(v1, tmp, 8, 0x76543210);
            result |= ::fplt_safe(v2, tmp, 0, 0x76543210);
        }
        else {
            utils::unroll_times<std::max(1u, Elems / 8u)>([&](unsigned idx) __aie_inline {
                result |= fplt_safe(v1.template grow_extract<8>(idx), v2.template grow_extract<16>(idx / 2), 8 * (idx & 1), 0x76543210);
                result |= fplt_safe(v2.template grow_extract<8>(idx), v1.template grow_extract<16>(idx / 2), 8 * (idx & 1), 0x76543210);
            });
        }

        return result == 0;
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<float, Elems2> a, const vector_type &v)
    {
        return (lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a)).empty();
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<float, Elems2> a)
    {
        return run(a, v);
    }

    __aie_inline
    static bool run(const float &a, const vector_type &v)
    {
        const vector<float, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static bool run(const vector_type &v, const float &a)
    {
        return run(a, v);
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<16, int16, Elems * 4>::run(vector_cast<int16>(v1),
                                                          vector_cast<int16>(v2));
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &_v)
    {
        using mask_type = mask<2 * Elems>;

        const auto v = vector_cast<int32>(_v);
        const auto a_parent = vector_cast<int32>(a.parent);

        unsigned subv_idx, subv_lane;

        if constexpr (Elems2 <= 8) {
            subv_idx  = 0;
            subv_lane = a.offset;
        }
        else {
            subv_idx  = a.offset / 8;
            subv_lane = a.offset % 8;
        }

        unsigned result = 0;

        constexpr unsigned RealElems = 2 * Elems;
        utils::unroll_times<std::max(1u, RealElems / 16u)>([&](unsigned idx) __aie_inline {
            result |= ::lt16(a_parent.template grow_extract<16>(subv_idx), subv_lane * 2, 0x10101010, 0x10101010,
                             v.template grow_extract<16>(idx),                         0, 0x76543210, 0xfedcba98);
            result |= ::lt16(v.template grow_extract<16>(idx),                         0, 0x76543210, 0xfedcba98,
                             a_parent.template grow_extract<16>(subv_idx), subv_lane * 2, 0x10101010, 0x10101010);
        });

        return mask_type::from_uint32(result).empty();
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(a, v);
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        const vector<T, 8> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        return run(a, v);
    }
};

template <unsigned Elems>
struct equal_bits_impl<64, cfloat, Elems>
{
    using           T = cfloat;
    using vector_type = vector<T, Elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<32, float, Elems * 2>::run(vector_cast<float>(v1),
                                                          vector_cast<float>(v2));
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(vector_elem_const_ref<T, Elems2> a, const vector_type &_v)
    {
        using mask_type = mask<2 * Elems>;

        unsigned result = 0;

        vector<float, 8> a_vector;
        
        if constexpr (Elems2 <= 8){
            a_vector = ::as_v8float(::ext_w(::fpshuffle8(a.parent.template grow<8>(), a.offset, 0x00000000), 0));
        }
        else {
            const unsigned subv_idx  = a.offset / 8;
            const unsigned subv_lane = a.offset % 8;

            a_vector = ::as_v8float(::ext_w(::fpshuffle8(a.parent.template extract<8>(subv_idx), subv_lane, 0x00000000), 0));
        }

        const auto v = vector_cast<float>(_v);

        constexpr unsigned RealElems = 2 * Elems;
        utils::unroll_times<std::max(1u, RealElems / 8u)>([&](unsigned idx) __aie_inline {
            result |= ::fplt_safe(a_vector,                        v.template grow_extract<16>(idx / 2), 8 * (idx & 1), 0x76543210);
            result |= ::fplt_safe(v.template grow_extract<8>(idx), a_vector.template grow<16>(),                     0, 0x76543210);
        });

        return mask_type::from_uint32(result).empty();
    }

    template <unsigned Elems2>
    __aie_inline
    static bool run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(a, v);
    }

    __aie_inline
    static bool run(const T &a, const vector_type &v)
    {
        const vector<T, 4> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    __aie_inline
    static bool run(const vector_type &v, const T &a)
    {
        const vector<T, 4> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

}

#endif
