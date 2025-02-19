// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_SHUFFLE_REPLICATE_HPP__
#define __AIE_API_DETAIL_AIE1_SHUFFLE_REPLICATE_HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if      constexpr (vector_type::bits() <= 512)  return shuffle_up_replicate<int16, Elems>::run(v.unpack(), n).template pack<T>();
        else if constexpr (vector_type::bits() == 1024) return shuffle_up_replicate_bits_impl_scalar<8, T, Elems>::run(v, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl<8, T, Elems>
{
    using vector_type = vector<T, Elems>;

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if      constexpr (vector_type::bits() <= 512)  return shuffle_down_replicate<int16, Elems>::run(v.unpack(), n).template pack<T>();
        else if constexpr (vector_type::bits() == 1024) return shuffle_down_replicate_bits_impl_scalar<8, T, Elems>::run(v, n);
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <unsigned Section>
    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = std::max(n, Section * 8u); i < (Section + 1) * 8u; ++i)
            ret |= (int(i) - n) << ((4 * i) % 32);

        return ret;
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        if (chess_manifest(n % 2 == 1)) {
            vector_type ret = v;

            const T a = v.get(0);

            ret.push(a);

            if (n == 1)
                return ret;

            return vector_cast<T>(shuffle_up_replicate_bits_impl<32, int32, Elems / 2>::run(vector_cast<int32>(ret), n / 2));
        }
        else {
            vector_type ret;

            vector<int32, 8> one;

            one[0] = 1;

            // TODO: explore using select instead
            if constexpr (Elems == 8) {
                const vector<T, 16> tmp = ::srs(::mul16(v.template grow<32>(), 0, compute_offsets<0>(n),            0x00000000, one, 0, 0x00000000, 0x00000000), 0);
                ret = tmp.template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                const vector<T, 16> tmp = ::srs(::mul16(v.template grow<32>(), 0, compute_offsets<0>(n), compute_offsets<1>(n), one, 0, 0x00000000, 0x00000000), 0);
                ret = tmp.template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector<T, 16> tmp = ::srs(::mul16(v,     0, compute_offsets<0>(n), compute_offsets<1>(n), one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(0, tmp);

                if (n >= 16) tmp = ::srs(::mul16(v,      0, compute_offsets<2>(n), compute_offsets<3>(n), one, 0, 0x00000000, 0x00000000), 0);
                else         tmp = ::srs(::mul16(v, 16 - n,            0x76543210,            0xfedcba98, one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(1, tmp);
            }
            else if constexpr (Elems == 64) {
                vector<T, 16> tmp = ::srs(::mul16(v,     0, compute_offsets<0>(n), compute_offsets<1>(n), one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(0, tmp);

                if (n >= 16) tmp = ::srs(::mul16(v,      0, compute_offsets<2>(n), compute_offsets<3>(n), one, 0, 0x00000000, 0x00000000), 0);
                else         tmp = ::srs(::mul16(v, 16 - n,            0x76543210,            0xfedcba98, one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(1, tmp);

                if (n >= 32) tmp = ::srs(::mul16(v,      0, compute_offsets<4>(n), compute_offsets<5>(n), one, 0, 0x00000000, 0x00000000), 0);
                else         tmp = ::srs(::mul16(v, 32 - n,            0x76543210,            0xfedcba98, one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(2, tmp);

                if (n >= 48) tmp = ::srs(::mul16(v,      0, compute_offsets<6>(n), compute_offsets<7>(n), one, 0, 0x00000000, 0x00000000), 0);
                else         tmp = ::srs(::mul16(v, 48 - n,            0x76543210,            0xfedcba98, one, 0, 0x00000000, 0x00000000), 0);

                ret.insert(3, tmp);
            }

            return ret;
        }
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl<16, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <unsigned Start, unsigned End = Start + 8, unsigned Offset = 0>
    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = Start; i < std::min(End, Elems - Offset - n); ++i)
            ret |= (n + i) << ((4 * i) % 32);

        for (unsigned i = std::min(End, Elems - Offset - n); i < End; ++i)
            ret |= (Elems - Offset - 1) << ((4 * i) % 32);

        return ret;
    }


    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        {
            vector_type ret;

            vector<int32, 8> one;

            one[0] = 1;

            // TODO: explore using select instead
            if constexpr (Elems == 8) {
                const vector<T, 16> tmp = ::srs(::mul16(v.template grow<32>(), 0, compute_offsets<0, Elems>(n),            0x00000000, one, 0, 0x00000000, 0x00000000), 0);
                ret = tmp.template extract<Elems>(0);
            }
            else if constexpr (Elems == 16) {
                const vector<T, 16> tmp = ::srs(::mul16(v.template grow<32>(), 0, compute_offsets<0, 8>(n), compute_offsets<8, 16>(n), one, 0, 0x00000000, 0x00000000), 0);
                ret = tmp.template extract<Elems>(0);
            }
            else if constexpr (Elems == 32) {
                vector<T, 16> tmp1, tmp2;

                if (n >= 16) {
                    tmp1 = ::srs(::mul16(v,    16, compute_offsets<0, 8, 16>(n - 16), compute_offsets<8, 16, 16>(n - 16), one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v,    16, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                }
                else {
                    tmp1 = ::srs(::mul16(v,     n, 0x76543210,                        0xfedcba98,                    one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v,    16, compute_offsets<0, 8, 16>(n),      compute_offsets<8, 16, 16>(n), one, 0, 0x00000000, 0x00000000), 0);
                }

                ret.insert(0, tmp1);
                ret.insert(1, tmp2);
            }
            else if constexpr (Elems == 64) {
                vector<T, 16> tmp1, tmp2, tmp3, tmp4;

                if (n > 48) {
                    tmp1 = ::srs(::mul16(v,    48, compute_offsets<0, 8, 48>(n - 48), compute_offsets<8, 16, 48>(n - 48), one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v,    48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp3 = ::srs(::mul16(v,    48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp4 = ::srs(::mul16(v,    48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                }
                else if (n > 32) {
                    tmp1 = ::srs(::mul16(v,      n, 0x76543210,                        0xfedcba98,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v,     48, compute_offsets<0, 8, 48>(n - 32), compute_offsets<8, 16, 48>(n - 32), one, 0, 0x00000000, 0x00000000), 0);
                    tmp3 = ::srs(::mul16(v,     48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp4 = ::srs(::mul16(v,     48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                }
                else if (n > 16) {
                    tmp1 = ::srs(::mul16(v,      n, 0x76543210,                        0xfedcba98,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v, n + 16, 0x76543210,                        0xfedcba98,                         one, 0, 0x00000000, 0x00000000), 0);
                    tmp3 = ::srs(::mul16(v,     48, compute_offsets<0, 8, 48>(n - 16), compute_offsets<8, 16, 48>(n - 16), one, 0, 0x00000000, 0x00000000), 0);
                    tmp4 = ::srs(::mul16(v,     48, 0xffffffff,                        0xffffffff,                         one, 0, 0x00000000, 0x00000000), 0);
                }
                else {
                    tmp1 = ::srs(::mul16(v,      n, 0x76543210,                        0xfedcba98,                    one, 0, 0x00000000, 0x00000000), 0);
                    tmp2 = ::srs(::mul16(v, n + 16, 0x76543210,                        0xfedcba98,                    one, 0, 0x00000000, 0x00000000), 0);
                    tmp3 = ::srs(::mul16(v, n + 32, 0x76543210,                        0xfedcba98,                    one, 0, 0x00000000, 0x00000000), 0);
                    tmp4 = ::srs(::mul16(v,     48, compute_offsets<0, 8, 48>(n),      compute_offsets<8, 16, 48>(n), one, 0, 0x00000000, 0x00000000), 0);
                }

                ret.insert(0, tmp1);
                ret.insert(1, tmp2);
                ret.insert(2, tmp3);
                ret.insert(3, tmp4);
            }

            return ret;
        }
    }
};


template <typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <unsigned Section>
    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = std::max(n, Section * 8u); i < (Section + 1) * 8u; ++i)
            ret |= (int(i) - n) << ((4 * i) % 32);

        return ret;
    }

    static constexpr auto get_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_op();

        if constexpr (Elems <= 8) {
            const vector<T, 16> tmp = op(v.template grow<16>(), 0, compute_offsets<0>(n), 0);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 16> tmp = op(v,  0, compute_offsets<0>(n), compute_offsets<1>(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            const vector<T, 16> tmp1 = op(v, 0, compute_offsets<0>(n), compute_offsets<1>(n));
            vector<T, 16> tmp2;

            if (n >= 16)
                tmp2 = op(v,      0, compute_offsets<2>(n), compute_offsets<3>(n));
            else
                tmp2 = op(v, 16 - n, 0x76543210, 0xfedcba98);

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl<32, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <unsigned Start, unsigned End, unsigned Offset = 0>
    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = Start; i < std::min(End, Elems - Offset - n); ++i)
            ret |= (n + i) << ((4 * i) % 32);

        for (unsigned i = std::min(End, Elems - Offset - n); i < End; ++i)
            ret |= (Elems - Offset - 1) << ((4 * i) % 32);

        return ret;
    }

    static constexpr auto get_op()
    {
        if constexpr (std::is_same_v<T, float>) return [](auto &&... args) __aie_inline { return ::fpshuffle16(args...); };
        else                                    return [](auto &&... args) __aie_inline { return ::shuffle16(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_op();

        if constexpr (Elems <= 8) {
            const vector<T, 16> tmp = op(v.template grow<16>(), 0, compute_offsets<0, Elems>(n), 0);
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 16> tmp = op(v,  0, compute_offsets<0, 8>(n), compute_offsets<8, 16>(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 32) {
            vector<T, 16> tmp1;
            vector<T, 16> tmp2;

            if (n >= 16) {
                tmp1 = op(v, 16, compute_offsets<0, 8, 16>(n - 16), compute_offsets<8, 16, 16>(n - 16));
                tmp2 = op(v, 16, 0xffffffff, 0xffffffff);
            }
            else {
                tmp1 = op(v,  n, 0x76543210, 0xfedcba98);
                tmp2 = op(v, 16, compute_offsets<0, 8, 16>(n),      compute_offsets<8, 16, 16>(n));
            }

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_up_replicate_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    template <unsigned Section>
    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = std::max(n, Section * 8u); i < (Section + 1) * 8u; ++i)
            ret |= (int(i) - n) << ((4 * i) % 32);

        return ret;
    }

    static constexpr auto get_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<T, 8> tmp = op(v.template grow<8>(), 0, compute_offsets<0>(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> tmp = op(v,  0, compute_offsets<0>(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            const vector<T, 8> tmp1 = op(v, 0, compute_offsets<0>(n));
            vector<T, 8> tmp2;

            if (n >= 8)
                tmp2 = op(v,     0, compute_offsets<1>(n));
            else
                tmp2 = op(v, 8 - n, 0x76543210);

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);
        }

        return ret;
    }
};

template <typename T, unsigned Elems>
struct shuffle_down_replicate_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned compute_offsets(unsigned n)
    {
        unsigned ret = 0;

        for (unsigned i = 0; i < std::min(Elems, 8u) - n; ++i)
            ret |= (n + i) << ((4 * i) % 32);

        for (unsigned i = std::min(Elems, 8u) - n; i < std::min(Elems, 8u); ++i)
            ret |= (std::min(Elems, 8u) - 1) << ((4 * i) % 32);

        return ret;
    }

    static constexpr auto get_op()
    {
        if constexpr (std::is_same_v<T, cfloat>) return [](auto &&... args) __aie_inline { return ::fpshuffle8(args...); };
        else                                     return [](auto &&... args) __aie_inline { return ::shuffle8(args...); };
    }

    __aie_inline static vector_type run(const vector_type &v, unsigned n)
    {
        vector_type ret;

        constexpr auto op = get_op();

        if constexpr (Elems <= 4) {
            const vector<T, 8> tmp = op(v.template grow<8>(), 0, compute_offsets(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 8) {
            const vector<T, 8> tmp = op(v,  0, compute_offsets(n));
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == 16) {
            vector<T, 8> tmp1;
            vector<T, 8> tmp2;

            if (n >= 8) {
                tmp1 = op(v, 8, compute_offsets(n - 8));
                tmp2 = op(v, 8, 0x77777777);
            }
            else {
                tmp1 = op(v, n, 0x76543210);
                tmp2 = op(v, 8, compute_offsets(n));
            }

            ret.insert(0, tmp1);
            ret.insert(1, tmp2);
        }

        return ret;
    }
};

}

#endif
