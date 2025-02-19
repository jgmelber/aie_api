// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_MAX_MIN__HPP__
#define __AIE_API_DETAIL_AIE1_MAX_MIN__HPP__

#include "../vector.hpp"

namespace aie::detail {

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<8, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            ret.insert(0, max_min_bits_impl<8, T, Elems / 2, Op>::run(v1.template extract<Elems / 2>(0), v2.template extract<Elems / 2>(0)));
            ret.insert(1, max_min_bits_impl<8, T, Elems / 2, Op>::run(v1.template extract<Elems / 2>(1), v2.template extract<Elems / 2>(1)));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp1 = v1.unpack();
            const vector<int16, Elems> tmp2 = v2.unpack();

            return max_min_bits_impl<16, int16, Elems, Op>::run(tmp1, tmp2).template pack<T>();
        }
    }

    static vector_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            ret.insert(0, max_min_bits_impl<8, T, Elems / 2, Op>::run(a, v.template extract<Elems / 2>(0)));
            ret.insert(1, max_min_bits_impl<8, T, Elems / 2, Op>::run(a, v.template extract<Elems / 2>(1)));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return max_min_bits_impl<16, int16, Elems, Op>::run((int16)a, tmp).template pack<T>();
        }
    }

    static vector_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems == 128) {
            vector_type ret;

            ret.insert(0, max_min_bits_impl<8, T, Elems / 2, Op>::run(v.template extract<Elems / 2>(0), a));
            ret.insert(1, max_min_bits_impl<8, T, Elems / 2, Op>::run(v.template extract<Elems / 2>(1), a));

            return ret;
        }
        else {
            const vector<int16, Elems> tmp = v.unpack();

            return max_min_bits_impl<16, int16, Elems, Op>::run(tmp, (int16)a).template pack<T>();
        }
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<16, int16, Elems, Op>
{
    using           T = int16;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == MaxMinOperation::Max)     return [](auto &&... args) __aie_inline { return ::max32(args...); };
        else if constexpr (Op == MaxMinOperation::Min)     return [](auto &&... args) __aie_inline { return ::min32(args...); };
        else if constexpr (Op == MaxMinOperation::MaxDiff) return [](auto &&... args) __aie_inline { return ::maxdiff32(args...); };
    }

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int16, 32> tmp = op(v1.template grow_extract<32>(idx), v2.template grow_extract<32>(idx));
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<int16, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        const unsigned odd = a.offset & 0x1;
        const int offset = a.offset - odd;
        const int square = 0x1111 * odd;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int16, 32> tmp = op(a.parent.template grow<std::max(Elems2, 32u)>(), offset, 0x00000000, 0x00000000, square,
                                                            v.template grow_extract<32>(idx),      0, 0x06040200, 0x0e0c0a08, 0x3210);
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<int16, Elems2> a)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 32? 1 : Elems / 32;

        const unsigned odd = a.offset & 0x1;
        const int offset = a.offset - odd;
        const int square = 0x1111 * odd;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int16, 32> tmp = op(               v.template grow_extract<32>(idx),      0, 0x06040200, 0x0e0c0a08, 0x3210,
                                             a.parent.template grow<std::max(Elems2, 32u)>(), offset, 0x00000000, 0x00000000, square);
            ret.insert(idx, tmp.template extract<(Elems < 32? Elems : 32)>(0));
        });

        return ret;
    }

    static vector_type run(const int16 &a, const vector_type &v)
    {
        const vector<int16, 16> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    static vector_type run(const vector_type &v, const int16 &a)
    {
        const vector<int16, 16> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<32, int32, Elems, Op>
{
    using           T = int32;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == MaxMinOperation::Max)     return [](auto &&... args) __aie_inline { return ::max16(args...); };
        else if constexpr (Op == MaxMinOperation::Min)     return [](auto &&... args) __aie_inline { return ::min16(args...); };
        else if constexpr (Op == MaxMinOperation::MaxDiff) return [](auto &&... args) __aie_inline { return ::maxdiff16(args...); };
    }

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int32, 16> tmp = op(v1.template grow_extract<16>(idx), v2.template grow_extract<16>(idx));
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<int32, Elems2> a, const vector_type &v)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int32, 16> tmp = op(a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000,
                                                            v.template grow_extract<16>(idx),        0, 0x76543210, 0xfedcba98);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }
    
    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<int32, Elems2> a)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 16? 1 : Elems / 16;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<int32, 16> tmp = op(               v.template grow_extract<16>(idx),        0, 0x76543210, 0xfedcba98,
                                             a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 16? Elems : 16)>(0));
        });

        return ret;
    }

    static vector_type run(const int32 &a, const vector_type &v)
    {
        const vector<int32, 8> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    static vector_type run(const vector_type &v, const int32 &a)
    {
        const vector<int32, 8> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<32, float, Elems, Op>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static constexpr auto get_op()
    {
        if      constexpr (Op == MaxMinOperation::Max) return [](auto &&... args) __aie_inline { return ::fpmax(args...); };
        else if constexpr (Op == MaxMinOperation::Min) return [](auto &&... args) __aie_inline { return ::fpmin(args...); };
    }

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<T, 8> tmp = op(v1.template grow_extract<8>(idx), v2.template grow_extract<16>(idx / 2), 8 * (idx % 2), 0x76543210);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<float, Elems2> a, const vector_type &v)
    {
        return run(v, a);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<float, Elems2> a)
    {
        constexpr auto op = get_op();
        constexpr unsigned num_mul = Elems < 8? 1 : Elems / 8;

        vector_type ret;

        utils::unroll_times<num_mul>([&](auto idx) __aie_inline {
            const vector<T, 8> tmp = op(v.template grow_extract<8>(idx), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000);
            ret.insert(idx, tmp.template extract<(Elems < 8? Elems : 8)>(0));
        });

        return ret;
    }

    static vector_type run(const float &a, const vector_type &v)
    {
        const vector<float, 8> tmp_scalar(a);

        return run(tmp_scalar[0], v);
    }

    static vector_type run(const vector_type &v, const float &a)
    {
        const vector<float, 8> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

template <unsigned Elems>
struct max_min_bits_impl<32, float, Elems, MaxMinOperation::MaxDiff>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        const vector<float, 8> zeros(0.f);

        if constexpr (Elems <= 8) {
            unsigned mask;

            vector<float, 8>  sub;
            vector<float, 16> sel;

            sub  = ::fpsub    (v1.template grow<8>(), v2.template grow<16>(), 0, 0x76543210);
            mask = ::fplt_fast(v1.template grow<8>(), v2.template grow<16>(), 0, 0x76543210);

            sel = ::fpselect16(mask, sub.template grow<16>(), 0, 0x76543210, 0x0,
                                            zeros.grow<16>(), 0, 0x00000000, 0x0);

            ret = sel.template extract<Elems>(0);

            return ret;
        }
        else if constexpr (Elems == 16 || Elems == 32) {
            vector_type ret;

            utils::unroll_times<Elems / 16>([&](auto idx) __aie_inline {
                unsigned mask;

                vector<float, 16> sub;

                sub.insert<8>(0, ::fpsub(v1.template extract<8>(0 + 2 * idx), v2.template extract<16>(idx), 0, 0x76543210));
                sub.insert<8>(1, ::fpsub(v1.template extract<8>(1 + 2 * idx), v2.template extract<16>(idx), 8, 0x76543210));

                mask = ::fplt_fast(v1.template extract<8>(0 + 2 * idx), v2.template extract<16>(idx), 0, 0x76543210) |
                      (::fplt_fast(v1.template extract<8>(1 + 2 * idx), v2.template extract<16>(idx), 8, 0x76543210) << 8);

                ret.template insert<16>(idx, ::fpselect16(mask,             sub, 0, 0x76543210, 0xfedcba98,
                                                               zeros.grow<16>(), 0, 0x00000000, 0x00000000));
            });

            return ret;
        }
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<float, Elems2> a, const vector_type &v)
    {
        // This case cannot be optimized as there is no permute for the first operand of the the fsub intrinsic
        return run(a.get(), v);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<float, Elems2> a)
    {
        vector_type ret;

        const vector<float, 8> zeros(0.f);

        if constexpr (Elems <= 8) {
            unsigned mask;

            vector<float, 8>  sub;
            vector<float, 16> sel;

            sub  = ::fpsub    (v.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000);
            mask = ::fplt_fast(v.template grow<8>(), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000);

            sel = ::fpselect16(mask, sub.template grow<16>(), 0, 0x76543210, 0x0,
                                            zeros.grow<16>(), 0, 0x00000000, 0x0);

            ret = sel.template extract<Elems>(0);

            return ret;
        }
        else if constexpr (Elems == 16 || Elems == 32) {
            vector_type ret;

            utils::unroll_times<Elems / 16>([&](auto idx) __aie_inline {
                unsigned mask;

                vector<float, 16> sub;

                sub.insert<8>(0, ::fpsub(v.template extract<8>(idx * 2 + 0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));
                sub.insert<8>(1, ::fpsub(v.template extract<8>(idx * 2 + 1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000));

                mask = ::fplt_fast(v.template extract<8>(idx * 2 + 0), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000) |
                      (::fplt_fast(v.template extract<8>(idx * 2 + 1), a.parent.template grow<std::max(Elems2, 16u)>(), a.offset, 0x00000000) << 8);

                ret.template insert<16>(idx, ::fpselect16(mask,              sub, 0, 0x76543210, 0xfedcba98,
                                                                zeros.grow<16>(), 0, 0x00000000, 0x00000000));
            });

            return ret;
        }
    }

    static vector_type run(const float &a, const vector_type &v)
    {
        vector_type ret;

        v8float a_vector = ::undef_v8float();

        a_vector = ::upd_elem(a_vector, 0, a, a);
        a_vector = ::upd_elem(a_vector, 1, a, a);
        a_vector = ::upd_elem(a_vector, 2, a, a);
        a_vector = ::upd_elem(a_vector, 3, a, a);

        const vector<float, 8> zeros(0.f);

        if constexpr (Elems <= 8) {
            unsigned mask;

            vector<float, 8>  sub;
            vector<float, 16> sel;

            sub  = ::fpsub    (a_vector, v.template grow<16>(), 0, 0x76543210);
            mask = ::fplt_fast(a_vector, v.template grow<16>(), 0, 0x76543210);

            sel = ::fpselect16(mask, sub.template grow<16>(), 0, 0x76543210, 0x0,
                                            zeros.grow<16>(), 0, 0x00000000, 0x0);

            ret = sel.template extract<Elems>(0);

            return ret;
        }
        else if constexpr (Elems == 16 || Elems == 32) {
            vector_type ret;

            utils::unroll_times<Elems / 16>([&](auto idx) __aie_inline {
                unsigned mask;

                vector<float, 16> sub;

                sub.insert<8>(0, ::fpsub(a_vector, v.template extract<16>(idx), 0, 0x76543210));
                sub.insert<8>(1, ::fpsub(a_vector, v.template extract<16>(idx), 8, 0x76543210));

                mask = ::fplt_fast(a_vector, v.template extract<16>(idx), 0, 0x76543210) |
                      (::fplt_fast(a_vector, v.template extract<16>(idx), 8, 0x76543210) << 8);

                ret.template insert<16>(idx, ::fpselect16(mask,              sub, 0, 0x76543210, 0xfedcba98,
                                                                zeros.grow<16>(), 0, 0x00000000, 0x00000000));
            });

            return ret;
        }
    }

    static vector_type run(const vector_type &v, const float &a)
    {
        const vector<float, 8> tmp_scalar(a);

        return run(v, tmp_scalar[0]);
    }
};

}

#endif
