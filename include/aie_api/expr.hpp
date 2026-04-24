// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

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
    Neg,
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

#if AIE_API_ML_VERSION >= 210
template <typename T>
struct is_block_vector_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_block_vector_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_block_vector_v<typename unary_op<Parent, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_block_vector_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_block_vector_v<typename binary_op<Parent1, Parent2, Op>::result_type>;
};

#if __AIE_ARCH__ == 21
template <typename T>
struct is_bfp_vector_op
{
    static constexpr bool value = is_block_vector_op<T>::value;
};

template <typename Parent, Operation Op>
struct is_bfp_vector_op<unary_op<Parent, Op>>
{
    static constexpr bool value = is_block_vector_op<unary_op<Parent, Op>>::value;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_bfp_vector_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = is_block_vector_op<binary_op<Parent1, Parent2, Op>>::value;
};
#endif

#endif

template <typename T>
struct is_tbs_op
{
    static constexpr bool value = false;
};

template <typename Parent, Operation Op>
struct is_tbs_op<unary_op<Parent, Op>>
{
    static constexpr bool value = detail::is_tbs_v<typename unary_op<aie_dm_resource_remove_t<std::decay_t<Parent>>, Op>::result_type>;
};

template <typename Parent1, typename Parent2, Operation Op>
struct is_tbs_op<binary_op<Parent1, Parent2, Op>>
{
    static constexpr bool value = detail::is_tbs_v<typename binary_op<aie_dm_resource_remove_t<std::decay_t<Parent1>>,
                                                                      aie_dm_resource_remove_t<std::decay_t<Parent2>>, Op>::result_type>;
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
static constexpr bool is_block_vector_op_v = is_block_vector_op<T>::value;

template <typename T>
static constexpr bool is_tbs_op_v = is_tbs_op<T>::value;

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

#if AIE_API_ML_VERSION >= 210

template <typename T, unsigned Elems>
struct op_value_type_helper<block_vector<T, Elems>>
{
    using type = typename block_vector<T, Elems>::value_type;
};

#endif

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

template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc, detail::data_layout Layout,
          typename ResourceType, ResourceType Resource, detail::tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U>
struct op_value_type_helper<detail::tbs<T, Elems, Level, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>> 
{ 
    using type = typename detail::tbs<T, Elems, Level, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>::value_type;
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

template <typename T, unsigned Elems,
          unsigned Level, unsigned NumLevels, bool SlidingInner,
          typename IterDesc, detail::data_layout Layout,
          typename ResourceType, ResourceType Resource, detail::tbs_mode Mode,
          bool Restrict, bool Unaligned,
          typename U,
          Operation Op>
    requires(Op != Operation::None)
struct op_result_helper<detail::tbs<T, Elems, Level, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>, Op> 
{ 
    using type = detail::tbs<T, Elems, Level, NumLevels, SlidingInner, IterDesc, Layout, ResourceType, Resource, Mode, Restrict, Unaligned, U>;
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

    static constexpr Operation operation = Op;
};

// Unary common with the input as a reference
template <typename Parent, Operation Op>
struct unary_op_common_ref : public unary_op_common<Parent, Op>
{
    using parent1_type = aie_dm_resource_remove_t<Parent>;

    __aie_inline
    auto &parent1()
    {
        if constexpr(is_op_v<parent1_type>)
            return parent_.parent1();
        else
            return parent_;
    }

    __aie_inline
    constexpr unary_op_common_ref(parent1_type & parent1) :
        parent_(parent1)
    {
    }

private:
    parent1_type & parent_;
};

// Unary common with the input as a constant value
template <typename Parent, Operation Op>
struct unary_op_common_const : public unary_op_common<Parent, Op>
{
    using parent1_type = aie_dm_resource_remove_t<Parent>;

    __aie_inline
    auto parent1() const
    {
        if constexpr(is_op_v<parent1_type>)
            return parent_.parent1();
        else
            return parent_;
    }

    __aie_inline
    constexpr unary_op_common_const(const parent1_type parent1) :
        parent_(parent1)
    {
    }

private:
    const parent1_type parent_;
};

template <typename Parent, Operation Op>
struct unary_op;

#define UNARY_OP(op)                                                                         \
template <typename Parent>                                                                   \
struct unary_op<Parent, Operation::op> : public unary_op_common_const<Parent, Operation::op> \
{                                                                                            \
    using parent1_type = Parent;                                                             \
    using result_type = op_result_type_t<parent1_type, Operation::op>;                       \
    using  value_type = op_value_type_t<result_type>;                                        \
                                                                                             \
    using unary_op_common_const<Parent, Operation::op>::unary_op_common_const;               \
                                                                                             \
    result_type operator()() const;                                                          \
};

#define UNARY_OP_IMPL(op)                                                                                 \
template <typename Parent>                                                                                \
__aie_inline                                                                                              \
typename unary_op<Parent, Operation::op>::result_type unary_op<Parent, Operation::op>::operator()() const

UNARY_OP(None)
UNARY_OP(Abs)
UNARY_OP(Conj)
UNARY_OP(Transpose)

UNARY_OP(Acc_Add)
UNARY_OP(Acc_Sub)

// Unary none operation implementation for tensor buffer stream
// Placed below UNARY_OP(None) to avoid issues in Native compilation
template <TensorBufferStream Parent>
struct unary_op<Parent, Operation::None> : public unary_op_common_ref<Parent, Operation::None>
{
    using parent1_type = Parent;
    using result_type = op_result_type_t<parent1_type, Operation::None>;
    using value_type = op_value_type_t<result_type>;
    using elem_type = typename Parent::elem_type;

    using unary_op_common_ref<Parent, Operation::None>::unary_op_common_ref;

    result_type operator()() const 
    {
        return this->parent1();
    }
};

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

    static constexpr Operation operation = Op;
};

// Binary common with the first input as a reference
template <typename Parent1, typename Parent2, Operation Op>
struct binary_op_common_ref : public binary_op_common<Parent1, Parent2, Op>
{
    using parent1_type = aie_dm_resource_remove_t<Parent1>;
    using parent2_type = aie_dm_resource_remove_t<Parent2>;

    __aie_inline
    auto &parent1()
    {
        if constexpr(is_op_v<parent1_type>)
            return parent1_.parent1();
        else
            return parent1_;
    }

    __aie_inline
    auto parent2() const
    {
        if constexpr(is_op_v<parent2_type>)
            return parent2_.parent2();
        else
            return parent2_;
    }

    __aie_inline
    constexpr binary_op_common_ref(parent1_type & parent1, const parent2_type parent2) :
        parent1_(parent1),
        parent2_(parent2)
    {
    }

private:
    parent1_type & parent1_;
    const parent2_type parent2_;
};

// Binary common with the first input as a constant value
template <typename Parent1, typename Parent2, Operation Op>
struct binary_op_common_const : public binary_op_common<Parent1, Parent2, Op>
{
    using parent1_type = aie_dm_resource_remove_t<Parent1>;
    using parent2_type = aie_dm_resource_remove_t<Parent2>;

    __aie_inline
    auto parent1() const
    {
        if constexpr(is_op_v<parent1_type>)
            return parent1_.parent1();
        else
            return parent1_;
    }

    __aie_inline
    auto parent2() const
    {
        if constexpr(is_op_v<parent2_type>)
            return parent2_.parent2();
        else
            return parent2_;
    }

    __aie_inline
    constexpr binary_op_common_const(const parent1_type parent1, const parent2_type parent2) :
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

// Binary sign operation implementation for tensor buffer stream
template <TensorBufferStream Parent1, typename Parent2>
struct binary_op<Parent1, Parent2, Operation::Sign> : public binary_op_common_ref<Parent1, Parent2, Operation::Sign>
{
    using parent1_type = Parent1;
    using parent2_type = Parent2;
    using result_type = op_result_type_t<parent1_type, Operation::Sign>;
    using value_type = op_value_type_t<result_type>;
    using elem_type = typename Parent1::elem_type;

    using binary_op_common_ref<Parent1, Parent2, Operation::Sign>::binary_op_common_ref;

    result_type operator()() const;
};

#define BINARY_OP(op)                                                                                              \
                                                                                                                   \
template <typename Parent1, typename Parent2>                                                                      \
struct binary_op<Parent1, Parent2, Operation::op> : public binary_op_common_const<Parent1, Parent2, Operation::op> \
{                                                                                                                  \
    using parent1_type = Parent1;                                                                                  \
    using parent2_type = Parent2;                                                                                  \
    using  result_type = op_result_type_t<parent1_type, Operation::op>;                                            \
    using   value_type = op_value_type_t<result_type>;                                                             \
                                                                                                                   \
    using binary_op_common_const<Parent1, Parent2, Operation::op>::binary_op_common_const;                         \
                                                                                                                   \
    result_type operator()() const;                                                                                \
};

#define BINARY_OP_IMPL(op)                                                                                                      \
template <typename Parent1, typename Parent2>                                                                                   \
__aie_inline                                                                                                                    \
typename binary_op<Parent1, Parent2, Operation::op>::result_type binary_op<Parent1, Parent2, Operation::op>::operator()() const

BINARY_OP(Max)
BINARY_OP(Min)
BINARY_OP(Sign)
BINARY_OP(Zero)
BINARY_OP(Neg)


}

#endif
