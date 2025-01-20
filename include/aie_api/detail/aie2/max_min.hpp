// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_MAX_MIN__HPP__
#define __AIE_API_DETAIL_AIE2_MAX_MIN__HPP__

#include "../add.hpp"
#include "../broadcast.hpp"
#include "../interleave.hpp"

namespace aie::detail {

template <MaxMinOperation Op>
static constexpr auto get_max_min_op()
{
    if      constexpr (Op == MaxMinOperation::Max)     return [](auto &&... args) __aie_inline { return ::max(args...); };
    else if constexpr (Op == MaxMinOperation::Min)     return [](auto &&... args) __aie_inline { return ::min(args...); };
    else if constexpr (Op == MaxMinOperation::MaxDiff) return [](auto &&... args) __aie_inline { return ::maxdiff(args...); };
}

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<4, T, Elems, Op>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   next_type = utils::get_next_integer_type_t<T>;
    using native_impl = max_min_bits_impl<8, next_type, native_elems, Op>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {

        if constexpr (Elems <= native_elems) {
            return max_min_bits_impl<8, next_type, Elems, Op>::run(v1.unpack(), v2.unpack()).pack();
        }
        else {
            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                const vector<next_type, native_elems> tmp = native_impl::run(v1.template extract<native_elems>(idx).unpack(),
                                                                             v2.template extract<native_elems>(idx).unpack());
                ret.insert(idx, tmp.pack());
            
            });

            return ret;
        }
   }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, bool sign)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return max_min_bits_impl<8, next_type, Elems, Op>::run(v1.unpack_sign(sign), v2.unpack_sign(sign), sign).pack_sign(sign);
   }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        return run(broadcast<T, Elems>::run(a), v);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        return run(v, broadcast<T, Elems>::run(a));
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v, bool sign)
    {
        return run(broadcast<T, Elems>::run(a), v, sign);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a, bool sign)
    {
        return run(v, broadcast<T, Elems>::run(a), sign);
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl_common
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_bits_impl_common<T, native_elems, Op>;

    static constexpr auto op = get_max_min_op<Op>();

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        if constexpr (vector_type::bits() <= 512) {
            vector<T, native_elems> tmp;

            tmp = op(v1.template grow<native_elems>(), v2.template grow<native_elems>());

            ret = tmp.template extract<Elems>(0);
        }
        else {
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(v1.template extract<native_elems>(idx),
                                                 v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, bool sign)
    {
        vector_type ret;

        if constexpr (vector_type::bits() <= 512) {
            vector<T, native_elems> tmp;

            if constexpr (vector_type::is_floating_point())
                tmp = op(v1.template grow<native_elems>(), v2.template grow<native_elems>());
            else
                tmp = op(v1.template grow<native_elems>(), v2.template grow<native_elems>(), sign);

            ret = tmp.template extract<Elems>(0);
        }
        else {
            constexpr unsigned num_ops = Elems / native_elems;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(v1.template extract<native_elems>(idx),
                                                 v2.template extract<native_elems>(idx),
                                                 sign));
            });
        }

        return ret;
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        return run(a, v, is_signed_v<T>);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        return run(v, a, is_signed_v<T>);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v, bool sign)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(vals, v.template extract<native_elems>(idx), sign));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v, sign);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a, bool sign)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(v.template extract<native_elems>(idx), vals, sign));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a), sign);
        }
    }
};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<8, T, Elems, Op> : public max_min_bits_impl_common<T, Elems, Op> {};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<16, T, Elems, Op> : public max_min_bits_impl_common<T, Elems, Op> {};

template <typename T, unsigned Elems, MaxMinOperation Op>
struct max_min_bits_impl<32, T, Elems, Op> : public max_min_bits_impl_common<T, Elems, Op> {};

template <typename T, unsigned Elems>
struct max_min_bits_impl_maxdiff_float_common
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = max_min_bits_impl_maxdiff_float_common<T, native_elems>;

    static constexpr MaxMinOperation Op = MaxMinOperation::MaxDiff;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        vector_type ret;

        if constexpr (Elems <= native_elems) {
            vector<T, native_elems> tmp;
            const unsigned cmp = ::lt(v2.template grow<native_elems>(), v1.template grow<native_elems>());
            tmp = ::sel(zeros<T, native_elems>::run(),
                        sub<T, native_elems>::run(v1.template grow<native_elems>(), v2.template grow<native_elems>()),
                        cmp);

            ret = tmp.template extract<Elems>(0);
        }
        else {
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(v1.template extract<native_elems>(idx),
                                                 v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2, bool sign)
    {
        return run(v1, v2);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        return run(a, v, true);
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        return run(v, a, true);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v, bool sign)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(vals, v.template extract<native_elems>(idx), sign));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a, bool sign)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, native_impl::run(v.template extract<native_elems>(idx), vals, sign));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <unsigned Elems>
struct max_min_bits_impl<16, bfloat16, Elems, MaxMinOperation::MaxDiff> : public max_min_bits_impl_maxdiff_float_common<bfloat16, Elems> {};

#if __AIE_API_FP32_EMULATION__

template <unsigned Elems>
struct max_min_bits_impl<32,  float,   Elems, MaxMinOperation::MaxDiff> : public max_min_bits_impl_maxdiff_float_common< float,   Elems> {};

#endif

}

#endif
