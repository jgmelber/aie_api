// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ARRAY_HELPERS__HPP__
#define __AIE_API_DETAIL_AIE2_ARRAY_HELPERS__HPP__

#include <iterator>

#include "../../concepts.hpp"
#include "../../vector.hpp"
#include "../array_helpers.hpp"

#if AIE_API_NATIVE == 1

template <typename T>
T * cyclic_add(T * ptr, int count, T * base, int size)
{
    T *res = ptr + count;

    if (res >= base + size)
        res -= size;

    if (res < base)
        res += size;

    return res;
}

#else

// Emulated version of the cyclic_add intrinsic from AIE1
template <typename T>
T * cyclic_add(T * ptr, int count, T * base, int size)
{
    addr_t c0 = (long)ptr;
    addr_t c1 = (long)ptr;

    return ::add_3d_ptr(ptr, count-size, (long)(base-count), c0, count+size, (long)(base-count+size), c1, count);
}

#endif

namespace aie::detail {

template <typename Pointer, size_t Elems>
struct circular_iterator_storage_static property(keep_in_registers)
{
    Pointer ptr;
    addr_t current_index;
    static constexpr size_t elems = Elems;
};

template <typename Pointer>
struct circular_iterator_storage_dynamic property(keep_in_registers)
{
    Pointer ptr;
    addr_t current_index;
    size_t elems;
};

template <typename T, size_t Elems, size_t Stride, aie_dm_resource Resource>
class circular_iterator property(keep_in_registers)
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
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr circular_iterator(pointer ptr, size_t elems, size_t stride) :
        storage_{ptr, 0, elems},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr circular_iterator(pointer ptr, size_t elems) :
        storage_{ptr, 0, elems}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr circular_iterator(pointer ptr, size_t stride) :
        storage_{ptr, 0},
        stride_{stride}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr circular_iterator(pointer ptr) :
        storage_{ptr, 0}
    {}

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr circular_iterator(pointer ptr, pointer base, size_t elems, size_t stride) :
        storage_{ptr, (ptr - base), elems},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr circular_iterator(pointer ptr, pointer base, size_t elems) :
        storage_{ptr, (ptr - base), elems}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr circular_iterator(pointer ptr, pointer base, size_t stride) :
        storage_{ptr, (ptr - base)},
        stride_{stride}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr circular_iterator(pointer ptr, pointer base) :
        storage_{ptr, (ptr - base)}
    {
        REQUIRES_MSG(ptr >= base, "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + Elems, "Start address must be less than base address plus array size");
    }

    /** \brief Advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    circular_iterator  &operator++()
    {
        storage_.ptr = ::add_2d_ptr(storage_.ptr,
                                    -int(storage_.elems - stride_.value()),
                                    storage_.elems / stride_.value() - 1,
                                    storage_.current_index,
                                    stride_.value());
        return *this;
    }

    /** \brief Advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    circular_iterator   operator++(int)
    {
        circular_iterator it = *this;
        ++(*this);
        return it;
    }

    /** \brief Accesses the value in the iterator. */
    constexpr reference operator*()                                   { return *storage_.ptr;                    }

    /** \brief Accesses the value in the iterator. */
    constexpr pointer   operator->()                                  { return storage_.ptr;                     }

    /** \brief Return true if the two iterators reference the same value. */
    constexpr bool      operator==(const circular_iterator& rhs) { return storage_.ptr == rhs.storage_.ptr; }

    /** \brief Return true if the two iterators reference different values. */
    constexpr bool      operator!=(const circular_iterator& rhs) { return storage_.ptr != rhs.storage_.ptr; }

private:
    using storage_type = std::conditional_t<is_static(),
                                            circular_iterator_storage_static<pointer, Elems>,
                                            circular_iterator_storage_dynamic<pointer>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <typename T, unsigned Elems, size_t ArrayElems, size_t Stride, aie_dm_resource Resource>
class vector_circular_iterator property(keep_in_registers)
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
    constexpr vector_circular_iterator(T *ptr, size_t elems, size_t stride) :
        storage_{ptr, 0, elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, size_t elems) :
        storage_{ptr, 0, elems}
    {
        REQUIRES_MSG(elems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, size_t stride) :
        storage_{ptr, 0},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr) :
        storage_{ptr, 0}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0, "Array size needs to be a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && !IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, T *base, size_t elems, size_t stride) :
        storage_{ptr, (ptr - base) / (Elems / subbyte_elems), elems},
        stride_{stride}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(!IsStatic && IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, T *base, size_t elems) :
        storage_{ptr, (ptr - base) / (Elems / subbyte_elems), elems}
    {
        REQUIRES_MSG(elems % Elems == 0,                          "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + elems / subbyte_elems,          "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && !IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, T *base, size_t stride) :
        storage_{ptr, (ptr - base) / (Elems / subbyte_elems)},
        stride_{stride}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                     "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                 "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,     "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems) == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    template <bool IsStatic = is_static(), bool IsStrideStatic = is_stride_static()> requires(IsStatic && IsStrideStatic)
    constexpr vector_circular_iterator(T *ptr, T *base) :
        storage_{ptr, (ptr - base) / (Elems / subbyte_elems)}
    {
        REQUIRES_MSG(ArrayElems % Elems == 0,                      "Array size needs to be a multiple of vector size");
        REQUIRES_MSG(ptr >= base,                                  "Start address must be greater or equal to base address");
        REQUIRES_MSG(ptr < base + ArrayElems / subbyte_elems,      "Start address must be less than base address plus array size");
        REQUIRES_MSG((ptr - base) % (Elems / subbyte_elems)  == 0, "Start address must be offset from base address by a multiple of vector size");
    }

    /** \brief Advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    vector_circular_iterator  &operator++()
    {
        storage_.ptr = ::add_2d_ptr(storage_.ptr,
                                    -int((storage_.elems - (Elems) * stride_.value()) / subbyte_elems),
                                    storage_.elems / (Elems * stride_.value()) - 1,
                                    storage_.current_index,
                                    (Elems / subbyte_elems) * stride_.value());
        return *this;
    }

    /** \brief Advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    vector_circular_iterator   operator++(int)
    {
        vector_circular_iterator it = *this;
        ++(*this);
        return it;
    }

    /** \brief Accesses the first `Elems` contiguous elements starting at the the iterator's current position. */
    constexpr reference operator*()                                          { return *(pointer)storage_.ptr;           }

    /** \brief Accesses the first `Elems` contiguous elements starting at the the iterator's current position. */
    constexpr pointer   operator->()                                         { return (pointer)storage_.ptr;            }

    /** \brief Return true if the two iterators reference the same value. */
    constexpr bool      operator==(const vector_circular_iterator& rhs) { return storage_.ptr == rhs.storage_.ptr; }

    /** \brief Return true if the two iterators reference different values. */
    constexpr bool      operator!=(const vector_circular_iterator& rhs) { return storage_.ptr != rhs.storage_.ptr; }

private:

    using storage_type = std::conditional_t<is_static(),
                                            circular_iterator_storage_static<T *, ArrayElems>,
                                            circular_iterator_storage_dynamic<T *>>;

    storage_type storage_;
    [[no_unique_address]] iterator_stride<Stride> stride_;
};

template <unsigned VectorBits>
struct sparse_vector_fill_index {};

template <>
struct sparse_vector_fill_index<1024> {
    unsigned value = 0;
};

#if __AIE_ARCH__ == 20

template <SparseVector Vector, aie_dm_resource Resource, FifoDirection Direction>
class fifo_buffer_stream property(keep_in_registers)
{
public:
    using         vector_type = Vector;
    using           elem_type = typename vector_type::value_type;
    using native_pointer_type = typename vector_type::native_pointer_type;

    using        value_type = vector_type;

    // The memory interface is 256b, but ::sparse_pop generates two POP instructions
    static constexpr unsigned native_ld_bits = 256 * 2;

    static constexpr unsigned ld_fill_ratio = []() {
        if constexpr (vector_type::bits() > 1024)
            return native_ld_bits / (vector_type::memory_bits() - 1024);
        else
            return native_ld_bits / (vector_type::memory_bits() - 512);
    }();

    static_assert(Direction == FifoDirection::In);

    constexpr fifo_buffer_stream(const elem_type *ptr) :
        ptr_((native_pointer_type *)ptr)
    {
        ::sparse_reset(ptr_);
        ::sparse_fill(ptr_);
        ::sparse_fill(ptr_);
        ::sparse_fill(ptr_);
    }

    value_type pop()
    {
        vector_type ret;

        // Each call to POP reads 64b of mask (in addition to the data). An additional refill is needed every 4 calls
        // to POP
        if (!chess_manifest(idx_ != 0))
            ::sparse_fill(ptr_);

        if constexpr (vector_type::bits() < 2048) {
            ret = ::sparse_pop(ptr_);
        }
        else {
            ret.template insert<vector_type::size() / 2>(0, ::sparse_pop(ptr_));
            ret.template insert<vector_type::size() / 2>(1, ::sparse_pop(ptr_));
        }

        idx_ = (idx_ + 1) % ld_fill_ratio;

        return ret;
    }

private:
    unsigned idx_;
    native_pointer_type *ptr_;
};

template <typename T, unsigned Elems, typename IterDescriptor, aie_dm_resource Resource>
    requires (arch::is(arch::Gen2))
class sliding_window_buffer_stream property(keep_in_registers)
{
public:
    using   elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using vector_type = vector<elem_type, Elems>;
    using  value_type = vector_type;

    using        iter_desc_t = std::decay_t<IterDescriptor>;
    using  iter_desc_storage = const iter_desc_t&;
    using iter_state_storage = iter_state<iter_desc_t>;

    __aie_inline
    constexpr sliding_window_buffer_stream(T *ptr, iter_desc_storage iter_desc) :
        ptr_((aliased_type*)::byte_incr(ptr, 31)), iter_desc_(iter_desc), step_(inc_to_step(iter_desc.inc1, true))
    {
        RUNTIME_ASSERT(ptr != nullptr, "Input buffer stream cannot be created from nullptr");
    }

    __aie_inline
    sliding_window_buffer_stream& operator>>(vector_type& res)
    {
        res = pop();
        return *this;
    }

    __aie_inline
    value_type pop()
    {
        unsigned frac = (((long)ptr_) & 31) + 33;

        constexpr unsigned tmp_elems = std::max(Elems, native_elems);
        vector<elem_type, tmp_elems> tmp;

        native_type buff0;
        buff0.template load<Resource>(utils::floor_ptr(ptr_));

        state_buff_ = ::shiftx(state_buff_, buff0, step_, frac);

        tmp.insert(0, state_buff_);

        if constexpr (Elems > native_elems) {
            native_type buff1;
            buff1.template load<Resource>(utils::floor_ptr(ptr_ + native_elems));

            if constexpr (subbyte_elems == 2)
                tmp.insert(1, native_type(::shift_bytes(buff0, buff1, frac)));
            else
                tmp.insert(1, native_type(::shift(buff0, buff1, frac / sizeof(T))));
        }

        increment();

        return tmp.template extract<Elems>(0);
    }

    constexpr bool operator==(const sliding_window_buffer_stream& rhs) { return ptr_ == rhs.ptr_; }
    constexpr bool operator!=(const sliding_window_buffer_stream& rhs) { return ptr_ != rhs.ptr_; }

private:
    static constexpr unsigned subbyte_elems = type_bits_v<elem_type> == 4 ? 2 : 1;
    static constexpr unsigned native_elems  = native_vector_length_v<elem_type>;
    using native_type  = vector<elem_type, native_elems>;
    using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<elem_type, aie_dm_resource_get_v<T>>>;

    __aie_inline
    static constexpr unsigned inc_to_step(unsigned inc, bool byte_input)
    {
        unsigned inc_bytes = byte_input ? inc : inc * sizeof(T) / subbyte_elems;

        REQUIRES(inc_bytes == 4 || inc_bytes ==  8 || inc_bytes ==  16 || inc_bytes ==  32);

        return utils::ffs(inc_bytes >> 1);
    }

    __aie_inline
    constexpr void increment(void)
    {
        if      constexpr (std::is_same_v<sliding_window_dim_3d, iter_desc_t>) {
            ptr_ = ::add_3d_byte(ptr_, iter_desc_.inc3,
                                       iter_desc_.num1, iter_state_.state_.c1, iter_desc_.inc1,
                                       iter_desc_.num2, iter_state_.state_.c2, iter_desc_.inc2);
        }
        else if constexpr (std::is_same_v<sliding_window_dim_2d, iter_desc_t>) {
            ptr_ = ::add_2d_byte(ptr_, iter_desc_.inc2,
                                       iter_desc_.num1, iter_state_.state_.c1, iter_desc_.inc1);
        }
        else {
            ptr_ = ::byte_incr(ptr_, iter_desc_.inc1);
        }
    }

    aliased_type *ptr_;
    iter_desc_storage iter_desc_;
    iter_state_storage iter_state_;
    unsigned step_;
    native_type state_buff_;
};

#endif

}

#endif
