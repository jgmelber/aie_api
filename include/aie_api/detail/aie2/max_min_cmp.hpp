// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MAX_MIN_CMP__HPP__
#define __AIE_API_DETAIL_AIE2_MAX_MIN_CMP__HPP__

#include "../max_min.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MaxMinOperation Op>
static constexpr auto get_max_min_cmp_op()
{
    if constexpr (Op == MaxMinOperation::MaxCmp)
        return [](auto &&... args) __aie_inline { return ::max_lt(args...); };
    else if constexpr (Op == MaxMinOperation::MinCmp)
        return [](auto &&... args) __aie_inline { return ::min_ge(args...); };
}

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<4, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   next_type = utils::get_next_integer_type_t<T>;
    using native_impl = max_min_cmp_bits_impl<8, next_type, native_elems, Op>;

    __aie_inline
    static auto run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            auto [tmp, mask] = max_min_cmp_bits_impl<8, next_type, Elems, Op>::run(v1.unpack(), v2.unpack());
            return std::make_tuple(tmp.pack(), mask);
        }
        else {
            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(v1.template extract<native_elems>(idx).unpack(),
                                                        v2.template extract<native_elems>(idx).unpack());
                ret.insert(idx, tmp.pack());
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
   }

    template <unsigned Elems2>
    __aie_inline
    static auto run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static auto run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<next_type, native_elems> vals = broadcast<next_type, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(vals, v.template extract<native_elems>(idx));

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static auto run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<next_type, native_elems> vals = broadcast<next_type, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(v.template extract<native_elems>(idx), vals);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<8, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_cmp_bits_impl<8, T, native_elems, Op>;

    static constexpr auto op = get_max_min_cmp_op<Op>();

    __aie_inline
    static auto run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        if constexpr (Elems <= native_elems) {
            unsigned long long cmp;

            const vector<T, native_elems> tmp = op(v1.template grow<native_elems>(),
                                                   v2.template grow<native_elems>(),
                                                   cmp);
            ret = tmp.template extract<Elems>(0);

            if constexpr (Elems <= 32) {
                // Avoid unnecessary register copy
                return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
            }
            else {
                return std::make_tuple(ret, mask<Elems>::from_uint64(cmp));
            }
        }
        else {
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                unsigned long long cmp;

                const vector<T, native_elems> tmp = op(v1.template extract<native_elems>(idx),
                                                       v2.template extract<native_elems>(idx),
                                                       cmp);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, mask<native_elems>::from_uint64(cmp));
            });

            return std::make_tuple(ret, ret_mask);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static auto run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(vals, v.template extract<native_elems>(idx));

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static auto run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(v.template extract<native_elems>(idx), vals);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<16, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_cmp_bits_impl<16, T, native_elems, Op>;

    static constexpr auto op = get_max_min_cmp_op<Op>();

    __aie_inline
    static auto run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        if constexpr (Elems <= native_elems) {
            unsigned cmp;

            const vector<T, native_elems> tmp = op(v1.template grow<native_elems>(),
                                                   v2.template grow<native_elems>(),
                                                   cmp);
            ret = tmp.template extract<Elems>(0);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else {
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                unsigned cmp;

                const vector<T, native_elems> tmp = op(v1.template extract<native_elems>(idx),
                                                       v2.template extract<native_elems>(idx),
                                                       cmp);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, mask<native_elems>::from_uint32(cmp));
            });

            return std::make_tuple(ret, ret_mask);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static auto run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(vals, v.template extract<native_elems>(idx));

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static auto run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(v.template extract<native_elems>(idx), vals);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_cmp_bits_impl<32, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_cmp_bits_impl<32, T, native_elems, Op>;

    static constexpr auto op = get_max_min_cmp_op<Op>();

    __aie_inline
    static auto run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        if constexpr (Elems <= native_elems) {
            unsigned cmp;

            const vector<T, native_elems> tmp = op(v1.template grow<native_elems>(),
                                                   v2.template grow<native_elems>(),
                                                   cmp);
            ret = tmp.template extract<Elems>(0);

            return std::make_tuple(ret, mask<Elems>::from_uint32(cmp));
        }
        else {
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                unsigned cmp1, cmp2;

                const vector<T, native_elems> tmp1 = op(v1.template extract<native_elems>(2 * idx + 0),
                                                        v2.template extract<native_elems>(2 * idx + 0),
                                                        cmp1);
                const vector<T, native_elems> tmp2 = op(v1.template extract<native_elems>(2 * idx + 1),
                                                        v2.template extract<native_elems>(2 * idx + 1),
                                                        cmp2);

                ret.insert(2 * idx + 0, tmp1);
                ret.insert(2 * idx + 1, tmp2);
                ret_mask.insert(idx, mask<native_elems * 2>::from_uint32((cmp2 << 16) | cmp1));
            });

            return std::make_tuple(ret, ret_mask);
        }
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static auto run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static auto run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(vals, v.template extract<native_elems>(idx));

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static auto run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;
            mask<Elems> ret_mask;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [tmp, tmp_mask] = native_impl::run(v.template extract<native_elems>(idx), vals);

                ret.insert(idx, tmp);
                ret_mask.insert(idx, tmp_mask);
            });

            return std::make_tuple(ret, ret_mask);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

}

#endif
