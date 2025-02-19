// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_COMPARE__HPP__
#define __AIE_API_DETAIL_AIE2_COMPARE__HPP__

#include "../../mask.hpp"
#include "../broadcast.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <CmpOp Op, typename T>
struct compare_vector_traits {
    static constexpr auto get_op()
    {
        if      constexpr (Op == CmpOp::LT)  return [](auto a, auto b) __aie_inline { return ::lt(a, b); };
        else if constexpr (Op == CmpOp::GE)  return [](auto a, auto b) __aie_inline { return ::ge(a, b); };
        else if constexpr (Op == CmpOp::EQ)  return [](auto a, auto b) __aie_inline { return ::eq(a, b); };
        else if constexpr (Op == CmpOp::NEQ) return [](auto a, auto b) __aie_inline { return ::ne(a, b); };
        else if constexpr (Op == CmpOp::LE)  return [](auto a, auto b) __aie_inline { return ::le(a, b); };
        else if constexpr (Op == CmpOp::GT)  return [](auto a, auto b) __aie_inline { return ::gt(a, b); };
    }

    // Returns whether an operator is derived from the negation of another and
    // returns that operator if that's the case.
    static constexpr bool is_derived()
    {
        if constexpr (Op == CmpOp::NEQ) return true;
        return false;
    }

    static constexpr CmpOp get_derived()
    {
        if constexpr (Op == CmpOp::NEQ) return CmpOp::EQ;
        return Op;
    }

    static constexpr auto get_derived_op()
    {
        return compare_vector_traits<get_derived(), T>::get_op();
    }
};

template <CmpOp Op, typename T>
struct compare_zero_traits {
    static constexpr auto get_op()
    {
        if constexpr (is_unsigned_v<T>) {
            // ltz intrinsic for unsigned vector types is wrong in AIE2(P)(PS). See CRVO-9293
            if      constexpr (Op == CmpOp::LT)  return [](auto a) __aie_inline { return 0llu; };
            else if constexpr (Op == CmpOp::GE)  return [](auto a) __aie_inline { return ~0llu; };
            else if constexpr (Op == CmpOp::EQ)  return [](auto a) __aie_inline { return ::eqz(a); };
            else if constexpr (Op == CmpOp::NEQ) return [](auto a) __aie_inline { return ::gtz(a); };
            else if constexpr (Op == CmpOp::LE)  return [](auto a) __aie_inline { return ::eqz(a); };
            else if constexpr (Op == CmpOp::GT)  return [](auto a) __aie_inline { return ::gtz(a); };
        }
        else if constexpr (Op == CmpOp::LT)  return [](auto a) __aie_inline { return ::ltz(a); };
        else if constexpr (Op == CmpOp::GE)  return [](auto a) __aie_inline { return ~::ltz(a); };
        else if constexpr (Op == CmpOp::EQ)  return [](auto a) __aie_inline { return ::eqz(a); };
        else if constexpr (Op == CmpOp::NEQ) return [](auto a) __aie_inline { return ~::eqz(a); };
        else if constexpr (Op == CmpOp::LE)  return [](auto a) __aie_inline { return ~::gtz(a); };
        else if constexpr (Op == CmpOp::GT)  return [](auto a) __aie_inline { return ::gtz(a); };
    }

    // Returns whether an operator is derived from the negation of another and
    // returns that operator if that's the case.
    static constexpr bool is_derived()
    {
        if constexpr (is_unsigned_v<T>) {
            if constexpr (Op == CmpOp::GE) return true;
        }
        else {
            if      constexpr (Op == CmpOp::GE)  return true;
            else if constexpr (Op == CmpOp::NEQ) return true;
            else if constexpr (Op == CmpOp::LE)  return true;
        }
        return false;
    }

    static constexpr CmpOp get_derived()
    {
        if constexpr (is_unsigned_v<T>) {
            if constexpr (Op == CmpOp::GE) return CmpOp::LT;
        }
        else {
            if      constexpr (Op == CmpOp::GE)  return CmpOp::LT;
            else if constexpr (Op == CmpOp::NEQ) return CmpOp::EQ;
            else if constexpr (Op == CmpOp::LE)  return CmpOp::GT;
        }
        return Op;
    }

    static constexpr auto get_derived_op()
    {
        return compare_zero_traits<get_derived(), T>::get_op();
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 4, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T> / 2;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   next_type = utils::get_next_integer_type_t<T>;

    template <unsigned Elems2 = Elems>
    using  unpack_cmp = cmp_bits_impl<Op, 8, next_type, Elems2>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            return unpack_cmp<>::run(v1.unpack(), v2.unpack());
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                m.insert(idx, unpack_cmp<native_elems>::run(v1.template extract<native_elems>(idx).unpack(),
                                                            v2.template extract<native_elems>(idx).unpack()));
            });

            return m;
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        return run(broadcast<T, Elems>::run(a), v);
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        return run(v, broadcast<T, Elems>::run(a));
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_zero_bits_impl<Op, 4, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T> / 2;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   next_type = utils::get_next_integer_type_t<T>;
    using   op_traits = compare_zero_traits<Op, T>;

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            using sub_op = cmp_zero_bits_impl<Op, 8, next_type, native_elems>;
            auto m = sub_op::run(v.template grow<native_elems>().unpack());
            return m.template extract<Elems>(0);
        }
        else {
            constexpr bool is_derived = op_traits::is_derived();
            constexpr CmpOp derived_op = op_traits::get_derived();
            using sub_op = cmp_zero_bits_impl<derived_op, 8, next_type, native_elems>;

            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                m.insert(idx, sub_op::run(v.template extract<native_elems>(idx).unpack()));
            });

            if constexpr (is_derived)
                return ~m;
            else
                return m;
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 8, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 8, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems < native_elems) {
            const uint64_t a = op(v1.template grow<native_elems>(), v2.template grow<native_elems>());

            return mask_type::from_uint32((unsigned)a);
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                m.insert(idx, mask<native_elems>::from_uint64(op(v1.template extract<native_elems>(idx),
                                                                 v2.template extract<native_elems>(idx))));
            });

            return m;
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_zero_bits_impl<Op, 8, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op, T>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems < native_elems) {
            const uint64_t a = op(v.template grow<64>());

            return mask_type::from_uint32((unsigned)a);
        }
        else {
            using sub_op = cmp_zero_bits_impl<Op, 8, T, native_elems>;

            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                m.insert(idx, sub_op::run(v.template extract<native_elems>(idx)));
            });

            return m;
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 16, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 16, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            const unsigned result = op(v1.template grow<32>(), v2.template grow<32>());

            return mask_type::from_uint32(result);
        }
        else {
            mask_type ret;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret.insert(idx, mask<native_elems>::from_uint32(op(v1.template extract<native_elems>(idx),
                                                                   v2.template extract<native_elems>(idx))));
            });

            return ret;
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_zero_bits_impl<Op, 16, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op, T>;

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            constexpr auto op = op_traits::get_op();
            const uint32_t a = op(v.template grow<native_elems>());

            return mask_type::from_uint32(a);
        }
        else {
            // Some operators such as ::ne negate the bits coming from another op (::eq)
            // since we need to mask some of the bits to combine the upper and lower 16bit,
            // we use the original operator instead and then perform the bitwise negation once the combination is done
            constexpr bool is_derived = op_traits::is_derived();
            constexpr CmpOp derived_op = op_traits::get_derived();
            using sub_op = cmp_zero_bits_impl<derived_op, 16, T, native_elems>;

            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                m.insert(idx, sub_op::run(v.template extract<native_elems>(idx)));
            });

            if constexpr (is_derived)
                return ~m;
            else
                return m;
        }
    }
};

#if __AIE_ARCH__ == 20
template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 16, bfloat16, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<bfloat16>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    static constexpr unsigned    acc_elems = 16;

    using           T = bfloat16;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, bfloat16>;
    using native_impl = cmp_bits_impl<Op, 16, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= acc_elems) {
            accum<accfloat, acc_elems> acc(v1.template grow<acc_elems>());
            accum<accfloat, acc_elems> zero = zeros_acc<AccumClass::FP, 32, acc_elems>::run();

            const vector<T, acc_elems> v1_1 = ::to_v16bfloat16(::add(acc, zero));

            const unsigned result = op(v1_1.template grow<native_elems>(), v2.template grow<native_elems>());

            return mask_type::from_uint32(result);
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                accum<accfloat, native_elems> acc(v1.template extract<native_elems>(idx));
                accum<accfloat, acc_elems> zero = zeros_acc<AccumClass::FP, 32, acc_elems>::run();

                const vector<T, acc_elems> v1_1 = ::to_v16bfloat16(::add(acc.template extract<acc_elems>(0), zero));
                const vector<T, acc_elems> v1_2 = ::to_v16bfloat16(::add(acc.template extract<acc_elems>(1), zero));

                m.insert(idx, mask<native_elems>::from_uint32(op(::concat(v1_1, v1_2), v2.template extract<native_elems>(idx))));
            });

            return m;
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, const T &a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};
#elif __AIE_ARCH__ == 21
template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl_float16_common
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);
    static constexpr unsigned    acc_elems = 64;

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 16, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            accum<accfloat, native_elems> acc(v1.template grow<native_elems>());
            accum<accfloat, native_elems> zero = zeros_acc<AccumClass::FP, 32, native_elems>::run();

            const accum<accfloat, acc_elems> acc_war = ::add(acc.template grow<acc_elems>(), zero.template grow<acc_elems>());

            const vector<T, native_elems> v1_1 = acc_war.template extract<native_elems>(0).template to_vector<T>();

            const unsigned result = op(v1_1, v2.template grow<native_elems>());

            return mask_type::from_uint32(result);
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                accum<accfloat, acc_elems> acc(v1.template extract<acc_elems>(idx));
                accum<accfloat, acc_elems> zero = zeros_acc<AccumClass::FP, 32, acc_elems>::run();

                const accum<accfloat, acc_elems> acc_war = ::add(acc, zero);

                const vector<T, native_elems> v1_1 = acc_war.template extract<native_elems>(0).template to_vector<T>();
                const vector<T, native_elems> v1_2 = acc_war.template extract<native_elems>(1).template to_vector<T>();

                const unsigned result1 = op(v1_1, v2.template extract<native_elems>(0));
                const unsigned result2 = op(v1_2, v2.template extract<native_elems>(1));

                m.insert(idx, mask<2 * native_elems>::from_uint32(result1, result2));
            });

            return m;
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 16, bfloat16, Elems> : public cmp_bits_impl_float16_common<Op, bfloat16, Elems> {};

#endif

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 32, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 32, T, native_elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (vector_type::is_complex()) {
            return cmp_bits_impl<Op, 32, int32, Elems>::run(v1.template cast_to<int32>(), v2.template cast_to<int32>());
        }
        else {
            if constexpr (Elems <= native_elems) {
                auto op = op_traits::get_op();
                const unsigned result = op(v1.template grow<native_elems>(), v2.template grow<native_elems>());

                return mask_type::from_uint32(result);
            }
            else {
                // Some operators such as ::ne negate the bits coming from another op (::eq)
                // since we need to mask some of the bits to combine the upper and lower 16bit,
                // we use the original operator instead and then perform the bitwise negation once the combination is done
                constexpr bool is_derived = op_traits::is_derived();
                constexpr auto op = op_traits::get_derived_op();

                mask_type m;

                utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                    const unsigned result1 = op(v1.template extract<16>(2 * idx + 0), v2.template extract<16>(2 * idx + 0));
                    const unsigned result2 = op(v1.template extract<16>(2 * idx + 1), v2.template extract<16>(2 * idx + 1));

                    m.insert(idx, mask<2 * native_elems>::from_uint32(result2 << 16 | result1));
                });

                if constexpr (is_derived)
                    return ~m;
                else
                    return m;
            }
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
    requires(!is_floating_point_v<T>)
struct cmp_zero_bits_impl<Op, 32, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op, T>;

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (vector_type::is_complex()) {
            return cmp_zero_bits_impl<Op, 32, int32, Elems>::run(v.template cast_to<int32>());
        }
        else {
            if constexpr (Elems <= native_elems) {
                constexpr auto op = op_traits::get_op();
                const unsigned result = op(v.template grow<16>());

                return mask_type::from_uint32(result);
            }
            else {
                // Some operators such as ::ne negate the bits coming from another op (::eq)
                // since we need to mask some of the bits to combine the upper and lower 16bit,
                // we use the original operator instead and then perform the bitwise negation once the combination is done
                constexpr bool is_derived = op_traits::is_derived();
                constexpr auto op = op_traits::get_derived_op();

                mask_type m;

                utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                    const unsigned result1 = op(v.template extract<native_elems>(2 * idx + 0));
                    const unsigned result2 = op(v.template extract<native_elems>(2 * idx + 1));

                    m.insert(idx, mask<2 * native_elems>::from_uint32(result2 << 16 | result1));
                });

                if constexpr (is_derived)
                    return ~m;
                else
                    return m;
            }
        }
    }
};

// We need this specialisation because comparisons against zero have support for
// an additional comparison mode (GT, and the complementary LE)
// None of the zero comparators are available for float, so we fall back to vector
// comparisons with the operators that are available there.
template <CmpOp Op, typename T, unsigned Elems>
    requires(is_floating_point_v<T>)
struct cmp_zero_bits_impl<Op, 32, T, Elems>
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Op == CmpOp::LE) {
            using base_impl = cmp_bits_impl<CmpOp::GE, 32, T, Elems>;
            return base_impl::run(zeros<T, Elems>::run(), v);
        }
        else if constexpr (Op == CmpOp::GT) {
            using base_impl = cmp_bits_impl<CmpOp::LT, 32, T, Elems>;
            return base_impl::run(zeros<T, Elems>::run(), v);
        }
        else {
            using base_impl = cmp_bits_impl<Op, 32, T, Elems>;
            return base_impl::run(v, zeros<T, Elems>::run());
        }
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 64, cint32, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<cint32>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 64, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            // TODO: investigate performance optimizations for vectors smaller than 16
            vector<int32, 16> v1_r = (v16int32)::shuffle(v1.template grow<8>(), vector<cint32, 8>(), DINTLV_lo_32o64);
            vector<int32, 16> v1_i = (v16int32)::shuffle(v1.template grow<8>(), vector<cint32, 8>(), DINTLV_hi_32o64);
            vector<int32, 16> v2_r = (v16int32)::shuffle(v2.template grow<8>(), vector<cint32, 8>(), DINTLV_lo_32o64);
            vector<int32, 16> v2_i = (v16int32)::shuffle(v2.template grow<8>(), vector<cint32, 8>(), DINTLV_hi_32o64);

            const unsigned result_r = op(v1_r, v2_r);
            const unsigned result_i = op(v1_i, v2_i);

            if constexpr (Op == CmpOp::NEQ)
                return mask_type::from_uint32(result_r | result_i);
            else
                return mask_type::from_uint32(result_r & result_i);
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                vector<int32, 16> v1_r = (v16int32)::shuffle(v1.template extract<8>(2 * idx + 0), v1.template extract<8>(2 * idx + 1), DINTLV_lo_32o64);
                vector<int32, 16> v1_i = (v16int32)::shuffle(v1.template extract<8>(2 * idx + 0), v1.template extract<8>(2 * idx + 1), DINTLV_hi_32o64);
                vector<int32, 16> v2_r = (v16int32)::shuffle(v2.template extract<8>(2 * idx + 0), v2.template extract<8>(2 * idx + 1), DINTLV_lo_32o64);
                vector<int32, 16> v2_i = (v16int32)::shuffle(v2.template extract<8>(2 * idx + 0), v2.template extract<8>(2 * idx + 1), DINTLV_hi_32o64);

                const unsigned result_r = op(v1_r, v2_r);
                const unsigned result_i = op(v1_i, v2_i);

                if constexpr (Op == CmpOp::NEQ)
                    m.insert(idx, mask<2 * native_elems>::from_uint32(result_r | result_i));
                else
                    m.insert(idx, mask<2 * native_elems>::from_uint32(result_r & result_i));
            });

            return m;
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(vals, v.template extract<native_elems>(idx)));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask_type ret;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret.template insert<native_elems>(idx, native_impl::run(v.template extract<native_elems>(idx), vals));
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_zero_bits_impl<Op, 64, cint32, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<cint32>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op, T>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            // TODO: investigate performance optimizations for vectors smaller than 16
            // TODO: use detail::unzip_complex
            auto lo = v.template cast_to<int32>().template grow<16>();
            vector<int32, 16> v_r = ::shuffle(lo, {}, DINTLV_lo_32o64);
            vector<int32, 16> v_i = ::shuffle(lo, {}, DINTLV_hi_32o64);

            const unsigned result_r = op(v_r);
            const unsigned result_i = op(v_i);

            if constexpr (Op == CmpOp::NEQ)
                return mask_type::from_uint32(result_r | result_i);
            else
                return mask_type::from_uint32(result_r & result_i);
        }
        else {
            mask_type m;

            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                auto [lo, hi] = v.template extract<16>(idx)
                                 .template cast_to<int32>()
                                 .template split<16>();
                vector<int32, 16> v_r = ::shuffle(lo, hi, DINTLV_lo_32o64);
                vector<int32, 16> v_i = ::shuffle(lo, hi, DINTLV_hi_32o64);

                const unsigned result_r = op(v_r);
                const unsigned result_i = op(v_i);

                if constexpr (Op == CmpOp::NEQ)
                    m.insert(idx, mask<16>::from_uint32(result_r | result_i));
                else
                    m.insert(idx, mask<16>::from_uint32(result_r & result_i));
            });

            return m;
        }
    }
};

template <CmpOp Op, unsigned TypeBits, typename T, unsigned Elems>
struct cmp_bits_impl_float_common
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if      constexpr (Op == CmpOp::NEQ) {
            return lt<T, Elems>::run(v1, v2) | lt<T, Elems>::run(v2, v1);
        }
        else if constexpr (Op == CmpOp::EQ) {
            return ge<T, Elems>::run(v1, v2) & ge<T, Elems>::run(v2, v1);
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if      constexpr (Op == CmpOp::NEQ) {
            return lt<T, Elems>::run(a, v) | lt<T, Elems>::run(v, a);
        }
        else if constexpr (Op == CmpOp::EQ) {
            return ge<T, Elems>::run(a, v) & ge<T, Elems>::run(v, a);
        }
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        if      constexpr (Op == CmpOp::NEQ) {
            return lt<T, Elems>::run(v, a) | lt<T, Elems>::run(a, v);
        }
        else if constexpr (Op == CmpOp::EQ) {
            return ge<T, Elems>::run(v, a) & ge<T, Elems>::run(a, v);
        }
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::EQ,  16, bfloat16, Elems> : public cmp_bits_impl_float_common<CmpOp::EQ,  16, bfloat16, Elems> {};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 16, bfloat16, Elems> : public cmp_bits_impl_float_common<CmpOp::NEQ, 16, bfloat16, Elems> {};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::EQ,  32, float,    Elems> : public cmp_bits_impl_float_common<CmpOp::EQ,  32, float,    Elems> {};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 32, float,    Elems> : public cmp_bits_impl_float_common<CmpOp::NEQ, 32, float,    Elems> {};

template <typename T, unsigned Elems>
struct equal_bits_impl<4, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T> / 2;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = equal_bits_impl<4, T, native_elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<16, uint16, Elems / 4>::run(v1.template cast_to<uint16>(), v2.template cast_to<uint16>());
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<8, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = equal_bits_impl<8, T, native_elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return equal_bits_impl<16, uint16, Elems / 2>::run(v1.template cast_to<uint16>(), v2.template cast_to<uint16>());
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<16, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = equal_bits_impl<16, T, native_elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        // Use the 16b implementation as masks are 32b and don't introduce EXTEND instructions
        if constexpr (Elems <= native_elems) {
            return eq<T, Elems>::run(v1, v2).full();
        }
        else {
            auto mask1 = eq<T, native_elems>::run(v1.template extract<native_elems>(0), v2.template extract<native_elems>(0));

            utils::unroll_for<unsigned, 1, num_ops>([&](unsigned idx) __aie_inline {
                const auto mask2 = eq<T, native_elems>::run(v1.template extract<native_elems>(idx), v2.template extract<native_elems>(idx));
                mask1 &= mask2;
            });

            return mask1.full();
        }
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<32, T, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using native_impl = equal_bits_impl<32, T, native_elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        // Use the 16b implementation as masks are 32b and don't introduce EXTEND instructions
        const vector<uint16, Elems * 2> tmp1 = vector_cast<uint16>(v1);
        const vector<uint16, Elems * 2> tmp2 = vector_cast<uint16>(v2);

        return equal_bits_impl<16, uint16, Elems * 2>::run(tmp1, tmp2);
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <unsigned Elems>
struct equal_bits_impl<32, float, Elems>
{
    using           T = float;
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using native_impl = equal_bits_impl<32, T, native_elems>;

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        return eq<T, Elems>::run(v1, v2).full();
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <typename T, unsigned Elems>
struct equal_bits_impl<64, T, Elems>
{
    using vector_type = vector<T, Elems>;

    static constexpr unsigned native_elems = native_vector_length_v<T>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using native_impl = equal_bits_impl<64, T, native_elems>;

    static_assert(vector_type::is_complex());

    __aie_inline
    static bool run(const vector_type &v1, const vector_type &v2)
    {
        // Use the 16b implementation as masks are 32b and don't introduce EXTEND instructions
        const vector<uint16, Elems * 4> tmp1 = vector_cast<uint16>(v1);
        const vector<uint16, Elems * 4> tmp2 = vector_cast<uint16>(v2);

        return equal_bits_impl<16, uint16, Elems * 4>::run(tmp1, tmp2);
    }

    __aie_inline
    static bool run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return ret;
        }
        else {
            return run(broadcast<T, Elems>::run(a), v);
        }
    }

    __aie_inline
    static bool run(const vector_type &v, T a)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            bool ret = true;

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                ret &= native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return ret;
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

}

#endif
