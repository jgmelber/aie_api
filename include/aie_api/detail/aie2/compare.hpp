// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_COMPARE__HPP__
#define __AIE_API_DETAIL_AIE2_COMPARE__HPP__

#include <algorithm>

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

    static constexpr auto get_post_op()
    {
        if constexpr (Op == CmpOp::NEQ)
            return std::bit_not<>{};
        else
            return std::identity{};
    }
};

template <CmpOp Op>
struct compare_zero_traits {
    static constexpr auto get_op()
    {
             if constexpr (Op == CmpOp::LT)  return [](auto a) __aie_inline { return ::ltz(a); };
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
        if      constexpr (Op == CmpOp::GE)  return true;
        else if constexpr (Op == CmpOp::NEQ) return true;
        else if constexpr (Op == CmpOp::LE)  return true;
        return false;
    }

    static constexpr CmpOp get_derived()
    {
        if      constexpr (Op == CmpOp::GE)  return CmpOp::LT;
        else if constexpr (Op == CmpOp::NEQ) return CmpOp::EQ;
        else if constexpr (Op == CmpOp::LE)  return CmpOp::GT;
        return Op;
    }

    static constexpr auto get_derived_op()
    {
        return compare_zero_traits<get_derived()>::get_op();
    }

    static constexpr auto get_post_op()
    {
        if constexpr (is_derived())
            return std::bit_not<>{};
        else
            return std::identity{};
    }
};

template <CmpOp Op, unsigned TypeBits, typename T, unsigned Elems>
    requires (TypeBits < 8)
struct cmp_bits_impl<Op, TypeBits, T, Elems>
{
    static constexpr unsigned subbyte_elems = 8 / TypeBits;
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems> / subbyte_elems;
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
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = unpack_cmp<native_elems>::run(v1.template extract<native_elems>(idx).unpack(),
                                                           v2.template extract<native_elems>(idx).unpack());
            });

            return mask_type::from_masks(masks);
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

template <CmpOp Op, unsigned TypeBits, typename T, unsigned Elems>
    requires (TypeBits < 8)
struct cmp_zero_bits_impl<Op, TypeBits, T, Elems>
{
    static constexpr unsigned subbyte_elems = 8 / TypeBits;
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems> / subbyte_elems;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   next_type = utils::get_next_integer_type_t<T>;
    using   op_traits = compare_zero_traits<Op>;

    // Some operators such as ::ne negate the bits coming from another op (::eq) since we may need to combine multiple
    // masks together, we use the original operator instead and then perform the bitwise negation once the combination
    // is done
    using native_impl = cmp_zero_bits_impl<op_traits::get_derived(), 8, next_type, native_elems>;
    static constexpr auto derived_post_op = op_traits::get_post_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems <= native_elems) {
            auto m = native_impl::run(v.template grow<native_elems>().unpack());
            return derived_post_op(m.template extract<Elems>(0));
        }
        else {
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx).unpack());
            });

            return derived_post_op(mask_type::from_masks(masks));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 8, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using      vector_type = vector<T, Elems>;
    using        mask_type = mask<Elems>;
    using        op_traits = compare_vector_traits<Op, T>;
    using      native_impl = cmp_bits_impl<Op, 8, T, native_elems>;


    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems <= native_elems) {
            const auto a = op(v1.template grow<native_elems>(), v2.template grow<native_elems>());

                return mask_type::from_uint64(a);
        }
        else {
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v1.template extract<native_elems>(idx),
                                              v2.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
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

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return mask_type::from_masks(masks);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_zero_bits_impl<Op, 8, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using      vector_type = vector<T, Elems>;
    using        mask_type = mask<Elems>;
    using native_mask_type = mask<native_elems>;
    using        op_traits = compare_zero_traits<Op>;
    using        native_op = cmp_zero_bits_impl<Op, 8, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems < native_elems) {
            native_mask_type result = native_op::run(v.template grow<native_elems>());
            return result.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const auto a = op(v);

                return mask_type::from_uint64(a);
        }
        else {
            native_mask_type masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_op::run(v.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 16, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 16, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        if constexpr (Elems < native_elems) {
            mask<native_elems> result = native_impl::run(v1.template grow<native_elems>(),
                                                         v2.template grow<native_elems>());

            return result.template extract<Elems>(0);
        }
        else if constexpr (Elems == native_elems) {
            const auto result = op(v1, v2);

            // Compare instructions for less than 32 vector lanes have all insignificant bits set to zero
            static_assert(native_elems <= 64);
            if constexpr (native_elems <= 32)
                return mask_type::from_uint32(assume_zero_padding_t{}, result);
            else
                return mask_type::from_uint64(assume_zero_padding_t{}, result);
        }
        else {
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v1.template extract<native_elems>(idx),
                                              v2.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
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

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return mask_type::from_masks(masks);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_zero_bits_impl<Op, 16, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op>;

    // Some operators such as ::ne negate the bits coming from another op (::eq) since we may need to combine multiple
    // masks together, we use the original operator instead and then perform the bitwise negation once the combination
    // is done
    using native_impl = cmp_zero_bits_impl<op_traits::get_derived(), 16, T, native_elems>;
    static constexpr auto derived_post_op = op_traits::get_post_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (Elems < native_elems) {
            auto result = native_impl::run(v.template grow<native_elems>());
            return derived_post_op(result.template extract<Elems>(0));
        } else if constexpr (Elems == native_elems) {
            constexpr auto op = op_traits::get_op();
            const auto a = op(v);

            static_assert(native_elems <= 64);
            if constexpr (native_elems <= 32)
                return mask_type::from_uint32(assume_zero_padding_t{}, a);
            else
                return mask_type::from_uint64(assume_zero_padding_t{}, a);
        }
        else {
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx));
            });

            return derived_post_op(mask_type::from_masks(masks));
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
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                accum<accfloat, native_elems> acc(v1.template extract<native_elems>(idx));
                accum<accfloat, acc_elems> zero = zeros_acc<AccumClass::FP, 32, acc_elems>::run();

                const vector<T, acc_elems> v1_1 = ::to_v16bfloat16(::add(acc.template extract<acc_elems>(0), zero));
                const vector<T, acc_elems> v1_2 = ::to_v16bfloat16(::add(acc.template extract<acc_elems>(1), zero));

                masks[idx] = mask<native_elems>::from_uint32(op(::concat(v1_1, v1_2), v2.template extract<native_elems>(idx)));
            });

            return mask_type::from_masks(masks);
        }
    }

    __aie_inline
    static mask_type run(const T &a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
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

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return mask_type::from_masks(masks);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};
#elif __AIE_ARCH__ == 21 ||  __AIE_ARCH__ == 22
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
            mask<native_elems> masks[num_ops];

            auto out_it = std::begin(masks);
            utils::unroll_times<num_ops / 2>([&](unsigned idx) __aie_inline {
                accum<accfloat, acc_elems> acc(v1.template extract<acc_elems>(idx));
                accum<accfloat, acc_elems> zero = zeros_acc<AccumClass::FP, 32, acc_elems>::run();

                const accum<accfloat, acc_elems> acc_war = ::add(acc, zero);

                vector<T, native_elems> v1_1, v1_2, v2_1, v2_2;
                v1_1 = acc_war.template extract<native_elems>(0).template to_vector<T>();
                v1_2 = acc_war.template extract<native_elems>(1).template to_vector<T>();

                std::tie(v2_1, v2_2) = v2.template extract<acc_elems>(idx).template split<native_elems>();

                const unsigned result1 = op(v1_1, v2_1);
                const unsigned result2 = op(v1_2, v2_2);

                *out_it++ = mask<native_elems>::from_uint32(assume_zero_padding_t{}, result1);
                *out_it++ = mask<native_elems>::from_uint32(assume_zero_padding_t{}, result2);
            });

            return mask_type::from_masks(masks);
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return mask_type::from_masks(masks);
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

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return mask_type::from_masks(masks);
        }
        else {
            return run(v, broadcast<T, Elems>::run(a));
        }
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 16, bfloat16, Elems> : public cmp_bits_impl_float16_common<Op, bfloat16, Elems> {};

#if __AIE_API_FP16_SUPPORT__
template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 16,  float16, Elems> : public cmp_bits_impl_float16_common<Op,  float16, Elems> {};
#endif

#endif

template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl<Op, 32, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;

    // Some operators such as ::ne negate the bits coming from another op (::eq) since we may need to combine multiple
    // masks together, we use the original operator instead and then perform the bitwise negation once the combination
    // is done
    using native_impl = cmp_bits_impl<op_traits::get_derived(), 32, T, native_elems>;
    static constexpr auto derived_post_op = op_traits::get_post_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {

        if constexpr (vector_type::is_complex()) {
            return cmp_bits_impl<Op, 32, int32, Elems>::run(v1.template cast_to<int32>(), v2.template cast_to<int32>());
        }
        else if constexpr (Elems < native_elems) {
            mask<native_elems> result = native_impl::run(v1.template grow<native_elems>(),
                                                         v2.template grow<native_elems>());
            return derived_post_op(result.template extract<Elems>(0));
        }
        else if constexpr (Elems == native_elems) {
            auto op = op_traits::get_op();
            const auto result = op(v1, v2);

            static_assert(native_elems <= 64);
            if constexpr (native_elems <= 32)
                return mask_type::from_uint32(assume_zero_padding_t{}, result);
            else
                return mask_type::from_uint64(assume_zero_padding_t{}, result);
        }
        else {
            mask<native_elems> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v1.template extract<native_elems>(idx),
                                              v2.template extract<native_elems>(idx));
            });

            return derived_post_op(mask_type::from_masks(masks));
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        if constexpr (Elems > native_elems) {
            const vector<T, native_elems> vals = broadcast<T, native_elems>::run(a);

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(vals, v.template extract<native_elems>(idx));
            });

            return derived_post_op(mask_type::from_masks(masks));
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

            mask<native_elems> masks[num_ops];

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx), vals);
            });

            return derived_post_op(mask_type::from_masks(masks));
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
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / native_elems);

    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op>;

    // Some operators such as ::ne negate the bits coming from another op (::eq) since we may need to combine multiple
    // masks together, we use the original operator instead and then perform the bitwise negation once the combination
    // is done
    using native_impl = cmp_zero_bits_impl<op_traits::get_derived(), 32, T, native_elems>;
    static constexpr auto derived_post_op = op_traits::get_post_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        if constexpr (vector_type::is_complex()) {
            return cmp_zero_bits_impl<Op, 32, int32, Elems>::run(v.template cast_to<int32>());
        }
        else if constexpr (Elems < native_elems) {
            mask<native_elems> result = native_impl::run(v.template grow<native_elems>());
            return derived_post_op(result.template extract<Elems>(0));
        }
        else if constexpr (Elems == native_elems) {
            constexpr auto op = op_traits::get_op();
            const auto result = op(v);

            static_assert(native_elems <= 64);
            if constexpr (native_elems <= 32)
                return mask_type::from_uint32(assume_zero_padding_t{}, result);
            else
                return mask_type::from_uint64(assume_zero_padding_t{}, result);
        }
        else {

            mask<native_elems> masks[num_ops];
            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                masks[idx] = native_impl::run(v.template extract<native_elems>(idx));
            });

            return derived_post_op(mask_type::from_masks(masks));
        }
    }
};

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
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
#endif

#if __AIE_API_CINT_SUPPORT__
template <CmpOp Op, unsigned Elems>
struct cmp_bits_impl<Op, 64, cint32, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<cint32>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / 16);

    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_vector_traits<Op, T>;
    using native_impl = cmp_bits_impl<Op, 64, T, native_elems>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        constexpr auto base_op = op_traits::get_derived_op();
        constexpr auto post_op = op_traits::get_post_op();
        if constexpr (Elems <= native_elems) {
            // TODO: investigate performance optimizations for vectors smaller than 16
            vector<int32, native_elems> v1_r, v1_i, v2_r, v2_i;
            std::tie(v1_r, v1_i) = unzip_complex(v1.template grow<native_elems>());
            std::tie(v2_r, v2_i) = unzip_complex(v2.template grow<native_elems>());

            const unsigned result_r = base_op(v1_r.template grow<16>(), v2_r.template grow<16>());
            const unsigned result_i = base_op(v1_i.template grow<16>(), v2_i.template grow<16>());

            return post_op(mask_type::from_uint32(result_r & result_i));
        }
        else {
            mask<16> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                vector<int32, 16> v1_r, v1_i, v2_r, v2_i;
                std::tie(v1_r, v1_i) = unzip_complex(v1.template extract<16>(idx));
                std::tie(v2_r, v2_i) = unzip_complex(v2.template extract<16>(idx));

                const unsigned result_r = base_op(v1_r, v2_r);
                const unsigned result_i = base_op(v1_i, v2_i);

                masks[idx] = mask<16>::from_uint32(assume_zero_padding_t{}, result_r & result_i);
            });

            return post_op(mask_type::from_masks(masks));
        }
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        constexpr unsigned base_elems   = native_vector_length_v<int32>;
        constexpr unsigned base_num_ops = std::max(1u, Elems / base_elems);
        constexpr auto post_op          = op_traits::get_post_op();

        using base_impl = cmp_bits_impl<op_traits::get_derived(), 32, int32, base_elems>;

        mask<base_elems> masks[base_num_ops];
        utils::unroll_times<base_num_ops>([&](unsigned idx) __aie_inline {
            auto [real, imag] = unzip_complex(v.template grow_extract<base_elems>(idx));
            masks[idx] = base_impl::run(a.real, real)
                       & base_impl::run(a.imag, imag);
        });

        if constexpr (Elems < base_elems)
            return post_op(masks[0].template extract<Elems>(0));
        else
            return post_op(mask_type::from_masks(masks));
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        constexpr unsigned base_elems   = native_vector_length_v<int32>;
        constexpr unsigned base_num_ops = std::max(1u, Elems / base_elems);
        constexpr auto post_op          = op_traits::get_post_op();

        using base_impl = cmp_bits_impl<op_traits::get_derived(), 32, int32, base_elems>;

        mask<base_elems> masks[base_num_ops];
        utils::unroll_times<base_num_ops>([&](unsigned idx) __aie_inline {
            auto [real, imag] = unzip_complex(v.template grow_extract<base_elems>(idx));
            masks[idx] = base_impl::run(real, a.real)
                       & base_impl::run(imag, a.imag);
        });

        if constexpr (Elems < base_elems)
            return post_op(masks[0].template extract<Elems>(0));
        else
            return post_op(mask_type::from_masks(masks));
    }
};

template <CmpOp Op, unsigned Elems>
struct cmp_zero_bits_impl<Op, 64, cint32, Elems>
{
    static constexpr unsigned native_elems = native_vector_length_v<cint32>;
    static constexpr unsigned      num_ops = std::max(1u, Elems / 16);

    using           T = cint32;
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;
    using   op_traits = compare_zero_traits<Op>;

    static constexpr auto op = op_traits::get_op();

    __aie_inline
    static mask_type run(const vector_type &v)
    {
        constexpr auto base_op = op_traits::get_derived_op();
        constexpr auto post_op = op_traits::get_post_op();

        if constexpr (Elems <= native_elems) {
            // TODO: investigate performance optimizations for vectors smaller than 16
            auto [v_r, v_i] = unzip_complex(v.template grow<native_elems>());

            const unsigned result_r = base_op(v_r.template grow<16>());
            const unsigned result_i = base_op(v_i.template grow<16>());

            return post_op(mask_type::from_uint32(result_r & result_i));
        }
        else {
            mask<16> masks[num_ops];

            utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
                auto [v_r, v_i] = unzip_complex(v.template extract<16>(idx));

                const unsigned result_r = base_op(v_r);
                const unsigned result_i = base_op(v_i);

                masks[idx] = mask<16>::from_uint32(assume_zero_padding_t{}, result_r & result_i);
            });

            return post_op(mask_type::from_masks(masks));
        }
    }
};
#endif

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
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
#endif

#if __AIE_API_CFP32_SUPPORT__
template <CmpOp Op, typename T, unsigned Elems>
struct cmp_bits_impl_cplx_float
{
    using vector_type = vector<T, Elems>;
    using   mask_type = mask<Elems>;

    static constexpr unsigned tmp_elems = std::max(512 / type_bits_v<T>, Elems);
    using mask_tmp_type = mask<tmp_elems>;

    using component_type = decltype(T().real);

    __aie_inline
    static mask_type run(const vector_type &v1, const vector_type &v2)
    {
        auto [v1_real, v1_imag] = interleave_unzip<component_type, tmp_elems>::run(
            v1.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(0),
            v1.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(1),
            1
        );

        auto [v2_real, v2_imag] = interleave_unzip<component_type, tmp_elems>::run(
            v2.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(0),
            v2.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(1),
            1
        );

        mask_tmp_type m1 = cmp_impl<Op, component_type, tmp_elems>::run(v1_real, v2_real);
        mask_tmp_type m2 = cmp_impl<Op, component_type, tmp_elems>::run(v1_imag, v2_imag);

        if      constexpr (Op == CmpOp::EQ)
            return mask_type::from_uint32(m1.to_uint32(0) & m2.to_uint32(0));
        else if constexpr (Op == CmpOp::NEQ)
            return mask_type::from_uint32(m1.to_uint32(0) | m2.to_uint32(0));
    }

    __aie_inline
    static mask_type run(T a, const vector_type &v)
    {
        auto [v_real, v_imag] = interleave_unzip<component_type, tmp_elems>::run(
            v.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(0),
            v.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(1),
            1
        );

        mask_tmp_type m1 = cmp_impl<Op, component_type, tmp_elems>::run(a.real, v_real);
        mask_tmp_type m2 = cmp_impl<Op, component_type, tmp_elems>::run(a.imag, v_imag);

        if      constexpr (Op == CmpOp::EQ)
            return mask_type::from_uint32(m1.to_uint32(0) & m2.to_uint32(0));
        else if constexpr (Op == CmpOp::NEQ)
            return mask_type::from_uint32(m1.to_uint32(0) | m2.to_uint32(0));
    }

    __aie_inline
    static mask_type run(const vector_type &v, T a)
    {
        auto [v_real, v_imag] = interleave_unzip<component_type, tmp_elems>::run(
            v.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(0),
            v.template grow<tmp_elems>().template cast_to<component_type>().template extract<tmp_elems>(1),
            1
        );

        mask_tmp_type m1 = cmp_impl<Op, component_type, tmp_elems>::run(v_real, a.real);
        mask_tmp_type m2 = cmp_impl<Op, component_type, tmp_elems>::run(v_imag, a.imag);

        if      constexpr (Op == CmpOp::EQ)
            return mask_type::from_uint32(m1.to_uint32(0) & m2.to_uint32(0));
        else if constexpr (Op == CmpOp::NEQ)
            return mask_type::from_uint32(m1.to_uint32(0) | m2.to_uint32(0));
    }
};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::EQ,  64, cfloat,    Elems> : public cmp_bits_impl_cplx_float<CmpOp::EQ,  cfloat, Elems> {};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 64, cfloat,    Elems> : public cmp_bits_impl_cplx_float<CmpOp::NEQ, cfloat, Elems> {};

#if __AIE_API_CBF16_SUPPORT__
template <unsigned Elems>
struct cmp_bits_impl<CmpOp::EQ,  32, cbfloat16, Elems> : public cmp_bits_impl_cplx_float<CmpOp::EQ,  cbfloat16, Elems> {};

template <unsigned Elems>
struct cmp_bits_impl<CmpOp::NEQ, 32, cbfloat16, Elems> : public cmp_bits_impl_cplx_float<CmpOp::NEQ, cbfloat16, Elems> {};
#endif
#endif

template <typename T, unsigned Elems>
struct equal_bits_impl<4, T, Elems>
{
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems> / 2;
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
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
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
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
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
    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
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

    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
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

    static constexpr unsigned native_elems = max_intrinsic_vector_elems_v<T, Elems>;
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
