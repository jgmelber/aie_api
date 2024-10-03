// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SHUFFLE__HPP__
#define __AIE_API_DETAIL_AIE1_SHUFFLE__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct shuffle_up_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_up_bits_impl<16, int16, Elems>::run(v.unpack(), n).template pack<T>();
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (n % 4 == 0) return vector_cast<T>(shuffle_up_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_up_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), n / 2));

            return shuffle_up_bits_impl_scalar<8, T, Elems>::run(v, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_up_fill<int16, Elems>::run(v.unpack(), fill.unpack(), n).template pack<T>();
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (n % 4 == 0) return vector_cast<T>(shuffle_up_fill_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), vector_cast<int32>(fill), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_up_fill_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), vector_cast<int16>(fill), n / 2));

            return shuffle_up_fill_bits_impl_scalar<8, T, Elems>::run(v, fill, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_up_rotate_bits_impl<16, int16, Elems>::run(v.unpack(), n).template pack<T>();
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (n % 4 == 0) return vector_cast<T>(shuffle_up_rotate_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_up_rotate_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), n / 2));

            return shuffle_up_rotate_bits_impl_scalar<8, T, Elems>::run(v, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_down_bits_impl<16, int16, Elems>::run(v.unpack(), n).template pack<T>();
        }
        else {
            if (n % 4 == 0) return vector_cast<T>(shuffle_down_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_down_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), n / 2));

            return shuffle_down_bits_impl_scalar<8, T, Elems>::run(v, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_down_fill<int16, Elems>::run(v.unpack(), fill.unpack(), n).template pack<T>();
        }
        else if constexpr (vector_type::bits() == 1024) {
            if (n % 4 == 0) return vector_cast<T>(shuffle_down_fill_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), vector_cast<int32>(fill), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_down_fill_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), vector_cast<int16>(fill), n / 2));

            return shuffle_down_fill_bits_impl_scalar<8, T, Elems>::run(v, fill, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if constexpr (vector_type::bits() <= 512) {
            return shuffle_down_rotate_bits_impl<16, int16, Elems>::run(v.unpack(), n).template pack<T>();
        }
        else {
            if (n % 4 == 0) return vector_cast<T>(shuffle_down_rotate_bits_impl<32, int32, Elems / 4>::run(vector_cast<int32>(v), n / 4));
            if (n % 2 == 0) return vector_cast<T>(shuffle_down_rotate_bits_impl<16, int16, Elems / 2>::run(vector_cast<int16>(v), n / 2));

            return shuffle_down_rotate_bits_impl_scalar<8, T, Elems>::run(v, n);
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_rotate_bits_impl<16, T, Elems>::run(v, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if (n % 2 == 0) return vector_cast<T>(shuffle_up_fill_bits_impl<32, int32, Elems / 2>::run(vector_cast<int32>(v), vector_cast<int32>(fill), n / 2));

        if constexpr (Elems <= 32)
            return shuffle_up_rotate_bits_impl<16, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        else if constexpr (Elems == 64)
            return shuffle_up_fill_bits_impl_scalar<16, T, Elems>::run(v, fill, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_rotate_bits_impl<16, T, Elems>::run(v, Elems - n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_down_rotate_bits_impl<16, T, Elems>::run(v, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if (n % 2 == 0) return vector_cast<T>(shuffle_down_fill_bits_impl<32, int32, Elems / 2>::run(vector_cast<int32>(v), vector_cast<int32>(fill), n / 2));

        if constexpr (vector_type::bits() <= 512)
            return shuffle_down_rotate_bits_impl<16, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        else if constexpr (vector_type::bits() == 1024)
            return shuffle_down_fill_bits_impl_scalar<16, T, Elems>::run(v, fill, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        const unsigned odd = n & 0x1;

        // This implementation relies on start being automatically rounded to two by the HW
        if constexpr (vector_type::bits() == 1024) {
            vector_type ret;

            if (chess_manifest(odd == 0)) {
                const vector<T, 32> tmp1 = ::shuffle32(v,      n, 0x06040200, 0x0e0c0a08, 0x3210); ret.insert(0, tmp1);
                const vector<T, 32> tmp2 = ::shuffle32(v, 32 + n, 0x06040200, 0x0e0c0a08, 0x3210); ret.insert(1, tmp2);

                return ret;
            }
            else {
                const unsigned mask = 0x88888888 * odd;
                const unsigned square = odd? 0x0321 : 0x3210;

                const vector<T, 32> tmp1 = ::select32(mask, v, n,     0x06040200, 0x0e0c0a08, square,
                                                               n + 2, 0x06040200, 0x0e0c0a08, 0x2000);
                ret.insert(0, tmp1);
                const vector<T, 32> tmp2 = ::select32(mask, v, n + 32,      0x06040200, 0x0e0c0a08, square,
                                                               n + 32 + 2, 0x06040200, 0x0e0c0a08, 0x2000);
                ret.insert(1, tmp2);

                return ret;
            }
        }
        else {
            if (chess_manifest(odd == 0)) {
                const vector<T, 32> tmp = ::shuffle32(v.template grow_replicate<32>(),  n, 0x06040200, 0x0e0c0a08, 0x3210);

                return tmp.template extract<Elems>(0);
            }
            else {
                const unsigned mask = 0x88888888 * odd;
                const unsigned square = odd? 0x0321 : 0x3210;

                const vector<T, 32> tmp = ::select32(mask, v.template grow_replicate<32>(), n,     0x06040200, 0x0e0c0a08, square,
                                                                                            n + 2, 0x06040200, 0x0e0c0a08, 0x2000);

                return tmp.template extract<Elems>(0);
            }
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 8) {
            const vector<T, 16> tmp = op(v.template grow<16>(), -n, 0x76543210, 0x00000000);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 16> tmp = op(v,      -n, 0x76543210, 0xfedcba98);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<T, 16> tmp1 = op(v,     -n, 0x76543210, 0xfedcba98);
            const vector<T, 16> tmp2 = op(v, 16 - n, 0x76543210, 0xfedcba98);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    static constexpr auto get_select_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpselect16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::select16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (Elems <= 8) {
            return shuffle_up_rotate_bits_impl<32, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        }
        else if constexpr (Elems <= 16) {
            constexpr auto op = get_select_op();

            const vector<T, 16> ret = op(0xffff >> (Elems - n),    v,        -n, 0x76543210, 0xfedcba98,
                                                                fill, Elems - n, 0x76543210, 0xfedcba98);
            return ret;
        }
        else if constexpr (Elems == 32) {
            constexpr auto shuffle_op = get_shuffle_op();
            constexpr auto  select_op = get_select_op();

            vector_type ret;

            if (n < Elems / 2) {
                vector<T, 16> tmp = select_op(0xffff >> (Elems / 2 - n),    v.template extract<Elems / 2>(0),            -n, 0x76543210, 0xfedcba98,
                                                                         fill.template extract<Elems / 2>(1), Elems / 2 - n, 0x76543210, 0xfedcba98);

                ret.insert(0, tmp);

                tmp               = shuffle_op(v, Elems / 2 - n, 0x76543210, 0xfedcba98);

                ret.insert(1, tmp);

                return ret;
            }
            else {
                vector<T, 16> tmp = shuffle_op(fill, Elems - n, 0x76543210, 0xfedcba98);

                ret.insert(0, tmp);

                tmp               = select_op(0xffff >> (Elems - n),    v.template extract<Elems / 2>(0), Elems / 2 - n, 0x76543210, 0xfedcba98,
                                                                     fill.template extract<Elems / 2>(1), Elems - n,     0x76543210, 0xfedcba98);

                ret.insert(1, tmp);

                return ret;
            }
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned compute_offsets(unsigned start, unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = start; i < start + 8; ++i) {
            int src = int(i) - n;

            if (src < 0)
                src += Elems;

            ret |= src << (4 * i - 4 * start);
        }

        return ret;
    }

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 8) {
            const vector<T, 16> tmp = op(v.template grow<16>(), 0, compute_offsets(0, n), 0);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 16> tmp = op(v,      -n, 0x76543210, 0xfedcba98);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<T, 16> tmp1 = op(v,     -n, 0x76543210, 0xfedcba98);
            const vector<T, 16> tmp2 = op(v, 16 - n, 0x76543210, 0xfedcba98);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 8) {
            const vector<T, 16> tmp = op(v.template grow<16>(), n, 0x76543210, 0x00000000);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 16> tmp = op(v,      n, 0x76543210, 0xfedcba98);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<T, 16> tmp1 = op(v,      n, 0x76543210, 0xfedcba98);
            const vector<T, 16> tmp2 = op(v, 16 + n, 0x76543210, 0xfedcba98);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    static constexpr auto get_select_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpselect16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::select16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (Elems <= 8) {
            return shuffle_down_rotate_bits_impl<32, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            constexpr auto op = get_select_op();

            const vector<T, 16> ret = op(0xffff << (Elems - n),    v,         n, 0x76543210, 0xfedcba98,
                                                                fill, n - Elems, 0x76543210, 0xfedcba98);
            return ret;
        }
        else if constexpr (Elems == 32) {
            constexpr auto shuffle_op = get_shuffle_op();
            constexpr auto  select_op = get_select_op();

            vector_type ret;

            if (n < Elems / 2) {
                vector<T, 16> tmp = shuffle_op(v, n, 0x76543210, 0xfedcba98);

                ret.insert(0, tmp);

                tmp               = select_op(0xffff << (Elems / 2 - n),    v.template extract<Elems / 2>(1),         n, 0x76543210, 0xfedcba98,
                                                                         fill.template extract<Elems / 2>(0), n - Elems, 0x76543210, 0xfedcba98);

                ret.insert(1, tmp);

                return ret;
            }
            else {
                vector<T, 16> tmp;

                tmp               = select_op(0xffff << (Elems - n),    v.template extract<Elems / 2>(1),  n - Elems / 2, 0x76543210, 0xfedcba98,
                                                                     fill.template extract<Elems / 2>(0),  n - Elems / 2, 0x76543210, 0xfedcba98);

                ret.insert(0, tmp);

                tmp               = shuffle_op(fill, n - Elems / 2, 0x76543210, 0xfedcba98);

                ret.insert(1, tmp);

                return ret;
            }
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_rotate_bits_impl<32, T, Elems>::run(v, Elems - n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 4) {
            const vector<T, 8> tmp = op(v.template grow<8>(), -n, 0x3210);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> tmp = op(v, -n, 0x76543210);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = op(v,    -n, 0x76543210);
            const vector<T, 8> tmp2 = op(v, 8 - n, 0x76543210);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_fill_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = 0; i < 8; ++i) {
            int src = int(i) - n;

            if (src < 0)
                src += Elems;

            ret |= src << (4 * i);
        }

        return ret;
    }

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    static constexpr auto get_select_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpselect8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::select8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (Elems <= 4) {
            return shuffle_up_rotate_bits_impl<64, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        }
        else if constexpr (Elems <= 8) {
            constexpr auto op = get_select_op();

            const vector<T, 8> ret = op(0xff >> (Elems - n),    v,        -n, 0x76543210,
                                                             fill, Elems - n, 0x76543210);
            return ret;
        }
        else if constexpr (Elems == 16) {
            constexpr auto shuffle_op = get_shuffle_op();
            constexpr auto  select_op = get_select_op();

            vector_type ret;

            if (n < Elems / 2) {
                vector<T, 8> tmp = select_op(0xff >> (Elems / 2 - n),    v.template extract<Elems / 2>(0),            -n, 0x76543210,
                                                                      fill.template extract<Elems / 2>(1), Elems / 2 - n, 0x76543210);

                ret.insert(0, tmp);

                tmp              = shuffle_op(v, Elems / 2 - n, 0x76543210);

                ret.insert(1, tmp);

                return ret;
            }
            else {
                vector<T, 8> tmp = shuffle_op(fill, Elems - n, 0x76543210);

                ret.insert(0, tmp);

                tmp              = select_op(0xff >> (Elems - n),    v.template extract<Elems / 2>(0), Elems / 2 - n, 0x76543210,
                                                                  fill.template extract<Elems / 2>(1), Elems - n,     0x76543210);

                ret.insert(1, tmp);

                return ret;
            }
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_rotate_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = 0; i < 8; ++i) {
            int src = int(i) - n;

            if (src < 0)
                src += Elems;

            ret |= src << (4 * i);
        }

        return ret;
    }

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 4) {
            const vector<T, 8> tmp = op(v.template grow<8>(), 0, compute_offsets(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> tmp = op(v, -n, 0x76543210);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = op(v,    -n, 0x76543210);
            const vector<T, 8> tmp2 = op(v, 8 - n, 0x76543210);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_shuffle_op();

        if constexpr (Elems <= 4) {
            const vector<T, 8> tmp = op(v.template grow<8>(), n, 0x3210);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> tmp = op(v, n, 0x76543210);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = op(v,     n, 0x76543210);
            const vector<T, 8> tmp2 = op(v, 8 + n, 0x76543210);

            ret.insert(0, tmp1.template extract<Elems / 2>(0));
            ret.insert(1, tmp2.template extract<Elems / 2>(0));
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_fill_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr auto get_shuffle_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    static constexpr auto get_select_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpselect8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::select8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, const vector_type &fill, unsigned n)
    {
        if constexpr (Elems <= 4) {
            return shuffle_down_rotate_bits_impl<64, T, Elems * 2>::run(concat_vector(v, fill), n).template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            constexpr auto op = get_select_op();

            const vector<T, 8> ret = op(0xff << (Elems - n),    v,         n, 0x76543210,
                                                             fill, n - Elems, 0x76543210);
            return ret;
        }
        else if constexpr (Elems == 16) {
            constexpr auto shuffle_op = get_shuffle_op();
            constexpr auto  select_op = get_select_op();

            vector_type ret;

            if (n < Elems / 2) {
                vector<T, 8> tmp = shuffle_op(v, n, 0x76543210);

                ret.insert(0, tmp);

                tmp               = select_op(0xff << (Elems / 2 - n),    v.template extract<Elems / 2>(1),         n, 0x76543210,
                                                                       fill.template extract<Elems / 2>(0), n - Elems, 0x76543210);

                ret.insert(1, tmp);

                return ret;
            }
            else {
                vector<T, 8> tmp;

                tmp               = select_op(0xff << (Elems - n),    v.template extract<Elems / 2>(1),  n - Elems / 2, 0x76543210,
                                                                   fill.template extract<Elems / 2>(0),  n - Elems / 2, 0x76543210);

                ret.insert(0, tmp);

                tmp               = shuffle_op(fill, n - Elems / 2, 0x76543210);

                ret.insert(1, tmp);

                return ret;
            }
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_rotate_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        return shuffle_up_rotate_bits_impl<64, T, Elems>::run(v, Elems - n);
    }
};

}

#endif
