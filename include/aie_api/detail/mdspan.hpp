// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_MDSPAN__HPP__
#define __AIE_API_DETAIL_MDSPAN__HPP__

#include <array>

#include "../iterator.hpp"

#if __AIE_ARCH__ == 10

#include "aie1/mdspan.hpp"

#elif __AIE_ARCH__ == 20

#include "aie2/mdspan.hpp"

#endif

namespace aie::detail {

template <size_t... Extents>
using extents = std::integer_sequence<size_t, Extents...>;

template <size_t Rank>
using make_dynamic_extents = utils::make_integer_broadcast<size_t, Rank, dynamic_extent>;

template <typename Extents>
struct span_indexer
{
    using index_type = ptrdiff_t;

    template <unsigned DimIndex, typename... Indices>
    static constexpr index_type offset_helper(const Extents &extents, index_type idx, Indices &&... indices)
    {
        if constexpr (sizeof...(Indices) == 0)
            return idx;
        else
            return extents.offsets_[DimIndex - 1] * idx + span_indexer::offset_helper<DimIndex - 1>(extents, std::forward<Indices>(indices)...);
    }

    template <typename... Indices>
    static constexpr index_type offset(const Extents &extents, Indices &&... indices)
    {
        return offset_helper<Extents::rank() - 1>(extents, std::forward<Indices>(indices)...);
    }
};

template <typename Span>
class iterator
{
public:
    using        value_type = typename Span::value_type;
    using         reference = typename Span::reference;
    using           pointer = typename Span::pointer;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    static_assert(Span::rank() == 1);

    constexpr iterator(pointer ptr) : ptr_(ptr) { }
    constexpr iterator  operator++()                    { ++ptr_; return *this;    }
    constexpr iterator  operator++(int)                 { iterator i = *this; ++(*this); return i; }
    constexpr reference operator*()                     { return *ptr_;            }
    constexpr pointer   operator->()                    { return ptr_;             }
    constexpr bool      operator==(const iterator& rhs) { return ptr_ == rhs.ptr_; }
    constexpr bool      operator!=(const iterator& rhs) { return ptr_ != rhs.ptr_; }

private:
    pointer ptr_;
};

template <typename Span>
class const_iterator
{
public:
    using        value_type = typename Span::value_type;
    using         reference = typename Span::reference;
    using           pointer = typename Span::pointer;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    static_assert(Span::rank() == 1);

    constexpr const_iterator(pointer ptr) : ptr_(ptr) { }
    constexpr const_iterator operator++()                          { ++ptr_; return *this;    }
    constexpr const_iterator operator++(int)                       { const_iterator i = *this; ++(*this); return i; }
    constexpr reference      operator*() const                     { return *ptr_;            }
    constexpr pointer        operator->() const                    { return ptr_;             }
    constexpr bool           operator==(const const_iterator& rhs) { return ptr_ == rhs.ptr_; }
    constexpr bool           operator!=(const const_iterator& rhs) { return ptr_ != rhs.ptr_; }

private:
    pointer ptr_;
};

template <size_t Rank>
using dyn_extents = std::array<size_t, Rank>;

template <typename Extents>
struct span_extents_common;

template <size_t... Extents>
struct span_extents_common<extents<Extents...>>
{
    static_assert((... && (Extents != 0)), "Static span dimensions must be larger than zero");

    using index_type = ptrdiff_t;

    static constexpr size_t rank()
    {
        return sizeof...(Extents);
    }

    static constexpr size_t rank_dynamic()
    {
        return (... + int(Extents == dynamic_extent));
    }

    static constexpr size_t rank_static()
    {
        return (... + int(Extents != dynamic_extent));
    }

    static constexpr bool is_static()
    {
        return rank_dynamic() == 0;
    }

    static constexpr size_t elems_static()
    {
        return (... * Extents);
    }
};

template <bool IsStatic, typename Extents>
struct span_extents : public span_extents_common<Extents>
{
    using parent = span_extents_common<Extents>;

    using index_type = typename parent::index_type;

    template <typename... DynamicExtents>
    constexpr span_extents(DynamicExtents... ext) :
        ext_{ext...}
    {
        index_type offset = 1;
        for (unsigned i = 0; i < parent::rank(); ++i) {
            offset *= ext_[parent::rank() - i - 1];
            offsets_[i] = offset;
        }
    }

    constexpr size_t extent(size_t index) const
    {
        return ext_[index];
    }

    dyn_extents<parent::rank()> ext_;
    std::array<index_type, parent::rank()> offsets_;
};

template <size_t... Extents>
struct span_extents<true, extents<Extents...>> : public span_extents_common<extents<Extents...>>
{
    using parent = span_extents_common<extents<Extents...>>;

    using index_type = typename parent::index_type;

    static constexpr std::array<index_type, parent::rank() - 1> compute_offsets()
    {
        constexpr std::array<index_type, parent::rank()> ext = {Extents...};
        std::array<index_type, parent::rank() - 1> ret{};

        index_type offset = 1;

        for (unsigned i = 0; i < parent::rank() - 1; ++i) {
            offset *= ext[parent::rank() - i - 1];
            ret[i] = offset;
        }

        return ret;
    }

    constexpr size_t extent(size_t index) const
    {
        constexpr std::array<index_type, parent::rank()> ext = {Extents...};
        return ext[index];
    }

    static constexpr std::array<index_type, parent::rank() - 1> offsets_ = compute_offsets();
};

template <class T>
struct accessor_basic
{
    using offset_policy = accessor_basic;
    using  element_type = T;
    using     reference = T &;
    using       pointer = T *;

    constexpr pointer offset(pointer p, ptrdiff_t i) const
    {
        return p + i;
    }

    constexpr reference access(pointer p, ptrdiff_t i) const
    {
        return p[i];
    }

    constexpr pointer decay(pointer p) const
    {
        return p;
    }
};

template <class T>
struct accessor_restrict
{
    using offset_policy = accessor_restrict;
    using  element_type = T;
    using     reference = T & __restrict;
    using       pointer = T * __restrict;

    constexpr pointer offset(pointer p, ptrdiff_t i) const
    {
        return p + i;
    }

    constexpr reference access(pointer __restrict p, ptrdiff_t i) const
    {
        return p[i];
    }

    constexpr pointer decay(pointer p) const
    {
        return p;
    }
};

template <typename T, typename Extents, typename LayoutPolicy, typename AccessorPolicy>
class basic_mdspan;

template <typename T, size_t... Extents, typename LayoutPolicy, typename AccessorPolicy>
class basic_mdspan<T, extents<Extents...>, LayoutPolicy, AccessorPolicy> : public span_extents<(... && (Extents != dynamic_extent)), extents<Extents...>>
{
    basic_mdspan() = delete;

public:
    using            extents_type = span_extents<(... && (Extents != dynamic_extent)), extents<Extents...>>;
    using             layout_type = LayoutPolicy;
    using           accessor_type = AccessorPolicy;
    using              index_type = typename extents_type::index_type;
    using            indexer_type = span_indexer<extents_type>;

    using              value_type = T;

    using                 pointer = typename accessor_type::pointer;
    using               reference = typename accessor_type::reference;

    using                iterator = iterator<basic_mdspan>;
    using          const_iterator = const_iterator<basic_mdspan>;

    template <typename... DynamicExtents>
    constexpr basic_mdspan(pointer data, DynamicExtents &&... ext) :
        extents_type(size_t(ext)...),
        data_(data)
    {
        static_assert(sizeof...(DynamicExtents) == extents_type::rank_dynamic(), "Constructor needs to specify all dynamic extents");
    }

    constexpr iterator begin()
    {
        return iterator(data_);
    }

    constexpr iterator end()
    {
        return iterator(data_ + this->size());
    }

    constexpr const_iterator begin() const
    {
        return cbegin();
    }

    constexpr const_iterator end() const
    {
        return cend();
    }

    constexpr const_iterator cbegin() const
    {
        return const_iterator(data_);
    }

    constexpr const_iterator cend() const
    {
        return const_iterator(data_ + this->size());
    }

    constexpr auto begin_circular()
    {
        if constexpr (extents_type::is_static())
            return aie::circular_iterator<T, this->extent(0)>(data_);
        else
            return aie::circular_iterator<T, dynamic_extent>(data_, this->size());
    }

    constexpr auto begin_circular() const
    {
        return cbegin_circular();
    }

    constexpr auto cbegin_circular() const
    {
        if constexpr (extents_type::is_static())
            return aie::const_circular_iterator<T, this->extent(0)>(data_);
        else
            return aie::const_circular_iterator<T, dynamic_extent>(data_, this->size());
    }

    template <typename... Indices>
    constexpr reference operator()(Indices &&... indices)
    {
        static_assert(sizeof...(indices) == basic_mdspan::rank(),
                      "The number of given indices must match the array's rank");

        const auto off = indexer_type::offset(*this, std::forward<Indices>(indices)...);
        return acc_.access(data_, off);
    }

    template <typename... Indices>
    constexpr reference operator()(Indices &&... indices) const
    {
        static_assert(sizeof...(indices) == basic_mdspan::rank(),
                      "The number of given indices must match the array's rank");

        const auto off = indexer_type::offset(*this, std::forward<Indices>(indices)...);
        return acc_.access(data_, off);
    }

    constexpr size_t size() const
    {
        if constexpr (extents_type::is_static())
            return extents_type::elems_static();
        else
            return this->offsets_[this->rank() - 1];
    }

private:
    pointer data_;
    [[no_unique_address]] accessor_type acc_;

    friend       iterator;
    friend const_iterator;
};

} // namespace aie::detail

#endif
