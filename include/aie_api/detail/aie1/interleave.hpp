// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_INTERLEAVE__HPP__
#define __AIE_API_DETAIL_AIE1_INTERLEAVE__HPP__

#include "../vector.hpp"
#include "../reverse.hpp"

namespace aie::detail {

template <unsigned TypeBits, typename T, unsigned Elems>
struct interleave_common_helper;

template <typename T>
struct interleave_common_helper<32, T, 16>
{
    static constexpr unsigned Elems = 16;

    using vector_type = vector<T, Elems>;

    template <typename... SelectNext>
    static constexpr unsigned compute_select_mask(unsigned select, SelectNext... select_next)
    {
        unsigned ret = (select >= Elems? 1 : 0);

        if constexpr (sizeof...(SelectNext) > 0)
            ret |= compute_select_mask(select_next...) << 1;

        return ret;
    }

    template <bool First, typename... SelectNext>
    static constexpr unsigned compute_offset_mask(unsigned select, SelectNext... select_next)
    {
        unsigned ret = 0;

        if constexpr (First)
            ret = (select < Elems? select : 0);
        else
            ret = (select >= Elems? (select - Elems) : 0);

        if constexpr (sizeof...(SelectNext) > 0)
            ret |= compute_offset_mask<First>(select_next...) << 4;

        return ret;
    }

    template <typename... Select>
    __aie_inline
    static constexpr auto compute_offsets(Select... select)
    {
        const auto select_tuple = std::make_tuple(select...);

        const unsigned a_offset_lo = compute_offset_mask<true> (std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                                std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                                std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                                std::get<6>(select_tuple),  std::get<7>(select_tuple));
        const unsigned a_offset_hi = compute_offset_mask<true> (std::get<8>(select_tuple),  std::get<9>(select_tuple),
                                                                std::get<10>(select_tuple), std::get<11>(select_tuple),
                                                                std::get<12>(select_tuple), std::get<13>(select_tuple),
                                                                std::get<14>(select_tuple), std::get<15>(select_tuple));

        const unsigned b_offset_lo = compute_offset_mask<false>(std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                                std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                                std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                                std::get<6>(select_tuple),  std::get<7>(select_tuple));
        const unsigned b_offset_hi = compute_offset_mask<false>(std::get<8>(select_tuple),  std::get<9>(select_tuple),
                                                                std::get<10>(select_tuple), std::get<11>(select_tuple),
                                                                std::get<12>(select_tuple), std::get<13>(select_tuple),
                                                                std::get<14>(select_tuple), std::get<15>(select_tuple));



        return std::tuple(a_offset_lo, a_offset_hi, b_offset_lo, b_offset_hi);
    }

    template <typename... Select>
    static vector_type run(const vector_type &a, const vector_type &b, Select... select)
    {
        static_assert(sizeof...(Select) == Elems);

        vector_type ret;
        const unsigned select_mask = compute_select_mask(select...);

        const auto [a_offset_lo, a_offset_hi, b_offset_lo, b_offset_hi] = compute_offsets(std::forward<Select>(select)...);

        if constexpr (is_floating_point_v<T>) {
            ret = ::fpselect16(select_mask, a, 0, a_offset_lo, a_offset_hi,
                                            b, 0, b_offset_lo, b_offset_hi);
        }
        else {
            ret = ::select16(select_mask, a, 0, a_offset_lo, a_offset_hi,
                                          b, 0, b_offset_lo, b_offset_hi);
        }

        return ret;
    }
};

template <typename T>
struct interleave_common_helper<64, T, 8>
{
    static constexpr unsigned Elems = 8;

    using vector_type = vector<T, Elems>;

    template <typename... SelectNext>
    static constexpr unsigned compute_select_mask(unsigned select, SelectNext... select_next)
    {
        unsigned ret = (select >= Elems? 1 : 0);

        if constexpr (sizeof...(SelectNext) > 0)
            ret |= compute_select_mask(select_next...) << 1;

        return ret;
    }

    template <bool First, typename... SelectNext>
    static constexpr unsigned compute_offset_mask(unsigned select, SelectNext... select_next)
    {
        unsigned ret = 0;

        if constexpr (First)
            ret = (select < Elems? select : 0);
        else
            ret = (select >= Elems? (select - Elems) : 0);

        if constexpr (sizeof...(SelectNext) > 0)
            ret |= compute_offset_mask<First>(select_next...) << 4;

        return ret;
    }

    template <typename... Select>
    __aie_inline
    static constexpr auto compute_offsets(Select... select)
    {
        const auto select_tuple = std::make_tuple(select...);

        const unsigned a_offset = compute_offset_mask<true> (std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                             std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                             std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                             std::get<6>(select_tuple),  std::get<7>(select_tuple));

        const unsigned b_offset = compute_offset_mask<false>(std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                             std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                             std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                             std::get<6>(select_tuple),  std::get<7>(select_tuple));

        return std::tuple(a_offset, b_offset);
    }

    template <typename... Select>
    static vector_type run(const vector_type &a, const vector_type &b, Select... select)
    {
        static_assert(sizeof...(Select) == Elems);

        vector_type ret;
        const unsigned select_mask = compute_select_mask(select...);
        const auto select_tuple = std::make_tuple(select...);

        const auto [a_offset, b_offset] = compute_offsets(std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                          std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                          std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                          std::get<6>(select_tuple),  std::get<7>(select_tuple));

        if constexpr (is_floating_point_v<T>) {
            ret = ::fpselect8(select_mask, a, 0, a_offset,
                                           b, 0, b_offset);
        }
        else {
            ret = ::select8(select_mask, a, 0, a_offset,
                                         b, 0, b_offset);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_zip<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        if (step == 1)
            return zip_serial(a, b, step);

        vector<int16, Elems / 2> tmp1, tmp2;

        std::tie(tmp1, tmp2) = interleave_bits_zip<16, int16, Elems / 2>::run(vector_cast<int16>(a),
                                                                              vector_cast<int16>(b), step / 2);

        return std::make_pair(vector_cast<T>(tmp1), vector_cast<T>(tmp2));
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_zip<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<16, T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        if (step > 1) {
            auto [tmp1, tmp2] = interleave_bits_zip<32, int32, Elems / 2>::run(vector_cast<int32>(a),
                                                                               vector_cast<int32>(b), step / 2);

            return std::make_pair(vector_cast<T>(tmp1), vector_cast<T>(tmp2));
        }

        if constexpr (Elems == 8) {
            const vector<T, 32> tmp = ::select32(0xaaaaaaaa, a.template grow<32>(), 0, 0x03020100, 0x00000000, 0x0100,
                                                             b.template grow<32>(), 0, 0x03020100, 0x00000000, 0x1000);

            return std::make_pair(tmp.template extract<Elems>(0),
                                  tmp.template extract<Elems>(1));
        }
        else if constexpr (Elems == 16) {
            const vector<T, 32> tmp = ::select32(0xaaaaaaaa, a.template grow<32>(), 0, 0x03020100, 0x07060504, 0x0100,
                                                             b.template grow<32>(), 0, 0x03020100, 0x07060504, 0x1000);

            return std::make_pair(tmp.template extract<Elems>(0),
                                  tmp.template extract<Elems>(1));
        }
        else if constexpr (Elems == 32) {
            const vector<T, 32> tmp1 = ::select32(0xaaaaaaaa, a,  0, 0x03020100, 0x07060504, 0x0100,
                                                              b,  0, 0x03020100, 0x07060504, 0x1000);

            const vector<T, 32> tmp2 = ::select32(0xaaaaaaaa, a, 16, 0x03020100, 0x07060504, 0x0100,
                                                              b, 16, 0x03020100, 0x07060504, 0x1000);

            return std::make_pair(tmp1, tmp2);
        }
        else if constexpr (Elems == 64) {
            vector_type first, second;

            const vector<T, 32> tmp1 = ::select32(0xaaaaaaaa, a.template extract<32>(0),  0, 0x03020100, 0x07060504, 0x0100,
                                                              b.template extract<32>(0),  0, 0x03020100, 0x07060504, 0x1000);
            first.insert(0, tmp1);

            const vector<T, 32> tmp2 = ::select32(0xaaaaaaaa, a.template extract<32>(0), 16, 0x03020100, 0x07060504, 0x0100,
                                                              b.template extract<32>(0), 16, 0x03020100, 0x07060504, 0x1000);
            first.insert(1, tmp2);

            const vector<T, 32> tmp3 = ::select32(0xaaaaaaaa, a.template extract<32>(1),  0, 0x03020100, 0x07060504, 0x0100,
                                                              b.template extract<32>(1),  0, 0x03020100, 0x07060504, 0x1000);
            second.insert(0, tmp3);

            const vector<T, 32> tmp4 = ::select32(0xaaaaaaaa, a.template extract<32>(1), 16, 0x03020100, 0x07060504, 0x0100,
                                                              b.template extract<32>(1), 16, 0x03020100, 0x07060504, 0x1000);
            second.insert(1, tmp4);

            return std::make_pair(first, second);
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_zip<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 8) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle16(params...); };
            else                                  return [](auto &&...params) { return ::shuffle16(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect16(params...); };
            else                                  return [](auto &&...params) { return ::select16(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 4) {
            vector<T, 8> tmp1, tmp2;

            std::tie(tmp1, tmp2) = interleave_bits_zip<32, T, 8>::run(a.template grow<8>(), b.template grow<8>(), step);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp1.template extract<Elems>(1));
        }
        else if constexpr (Elems == 8) {
            vector_type ret1, ret2;
            vector<T, 16> tmp = concat_vector(a, b);

            unsigned offset_lo;
            unsigned offset_hi;

            if (step == 4) {
                offset_lo = 0xba983210;
                offset_hi = 0xfedc7654;
            }
            else if (step == 2) {
                offset_lo = 0xba329810;
                offset_hi = 0xfe76dc54;
            }
            else if (step == 1) {
                offset_lo = 0xb3a29180;
                offset_hi = 0xf7e6d5c4;
            }

            tmp = op(tmp, 0, offset_lo, offset_hi);

            ret1 = tmp.template extract<8>(0);
            ret2 = tmp.template extract<8>(1);

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 16) {
            vector_type ret1, ret2;

            if (chess_manifest(step == 8)) {
                ret1 = concat_vector(a.template extract<8>(0), b.template extract<8>(0));
                ret2 = concat_vector(a.template extract<8>(1), b.template extract<8>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset_lo1, a_offset_lo2;
                unsigned a_offset_hi1, a_offset_hi2;
                unsigned b_offset_lo1, b_offset_lo2;
                unsigned b_offset_hi1, b_offset_hi2;

                if (step == 8) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                }
                else if (step == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3, 16, 17, 18, 19,  4,  5,  6,  7, 20, 21, 22, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3, 16, 17, 18, 19,  4,  5,  6,  7, 20, 21, 22, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31);
                }
                else if (step == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1, 16, 17,  2,  3, 18, 19,  4,  5, 20, 21,  6,  7, 22, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 24, 25, 10, 11, 26, 27, 12, 13, 28, 29, 14, 15, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1, 16, 17,  2,  3, 18, 19,  4,  5, 20, 21,  6,  7, 22, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 24, 25, 10, 11, 26, 27, 12, 13, 28, 29, 14, 15, 30, 31);
                }
                else if (step == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
                }

                ret1 = op(select_mask1, a, 0, a_offset_lo1, a_offset_hi1,
                                        b, 0, b_offset_lo1, b_offset_hi1);
                ret2 = op(select_mask2, a, 0, a_offset_lo2, a_offset_hi2,
                                        b, 0, b_offset_lo2, b_offset_hi2);
            }

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2, tmp3, tmp4;

            std::tie(tmp1, tmp2) = interleave_bits_zip<32, T, 16>::run(a.template extract<16>(0), b.template extract<16>(0), step);
            std::tie(tmp3, tmp4) = interleave_bits_zip<32, T, 16>::run(a.template extract<16>(1), b.template extract<16>(1), step);

            return std::make_pair(concat_vector(tmp1, tmp2),
                                  concat_vector(tmp3, tmp4));
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_zip<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<64, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 4) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle8(params...); };
            else                                  return [](auto &&...params) { return ::shuffle8(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect8(params...); };
            else                                  return [](auto &&...params) { return ::select8(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 2) {
            vector<T, 4> tmp1, tmp2;

            std::tie(tmp1, tmp2) = interleave_bits_zip<64, T, 4>::run(a.template grow<4>(), b.template grow<4>(), step);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp1.template extract<Elems>(1));
        }
        else if constexpr (Elems == 4) {
            vector_type ret1, ret2;
            vector<T, 8> tmp = concat_vector(a, b);

            unsigned offset;

            if (step == 2)
                offset = 0x76325410;
            else if (step == 1)
                offset = 0x73625140;

            tmp = op(tmp, 0, offset);

            ret1 = tmp.template extract<4>(0);
            ret2 = tmp.template extract<4>(1);

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 8) {
            vector_type ret1, ret2;

            if (chess_manifest(step == 4)) {
                ret1 = concat_vector(a.template extract<4>(0), b.template extract<4>(0));
                ret2 = concat_vector(a.template extract<4>(1), b.template extract<4>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset1, a_offset2;
                unsigned b_offset1, b_offset2;

                if (step == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  8,  9, 10, 11);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 12, 13, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  8,  9, 10, 11);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 12, 13, 14, 15);
                }
                else if (step == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  8,  9,  2,  3, 10, 11);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5, 12, 13,  6,  7, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  8,  9,  2,  3, 10, 11);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4,  5, 12, 13,  6,  7, 14, 15);
                }
                else if (step == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  8,  1,  9,  2, 10,  3, 11);
                    select_mask2 = interleave_common_type::template compute_select_mask(4, 12,  5, 13,  6, 14,  7, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  8,  1,  9,  2, 10,  3, 11);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4, 12,  5, 13,  6, 14,  7, 15);
                }

                ret1 = op(select_mask1, a, 0, a_offset1, b, 0, b_offset1);
                ret2 = op(select_mask2, a, 0, a_offset2, b, 0, b_offset2);
            }

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2, tmp3, tmp4;

            std::tie(tmp1, tmp2) = interleave_bits_zip<64, T, 8>::run(a.template extract<8>(0), b.template extract<8>(0), step);
            std::tie(tmp3, tmp4) = interleave_bits_zip<64, T, 8>::run(a.template extract<8>(1), b.template extract<8>(1), step);

            return std::make_pair(concat_vector(tmp1, tmp2),
                                  concat_vector(tmp3, tmp4));
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_unzip<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == 1)
            return unzip_serial(a, b, step);

        if (step == Elems)
            return std::make_pair(a, b);

        vector<int16, Elems / 2> tmp1, tmp2;

        std::tie(tmp1, tmp2) = interleave_bits_unzip<16, int16, Elems / 2>::run(vector_cast<int16>(a),
                                                                                vector_cast<int16>(b), step / 2);

        return std::make_pair(vector_cast<T>(tmp1), vector_cast<T>(tmp2));
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_unzip<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<16, T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        if (step > 1) {
            auto [tmp1, tmp2] = interleave_bits_unzip<32, int32, Elems / 2>::run(vector_cast<int32>(a),
                                                                                 vector_cast<int32>(b), step / 2);

            return std::make_pair(vector_cast<T>(tmp1), vector_cast<T>(tmp2));
        }

        // Half of the lanes are wasted for now
        if constexpr (Elems == 8) {
            vector<T, 32> tmp1, tmp2;

            tmp1 = ::select32(0xcccccccc, concat_vector(a, b, vector_type(), vector_type()), 0, 0x0c080400, 0x00000000, 0x0020,
                                                                                             0, 0x0e0a0602, 0x00000000, 0x2000);

            tmp2 = ::select32(0xcccccccc, concat_vector(a, b, vector_type(), vector_type()), 0, 0x0c080400, 0x00000000, 0x0031,
                                                                                             0, 0x0e0a0602, 0x00000000, 0x3100);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp2.template extract<Elems>(0));
        }
        else if constexpr (Elems == 16) {
            vector<T, 32> tmp1, tmp2;

            tmp1 = ::select32(0xcccccccc, concat_vector(a, b), 0, 0x0c080400, 0x00000000, 0x0020,
                                                               0, 0x0e0a0602, 0x00000000, 0x2000);

            tmp2 = ::select32(0xcccccccc, concat_vector(a, b), 0, 0x0c080400, 0x00000000, 0x0031,
                                                               0, 0x0e0a0602, 0x00000000, 0x3100);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp2.template extract<Elems>(0));
        }
        else if constexpr (Elems == 32) {
            vector<T, 32> tmp;
            vector_type ret1, ret2;

            tmp = ::select32(0xcccccccc, a,  0, 0x0c080400, 0x00000000, 0x0020,
                                             0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(0, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, b,  0, 0x0c080400, 0x00000000, 0x0020,
                                             0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(1, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, a,  0, 0x0c080400, 0x00000000, 0x0031,
                                             0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(0, tmp.template extract<16>(0));

            tmp = ::select32(0xccccccc,  b,  0, 0x0c080400, 0x00000000, 0x0031,
                                             0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(1, tmp.template extract<16>(0));

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 64) {
            vector<T, 32> tmp;
            vector_type ret1, ret2;

            tmp = ::select32(0xcccccccc, a.template extract<32>(0),  0, 0x0c080400, 0x00000000, 0x0020,
                                                                     0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(0, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, a.template extract<32>(1),  0, 0x0c080400, 0x00000000, 0x0020,
                                                                     0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(1, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, b.template extract<32>(0),  0, 0x0c080400, 0x00000000, 0x0020,
                                                                     0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(2, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, b.template extract<32>(1),  0, 0x0c080400, 0x00000000, 0x0020,
                                                                     0, 0x0e0a0602, 0x00000000, 0x2000);
            ret1.insert(3, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, a.template extract<32>(0),  0, 0x0c080400, 0x00000000, 0x0031,
                                                                     0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(0, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, a.template extract<32>(1),  0, 0x0c080400, 0x00000000, 0x0031,
                                                                     0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(1, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, b.template extract<32>(0),  0, 0x0c080400, 0x00000000, 0x0031,
                                                                     0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(2, tmp.template extract<16>(0));

            tmp = ::select32(0xcccccccc, b.template extract<32>(1),  0, 0x0c080400, 0x00000000, 0x0031,
                                                                     0, 0x0e0a0602, 0x00000000, 0x3100);
            ret2.insert(3, tmp.template extract<16>(0));

            return std::make_pair(ret1, ret2);
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_unzip<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 8) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle16(params...); };
            else                                  return [](auto &&...params) { return ::shuffle16(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect16(params...); };
            else                                  return [](auto &&...params) { return ::select16(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 4) {
            vector<T, 8> tmp1, tmp2;

            std::tie(tmp1, tmp2) = interleave_bits_unzip<32, T, 8>::run(concat_vector(a, b), vector<T, 8>(), step);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp2.template extract<Elems>(0));
        }
        else if constexpr (Elems == 8) {
            vector_type ret1, ret2;
            vector<T, 16> tmp = concat_vector(a, b);

            unsigned offset_lo;
            unsigned offset_hi;

            if (step == 4) {
                offset_lo = 0xba983210;
                offset_hi = 0xfedc7654;
            }
            else if (step == 2) {
                offset_lo = 0xdc985410;
                offset_hi = 0xfeba7632;
            }
            else if (step == 1) {
                offset_lo = 0xeca86420;
                offset_hi = 0xfdb97531;
            }

            tmp = op(tmp, 0, offset_lo, offset_hi);

            ret1 = tmp.template extract<8>(0);
            ret2 = tmp.template extract<8>(1);

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 16) {
            vector_type ret1, ret2;

            if (chess_manifest(step == 8)) {
                ret1 = concat_vector(a.template extract<8>(0), b.template extract<8>(0));
                ret2 = concat_vector(a.template extract<8>(1), b.template extract<8>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset_lo1, a_offset_lo2;
                unsigned a_offset_hi1, a_offset_hi2;
                unsigned b_offset_lo1, b_offset_lo2;
                unsigned b_offset_hi1, b_offset_hi2;

                if (step == 8) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                }
                else if (step == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  8,  9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  8,  9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31);
                }
                else if (step == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29);
                    select_mask2 = interleave_common_type::template compute_select_mask(2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  4,  5,  8,  9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(2,  3,  6,  7, 10, 11, 14, 15, 18, 19, 22, 23, 26, 27, 30, 31);
                }
                else if (step == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
                    select_mask2 = interleave_common_type::template compute_select_mask(1,  3,  5,  7,  9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(1,  3,  5,  7,  9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
                }

                ret1 = op(select_mask1, a, 0, a_offset_lo1, a_offset_hi1,
                                        b, 0, b_offset_lo1, b_offset_hi1);
                ret2 = op(select_mask2, a, 0, a_offset_lo2, a_offset_hi2,
                                        b, 0, b_offset_lo2, b_offset_hi2);
            }

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2, tmp3, tmp4;

            std::tie(tmp1, tmp2) = interleave_bits_unzip<32, T, 16>::run(a.template extract<16>(0), a.template extract<16>(1), step);
            std::tie(tmp3, tmp4) = interleave_bits_unzip<32, T, 16>::run(b.template extract<16>(0), b.template extract<16>(1), step);

            return std::make_pair(concat_vector(tmp1, tmp3),
                                  concat_vector(tmp2, tmp4));
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_unzip<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<64, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 4) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle8(params...); };
            else                                  return [](auto &&...params) { return ::shuffle8(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect8(params...); };
            else                                  return [](auto &&...params) { return ::select8(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned step)
    {
        if (step == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 2) {
            vector<T, 4> tmp1, tmp2;

            std::tie(tmp1, tmp2) = interleave_bits_unzip<64, T, 4>::run(concat_vector(a, b), vector<T, 4>(), step);

            return std::make_pair(tmp1.template extract<Elems>(0),
                                  tmp2.template extract<Elems>(0));
        }
        else if constexpr (Elems == 4) {
            vector_type ret1, ret2;
            vector<T, 8> tmp = concat_vector(a, b);

            unsigned offset;

            if (step == 2)
                offset = 0x76325410;
            else if (step == 1)
                offset = 0x75316420;

            tmp = op(tmp, 0, offset);

            ret1 = tmp.template extract<4>(0);
            ret2 = tmp.template extract<4>(1);

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 8) {
            vector_type ret1, ret2;


            if (chess_manifest(step == 4)) {
                ret1 = concat_vector(a.template extract<4>(0), b.template extract<4>(0));
                ret2 = concat_vector(a.template extract<4>(1), b.template extract<4>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset1, a_offset2;
                unsigned b_offset1, b_offset2;

                if (step == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  8,  9, 10, 11);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 12, 13, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  8,  9, 10, 11);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 12, 13, 14, 15);
                }
                else if (step == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  4,  5,  8,  9, 12, 13);
                    select_mask2 = interleave_common_type::template compute_select_mask(2,  3,  6,  7, 10, 11, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  4,  5,  8,  9, 12, 13);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(2,  3,  6,  7, 10, 11, 14, 15);
                }
                else if (step == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  2,  4,  6,  8, 10, 12, 14);
                    select_mask2 = interleave_common_type::template compute_select_mask(1,  3,  5,  7,  9, 11, 13, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  2,  4,  6,  8, 10, 12, 14);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(1,  3,  5,  7,  9, 11, 13, 15);
                }

                ret1 = op(select_mask1, a, 0, a_offset1, b, 0, b_offset1);
                ret2 = op(select_mask2, a, 0, a_offset2, b, 0, b_offset2);
            }

            return std::make_pair(ret1, ret2);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2, tmp3, tmp4;

            std::tie(tmp1, tmp2) = interleave_bits_unzip<64, T, 8>::run(a.template extract<8>(0), a.template extract<8>(1), step);
            std::tie(tmp3, tmp4) = interleave_bits_unzip<64, T, 8>::run(b.template extract<8>(0), b.template extract<8>(1), step);

            return std::make_pair(concat_vector(tmp1, tmp3),
                                  concat_vector(tmp2, tmp4));
        }
    }
};


template <typename T, unsigned Elems>
struct interleave_bits_butterfly<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 8) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle16(params...); };
            else                                  return [](auto &&...params) { return ::shuffle16(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect16(params...); };
            else                                  return [](auto &&...params) { return ::select16(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        //TODO: Look to do this with a single shuffle call (same for all variants lower two ELem number variants on butterfly, butterfly_half and crossover)
        if constexpr (Elems == 4) {
            auto [tmp1, tmp2] = interleave_bits_butterfly<32, T, 8>::run(concat_vector(a, b), concat_vector(b, a), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 8) {
            vector<T, 16> tmp = concat_vector(a, b);

            unsigned offset_lo;
            unsigned offset_hi;

            if (radix == 4) {
                offset_lo = 0xba983210;
                offset_hi = 0xfedc7654;
            }
            else if (radix == 2) {
                offset_lo = 0xdc549810;
                offset_hi = 0xfe76ba32;
            }
            else if (radix == 1) {
                offset_lo = 0xe6c4a280;
                offset_hi = 0xf7d5b391;
            }

            tmp = op(tmp, 0, offset_lo, offset_hi);

            return std::make_pair(tmp.template extract<8>(0), tmp.template extract<8>(1));
        }
        else if constexpr (Elems == 16) {
            std::pair<vector_type, vector_type> ret;

            if (chess_manifest(radix == 8)) {
                ret.first  = concat_vector(a.template extract<8>(0), b.template extract<8>(0));
                ret.second = concat_vector(a.template extract<8>(1), b.template extract<8>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset_lo1, a_offset_lo2;
                unsigned a_offset_hi1, a_offset_hi2;
                unsigned b_offset_lo1, b_offset_lo2;
                unsigned b_offset_hi1, b_offset_hi2;

                if (radix == 8) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  4,  5,  6,  7, 16, 17, 18, 19, 20, 21, 22, 23);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
                }
                else if (radix == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3, 16, 17, 18, 19,  8,  9, 10, 11, 24, 25, 26, 27);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 20, 21, 22, 23, 12, 13, 14, 15, 28, 29, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3, 16, 17, 18, 19,  8,  9, 10, 11, 24, 25, 26, 27);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 20, 21, 22, 23, 12, 13, 14, 15, 28, 29, 30, 31);
                }
                else if (radix == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1, 16, 17,  4,  5, 20, 21,  8,  9, 24, 25, 12, 13, 28, 29);
                    select_mask2 = interleave_common_type::template compute_select_mask(2,  3, 18, 19,  6,  7, 22, 23, 10, 11, 26, 27, 14, 15, 30, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1, 16, 17,  4,  5, 20, 21,  8,  9, 24, 25, 12, 13, 28, 29);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(2,  3, 18, 19,  6,  7, 22, 23, 10, 11, 26, 27, 14, 15, 30, 31);
                }
                else if (radix == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0, 16,  2, 18,  4, 20,  6, 22,  8, 24, 10, 26, 12, 28, 14, 30);
                    select_mask2 = interleave_common_type::template compute_select_mask(1, 17,  3, 19,  5, 21,  7, 23,  9, 25, 11, 27, 13, 29, 15, 31);
                    std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0, 16,  2, 18,  4, 20,  6, 22,  8, 24, 10, 26, 12, 28, 14, 30);
                    std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(1, 17,  3, 19,  5, 21,  7, 23,  9, 25, 11, 27, 13, 29, 15, 31);
                }

                ret.first  = op(select_mask1, a, 0, a_offset_lo1, a_offset_hi1,
                                              b, 0, b_offset_lo1, b_offset_hi1);
                ret.second = op(select_mask2, a, 0, a_offset_lo2, a_offset_hi2,
                                              b, 0, b_offset_lo2, b_offset_hi2);
            }

            return ret;
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2, tmp3, tmp4;

            if (radix == 16) {
                return std::make_pair(concat_vector(a.template extract<16>(0), b.template extract<16>(0)),
                                      concat_vector(a.template extract<16>(1), b.template extract<16>(1)));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_butterfly<32, T, 16>::run(a.template extract<16>(0), b.template extract<16>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_butterfly<32, T, 16>::run(a.template extract<16>(1), b.template extract<16>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_butterfly<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<64, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 4) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle8(params...); };
            else                                  return [](auto &&...params) { return ::shuffle8(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect8(params...); };
            else                                  return [](auto &&...params) { return ::select8(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == Elems)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 2) {
            const auto [tmp1, tmp2] = interleave_bits_butterfly<64, T, 4>::run(concat_vector(a, b), concat_vector(b, a), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> tmp = concat_vector(a, b);

            unsigned offset;

            if (radix == 2)
                offset = 0x76325410;
            else if (radix == 1)
                offset = 0x73516240;

            tmp = op(tmp, 0, offset);

            return std::make_pair(tmp.template extract<4>(0), tmp.template extract<4>(1));
        }
        else if constexpr (Elems == 8) {
            std::pair<vector_type, vector_type> ret;

            if (chess_manifest(radix == 4)) {
                ret.first  = concat_vector(a.template extract<4>(0), b.template extract<4>(0));
                ret.second = concat_vector(a.template extract<4>(1), b.template extract<4>(1));
            }
            else {
                unsigned select_mask1;
                unsigned select_mask2;
                unsigned a_offset1, a_offset2;
                unsigned b_offset1, b_offset2;

                if (radix == 4) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  8,  9, 10, 11);
                    select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 12, 13, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  8,  9, 10, 11);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 12, 13, 14, 15);
                }
                else if (radix == 2) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  8,  9,  4,  5, 12, 13);
                    select_mask2 = interleave_common_type::template compute_select_mask(2,  3, 10, 11,  6,  7, 14, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  8,  9,  4,  5, 12, 13);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(2,  3, 10, 11,  6,  7, 14, 15);
                }
                else if (radix == 1) {
                    select_mask1 = interleave_common_type::template compute_select_mask(0,  8,  2, 10,  4, 12,  6, 14);
                    select_mask2 = interleave_common_type::template compute_select_mask(1,  9,  3, 11,  5, 13,  7, 15);
                    std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  8,  2, 10,  4, 12,  6, 14);
                    std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(1,  9,  3, 11,  5, 13,  7, 15);
                }

                ret.first  = op(select_mask1, a, 0, a_offset1, b, 0, b_offset1);
                ret.second = op(select_mask2, a, 0, a_offset2, b, 0, b_offset2);
            }

            return ret;
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2, tmp3, tmp4;

            if (radix == 8) {
                return std::make_pair(concat_vector(a.template extract<8>(0), b.template extract<8>(0)),
                                      concat_vector(a.template extract<8>(1), b.template extract<8>(1)));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_butterfly<64, T, 8>::run(a.template extract<8>(0), b.template extract<8>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_butterfly<64, T, 8>::run(a.template extract<8>(1), b.template extract<8>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_butterfly_half<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 8) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle16(params...); };
            else                                  return [](auto &&...params) { return ::shuffle16(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect16(params...); };
            else                                  return [](auto &&...params) { return ::select16(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == Elems)
            return std::make_pair(a, reverse<T, Elems>::run(b));

        constexpr auto op = get_op();

        if constexpr (Elems == 4) {
            auto [tmp1, tmp2] = interleave_bits_butterfly_half<32, T, 8>::run(concat_vector(a, reverse<T, Elems>::run(b)), concat_vector(b, a), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 8) {
            vector<T, 16> tmp = concat_vector(a, b);

            unsigned offset_lo;
            unsigned offset_hi;

            if (radix == 4) {
                offset_lo = 0xcdef3210;
                offset_hi = 0x89ab7654;
            }
            else if (radix == 2) {
                offset_lo = 0xef54ab10;
                offset_hi = 0xcd768932;
            }
            else if (radix == 1) {
                offset_lo = 0xf6d4b290;
                offset_hi = 0xe7c5a381;
            }

            tmp = op(tmp, 0, offset_lo, offset_hi);

            return std::make_pair(tmp.template extract<8>(0), tmp.template extract<8>(1));
        }
        else if constexpr (Elems == 16) {
            std::pair<vector_type, vector_type> ret;

            unsigned select_mask1;
            unsigned select_mask2;
            unsigned a_offset_lo1, a_offset_lo2;
            unsigned a_offset_hi1, a_offset_hi2;
            unsigned b_offset_lo1, b_offset_lo2;
            unsigned b_offset_hi1, b_offset_hi2;

            if (radix == 8) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3,  4,  5,  6,  7, 31, 30, 29, 28, 27, 26, 25, 24);
                select_mask2 = interleave_common_type::template compute_select_mask(8,  9, 10, 11, 12, 13, 14, 15, 23, 22, 21, 20, 19, 18, 17, 16);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3,  4,  5,  6,  7, 31, 30, 29, 28, 27, 26, 25, 24);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(8,  9, 10, 11, 12, 13, 14, 15, 23, 22, 21, 20, 19, 18, 17, 16);
            }
            else if (radix == 4) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3, 23, 22, 21, 20,  8,  9, 10, 11, 31, 30, 29, 28);
                select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 19, 18, 17, 16, 12, 13, 14, 15, 27, 26, 25, 24);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1,  2,  3, 23, 22, 21, 20,  8,  9, 10, 11, 31, 30, 29, 28);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 19, 18, 17, 16, 12, 13, 14, 15, 27, 26, 25, 24);
            }
            else if (radix == 2) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  1, 19, 18,  4,  5, 23, 22,  8,  9, 27, 26, 12, 13, 31, 30);
                select_mask2 = interleave_common_type::template compute_select_mask(2,  3, 17, 16,  6,  7, 21, 20, 10, 11, 25, 24, 14, 15, 29, 28);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0,  1, 19, 18,  4,  5, 23, 22,  8,  9, 27, 26, 12, 13, 31, 30);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(2,  3, 17, 16,  6,  7, 21, 20, 10, 11, 25, 24, 14, 15, 29, 28);
            }
            else if (radix == 1) {
                select_mask1 = interleave_common_type::template compute_select_mask(0, 17,  2, 19,  4, 21,  6, 23,  8, 25, 10, 27, 12, 29, 14, 31);
                select_mask2 = interleave_common_type::template compute_select_mask(1, 16,  3, 18,  5, 20,  7, 22,  9, 24, 11, 26, 13, 28, 15, 30);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets(0, 17,  2, 19,  4, 21,  6, 23,  8, 25, 10, 27, 12, 29, 14, 31);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(1, 16,  3, 18,  5, 20,  7, 22,  9, 24, 11, 26, 13, 28, 15, 30);
            }

            ret.first  = op(select_mask1, a, 0, a_offset_lo1, a_offset_hi1,
                                          b, 0, b_offset_lo1, b_offset_hi1);
            ret.second = op(select_mask2, a, 0, a_offset_lo2, a_offset_hi2,
                                          b, 0, b_offset_lo2, b_offset_hi2);

            return ret;
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2, tmp3, tmp4;

            if (radix == 16) {
                return std::make_pair(concat_vector(a.template extract<16>(0), reverse<T, 16>::run(b.template extract<16>(1))),
                                      concat_vector(a.template extract<16>(1), reverse<T, 16>::run(b.template extract<16>(0))));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_butterfly_half<32, T, 16>::run(a.template extract<16>(0), b.template extract<16>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_butterfly_half<32, T, 16>::run(a.template extract<16>(1), b.template extract<16>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_butterfly_half<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<64, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 4) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle8(params...); };
            else                                  return [](auto &&...params) { return ::shuffle8(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect8(params...); };
            else                                  return [](auto &&...params) { return ::select8(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == Elems)
            return std::make_pair(a, reverse<T, Elems>::run(b));

        constexpr auto op = get_op();

        if constexpr (Elems == 2) {
            auto [tmp1, tmp2] = interleave_bits_butterfly_half<64, T, 4>::run(concat_vector(a, reverse<T, Elems>::run(b)), concat_vector(b, a), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> tmp = concat_vector(a, b);

            unsigned offset;

            if (radix == 2)
                offset = 0x45326710;
            else if (radix == 1)
                offset = 0x63417250;

            tmp = op(tmp, 0, offset);

            return std::make_pair(tmp.template extract<4>(0), tmp.template extract<4>(1));
        }
        else if constexpr (Elems == 8) {
            std::pair<vector_type, vector_type> ret;

            unsigned select_mask1;
            unsigned select_mask2;
            unsigned a_offset1, a_offset2;
            unsigned b_offset1, b_offset2;

            if (radix == 4) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  1,  2,  3, 15, 14, 13, 12);
                select_mask2 = interleave_common_type::template compute_select_mask(4,  5,  6,  7, 11, 10,  9,  8);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1,  2,  3, 15, 14, 13, 12);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(4,  5,  6,  7, 11, 10,  9,  8);
            }
            else if (radix == 2) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  1, 11, 10,  4,  5, 15, 14);
                select_mask2 = interleave_common_type::template compute_select_mask(2,  3,  9,  8,  6,  7, 13, 12);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  1, 11, 10,  4,  5, 15, 14);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(2,  3,  9,  8,  6,  7, 13, 12);

            }
            else if (radix == 1) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  9,  2, 11,  4, 13,  6, 15);
                select_mask2 = interleave_common_type::template compute_select_mask(1,  8,  3, 10,  5, 12,  7, 14);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  9,  2, 11,  4, 13,  6, 15);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(1,  8,  3, 10,  5, 12,  7, 14);
            }

            ret.first  = op(select_mask1, a, 0, a_offset1, b, 0, b_offset1);
            ret.second = op(select_mask2, a, 0, a_offset2, b, 0, b_offset2);

            return ret;
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2, tmp3, tmp4;

            if (radix == 8) {
                return std::make_pair(concat_vector(a.template extract<8>(0), reverse<T, 8>::run(b.template extract<8>(1))),
                                      concat_vector(a.template extract<8>(1), reverse<T, 8>::run(b.template extract<8>(0))));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_butterfly_half<64, T, 8>::run(a.template extract<8>(0), b.template extract<8>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_butterfly_half<64, T, 8>::run(a.template extract<8>(1), b.template extract<8>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_crossover<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 8) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle16(params...); };
            else                                  return [](auto &&...params) { return ::shuffle16(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect16(params...); };
            else                                  return [](auto &&...params) { return ::select16(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == 1)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 4) {
            auto [tmp1, tmp2] = interleave_bits_crossover<32, T, 8>::run(concat_vector(a, a), concat_vector(b, b), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 8) {
            vector<T, 16> tmp = concat_vector(a, b);

            unsigned offset_lo;
            unsigned offset_hi;

            if (radix == 8) {
                offset_lo = 0xb3a29180;
                offset_hi = 0x4c5d6e7f;
            }
            else if (radix == 4) {
                offset_lo = 0xd5c49180;
                offset_hi = 0x6e7f2a3b;
            }
            else if (radix == 2) {
                offset_lo = 0xe6c4a280;
                offset_hi = 0x7f5d3b19;
            }

            tmp = op(tmp, 0, offset_lo, offset_hi);

            return std::make_pair(tmp.template extract<8>(0), tmp.template extract<8>(1));
        }
        else if constexpr (Elems == 16) {
            std::pair<vector_type, vector_type> ret;

            unsigned select_mask1;
            unsigned select_mask2;
            unsigned a_offset_lo1, a_offset_lo2;
            unsigned a_offset_hi1, a_offset_hi2;
            unsigned b_offset_lo1, b_offset_lo2;
            unsigned b_offset_hi1, b_offset_hi2;

            if (radix == 16) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23);
                select_mask2 = interleave_common_type::template compute_select_mask(31, 15, 30, 14, 29, 13, 28, 12, 27, 11, 26, 10, 25,  9, 24,  8);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(31, 15, 30, 14, 29, 13, 28, 12, 27, 11, 26, 10, 25,  9, 24,  8);
            }
            else if (radix == 8) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0, 16,  1, 17,  2, 18,  3, 19,  8, 24,  9, 25, 10, 26, 11, 27);
                select_mask2 = interleave_common_type::template compute_select_mask(23,  7, 22,  6, 21,  5, 20,  4, 31, 15, 30, 14, 29, 13, 28, 12);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets( 0, 16,  1, 17,  2, 18,  3, 19,  8, 24,  9, 25, 10, 26, 11, 27);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(23,  7, 22,  6, 21,  5, 20,  4, 31, 15, 30, 14, 29, 13, 28, 12);
            }
            else if (radix == 4) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0, 16,  1, 17,  4, 20,  5, 21,  8, 24,  9, 25, 12, 28, 13, 29);
                select_mask2 = interleave_common_type::template compute_select_mask(19,  3, 18,  2, 23,  7, 22,  6, 27, 11, 26, 10, 31, 15, 30, 14);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets( 0, 16,  1, 17,  4, 20,  5, 21,  8, 24,  9, 25, 12, 28, 13, 29);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(19,  3, 18,  2, 23,  7, 22,  6, 27, 11, 26, 10, 31, 15, 30, 14);
            }
            else if (radix == 2) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0, 16,  2, 18,  4, 20,  6, 22,  8, 24, 10, 26, 12, 28, 14, 30);
                select_mask2 = interleave_common_type::template compute_select_mask(17,  1, 19,  3, 21,  5, 23,  7, 25,  9, 27, 11, 29, 13, 31, 15);
                std::tie(a_offset_lo1, a_offset_hi1, b_offset_lo1, b_offset_hi1) = interleave_common_type::template compute_offsets( 0, 16,  2, 18,  4, 20,  6, 22,  8, 24, 10, 26, 12, 28, 14, 30);
                std::tie(a_offset_lo2, a_offset_hi2, b_offset_lo2, b_offset_hi2) = interleave_common_type::template compute_offsets(17,  1, 19,  3, 21,  5, 23,  7, 25,  9, 27, 11, 29, 13, 31, 15);
            }

            ret.first  = op(select_mask1, a, 0, a_offset_lo1, a_offset_hi1,
                                          b, 0, b_offset_lo1, b_offset_hi1);
            ret.second = op(select_mask2, a, 0, a_offset_lo2, a_offset_hi2,
                                          b, 0, b_offset_lo2, b_offset_hi2);

            return ret;
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1, tmp2, tmp3, tmp4;

            if (radix == 32) {
                std::tie(tmp1, tmp2) = interleave_bits_crossover<32, T, 16>::run(a.template extract<16>(0), b.template extract<16>(0), 16);
                std::tie(tmp3, tmp4) = interleave_bits_crossover<32, T, 16>::run(a.template extract<16>(1), b.template extract<16>(1), 16);

                return std::make_pair(concat_vector(tmp1, reverse<T, 16>::run(tmp2)), concat_vector(tmp4, reverse<T, 16>::run(tmp3)));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_crossover<32, T, 16>::run(a.template extract<16>(0), b.template extract<16>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_crossover<32, T, 16>::run(a.template extract<16>(1), b.template extract<16>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T, unsigned Elems>
struct interleave_bits_crossover<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<64, T, Elems>;

    static constexpr auto get_op()
    {
        if constexpr (Elems <= 4) {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpshuffle8(params...); };
            else                                  return [](auto &&...params) { return ::shuffle8(params...); };
        }
        else {
            if constexpr (is_floating_point_v<T>) return [](auto &&...params) { return ::fpselect8(params...); };
            else                                  return [](auto &&...params) { return ::select8(params...); };
        }
    }

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, unsigned radix)
    {
        if (radix == 1)
            return std::make_pair(a, b);

        constexpr auto op = get_op();

        if constexpr (Elems == 2) {
            auto [tmp1, tmp2] = interleave_bits_crossover<64, T, 4>::run(concat_vector(a, a), concat_vector(b, b), radix);

            return { tmp1.template extract<Elems>(0), tmp2.template extract<Elems>(0) };
        }
        else if constexpr (Elems == 4) {
            vector<T, 8> tmp = concat_vector(a, b);

            unsigned offset;

            if (radix == 4)
                offset = 0x26375140;
            else if (radix == 2)
                offset = 0x37156240;

            tmp = op(tmp, 0, offset);

            return std::make_pair(tmp.template extract<4>(0), tmp.template extract<4>(1));
        }
        else if constexpr (Elems == 8) {
            std::pair<vector_type, vector_type> ret;

            unsigned select_mask1;
            unsigned select_mask2;
            unsigned a_offset1, a_offset2;
            unsigned b_offset1, b_offset2;

            if (radix == 8) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0,  8,  1,  9,  2, 10,  3, 11);
                select_mask2 = interleave_common_type::template compute_select_mask(15,  7, 14,  6, 13,  5, 12,  4);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets( 0,  8,  1,  9,  2, 10,  3, 11);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(15,  7, 14,  6, 13,  5, 12,  4);
            }
            else if (radix == 4) {
                select_mask1 = interleave_common_type::template compute_select_mask( 0,  8,  1,  9,  4, 12,  5, 13);
                select_mask2 = interleave_common_type::template compute_select_mask(11,  3, 10,  2, 15,  7, 14,  6);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets( 0,  8,  1,  9,  4, 12,  5, 13);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(11,  3, 10,  2, 15,  7, 14,  6);
            }
            else if (radix == 2) {
                select_mask1 = interleave_common_type::template compute_select_mask(0,  8,  2, 10,  4, 12,  6, 14);
                select_mask2 = interleave_common_type::template compute_select_mask(9,  1, 11,  3, 13,  5, 15,  7);
                std::tie(a_offset1, b_offset1) = interleave_common_type::template compute_offsets(0,  8,  2, 10,  4, 12,  6, 14);
                std::tie(a_offset2, b_offset2) = interleave_common_type::template compute_offsets(9,  1, 11,  3, 13,  5, 15,  7);
            }

            ret.first  = op(select_mask1, a, 0, a_offset1, b, 0, b_offset1);
            ret.second = op(select_mask2, a, 0, a_offset2, b, 0, b_offset2);

            return ret;
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1, tmp2, tmp3, tmp4;

            if (radix == 16) {
                std::tie(tmp1, tmp2) = interleave_bits_crossover<64, T, 8>::run(a.template extract<8>(0), b.template extract<8>(0), 8);
                std::tie(tmp3, tmp4) = interleave_bits_crossover<64, T, 8>::run(a.template extract<8>(1), b.template extract<8>(1), 8);

                return std::make_pair(concat_vector(tmp1, reverse<T, 8>::run(tmp2)), concat_vector(tmp4, reverse<T, 8>::run(tmp3)));
            }
            else {
                std::tie(tmp1, tmp2) = interleave_bits_crossover<64, T, 8>::run(a.template extract<8>(0), b.template extract<8>(0), radix);
                std::tie(tmp3, tmp4) = interleave_bits_crossover<64, T, 8>::run(a.template extract<8>(1), b.template extract<8>(1), radix);

                return std::make_pair(concat_vector(tmp1, tmp3), concat_vector(tmp2, tmp4));
            }
        }
    }
};

template <typename T,
          unsigned S0,  unsigned S1,  unsigned S2,  unsigned S3,  unsigned S4,  unsigned S5,  unsigned S6,  unsigned S7,
          unsigned S8,  unsigned S9,  unsigned S10, unsigned S11, unsigned S12, unsigned S13, unsigned S14, unsigned S15,
          unsigned S16, unsigned S17, unsigned S18, unsigned S19, unsigned S20, unsigned S21, unsigned S22, unsigned S23,
          unsigned S24, unsigned S25, unsigned S26, unsigned S27, unsigned S28, unsigned S29, unsigned S30, unsigned S31>
struct interleave_bits_custom_static<32, T, 16,
                                S0,  S1,  S2,  S3,  S4,  S5,  S6,  S7,
                                S8,  S9,  S10, S11, S12, S13, S14, S15,
                                S16, S17, S18, S19, S20, S21, S22, S23,
                                S24, S25, S26, S27, S28, S29, S30, S31>
{
    static constexpr unsigned Elems = 16;

    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b)
    {
        std::pair<vector_type, vector_type> ret;

        ret.first  = interleave_common_type::template run(a, b, S0,  S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9,  S10, S11, S12, S13, S14, S15);
        ret.second = interleave_common_type::template run(a, b, S16, S17, S18, S19, S20, S21, S22, S23, S24, S25, S26, S27, S28, S29, S30, S31);

        return ret;
    }
};

template <typename T>
struct interleave_bits_custom<32, T, 16>
{
    static constexpr unsigned Elems = 16;

    using vector_type = vector<T, Elems>;

    using interleave_common_type = interleave_common_helper<32, T, Elems>;

    template <typename... Select>
    static std::pair<vector_type, vector_type> run(const vector_type &a, const vector_type &b, Select... select)
    {
        std::pair<vector_type, vector_type> ret;

        static_assert(sizeof...(Select) == Elems);

        const auto select_tuple = std::make_tuple(select...);

        ret.first  = interleave_common_type::template run(a, b, std::get<0>(select_tuple),  std::get<1>(select_tuple),
                                                                std::get<2>(select_tuple),  std::get<3>(select_tuple),
                                                                std::get<4>(select_tuple),  std::get<5>(select_tuple),
                                                                std::get<6>(select_tuple),  std::get<7>(select_tuple),
                                                                std::get<8>(select_tuple),  std::get<9>(select_tuple),
                                                                std::get<10>(select_tuple), std::get<11>(select_tuple),
                                                                std::get<12>(select_tuple), std::get<13>(select_tuple),
                                                                std::get<14>(select_tuple), std::get<15>(select_tuple));

        ret.second = interleave_common_type::template run(a, b, std::get<16>(select_tuple), std::get<17>(select_tuple),
                                                                std::get<18>(select_tuple), std::get<19>(select_tuple),
                                                                std::get<20>(select_tuple), std::get<21>(select_tuple),
                                                                std::get<22>(select_tuple), std::get<23>(select_tuple),
                                                                std::get<24>(select_tuple), std::get<25>(select_tuple),
                                                                std::get<26>(select_tuple), std::get<27>(select_tuple),
                                                                std::get<28>(select_tuple), std::get<29>(select_tuple),
                                                                std::get<30>(select_tuple), std::get<31>(select_tuple));

        return ret;
    }
};

}

#endif
