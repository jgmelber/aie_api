// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Expression templates types.
 */

#pragma once

#ifndef __AIE_API_EXPR__HPP__
#define __AIE_API_EXPR__HPP__

#include "aie_types.hpp"

namespace aie {

enum class Operation
{
    None,

    Acc_Add,
    Acc_Sub,

    Abs,
    Conj,
    Transpose,

    // Binary
    Max,
    Min,
    Sign,
    Zero,
};

template <typename Parent, Operation Op>
struct unary_op;

template <typename Parent1, typename Parent2, Operation Op>
struct binary_op;

template <typename T>
struct is_unary_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_unary_op<unary_op<Parent, Op>>
{
    static constexpr bool value = true;
};

template <typename T>
struct is_binary_op
{
    static constexpr bool value = false;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_binary_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = true;
};

template <typename T>
struct is_complex_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_complex_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_complex_v<typename unary_op<Parent, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_complex_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_complex_v<typename binary_op<Parent1, Parent2, Op>::result_type>;
};

template <typename T>
struct is_real_op
{
    static constexpr bool value = !is_complex_op<T>::value;
};

template <typename T>
struct is_elem_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_elem_op<unary_op<Parent, Op>>
{
    static constexpr bool value = unary_op<Parent, Op>::size() == 1;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_elem_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = binary_op<Parent1, Parent2, Op>::size() == 1;
};

template <typename T>
struct is_vector_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_vector_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_vector_v<typename unary_op<aie_dm_resource_remove_t<Parent>, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_vector_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_vector_v<typename binary_op<aie_dm_resource_remove_t<Parent1>,
                                                                         aie_dm_resource_remove_t<Parent2>, Op>::result_type>;
};

template <typename T>
struct is_sparse_vector_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_sparse_vector_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_sparse_vector_v<typename unary_op<aie_dm_resource_remove_t<Parent>, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_sparse_vector_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_sparse_vector_v<typename binary_op<aie_dm_resource_remove_t<Parent1>,
                                                                                aie_dm_resource_remove_t<Parent2>, Op>::result_type>;
};

template <typename T>
struct is_accum_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_accum_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_accum_v<typename unary_op<Parent, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_accum_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_accum_v<typename binary_op<Parent1, Parent2, Op>::result_type>;
};

template <typename T>
struct is_mmul_op
{
    static constexpr bool value = false;
};

template <typename T>
static constexpr bool is_unary_op_v  = is_unary_op<T>::value;

template <typename T>
static constexpr bool is_binary_op_v = is_binary_op<T>::value;

template <typename T>
static constexpr bool is_op_v = is_unary_op_v<T> || is_binary_op_v<T>;

template <typename T>
static constexpr bool is_complex_op_v  = is_complex_op<T>::value;

template <typename T>
static constexpr bool is_real_op_v  = is_real_op<T>::value;

template <typename T>
static constexpr bool is_elem_op_v   = is_elem_op<T>::value;

template <typename T>
static constexpr bool is_vector_op_v = is_vector_op<T>::value;

template <typename T>
static constexpr bool is_sparse_vector_op_v = is_sparse_vector_op<T>::value;

template <typename T>
static constexpr bool is_accum_op_v  = is_accum_op<T>::value;

template <typename T>
struct op_value_type_helper
{
    using type = T;
};

template <typename T, unsigned Elems>
struct op_value_type_helper<vector<T, Elems>>
{
    using type = typename vector<T, Elems>::value_type;
};

template <typename T, unsigned Elems>
struct op_value_type_helper<sparse_vector<T, Elems>>
{
    using type = typename sparse_vector<T, Elems>::value_type;
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct op_value_type_helper<vector_ref<T, Elems, Resource>>
{
    using type = typename vector_ref<T, Elems, Resource>::value_type;
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct op_value_type_helper<unaligned_vector_ref<T, Elems, Resource>>
{
    using type = typename unaligned_vector_ref<T, Elems, Resource>::value_type;
};

template <typename T, unsigned Elems>
struct op_value_type_helper<vector_elem_ref<T, Elems>>
{
    using type = typename vector_elem_ref<T, Elems>::value_type;
};

template <typename T, unsigned Elems>
struct op_value_type_helper<vector_elem_const_ref<T, Elems>>
{
    using type = typename vector_elem_const_ref<T, Elems>::value_type;
};

template <typename Parent, Operation Op>
struct op_value_type_helper<unary_op<Parent, Op>>
{
    using type = typename unary_op<Parent, Op>::value_type;
};

template <typename Parent1, typename Parent2, Operation Op>
struct op_value_type_helper<binary_op<Parent1, Parent2, Op>>
{
    using type = typename binary_op<Parent1, Parent2, Op>::value_type;
};

template <typename T, Operation Op>
struct op_result_helper
{
    using type = T;
};

template <typename Parent>
struct op_result_helper<Parent, Operation::None>
{
    using type = Parent;
};

template <typename T, unsigned Elems, aie_dm_resource Resource, Operation Op> requires(Op != Operation::None)
struct op_result_helper<vector_ref<T, Elems, Resource>, Op>
{
    using type = vector<T, Elems>;
};

template <typename T, unsigned Elems, aie_dm_resource Resource, Operation Op> requires(Op != Operation::None)
struct op_result_helper<unaligned_vector_ref<T, Elems, Resource>, Op>
{
    using type = vector<T, Elems>;
};

template <typename T, unsigned Elems, Operation Op> requires(Op != Operation::None)
struct op_result_helper<vector_elem_ref<T, Elems>, Op>
{
    using type = T;
};

template <typename T, unsigned Elems, Operation Op> requires(Op != Operation::None)
struct op_result_helper<vector_elem_const_ref<T, Elems>, Op>
{
    using type = T;
};

template <typename Parent, Operation ParentOp, Operation Op>
struct op_result_helper<unary_op<Parent, ParentOp>, Op>
{
    using type = typename op_result_helper<typename unary_op<Parent, ParentOp>::result_type, Op>::type;
};

template <typename Parent1, typename Parent2, Operation ParentOp, Operation Op>
struct op_result_helper<binary_op<Parent1, Parent2, ParentOp>, Op>
{
    using type = typename op_result_helper<typename binary_op<Parent1, Parent2, ParentOp>::result_type, Op>::type;
};

template <typename T, Operation Op>
using op_result_type_t = typename op_result_helper<T, Op>::type;

template <typename T>
using op_value_type_t = typename op_value_type_helper<aie_dm_resource_remove_t<T>>::type;

template <typename Parent, Operation Op>
struct unary_op_common
{
    using parent1_type = aie_dm_resource_remove_t<Parent>;
    using result_type = op_result_type_t<parent1_type, Op>;
    using  value_type = op_value_type_t<result_type>;

    static constexpr unsigned type_bits()
    {
        if constexpr (detail::is_valid_element_type_v<result_type> || detail::is_vector_elem_ref_v<result_type>)
            return detail::type_bits_v<result_type>;
        else
            return result_type::type_bits();
    }

    static constexpr unsigned size()
    {
        if constexpr (detail::is_valid_element_type_v<result_type> || detail::is_vector_elem_ref_v<result_type>)
            return 1;
        else
            return result_type::size();
    }

    static constexpr unsigned bits()
    {
        return type_bits() * size();
    }

    template <typename... Operations> requires(... && std::is_same_v<Operations, Operation>)
    static constexpr bool is_operation(Operation op, Operations... ops)
    {
        bool ret = Op != Operation::None && Op == op;

        if constexpr (sizeof...(Operations) > 0)
            ret = ret || is_operation(ops...);

        return ret;
    }

    template <typename... Operations> requires(... && std::is_same_v<Operations, Operation>)
    static constexpr bool is_operation_not(Operation op, Operations... ops)
    {
        bool ret = Op != Operation::None && Op != op;

        if constexpr (sizeof...(Operations) > 0)
            ret = ret && is_operation_not(ops...);

        return ret;
    }

    static constexpr bool is_operation_none()
    {
        return Op == Operation::None;
    }

    __aie_inline
    auto parent1() const //TODO: Use helper to decide between current + parent op, and drop redundant ones by returning parent's parent directly
    {
        if constexpr(is_op_v<parent1_type>)
            return parent_();
        else
            return parent_;
    }

    static constexpr Operation operation = Op;

    __aie_inline
    constexpr unary_op_common(const parent1_type parent) :
        parent_(parent)
    {
    }

private:
    const parent1_type parent_;
};

template <typename Parent, Operation Op>
struct unary_op;

#define UNARY_OP(op)                                                                   \
template <typename Parent>                                                             \
struct unary_op<Parent, Operation::op> : public unary_op_common<Parent, Operation::op> \
{                                                                                      \
    using parent1_type = Parent;                                                        \
    using result_type = op_result_type_t<parent1_type, Operation::op>;                  \
    using  value_type = op_value_type_t<result_type>;                                  \
                                                                                       \
    using unary_op_common<Parent, Operation::op>::unary_op_common;                     \
                                                                                       \
    result_type operator()() const;                                                    \
};

#define UNARY_OP_IMPL(op)                                                                                 \
template <typename Parent>                                                                                \
__aie_inline                                                                            \
typename unary_op<Parent, Operation::op>::result_type unary_op<Parent, Operation::op>::operator()() const

UNARY_OP(None)
UNARY_OP(Abs)
UNARY_OP(Conj)
UNARY_OP(Transpose)

UNARY_OP(Acc_Add)
UNARY_OP(Acc_Sub)

UNARY_OP_IMPL(None)
{
    return this->parent1();
}

template <typename Parent1, typename Parent2, Operation Op>
struct binary_op_common
{
    using parent1_type = aie_dm_resource_remove_t<Parent1>;
    using parent2_type = aie_dm_resource_remove_t<Parent2>;
    using  result_type = op_result_type_t<parent1_type, Op>; //TODO: Helper to correctly resolve result type as it won't always be parent1
    using  value_type = op_value_type_t<result_type>;

    static constexpr unsigned type_bits()
    {
        if constexpr (detail::is_valid_element_type_v<result_type> || detail::is_vector_elem_ref_v<result_type>)
            return detail::type_bits_v<result_type>;
        else
            return result_type::type_bits();
    }

    static constexpr unsigned size()
    {
        if constexpr (detail::is_valid_element_type_v<result_type> || detail::is_vector_elem_ref_v<result_type>)
            return 1;
        else
            return result_type::size();
    }

    static constexpr unsigned bits()
    {
        return type_bits() * size();
    }

    template <typename... Operations> requires(... && std::is_same_v<Operations, Operation>)
    static constexpr bool is_operation(Operation op, Operations... ops)
    {
        bool ret = Op == op;

        if constexpr (sizeof...(Operations) > 0)
            ret = ret || is_operation(ops...);

        return ret;
    }

    template <typename... Operations> requires(... && std::is_same_v<Operations, Operation>)
    static constexpr bool is_operation_not(Operation op, Operations... ops)
    {
        bool ret = Op != op;

        if constexpr (sizeof...(Operations) > 0)
            ret = ret && is_operation_not(ops...);

        return ret;
    }

    static constexpr bool is_operation_none()
    {
        return false;
    }

    __aie_inline
    auto parent1() const
    {
        if constexpr(is_op_v<parent1_type>) //TODO: Use helper to decide between current + parent op, and drop redundant ones by returning parent's parent directly
            return parent1_();
        else
            return parent1_;
    }

    __aie_inline
    auto parent2() const
    {
        if constexpr(is_op_v<parent2_type>)
            return parent2_();
        else
            return parent2_;
    }

    static constexpr Operation operation = Op;

    __aie_inline
    constexpr binary_op_common(const parent1_type parent1, const parent2_type parent2) :
        parent1_(parent1),
        parent2_(parent2)
    {
    }

private:
    const parent1_type parent1_;
    const parent2_type parent2_;
};

template <typename Parent1, typename Parent2, Operation Op>
struct binary_op;

#define BINARY_OP(op)                                                                                        \
                                                                                                             \
template <typename Parent1, typename Parent2>                                                                \
struct binary_op<Parent1, Parent2, Operation::op> : public binary_op_common<Parent1, Parent2, Operation::op> \
{                                                                                                            \
    using parent1_type = Parent1;                                                                            \
    using parent2_type = Parent2;                                                                            \
    using  result_type = op_result_type_t<parent1_type, Operation::op>;                                      \
    using   value_type = op_value_type_t<result_type>;                                                       \
                                                                                                             \
    using binary_op_common<Parent1, Parent2, Operation::op>::binary_op_common;                               \
                                                                                                             \
    result_type operator()() const;                                                                          \
};

#define BINARY_OP_IMPL(op)                                                                                                      \
template <typename Parent1, typename Parent2>                                                                                   \
__aie_inline                                                                                                  \
typename binary_op<Parent1, Parent2, Operation::op>::result_type binary_op<Parent1, Parent2, Operation::op>::operator()() const

BINARY_OP(Max)
BINARY_OP(Min)
BINARY_OP(Sign)
BINARY_OP(Zero)


}

#endif
