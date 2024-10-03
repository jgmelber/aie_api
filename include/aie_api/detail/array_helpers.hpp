// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_ARRAY_HELPERS__HPP__
#define __AIE_API_DETAIL_ARRAY_HELPERS__HPP__

#include <cstddef>
#include <type_traits>

#include "vector.hpp"
#include "../sparse_vector.hpp"
#include "../vector_elem_ref.hpp"

namespace aie {

inline constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

}

namespace aie::detail {

template <typename T>
struct get_value_type
{
    using type = T;
};

template <typename T, unsigned Elems>
struct get_value_type<vector<T, Elems>>
{
    using type = typename vector<T, Elems>::value_type;
};

template <typename T>
using get_value_type_t = typename get_value_type<T>::type;

template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource = aie_dm_resource::none>
    requires (arch::is(arch::AIE_ML))
class sliding_window_buffer_stream;

template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource = aie_dm_resource::none>
    requires (arch::is(arch::AIE_ML))
using          const_sliding_window_buffer_stream = sliding_window_buffer_stream<const std::remove_const_t<T>, Elems, IterDescriptor, Resource>;

template <typename T, unsigned Elems, unsigned Level, typename TensorIterDescriptor, aie_dm_resource Resource, bool Restrict = false>
#if __AIE_API_SUPPORTED_FRIEND_CONCEPTS__
    requires (arch::is(arch::AIE_ML))
#endif
class __AIE_API_KEEP_IN_REGISTERS__ tensor_buffer_stream;

struct dim_2d
{
    unsigned num1; int inc1; int inc2;

    constexpr dim_2d() = default;

    constexpr dim_2d(unsigned num1, int inc1, int inc2) : num1(num1), inc1(inc1), inc2(inc2) {}
};

struct dim_3d
{
    unsigned num1; int inc1; unsigned num2; int inc2; int inc3;

    constexpr dim_3d() = default;

    constexpr dim_3d(unsigned num1, int inc1, unsigned num2, int inc2, int inc3) : num1(num1), inc1(inc1), num2(num2), inc2(inc2), inc3(inc3) {}
};

struct sliding_window_dim_1d
{
    int inc1;

    constexpr sliding_window_dim_1d() = default;

    constexpr sliding_window_dim_1d(int step) :
        inc1(step)
    {}

    operator int() { return inc1; }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

struct sliding_window_dim_2d
{
    unsigned num1; int inc1; int inc2;

    constexpr sliding_window_dim_2d() = default;

    constexpr sliding_window_dim_2d(unsigned num1, int step,
                                                   int inc2) :
        num1(num1), inc1(step),       inc2(step + inc2)
    {}

    explicit constexpr sliding_window_dim_2d(const dim_2d& other) :
        sliding_window_dim_2d(other.num1, other.inc1, other.inc2) {}

    constexpr sliding_window_dim_2d(const sliding_window_dim_2d& other) = default;

    operator dim_2d() { return dim_2d(num1, inc1, inc2); }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

struct sliding_window_dim_3d
{
    unsigned num1; int inc1; unsigned num2; int inc2; int inc3;

    constexpr sliding_window_dim_3d() = default;

    constexpr sliding_window_dim_3d(unsigned num1, int step,
                                    unsigned num2, int inc2,
                                                   int inc3) :
        num1(num1), inc1(step),       num2(num2), inc2(step + inc2),       inc3(step + inc3)
    {}

    explicit constexpr sliding_window_dim_3d(const dim_3d& other) :
        sliding_window_dim_3d(other.num1, other.inc1, other.num2, other.inc2, other.inc3) {}

    constexpr sliding_window_dim_3d(const sliding_window_dim_3d& other) = default;

    operator dim_3d() { return dim_3d(num1, inc1, num2, inc2, inc3); }

    template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    friend class sliding_window_vector_stream;
};

template <typename... Args>
struct compute_rank
{
    template <typename T>
    struct compute_rank_helper
    {
        using U = std::decay_t<T>;
        static constexpr unsigned value = utils::is_one_of_v<U, dim_3d, sliding_window_dim_3d> ? 3
                                        : utils::is_one_of_v<U, dim_2d, sliding_window_dim_2d> ? 2
                                        : 1;
    };
    static constexpr unsigned value = (compute_rank_helper<Args>::value + ...);
};

template <typename... Args>
static constexpr unsigned compute_rank_v = compute_rank<Args...>::value;

template <unsigned N> struct default_repr;
template <>           struct default_repr<0> { using type = std::tuple<>; };
template <>           struct default_repr<1> { using type = std::tuple<int>; };
template <>           struct default_repr<2> { using type = std::tuple<dim_2d>; };
template <unsigned N> struct default_repr    { using type = decltype(std::tuple_cat(std::declval<std::tuple<dim_3d>>(),
                                                                                    std::declval<typename default_repr<N-3>::type>())); };
template <unsigned N> using default_repr_t = typename default_repr<N>::type;

template <typename U>
struct iter_state
{
    struct state_1d {};
    struct state_2d { addr_t c1 = 0; };
    struct state_3d { addr_t c1 = 0, c2 = 0; };
    using storage_t = std::conditional_t<utils::is_one_of_v<U, dim_3d, sliding_window_dim_3d>,
                                        state_3d,
                                        std::conditional_t<utils::is_one_of_v<U, dim_2d, sliding_window_dim_2d>,
                                                          state_2d,
                                                          state_1d>>;
    storage_t state_;
};

template <typename T, unsigned Elems> struct tensor_vector_type { using type = vector<T, Elems>; };
template <typename T, unsigned Elems> using tensor_vector_type_t = typename tensor_vector_type<T, Elems>::type;

#if AIE_API_ML_VERSION >= 200

template <typename T, unsigned Elems, unsigned Level, typename TensorIterDescriptor, aie_dm_resource Resource, bool Restrict>
#if __AIE_API_SUPPORTED_FRIEND_CONCEPTS__
    requires (arch::is(arch::AIE_ML))
#endif
class __AIE_API_KEEP_IN_REGISTERS__ tensor_buffer_stream
{
public:
    using          elem_type = aie_dm_resource_remove_t<T>;
    using        vector_type = tensor_vector_type_t<std::remove_const_t<elem_type>, Elems>;
    using         value_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector_type, aie_dm_resource_get_v<T>>>;
    using            pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using        iter_desc_t = std::decay_t<TensorIterDescriptor>;
    using  iter_desc_storage = const iter_desc_t&;
    using iter_state_storage = iter_state<std::tuple_element_t<Level, iter_desc_t>>;

    static constexpr unsigned MaxDepth = std::tuple_size_v<iter_desc_t> - 1;
    static constexpr bool next_sliding = (Level == (MaxDepth - 1)) && utils::is_one_of_v<std::tuple_element_t<MaxDepth, iter_desc_t>,
                                                                                         sliding_window_dim_1d,
                                                                                         sliding_window_dim_2d,
                                                                                         sliding_window_dim_3d>;
    using inner_type = std::conditional_t<Level == MaxDepth,
                                          vector_type,
                                          std::conditional_t<next_sliding,
                                                             sliding_window_buffer_stream<T, Elems, std::tuple_element_t<MaxDepth, iter_desc_t>, Resource>,
                                                             tensor_buffer_stream<T, Elems, Level + 1, iter_desc_storage, Resource, Restrict>>>;

    __aie_inline
    constexpr tensor_buffer_stream(T *ptr, const TensorIterDescriptor& iter_desc) : ptr_(ptr), iter_desc_(iter_desc) {}

    tensor_buffer_stream(const tensor_buffer_stream&)            = default;
    tensor_buffer_stream& operator=(const tensor_buffer_stream&) = default;
    tensor_buffer_stream(tensor_buffer_stream&&)                 = default;
    tensor_buffer_stream& operator=(tensor_buffer_stream&&)      = default;
    ~tensor_buffer_stream()                                      = default;

    __aie_inline
    constexpr inner_type pop()
    {
        if constexpr(Level == MaxDepth) {
            inner_type v = *(pointer)utils::floor_ptr<Elems>(ptr_);
            increment();
            return v;
        }
        else if constexpr (next_sliding) {
            inner_type v = inner_type(ptr_, std::get<MaxDepth>(iter_desc_));
            increment();
            return v;
        }
        else {
            inner_type v = inner_type(ptr_, iter_desc_);
            increment();
            return v;
        }
    }

    __aie_inline
    constexpr tensor_buffer_stream &operator>>(inner_type& v) requires (Level == MaxDepth)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    constexpr void push(const vector_type& v) requires (Level == MaxDepth)
    {
        *(pointer)ptr_ = v;
        increment();
    }

    __aie_inline
    constexpr tensor_buffer_stream &operator<<(const vector_type &v) requires (Level == MaxDepth)
    {
        push(v);
        return *this;
    }

    __aie_inline
    constexpr pointer data()
    {
        return (pointer)ptr_;
    }

    constexpr bool operator==(const tensor_buffer_stream& rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool operator!=(const tensor_buffer_stream& rhs) const { return ptr_ != rhs.ptr_; }

private:
    __aie_inline
    constexpr void increment(void)
    {
        const auto& inc = std::get<Level>(iter_desc_);
        if      constexpr (std::is_same_v<dim_3d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_3d_byte(ptr_, inc.inc3,
                                       inc.num1, iter_state_.state_.c1, inc.inc1,
                                       inc.num2, iter_state_.state_.c2, inc.inc2);
        }
        else if constexpr (std::is_same_v<dim_2d, std::decay_t<decltype(inc)>>) {
            ptr_ = ::add_2d_byte(ptr_, inc.inc2,
                                       inc.num1, iter_state_.state_.c1, inc.inc1);
        }
        else {
            ptr_ = ::byte_incr(ptr_, inc);
        }
    }

    template <typename T2, unsigned Elems2, unsigned Level2, typename TensorIterDescriptor2, aie_dm_resource Resource2, bool Restrict2>
#if __AIE_API_SUPPORTED_FRIEND_CONCEPTS__
        requires (arch::is(arch::AIE_ML))
#endif
    friend class tensor_buffer_stream;

#if AIE_API_NATIVE
    using ptr_type = T*;
#else
    using ptr_type = std::conditional_t<Restrict,
                                        T* __restrict,
                                        T*>;
#endif
    ptr_type ptr_;
    iter_desc_storage iter_desc_;
    iter_state_storage iter_state_;
};

#endif

template <size_t Stride>
struct iterator_stride
{
    static constexpr size_t value() { return Stride; }
};

template <>
struct iterator_stride<dynamic_extent>
{
    size_t stride_;

    constexpr size_t value() const { return stride_; }
};

enum class FifoDirection {
    In,
    Out
};

template <typename Pointer, size_t Elems, size_t Stride>
struct random_circular_iterator_storage;

} // namespace aie::detail

#if __AIE_ARCH__ == 10

#include "aie1/array_helpers.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/array_helpers.hpp"

#endif

namespace aie::detail {

template <typename Pointer, size_t Elems>
struct __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator_storage_static
{
    Pointer ptr;
    Pointer base;
    static constexpr size_t elems = Elems;
};

template <typename Pointer>
struct __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator_storage_dynamic
{
    Pointer ptr;
    Pointer base;
    size_t elems;
};

template <typename T, size_t Elems, size_t Stride, aie_dm_resource Resource>
class __AIE_API_KEEP_IN_REGISTERS__ random_circular_iterator
{
    static constexpr bool is_static()
    {
        return Elems != dynamic_extent;
    }

    static constexpr bool is_stride_static()
    {
        return Stride != dynamic_extent;
    }

public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type* ;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t elems, size_t stride) :
        storage_{ptr, ptr, elems},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t elems) :
        storage_{ptr, ptr, elems}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, size_t stride) :
        storage_{ptr, ptr},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr) :
        storage_{ptr, ptr}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t elems, size_t stride) :
        storage_{ptr, base, elems},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t elems) :
        storage_{ptr, base, elems}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr random_circular_iterator(pointer ptr, pointer base, size_t stride) :
        storage_{ptr, base},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
        constexpr random_circular_iterator(pointer ptr, pointer base) :
        storage_{ptr, base}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    random_circular_iterator  &operator++()
    {
        *this += 1;
        return *this;
    }

    random_circular_iterator   operator++(int)
    {
        random_circular_iterator it = *this;
        ++(*this);
        return it;
    }

    random_circular_iterator  &operator--()
    {
        *this -= 1;
        return *this;
    }

    random_circular_iterator   operator--(int)
    {
        random_circular_iterator it = *this;
        --(*this);
        return it;
    }

    random_circular_iterator   operator+(int off) const
    {
        random_circular_iterator it = *this;
        it += off;
        return it;
    }

    random_circular_iterator   operator-(int off) const
    {
        random_circular_iterator it = *this;
        it -= off;
        return it;
    }

    random_circular_iterator  &operator+=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,  off * stride_.value(), storage_.base, storage_.elems);
        return *this;
    }

    random_circular_iterator  &operator-=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr, -off * stride_.value(), storage_.base, storage_.elems);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()                                           { return *storage_.ptr;                    }
    constexpr pointer   operator->()                                          { return storage_.ptr;                     }
    constexpr bool      operator==(const random_circular_iterator& rhs) const { return storage_.ptr == rhs.storage_.ptr; }
    constexpr bool      operator!=(const random_circular_iterator& rhs) const { return storage_.ptr != rhs.storage_.ptr; }

private:
    using storage_type = std::conditional_t<is_static(),
                                            random_circular_iterator_storage_static<pointer, Elems>,
                                            random_circular_iterator_storage_dynamic<pointer>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Elems, size_t ArrayElems, size_t Stride, aie_dm_resource Resource>
class vector_random_circular_iterator
{
    static constexpr bool is_static()
    {
        return ArrayElems != dynamic_extent;
    }

    static constexpr bool is_stride_static()
    {
        return Stride != dynamic_extent;
    }

public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<elem_type, Elems>, aie_dm_resource_get_v<T>>>;

    static constexpr unsigned subbyte_elems = type_bits_v<elem_type> == 4? 2 : 1;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t elems, size_t stride) :
        storage_{ptr, ptr, elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t elems) :
        storage_{ptr, ptr, elems}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, size_t stride) :
        storage_{ptr, ptr},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr) :
        storage_{ptr, ptr}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t elems, size_t stride) :
        storage_{ptr, base, elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t elems) :
        storage_{ptr, base, elems}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_random_circular_iterator(T *ptr, T *base, size_t stride) :
        storage_{ptr, base},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                     "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,     "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
        constexpr vector_random_circular_iterator(T *ptr, T *base) :
        storage_{ptr, base}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                     "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,     "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    vector_random_circular_iterator  &operator++()
    {
        *this += 1;
        return *this;
    }

    vector_random_circular_iterator   operator++(int)
    {
        vector_random_circular_iterator it = *this;
        ++(*this);
        return it;
    }

    vector_random_circular_iterator  &operator--()
    {
        *this -= 1;
        return *this;
    }

    vector_random_circular_iterator   operator--(int)
    {
        vector_random_circular_iterator it = *this;
        --(*this);
        return it;
    }

    vector_random_circular_iterator   operator+(int off) const
    {
        vector_random_circular_iterator it = *this;
        it += off;
        return it;
    }

    vector_random_circular_iterator   operator-(int off) const
    {
        vector_random_circular_iterator it = *this;
        it -= off;
        return it;
    }

    vector_random_circular_iterator  &operator+=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,
                                    (off * (Elems / subbyte_elems) * stride_.value()),
                                    storage_.base,
                                    storage_.elems / subbyte_elems);

        return *this;
    }

    vector_random_circular_iterator  &operator-=(int off)
    {
        storage_.ptr = ::cyclic_add(storage_.ptr,
                                    -int(off * (Elems / subbyte_elems) * stride_.value()),
                                    storage_.base,
                                    storage_.elems / subbyte_elems);

        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(pointer)*this + off / subbyte_elems;
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(pointer)*this + off / subbyte_elems;
    }

    constexpr reference operator*()                                                  { return *(pointer)storage_.ptr;           }
    constexpr pointer   operator->()                                                 { return (pointer)storage_.ptr;            }
    constexpr bool      operator==(const vector_random_circular_iterator& rhs) const { return storage_.ptr == rhs.storage_.ptr; }
    constexpr bool      operator!=(const vector_random_circular_iterator& rhs) const { return storage_.ptr != rhs.storage_.ptr; }

private:
    using storage_type = std::conditional_t<is_static(),
                                            random_circular_iterator_storage_static<T *, ArrayElems>,
                                            random_circular_iterator_storage_dynamic<T *>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Elems, size_t Stride, aie_dm_resource Resource = aie_dm_resource::none>
class __AIE_API_KEEP_IN_REGISTERS__ vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <size_t Stride2 = Stride> requires(Stride2 == dynamic_extent)
    constexpr vector_iterator(T *ptr, size_t stride = 1) :
        ptr_(ptr),
        stride_{stride}
    {
    }

    template <size_t Stride2 = Stride> requires(Stride2 != dynamic_extent)
    constexpr vector_iterator(T *ptr) :
        ptr_(ptr)
    {
    }

    constexpr vector_iterator &operator++()
    {
        *this += 1;
        return *this;
    }

    constexpr vector_iterator  operator++(int)
    {
        const vector_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr vector_iterator &operator--()
    {
        *this -= 1;
        return *this;
    }

    constexpr vector_iterator  operator--(int)
    {
        const vector_iterator it = *this;
        --(*this);
        return it;
    }

    constexpr vector_iterator operator+(difference_type off) const
    {
        vector_iterator it = *this;
        it += off;
        return it;
    }

    constexpr vector_iterator operator-(difference_type off) const
    {
        vector_iterator it = *this;
        it -= off;
        return it;
    }

    constexpr vector_iterator &operator+=(difference_type off)
    {
        ptr_ += increment(off);
        return *this;
    }

    constexpr vector_iterator &operator-=(difference_type off)
    {
        ptr_ -= increment(off);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()
    {
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");

        return *(pointer)ptr_;
    }

    constexpr pointer   operator->()
    {
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");

        return (pointer)ptr_;
    }

    constexpr bool      operator==(const vector_iterator& rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool      operator!=(const vector_iterator& rhs) const { return ptr_ != rhs.ptr_; }

private:

    constexpr size_t increment(difference_type off) const
    {
        if constexpr (type_bits_v<std::remove_cv_t<T>> == 4)
            return (Elems / (sizeof(T) * 2)) * off * stride_.value();
        else
            return Elems * off * stride_.value();
    }

    T *ptr_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

//TODO : Improve / refactor this code (CRVO-3256)
template <typename T, unsigned Elems, size_t Stride, aie_dm_resource Resource = aie_dm_resource::none>
class __AIE_API_KEEP_IN_REGISTERS__ restrict_vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <size_t Stride2 = Stride> requires(Stride2 == dynamic_extent)
    constexpr restrict_vector_iterator(T * __restrict ptr, size_t stride = 1) :
        ptr_(ptr),
        stride_{stride}
    {
    }

    template <size_t Stride2 = Stride> requires(Stride2 != dynamic_extent)
    constexpr restrict_vector_iterator(T * __restrict ptr) :
        ptr_(ptr)
    {
    }

    constexpr restrict_vector_iterator &operator++()
    {
        *this += 1;
        return *this;
    }

    constexpr restrict_vector_iterator  operator++(int)
    {
        const restrict_vector_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr restrict_vector_iterator &operator--()
    {
        *this -= 1;
        return *this;
    }

    constexpr restrict_vector_iterator  operator--(int)
    {
        const restrict_vector_iterator it = *this;
        --(*this);
        return it;
    }

    constexpr restrict_vector_iterator operator+(difference_type off) const
    {
        restrict_vector_iterator it = *this;
        it += off;
        return it;
    }

    constexpr restrict_vector_iterator operator-(difference_type off) const
    {
        restrict_vector_iterator it = *this;
        it -= off;
        return it;
    }

    constexpr restrict_vector_iterator &operator+=(difference_type off)
    {
        ptr_ += increment(off);
        return *this;
    }

    constexpr restrict_vector_iterator &operator-=(difference_type off)
    {
        ptr_ -= increment(off);
        return *this;
    }

    constexpr reference operator[](difference_type off)
    {
        return *(*this + off);
    }

    constexpr reference operator[](difference_type off) const
    {
        return *(*this + off);
    }

    constexpr reference operator*()
    {
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");

        pointer __restrict tmp_ptr = (pointer) ptr_;
        return *tmp_ptr;
    }

    constexpr pointer operator->()
    {
        RUNTIME_ASSERT(check_vector_alignment<Elems>(ptr_), "Insufficient alignment");

        pointer __restrict tmp_ptr = (pointer) ptr_;
        return tmp_ptr;
    }

    constexpr bool      operator==(const restrict_vector_iterator& rhs) const { return ptr_ == rhs.ptr_;     }
    constexpr bool      operator!=(const restrict_vector_iterator& rhs) const { return ptr_ != rhs.ptr_;     }

private:

    constexpr size_t increment(difference_type count) const
    {
        if constexpr (type_bits_v<std::remove_cv_t<T>> == 4)
            return (Elems / (sizeof(T) * 2)) * count * stride_.value();
        else
            return Elems * count * stride_.value();
    }

    T * __restrict ptr_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Steps>
class pattern_iterator
{
    // TODO: check if using property(keep_in_registers) helps

public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <typename... StepOffsets>
    constexpr pattern_iterator(pointer ptr, StepOffsets... step_offsets) :
        ptr_(ptr),
        offsets_{step_offsets...},
        idx_(offsets_)
    {
        static_assert(sizeof...(StepOffsets) == Steps);
    }

    constexpr pattern_iterator &operator++()
    {
        ptr_ += *(idx_++);
        return *this;
    }

    constexpr pattern_iterator  operator++(int)
    {
        const pattern_iterator it = *this;
        ++(*this);
        return it;
    }

    constexpr reference operator*()                                   { return *ptr_;            }
    constexpr pointer   operator->()                                  { return ptr_;             }
    constexpr bool      operator==(const pattern_iterator& rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool      operator!=(const pattern_iterator& rhs) const { return ptr_ != rhs.ptr_; }

private:

    T *ptr_;
    ptrdiff_t offsets_[Steps];
    circular_iterator<ptrdiff_t, Steps, 1, aie_dm_resource::none> idx_;
};

// TODO: Add optimized implementations for AIE1/AIE2
template <typename T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_iterator
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<std::remove_const_t<elem_type>, Elems>;
    using   vector_ref_type = unaligned_vector_ref<T,
                                                   vector_type::size(),
                                                   Resource>;

    using        value_type = vector_type;
    using         reference = vector_ref_type;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    __aie_inline
    constexpr unaligned_vector_iterator(T *ptr) :
        ptr_(ptr)
    {
        RUNTIME_ASSERT(ptr_ != nullptr, "Iterator cannot be created from nullptr");

        if constexpr (type_bits_v<T> >= 8)
            alignment_ = 1 << utils::ffs(uintptr_t(ptr) / (type_bits_v<T> / 8));
        else if constexpr (type_bits_v<T> == 4)
            alignment_ = 2 << utils::ffs(uintptr_t(ptr));
    }

    __aie_inline
    constexpr unaligned_vector_iterator &operator++()
    {
        ptr_ += Elems;
        return *this;
    }

    __aie_inline
    constexpr unaligned_vector_iterator  operator++(int)
    {
        unaligned_vector_iterator it = *this;
        ++(*this);
        return it;
    }

    __aie_inline
    constexpr vector_ref_type operator*() const
    {
        return vector_ref_type(ptr_, alignment_);
    }

    __aie_inline
    constexpr vector_ref_type operator*() requires(!std::is_const_v<T>)
    {
        return vector_ref_type(ptr_, alignment_);
    }

    constexpr bool operator==(const unaligned_vector_iterator &rhs) const { return ptr_ == rhs.ptr_; }
    constexpr bool operator!=(const unaligned_vector_iterator &rhs) const { return ptr_ != rhs.ptr_; }

private:

    T *ptr_;
    unsigned alignment_;
};

// Default implementation of input and output buffer streams, relying on vector_iterator and unaligned_vector_iterator.
// The architecture backends can provide optimized specializations.
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class vector_input_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit vector_input_buffer_stream(const T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    constexpr vector_type pop()
    {
        vector_type v = *it_; ++it_;
        return v;
    }

private:
    vector_iterator<const T, Elems, 1, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class vector_output_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit vector_output_buffer_stream(T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr vector_output_buffer_stream &operator<<(const vector_type &v)
    {
        push(v);
        return *this;
    }

    __aie_inline
    constexpr void push(const vector_type &v)
    {
        *it_ = v; ++it_;
    }

private:
    vector_iterator<T, Elems, 1, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_input_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit unaligned_vector_input_buffer_stream(const T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr unaligned_vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = pop();
        return *this;
    }

    __aie_inline
    constexpr vector_type pop()
    {
        vector_type v = *it_; ++it_;
        return v;
    }

private:

    unaligned_vector_iterator<const T, Elems, Resource> it_;
};

template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_output_buffer_stream
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<elem_type, Elems>;

    using        value_type = vector_type;

    __aie_inline
    constexpr explicit unaligned_vector_output_buffer_stream(T *ptr) :
        it_(ptr)
    {
        RUNTIME_ASSERT(ptr != nullptr, "Iterator cannot be created from nullptr");
    }

    __aie_inline
    constexpr unaligned_vector_output_buffer_stream &operator<<(const vector_type &v)
    {
        push(v);
        return *this;
    }

    __aie_inline
    constexpr void push(const vector_type &v)
    {
        *it_ = v; ++it_;
    }

private:

    unaligned_vector_iterator<T, Elems, Resource> it_;
};

// Operator overloading to allow output << input patterns to avoid having to store vectors in temporary variables
template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr vector_output_buffer_stream<T, Elems, Resource> &
operator<<(vector_output_buffer_stream<T, Elems, Resource> &out, vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr unaligned_vector_output_buffer_stream<T, Elems, Resource> &
operator<<(unaligned_vector_output_buffer_stream<T, Elems, Resource> &out, vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr vector_output_buffer_stream<T, Elems, Resource> &
operator<<(vector_output_buffer_stream<T, Elems, Resource> &out, unaligned_vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename unaligned_vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

template <typename T, unsigned Elems, aie_dm_resource Resource, aie_dm_resource ResourceIn>
__aie_inline
constexpr unaligned_vector_output_buffer_stream<T, Elems, Resource> &
operator<<(unaligned_vector_output_buffer_stream<T, Elems, Resource> &out, unaligned_vector_input_buffer_stream<T, Elems, ResourceIn> &in)
{
    typename unaligned_vector_input_buffer_stream<T, Elems, ResourceIn>::vector_type v;
    in >> v;
    out << v;
    return out;
}

#if AIE_API_ML_VERSION >= 200

#if (AIE_API_NATIVE == 1 && __AIE_API_NATIVE_FIFO__ == 0)

template <typename Vector, aie_dm_resource Resource, FifoDirection Direction>
class fifo_buffer_stream
{
    fifo_buffer_stream(void *) {}
};

#endif

template <DecoratedElemBaseType T, unsigned N, aie_dm_resource Resource>
class __AIE_API_KEEP_IN_REGISTERS__ sparse_vector_input_buffer_stream :
    public fifo_buffer_stream<sparse_vector<std::remove_const_t<aie_dm_resource_remove_t<T>>, N>, Resource, FifoDirection::In>
{
public:
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = sparse_vector<elem_type, N>;

    using        value_type = vector_type;

    __aie_inline
    constexpr sparse_vector_input_buffer_stream(const T *ptr) :
        parent_type(ptr)
    {
    }

    __aie_inline
    constexpr sparse_vector_input_buffer_stream &operator>>(vector_type &v)
    {
        v = this->pop();
        return *this;
    }

private:
    using parent_type = fifo_buffer_stream<vector_type, Resource, FifoDirection::In>;
};

#endif

} // namespace aie::detail

#endif
