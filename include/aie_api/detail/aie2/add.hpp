// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ADD__HPP__
#define __AIE_API_DETAIL_AIE2_ADD__HPP__

#include "../broadcast.hpp"
#include "../config.hpp"
#include "../utils.hpp"

namespace aie::detail {

struct add_op {
    auto operator()(auto... args) const { return ::add(args...); }
};

struct sub_op {
    auto operator()(auto... args) const { return ::sub(args...); }
};

template <AddSubOperation Op>
constexpr auto get_add_sub_op()
{
    if      constexpr (Op == AddSubOperation::Add) return add_op{};
    else if constexpr (Op == AddSubOperation::Sub) return sub_op{};
}

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl<4, T, Elems, Op>
{
    using vector_type = vector<T, Elems>;

    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        return add_sub_bits_impl<8, next_type, Elems, Op>::run(v1.unpack(), v2.unpack()).pack();
    }

    template <unsigned Elems2>
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    static vector_type run(const T &a, const vector_type &v)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        const auto a_tmp = next_type(a);
        const auto v_tmp = v.unpack();
        return add_sub_bits_impl<8, next_type, Elems, Op>::run(a_tmp, v_tmp).pack();
    }

    static vector_type run(const vector_type &v, const T &a)
    {
        using next_type = utils::get_next_integer_type_t<T>;

        const auto v_tmp = v.unpack();
        const auto a_tmp = next_type(a);
        return add_sub_bits_impl<8, next_type, Elems, Op>::run(v_tmp, a_tmp).pack();
    }
};

template <unsigned TypeBits, typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_common
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    using native_op = add_sub_bits_impl_common<TypeBits, T, native_elems, Op>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_add_sub_op<Op>();
        vector_type ret;

        if constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = native_op::run(v1.template grow<native_elems>(),
                                                               v2.template grow<native_elems>());
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            ret = op(v1, v2);
        }
        else if constexpr (Elems > native_elems) {
            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(v1.template extract<native_elems>(idx),
                                                          v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        constexpr auto op = get_add_sub_op<Op>();

        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        constexpr auto op = get_add_sub_op<Op>();

        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, op(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl< 8, T, Elems, Op> : public add_sub_bits_impl_common< 8, T, Elems, Op> {};
template <typename T, unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<16, T, Elems, Op> : public add_sub_bits_impl_common<16, T, Elems, Op> {};
template <typename T, unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<32, T, Elems, Op> : public add_sub_bits_impl_common<32, T, Elems, Op> {};
template <typename T, unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<64, T, Elems, Op> : public add_sub_bits_impl_common<64, T, Elems, Op> {};

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_float_common;

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_complex_float_common;

#if __AIE_ARCH__ == 20

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;
    static constexpr unsigned native_elems = 16;
    using native_op = add_sub_bits_impl_float_common<T, native_elems, Op>;

    // There are no adders for bfloat16, but we can use the accumulator adders for float

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_add_sub_op<Op>();
        vector_type ret;

        if      constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = native_op::run(v1.template grow<native_elems>(), v2.template grow<native_elems>());
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const accum<accfloat, native_elems> acc1(v1);
            const accum<accfloat, native_elems> acc2(v2);
            const accum<accfloat, native_elems> res = op(acc1, acc2);

            ret = res.template to_vector<T>();
        }
        else if constexpr (Elems > native_elems) {
            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v1.template extract<native_elems>(idx),
                                                                      v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            vector_type ret;
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            vector_type ret;
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<16, bfloat16,  Elems, Op> : public add_sub_bits_impl_float_common<bfloat16, Elems, Op> {};
template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<32, float,     Elems, Op> : public add_sub_bits_impl_float_common<float,    Elems, Op> {};
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<32, cbfloat16, Elems, Op> : public add_sub_bits_impl_complex_float_common<cbfloat16, Elems, Op> {};
#endif
template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<64, cfloat,    Elems, Op> : public add_sub_bits_impl_complex_float_common<cfloat,    Elems, Op> {};
#endif

#elif __AIE_ARCH__ == 21

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_float_common
{
    static constexpr unsigned native_elems = 64;
    using vector_type = vector<T, Elems>;
    using native_op   = add_sub_bits_impl_float_common<T, native_elems, Op>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        // There are no adders for bfloat16, but we can use the accumulator adders for float
        constexpr auto op = get_add_sub_op<Op>();

        const accum<accfloat, Elems> acc1(v1);
        const accum<accfloat, Elems> acc2(v2);

        vector_type ret;

        utils::unroll_times<std::max(1u, Elems / native_elems)>([&](unsigned idx) __aie_inline {
            accum<accfloat, native_elems> tmp = op(acc1.template grow_extract<native_elems>(idx),
                                                   acc2.template grow_extract<native_elems>(idx));

            if constexpr (Elems < native_elems)
                ret = tmp.template extract<Elems>(0).template to_vector<T>();
            else
                ret.template insert<native_elems>(idx, tmp.template to_vector<T>());
        });

        return ret;
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(vector_elem_const_ref<T, Elems2> a, const vector_type &v)
    {
        return run((T)a, v);
    }

    template <unsigned Elems2>
    __aie_inline
    static vector_type run(const vector_type &v, vector_elem_const_ref<T, Elems2> a)
    {
        return run(v, (T)a);
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            vector_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<16, bfloat16, Elems, Op> : public add_sub_bits_impl_float_common<bfloat16, Elems, Op> {};

#if __AIE_API_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<32, float,    Elems, Op> : public add_sub_bits_impl_float_common<float,    Elems, Op> {};
#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned Elems, AddSubOperation Op> struct add_sub_bits_impl<64, cfloat,   Elems, Op> : public add_sub_bits_impl_complex_float_common<cfloat,   Elems, Op> {};
#endif

#endif

#if __AIE_API_COMPLEX_FP32_EMULATION__

template <typename T, unsigned Elems, AddSubOperation Op>
struct add_sub_bits_impl_complex_float_common
{
    static constexpr unsigned native_elems = 8;
    using vector_type = vector<T, Elems>;
    using native_op   = add_sub_bits_impl_complex_float_common<T, native_elems, Op>;

    __aie_inline
    static vector_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto op = get_add_sub_op<Op>();
        vector_type ret;

        if      constexpr (Elems < native_elems) {
            const vector<T, native_elems> tmp = native_op::run(v1.template grow<native_elems>(), v2.template grow<native_elems>());
            ret = tmp.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const accum<caccfloat, native_elems> acc1(v1);
            const accum<caccfloat, native_elems> acc2(v2);
            const accum<caccfloat, native_elems> res = op(acc1, acc2);

            ret = res.template to_vector<T>();
        }
        else {
            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v1.template extract<native_elems>(idx),
                                                                      v2.template extract<native_elems>(idx)));
            });
        }

        return ret;
    }

    __aie_inline
    static vector_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            vector_type ret;
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static vector_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            vector_type ret;
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            constexpr unsigned num_op = Elems / native_elems;

            utils::unroll_times<num_op>([&](auto idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_op::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

#endif


}

#endif
