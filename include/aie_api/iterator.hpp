// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_ITERATOR__HPP__
#define __AIE_API_ITERATOR__HPP__

#include <iterator>

#include "detail/array_helpers.hpp"

namespace aie {

/**
 * @ingroup group_memory
 *
 * Implements an iterator that wraps around when it reaches the end of the buffer and, thus, has no end.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/forward_iterator">std::forward_iterator</a>.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Size     Size of the array if it is different than dynamic_extent. Otherwise, the size is not known at
 *                  compile time.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, size_t Size = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
class circular_iterator : public detail::circular_iterator<T, Size, 1, Resource> {
private:
    using base_type = detail::circular_iterator<T, Size, 1, Resource>;

    __aie_inline
    circular_iterator(const base_type& base) : base_type{base} {}
public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type* ;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class circular_iterator
    using detail::circular_iterator<T, Size, 1, Resource>::circular_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    circular_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    circular_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const circular_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const circular_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref circular_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, size_t Size = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
using const_circular_iterator = circular_iterator<const T, Size, Resource>;

/**
 * @ingroup group_memory
 *
 * Implements a vector iterator that wraps around when it reaches the end of the buffer and, thus, has no end.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/forward_iterator">std::forward_iterator</a>.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector returned when dereferencing the iterator.
 * @tparam Size     Size of the array if it is different than dynamic_extent. Otherwise, the size is not known at
 *                  compile time.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, unsigned Elems, size_t Size = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
class vector_circular_iterator : public detail::vector_circular_iterator<T, Elems, Size, 1, Resource> {
private:
    using         base_type = detail::vector_circular_iterator<T, Elems, Size, 1, Resource>;
    using         elem_type = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using       vector_type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<elem_type, Elems>, aie_dm_resource_get_v<T>>>;

    __aie_inline
    vector_circular_iterator(const base_type &base) : base_type{base} {}
public:
    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const vector_type &, vector_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const vector_type *, vector_type *>;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class vector_circular_iterator
    using detail::vector_circular_iterator<T, Elems, Size, 1, Resource>::vector_circular_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    vector_circular_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    vector_circular_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const vector_circular_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const vector_circular_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref circular_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, unsigned Elems, size_t Size = dynamic_extent, aie_dm_resource Resource = aie_dm_resource::none>
using const_vector_circular_iterator = vector_circular_iterator<const T, Elems, Size, Resource>;

/**
 * @ingroup group_memory
 *
 * Implements an iterator that wraps around when it reaches the end or the beginning of the buffer and, thus, has no
 * end.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/random_access_iterator">std::random_access_iterator</a>.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Size     Size of the array if it is different than dynamic_extent. Otherwise, the size is not known at
 *                  compile time.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, size_t Size, aie_dm_resource Resource = aie_dm_resource::none>
class random_circular_iterator : public detail::random_circular_iterator<T, Size, 1, Resource> {
private:
    using base_type = detail::random_circular_iterator<T, Size, 1, Resource>;

    __aie_inline
    random_circular_iterator(const base_type& base) : base_type(base) {}
public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type*;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class random_circular_iterator
    using detail::random_circular_iterator<T, Size, 1, Resource>::random_circular_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    random_circular_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    random_circular_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Returns a copy of the iterator that is located a number of steps ahead. */
    __aie_inline
    random_circular_iterator operator+(difference_type off) const { return base_type::operator+(off); }

    /** \brief Returns a copy of the iterator that is located a number of steps behind. */
    __aie_inline
    random_circular_iterator operator-(difference_type off) const { return base_type::operator-(off); }

    /** \brief Moves the iterator a number of steps forward. */
    __aie_inline
    random_circular_iterator operator+=(difference_type off) { return base_type::operator+=(off); }

    /** \brief Moves the iterator a number of steps back. */
    __aie_inline
    random_circular_iterator operator-=(difference_type off) { return base_type::operator-=(off); }

    /** \brief Accesses the value a number of steps away from the iterator. */
    __aie_inline
    reference operator[](difference_type off) { return base_type::operator[](off); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const random_circular_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const random_circular_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref random_circular_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, size_t Size, aie_dm_resource Resource = aie_dm_resource::none>
using const_random_circular_iterator = random_circular_iterator<const T, Size, Resource>;

/**
 * @ingroup group_memory
 *
 * Implements a vector iterator that wraps around when it reaches the end or the beginning of the buffer and, thus, has
 * no end.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/random_access_iterator">random access iterator</a>.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector returned when dereferencing the iterator.
 * @tparam Size     Size of the array if it is different than dynamic_extent. Otherwise, the size is not known at compile time.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, unsigned Elems, size_t Size, aie_dm_resource Resource = aie_dm_resource::none>
class vector_random_circular_iterator : public detail::vector_random_circular_iterator<T, Elems, Size, 1, Resource> {
private:
    using base_type   = detail::vector_random_circular_iterator<T, Elems, Size, 1, Resource>;
    using elem_type   = std::remove_const_t<aie_dm_resource_remove_t<T>>;
    using vector_type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<elem_type, Elems>, aie_dm_resource_get_v<T>>>;

    __aie_inline
    vector_random_circular_iterator(const base_type& base) : base_type{base} {}
public:
    using value_type        = vector_type;
    using reference         = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using pointer           = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = ptrdiff_t;

    // Constructor for class vector_random_circular_iterator
    using detail::vector_random_circular_iterator<T, Elems, Size, 1, Resource>::vector_random_circular_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    vector_random_circular_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    vector_random_circular_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Returns a copy of the iterator that is located a number of steps ahead. */
    __aie_inline
    vector_random_circular_iterator operator+(difference_type off) const { return base_type::operator+(off); }

    /** \brief Returns a copy of the iterator that is located a number of steps behind. */
    __aie_inline
    vector_random_circular_iterator operator-(difference_type off) const { return base_type::operator-(off); }

    /** \brief Moves the iterator a number of steps forward. */
    __aie_inline
    vector_random_circular_iterator operator+=(difference_type off) { return base_type::operator+=(off); }

    /** \brief Moves the iterator a number of steps back. */
    __aie_inline
    vector_random_circular_iterator operator-=(difference_type off) { return base_type::operator-=(off); }

    /** \brief Accesses the value a number of steps away from the iterator. */
    __aie_inline
    reference operator[](difference_type off) { return base_type::operator[](off); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const vector_random_circular_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const vector_random_circular_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref vector_random_circular_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, unsigned Elems, size_t Size, aie_dm_resource Resource = aie_dm_resource::none>
using const_vector_random_circular_iterator = vector_random_circular_iterator<const T, Elems, Size, Resource>;

/**
 * @ingroup group_memory
 *
 * Implements an iterator that traverses an array using vectors instead of scalar values. The buffer being traversed
 * needs to meet the alignment requirements to load/store vectors.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/random_access_iterator">random access iterator</a>.
 *
 * @tparam T     Type of the elements in the array.
 * @tparam Elems Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class vector_iterator : public detail::vector_iterator<T, Elems, 1, Resource> {
private:
    using         base_type = detail::vector_iterator<T, Elems, 1, Resource>;
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    __aie_inline
    vector_iterator(const base_type &base) : base_type{base} {}
public:
    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class vector_iterator
    using detail::vector_iterator<T, Elems, 1, Resource>::vector_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    vector_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    vector_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Returns a copy of the iterator that is located a number of steps ahead. */
    __aie_inline
    vector_iterator operator+(difference_type off) const { return base_type::operator+(off); }

    /** \brief Returns a copy of the iterator that is located a number of steps behind. */
    __aie_inline
    vector_iterator operator-(difference_type off) const { return base_type::operator-(off); }

    /** \brief Moves the iterator a number of steps forward. */
    __aie_inline
    vector_iterator operator+=(difference_type off) { return base_type::operator+=(off); }

    /** \brief Moves the iterator a number of steps back. */
    __aie_inline
    vector_iterator operator-=(difference_type off) { return base_type::operator-=(off); }

    /** \brief Accesses the value a number of steps away from the iterator. */
    __aie_inline
    reference operator[](difference_type off) { return base_type::operator[](off); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const vector_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const vector_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref vector_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using const_vector_iterator = vector_iterator<const std::remove_const_t<T>, Elems, Resource>;

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class restrict_vector_iterator : public detail::restrict_vector_iterator<T, Elems, 1, Resource> {
private:
    using         base_type = detail::restrict_vector_iterator<T, Elems, 1, Resource>;
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;

    __aie_inline
    restrict_vector_iterator(const base_type &base) : base_type{base} {}
public:
    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class restrict_vector_iterator
    using detail::restrict_vector_iterator<T, Elems, 1, Resource>::restrict_vector_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    restrict_vector_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    restrict_vector_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Pre-fix decrement: moves the iterator one step back.
     *
     * \return a reference to the iterator
     * \sa operator--(int)
     */
    __aie_inline
    restrict_vector_iterator &operator--() { return base_type::operator--(); }

    /** \brief Post-fix increment: moves the iterator one step back and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator--()
     */
    __aie_inline
    restrict_vector_iterator operator--(int) { return base_type::operator--(0); }

    /** \brief Returns a copy of the iterator that is located a number of steps ahead. */
    __aie_inline
    restrict_vector_iterator operator+(difference_type off) const { return base_type::operator+(off); }

    /** \brief Returns a copy of the iterator that is located a number of steps behind. */
    __aie_inline
    restrict_vector_iterator operator-(difference_type off) const { return base_type::operator-(off); }

    /** \brief Moves the iterator a number of steps forward. */
    __aie_inline
    restrict_vector_iterator operator+=(difference_type off) { return base_type::operator+=(off); }

    /** \brief Moves the iterator a number of steps back. */
    __aie_inline
    restrict_vector_iterator operator-=(difference_type off) { return base_type::operator-=(off); }

    /** \brief Accesses the value a number of steps away from the iterator. */
    __aie_inline
    reference operator[](difference_type off) { return base_type::operator[](off); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const restrict_vector_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const restrict_vector_iterator &other) const { return base_type::operator!=(other); }
};

template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using const_restrict_vector_iterator = restrict_vector_iterator<const std::remove_const_t<T>, Elems, Resource>;

/**
 * @ingroup group_memory
 *
 * Implements an iterator that traverses an array using vectors instead of scalar values.
 *
 * The interface meets <a href="https://en.cppreference.com/w/cpp/iterator/random_access_iterator">random access iterator</a>.
 *
 * @tparam T     Type of the elements in the array.
 * @tparam Elems Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 */
template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class unaligned_vector_iterator : public detail::unaligned_vector_iterator<T, Elems, Resource> {
private:
    using         base_type = detail::unaligned_vector_iterator<T, Elems, Resource>;
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<vector<std::remove_const_t<elem_type>, Elems>, aie_dm_resource_get_v<T>>>;
    using   vector_ref_type = unaligned_vector_ref<T, vector_type::size(), Resource>;

    __aie_inline
    unaligned_vector_iterator(const base_type &base) : base_type{base} {}
public:
    using        value_type = vector_type;
    using         reference = std::conditional_t<std::is_const_v<T>, const value_type &, value_type &>;
    using           pointer = std::conditional_t<std::is_const_v<T>, const value_type *, value_type *>;
    using iterator_category = std::random_access_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class unaligned_vector_iterator
    using detail::unaligned_vector_iterator<T, Elems, Resource>::unaligned_vector_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    unaligned_vector_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    unaligned_vector_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    vector_ref_type operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const unaligned_vector_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const unaligned_vector_iterator &other) const { return base_type::operator!=(other); }
};

/**
 * @ingroup group_memory
 *
 * Same as @ref unaligned_vector_iterator, but the contents of the iterated array cannot be modified.
 */
template <typename T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using const_unaligned_vector_iterator = unaligned_vector_iterator<const std::remove_const_t<T>, Elems, Resource>;

/**
 * @ingroup group_memory
 *
 * Returns an iterator for the array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr auto begin(T *base, size_t n)
{
    using type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<T, aie_dm_resource_get_v<T>>>;

    return (type *)base;
}

/**
 * @ingroup group_memory
 *
 * Returns an iterator for the constant array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr auto cbegin(const T *base, size_t n)
{
    return begin(base, n);
}

/**
 * @ingroup group_memory
 *
 * Returns an iterator for the given statically-sized array.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, unsigned Elems>
__aie_inline
constexpr auto begin(T (&base)[Elems])
{
    using type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<T, aie_dm_resource_get_v<T>>>;

    return (type *)base;
}

/**
 * @ingroup group_memory
 *
 * Returns an iterator that points at the end of the array described by the given address and size
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr auto end(T *base, size_t n)
{
    using type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<T, aie_dm_resource_get_v<T>>>;

    return (type *)(base + n);
}

/**
 * @ingroup group_memory
 *
 * Returns an iterator that points at the end of the given statically-sized array.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, unsigned Elems>
__aie_inline
constexpr auto end(T (&base)[Elems])
{
    using type = detail::add_memory_bank_t<Resource, aie_dm_resource_set_t<T, aie_dm_resource_get_v<T>>>;

    return (type *)(base + Elems);
}

/**
 * @ingroup group_memory
 *
 * Returns an iterator that points at the end of the constant array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr auto cend(const T *base, size_t n)
{
    return end(base, n);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr circular_iterator<T, dynamic_extent, Resource> begin_circular(T *base, size_t n)
{
    return circular_iterator<T, dynamic_extent, Resource>(base, n);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <size_t Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr circular_iterator<T, Elems, Resource> begin_circular(T *base)
{
    return circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the given statically-sized array.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t Elems>
__aie_inline
constexpr circular_iterator<T, Elems, Resource> begin_circular(T (&base)[Elems])
{
    return circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_circular_iterator<T, dynamic_extent, Resource> cbegin_circular(const T *base, size_t n)
{
    return const_circular_iterator<T, dynamic_extent, Resource>(base, {n});
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <size_t Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_circular_iterator<T, Elems, Resource> cbegin_circular(const T *base)
{
    return const_circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t Elems>
__aie_inline
constexpr const_circular_iterator<T, Elems, Resource> cbegin_circular(const T (&base)[Elems])
{
    return const_circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr vector_circular_iterator<T, Elems, dynamic_extent, Resource> begin_vector_circular(T *base, size_t n)
{
    return vector_circular_iterator<T, Elems, dynamic_extent, Resource>(base, {n});
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam ArrayElems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, size_t ArrayElems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr vector_circular_iterator<T, Elems, ArrayElems, Resource> begin_vector_circular(T *base)
{
    return vector_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t ArrayElems>
__aie_inline
constexpr vector_circular_iterator<T, Elems, ArrayElems, Resource> begin_vector_circular(T (&base)[ArrayElems])
{
        return vector_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_vector_circular_iterator<T, Elems, dynamic_extent, Resource>
cbegin_vector_circular(const T *base, size_t n)
{
    return const_vector_circular_iterator<T, Elems, dynamic_extent, Resource>(base, {n});
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam ArrayElems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, size_t ArrayElems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_vector_circular_iterator<T, Elems, ArrayElems, Resource> cbegin_vector_circular(const T *base)
{
    return const_vector_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t ArrayElems>
__aie_inline
constexpr const_vector_circular_iterator<T, Elems, ArrayElems, Resource>
cbegin_vector_circular(const T (&base)[ArrayElems])
{
    return const_vector_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a random-access circular iterator for the array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr random_circular_iterator<T, dynamic_extent, Resource> begin_random_circular(T *base, size_t n)
{
    return random_circular_iterator<T, dynamic_extent, Resource>(base, {n});
}

/**
 * @ingroup group_memory
 *
 * Returns a random-access circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <size_t Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr random_circular_iterator<T, Elems, Resource> begin_random_circular(T *base)
{
    return random_circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a random-access circular iterator for the array described by the given address and size.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t Elems>
__aie_inline
constexpr random_circular_iterator<T, Elems, Resource> begin_random_circular(T (&base)[Elems])
{
    return random_circular_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_random_circular, but the returned iterator is constant.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_random_circular_iterator<T, dynamic_extent, Resource> cbegin_random_circular(const T *base, size_t n)
{
    return const_random_circular_iterator<T, dynamic_extent, Resource>(base, {n});
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_random_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <size_t Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_random_circular_iterator<T, Elems, Resource> cbegin_random_circular(const T *base)
{
    return const_random_circular_iterator<T, Elems, Resource>(base, Elems);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_random_circular, but the returned iterator is constant.
 *
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t Elems>
__aie_inline
constexpr const_random_circular_iterator<T, Elems> cbegin_random_circular(const T (&base)[Elems])
{
    return const_random_circular_iterator<T, Elems>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr vector_random_circular_iterator<T, Elems, dynamic_extent, Resource>
begin_vector_random_circular(T *base, size_t n)
{
    return vector_random_circular_iterator<T, Elems, dynamic_extent, Resource>(base, n);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam ArrayElems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, size_t ArrayElems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr vector_random_circular_iterator<T, Elems, ArrayElems, Resource> begin_vector_random_circular(T *base)
{
    return vector_random_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a circular iterator for the array described by the given address and size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t ArrayElems>
__aie_inline
constexpr vector_random_circular_iterator<T, Elems, ArrayElems, Resource>
begin_vector_random_circular(T (&base)[ArrayElems])
{
    return vector_random_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 * @param n Number of elements in the array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_vector_random_circular_iterator<T, Elems, dynamic_extent, Resource>
cbegin_vector_random_circular(const T *base, size_t n)
{
    return const_vector_random_circular_iterator<T, Elems, dynamic_extent, Resource>(base, n);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam ArrayElems Number of elements in the array.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, size_t ArrayElems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
constexpr const_vector_random_circular_iterator<T, Elems, ArrayElems, Resource>
cbegin_vector_random_circular(const T *base)
{
    return const_vector_random_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_circular, but the returned iterator is constant.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @tparam Resource Data Memory resource to be used for the access when dereferencing the iterator.
 * @param base Statically-sized array.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T, size_t ArrayElems>
__aie_inline
constexpr const_vector_random_circular_iterator<T, Elems, ArrayElems, Resource>
cbegin_vector_random_circular(const T (&base)[ArrayElems])
{
    return const_vector_random_circular_iterator<T, Elems, ArrayElems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a vector iterator starting at the given address. Elements in the vector will have the same type of the
 * pointer parameter, and the size of the vector is specified via a template argument. The pointer is assumed to meet
 * the alignment requirements for a vector load of this size.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr vector_iterator<T, Elems, Resource> begin_vector(T *base)
{
    return vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a vector iterator starting at the given address. Elements in the vector will have the same type of the
 * pointer parameter, and the size of the vector is specified via a template argument.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr unaligned_vector_iterator<T, Elems, Resource> begin_unaligned_vector(T *base)
{
    return unaligned_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a vector iterator starting at the given address. Elements in the vector will have the same type of the
 * pointer parameter, and the size of the vector is specified via a template argument. The pointer is assumed to meet
 * the alignment requirements for a vector load of this size.
 *
 * The returned iterator is const.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr const_vector_iterator<T, Elems, Resource> begin_vector(const T *base)
{
    return const_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Same as begin_vector.
 *
 * The returned iterator is const.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr const_vector_iterator<T, Elems, Resource> cbegin_vector(const T *base)
{
    return const_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Returns a vector iterator starting at the given address. Elements in the vector will have the same type of the
 * pointer parameter, and the size of the vector is specified via a template argument.
 *
 * The returned iterator is const.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr const_unaligned_vector_iterator<T, Elems, Resource> cbegin_unaligned_vector(T *base)
{
    return const_unaligned_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Same as begin_vector, but the given pointer is considered restrict.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr restrict_vector_iterator<T, Elems, Resource> begin_restrict_vector(T *base)
{
    return restrict_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Same as begin_vector, but the given pointer is considered restrict.
 *
 * The returned iterator is const.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr const_restrict_vector_iterator<T, Elems, Resource> begin_restrict_vector(const T *base)
{
    return const_restrict_vector_iterator<T, Elems, Resource>(base);
}

/**
 * @ingroup group_memory
 *
 * Same as begin_vector, but the given pointer is considered restrict.
 *
 * The returned iterator is const.
 *
 * @tparam Elems Number of elements in the vectors returned by the iterator.
 * @param base Starting address for the iterator.
 */
template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
__aie_inline
constexpr const_restrict_vector_iterator<T, Elems, Resource> cbegin_restrict_vector(const T *base)
{
    return const_restrict_vector_iterator<T, Elems, Resource>(base);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType... Types>
__aie_inline
constexpr auto begin_vectors(Types * ...ptrs)
{
    return std::make_tuple(begin_vector<Elems, Resource>(ptrs)...);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType... Types>
__aie_inline
constexpr auto cbegin_vectors(const Types * ...ptrs)
{
    return std::make_tuple(cbegin_vector<Elems, Resource>(ptrs)...);
}

template <typename T, unsigned Steps>
class pattern_iterator : public detail::pattern_iterator<T, Steps>
{
private:
    using base_type = detail::pattern_iterator<T, Steps>;

    __aie_inline
    pattern_iterator(const base_type& base) : base_type{base} {}
public:
    using        value_type = T;
    using         reference = value_type&;
    using           pointer = value_type* ;
    using iterator_category = std::forward_iterator_tag;
    using   difference_type = ptrdiff_t;

    // Constructor for class circular_iterator
    using detail::pattern_iterator<T, Steps>::pattern_iterator;

    /** \brief Pre-fix increment: advances the iterator one step.
     * Every time the iterator reaches the end, it jumps back to its base position.
     *
     * \return a reference to the iterator
     * \sa operator++(int)
     */
    __aie_inline
    pattern_iterator &operator++() { base_type::operator++(); return *this; }

    /** \brief Post-fix increment: advances the iterator one step and returns a copy of its old state.
     *
     * \return a copy of the iterator before the increment operation took place.
     * \sa operator++()
     */
    __aie_inline
    pattern_iterator operator++(int) { return base_type::operator++(0); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    reference operator*() { return base_type::operator*(); }

    /** \brief Accesses the value in the iterator. */
    __aie_inline
    pointer operator->() { return base_type::operator->(); }

    /** \brief Return true if the two iterators reference the same value. */
    __aie_inline
    bool operator==(const pattern_iterator &other) const { return base_type::operator==(other); }

    /** \brief Return true if the two iterators reference different values. */
    __aie_inline
    bool operator!=(const pattern_iterator &other) const { return base_type::operator!=(other); }
};

template <typename T, unsigned Steps>
using const_pattern_iterator = pattern_iterator<std::add_const_t<T>, Steps>;

/**
 * @ingroup group_memory
 *
 * Returns a forward iterator for the array described by the given address.
 * On increment, the iterator is advanced by the number of elements described in the offset argument.
 * While the pattern iterator is a forward iterator, the offsets are described by a circular iterator.
 * For example:
 *
 * @code
 * std::array<int, 16> arr;
 * std::iota(arr.begin(), arr.end(), 0);
 * auto it = aie::begin_pattern<4>(arr.data(), 2, -1, 2, 1);
 *
 * for (unsigned i = 0; i < arr.size(); ++i)
 *   printf("%d ", *it++);
 * @endcode
 *
 * will output
 *
 * @code
 * 0 2 1 3 4 6 5 7 8 10 9 11 12 14 13 15
 * @endcode
 *
 * @param base    Starting address for the iterator.
 * @param offsets A parameter pack describing the stride of the iterator at each increment.
 */
template <unsigned Steps, typename T, typename... Offsets>
__aie_inline
constexpr auto begin_pattern(T *base, Offsets &&... offsets)
{
    static_assert(Steps != 0, "pattern iterator requires at least one offset");
    if constexpr (std::is_const_v<T>)
        return const_pattern_iterator<T, Steps>(base, std::forward<Offsets>(offsets)...);
    else
        return       pattern_iterator<T, Steps>(base, std::forward<Offsets>(offsets)...);
}

/**
 * @ingroup group_memory
 *
 * Similar to begin_pattern, but the returned iterator is constant.
 *
 * @param base    Starting address for the iterator.
 * @param offsets A parameter pack describing the stride of the iterator at each increment.
 *
 * @sa begin_pattern
 */
template <unsigned Steps, typename T, typename... Offsets>
__aie_inline
constexpr const_pattern_iterator<T, Steps> cbegin_pattern(const T *base, Offsets &&... offsets)
{
    return const_pattern_iterator<T, Steps>(base, std::forward<Offsets>(offsets)...);
}

/**
 * @ingroup group_memory
 *
 * Implements an input stream that reads from a memory buffer with vector granularity. The buffer being traversed
 * needs to meet the alignment requirements to load vectors.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class vector_input_buffer_stream : public detail::vector_input_buffer_stream<T, Elems, Resource>
{
private:
    using base_type = detail::vector_input_buffer_stream<T, Elems, Resource>;
public:
    using base_type::vector_input_buffer_stream;

    using vector_type = typename base_type::vector_type;

    /** \brief Returns the value from the buffer stream and increments the stream state.
      *
      * \sa pop()
      */
    constexpr vector_input_buffer_stream& operator>>(vector_type& v) { base_type::operator>>(v); return *this; }

    /** \brief Returns the value from the buffer stream and increments the stream state. */
    constexpr vector_type pop() { return base_type::pop(); }
};

/**
 * @ingroup group_memory
 *
 * Implements an output stream that writes into a memory buffer with vector granularity. The buffer being traversed
 * needs to meet the alignment requirements to store vectors.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class vector_output_buffer_stream : public detail::vector_output_buffer_stream<T, Elems, Resource>
{
private:
    using base_type = detail::vector_output_buffer_stream<T, Elems, Resource>;
public:
    using base_type::vector_output_buffer_stream;

    using vector_type = typename base_type::vector_type;

    /** \brief Writes the value to the buffer stream.
      *
      * \sa push()
      */
    constexpr vector_output_buffer_stream& operator<<(const vector_type& v) { base_type::operator<<(v); return *this; }

    /** \brief Writes the value to the buffer stream. */
    constexpr void push(const vector_type& v) { return base_type::push(v); }
};

/**
 * @ingroup group_memory
 *
 * Implements an input stream that reads from a memory buffer with vector granularity.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class unaligned_vector_input_buffer_stream : public detail::unaligned_vector_input_buffer_stream<T, Elems, Resource>
{
private:
    using base_type = detail::unaligned_vector_input_buffer_stream<T, Elems, Resource>;
public:
    using base_type::unaligned_vector_input_buffer_stream;

    using vector_type = typename base_type::vector_type;

    /** \brief Returns the value from the buffer stream and increments the stream state.
      *
      * \sa pop()
      */
    constexpr unaligned_vector_input_buffer_stream& operator>>(vector_type& v) { base_type::operator>>(v); return *this; }

    /** \brief Returns the value from the buffer stream and increments the stream state. */
    constexpr vector_type pop() { return base_type::pop(); }
};

/**
 * @ingroup group_memory
 *
 * Implements an output stream that writes into a memory buffer with vector granularity.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class unaligned_vector_output_buffer_stream : public detail::unaligned_vector_output_buffer_stream<T, Elems, Resource>
{
private:
    using base_type = detail::unaligned_vector_output_buffer_stream<T, Elems, Resource>;
public:
    using base_type::unaligned_vector_output_buffer_stream;

    using vector_type = typename base_type::vector_type;

    /** \brief Writes the value to the buffer stream.
      *
      * \sa push()
      */
    constexpr unaligned_vector_output_buffer_stream& operator<<(const vector_type& v) { base_type::operator<<(v); return *this; }

    /** \brief Writes the value to the buffer stream. */
    constexpr void push(const vector_type& v) { return base_type::push(v); }
};

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
auto make_vector_input_buffer_stream(const T *ptr) -> vector_input_buffer_stream<T, Elems, Resource>
{
    return vector_input_buffer_stream<T, Elems, Resource>(ptr);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
auto make_vector_output_buffer_stream(T *ptr) -> vector_output_buffer_stream<T, Elems, Resource>
{
    return vector_output_buffer_stream<T, Elems, Resource>(ptr);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
auto make_unaligned_vector_input_buffer_stream(const T *ptr) -> unaligned_vector_input_buffer_stream<T, Elems, Resource>
{
    return unaligned_vector_input_buffer_stream<T, Elems, Resource>(ptr);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, DecoratedElemBaseType T>
auto make_unaligned_vector_output_buffer_stream(T *ptr) -> unaligned_vector_output_buffer_stream<T, Elems, Resource>
{
    return unaligned_vector_output_buffer_stream<T, Elems, Resource>(ptr);
}

/**
 * @ingroup group_memory
 *
 * Implements an input stream that reads sparse vectors from a memory buffer.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the sparse vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class sparse_vector_input_buffer_stream;

#if AIE_API_ML_VERSION >= 200
template <DecoratedElemBaseType T, unsigned Elems, aie_dm_resource Resource>
class sparse_vector_input_buffer_stream : public detail::sparse_vector_input_buffer_stream<T, Elems, Resource>
{
private:
    using base_type = detail::sparse_vector_input_buffer_stream<T, Elems, Resource>;
public:
    using base_type::sparse_vector_input_buffer_stream;

    using vector_type = typename base_type::vector_type;

    /** \brief Returns the value from the buffer stream and increments the stream state.
      *
      * \sa pop()
      */
    constexpr sparse_vector_input_buffer_stream& operator>>(vector_type& v) { base_type::operator>>(v); return *this; }

    /** \brief Returns the value from the buffer stream and increments the stream state. */
    constexpr vector_type pop() { return base_type::pop(); }
};
#endif

#if AIE_API_ML_VERSION >= 210

/**
 * @ingroup group_memory
 *
 * Implements an input stream that reads block vectors from a memory buffer.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the block vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class block_vector_input_buffer_stream : public detail::block_vector_input_buffer_stream<T, Elems, Resource, /*Restrict=*/false>
{
private:
    using base_type = detail::block_vector_input_buffer_stream<T, Elems, Resource, /*Restrict=*/false>;
public:
    __aie_inline
    constexpr block_vector_input_buffer_stream(const T *ptr) : base_type(ptr) {}

    using vector_type = typename base_type::vector_type;

    /** \brief Returns the value from the buffer stream and increments the stream state.
      *
      * \sa pop()
      */
    __aie_inline
    constexpr block_vector_input_buffer_stream& operator>>(vector_type& v) { base_type::operator>>(v); return *this; }

    /** \brief Returns the value from the buffer stream and increments the stream state. */
    __aie_inline
    constexpr vector_type pop() { return base_type::pop(); }
};

/**
 * @ingroup group_memory
 *
 * Implements an output stream that writes block vectors to a memory buffer.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the block vector.
 * @tparam Resource Data Memory resource to be used for the access when writing to the buffer.
 */
template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class block_vector_output_buffer_stream : public detail::block_vector_output_buffer_stream<T, Elems, Resource, /*Restrict=*/false>
{
private:
    using base_type = detail::block_vector_output_buffer_stream<T, Elems, Resource, /*Restrict=*/false>;
public:
    __aie_inline
    constexpr block_vector_output_buffer_stream(T *ptr) : base_type(ptr) {}

    using vector_type = typename base_type::vector_type;
    using  accum_type = typename base_type::accum_type;

    /** \brief Writes the value to the buffer stream.
      *
      * \sa push()
      */
    __aie_inline
    constexpr block_vector_output_buffer_stream& operator<<(const vector_type& v) { base_type::operator<<(v); return *this; }

    /** \brief Writes the accumulator to the buffer stream, converting the values to `T`.
      *
      * \sa push()
      */
    __aie_inline
    constexpr block_vector_output_buffer_stream& operator<<(const accum_type& v) { base_type::operator<<(v); return *this; }

    /** \brief Writes the value to the buffer stream. */
    __aie_inline
    constexpr void push(const vector_type& v) { return base_type::push(v); }

    /** \brief Writes the accumulator to the buffer stream, converting the values to `T`. */
    __aie_inline
    constexpr void push(const accum_type& v, int shoft = 0) { return base_type::push(v, 0); }
};

/**
 * @ingroup group_memory
 *
 * Implements a restrict input stream that reads block vectors from a memory buffer.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the block vector.
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer.
 */
template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class block_vector_restrict_input_buffer_stream : public detail::block_vector_input_buffer_stream<T, Elems, Resource, /*Restrict=*/true>
{
private:
    using base_type = detail::block_vector_input_buffer_stream<T, Elems, Resource, /*Restrict=*/true>;
public:
    __aie_inline
    constexpr block_vector_restrict_input_buffer_stream(const T *ptr) : base_type(ptr) {}

    using vector_type = typename base_type::vector_type;

    /** \brief Returns the value from the buffer stream and increments the stream state.
      *
      * \sa pop()
      */
    __aie_inline
    constexpr block_vector_restrict_input_buffer_stream& operator>>(vector_type& v) { base_type::operator>>(v); return *this; }

    /** \brief Returns the value from the buffer stream and increments the stream state. */
    __aie_inline
    constexpr vector_type pop() { return base_type::pop(); }
};

/**
 * @ingroup group_memory
 *
 * Implements a restrict output stream that writes block vectors to a memory buffer.
 *
 * @tparam T        Type of the elements in the array.
 * @tparam Elems    Size of the block vector.
 * @tparam Resource Data Memory resource to be used for the access when writing to the buffer.
 */
template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
class block_vector_restrict_output_buffer_stream : public detail::block_vector_output_buffer_stream<T, Elems, Resource, /*Restrict=*/true>
{
private:
    using base_type = detail::block_vector_output_buffer_stream<T, Elems, Resource, /*Restrict=*/true>;
public:
    __aie_inline
    constexpr block_vector_restrict_output_buffer_stream(T *ptr) : base_type(ptr) {}

    using vector_type = typename base_type::vector_type;

    /** \brief Writes the value to the buffer stream.
      *
      * \sa push()
      */
    __aie_inline
    constexpr block_vector_restrict_output_buffer_stream& operator<<(const vector_type& v) { base_type::operator<<(v); return *this; }

    /** \brief Writes the value to the buffer stream. */
    __aie_inline
    constexpr void push(const vector_type& v) { return base_type::push(v); }
};

#if __AIE_ARCH__ == 21
template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using bfp_vector_input_buffer_stream [[deprecated("Use block_vector_input_buffer_stream<T, Elems, Resource> instead")]] =
            block_vector_input_buffer_stream<T, Elems, Resource>;

template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using bfp_vector_output_buffer_stream [[deprecated("Use block_vector_output_buffer_stream<T, Elems, Resource> instead")]] =
        block_vector_output_buffer_stream<T, Elems, Resource>;

template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using bfp_vector_restrict_input_buffer_stream [[deprecated("Use block_vector_restrict_input_buffer_stream<T, Elems, Resource> instead")]] =
        block_vector_restrict_input_buffer_stream<T, Elems, Resource>;

template <BlockType T, unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none>
using bfp_vector_restrict_output_buffer_stream [[deprecated("Use block_vector_restrict_output_buffer_stream<T, Elems, Resource> instead")]] =
        block_vector_restrict_output_buffer_stream<T, Elems, Resource>;
#endif

#endif

// Tensor Buffer Streams & multidim addressing

#if AIE_API_ML_VERSION >= 200
/**
 * @ingroup group_memory
 *
 * Increments the pointer by inc.
 * Note that inc may be a multidimensional struct.
 *
 * @param ptr    The pointer to increment.
 * @param inc    The increment to apply to the pointer.
 *               Can be int, dims_2d_t, or dims_3d_t.
 */
template <typename T, typename Incr>
    requires(std::integral<Incr> || detail::utils::is_one_of_v<Incr, dims_2d_t, dims_3d_t>)
__aie_inline
T* add(T *ptr, Incr& inc) {
    constexpr bool ByteIncr = false;
    return detail::multidim_incr<ByteIncr>::run(ptr, inc);
}

/**
 * @ingroup group_memory
 *
 * Increments the pointer by inc bytes.
 * Note that inc may be a multidimensional struct.
 *
 * @param ptr    The pointer to increment.
 * @param inc    The byte increment to apply to the pointer.
 *               Can be int, dims_2d_t, or dims_3d_t.
 */
template <typename T, typename Incr>
    requires(std::integral<Incr> || detail::utils::is_one_of_v<Incr, dims_2d_t, dims_3d_t>)
__aie_inline
T* add_byte(T *ptr, Incr& inc) {
    constexpr bool ByteIncr = true;
    return detail::multidim_incr<ByteIncr>::run(ptr, inc);
}
#endif


using dim_2d                = detail::dim_2d;
using dim_3d                = detail::dim_3d;
using sliding_window_dim_1d = detail::sliding_window_dim_1d;
using sliding_window_dim_2d = detail::sliding_window_dim_2d;
using sliding_window_dim_3d = detail::sliding_window_dim_3d;
using contiguous_dim        = detail::contiguous_dim;

using tbs_mode = detail::tbs_mode;
using data_layout = detail::data_layout;

struct tensor_dim
{
    unsigned num;
    int step;

    __aie_inline constexpr tensor_dim() = default;
    __aie_inline constexpr explicit tensor_dim(unsigned num, int step) : num(num), step(step) {}
};

template <typename T> concept TensorDim          = std::is_same_v<T, tensor_dim>;
template <typename T> concept NativeDim          = detail::utils::is_one_of_v<std::decay_t<T>, int, dim_2d, dim_3d, contiguous_dim>;
template <typename T> concept SlidingDim         = detail::utils::is_one_of_v<std::decay_t<T>, sliding_window_dim_1d, sliding_window_dim_2d, sliding_window_dim_3d>;
template <typename T> concept NativeOrSlidingDim = NativeDim<T> || SlidingDim<T>;

template <unsigned Rank, typename T, unsigned Elems, data_layout Layout = data_layout::row_major, typename Repr = detail::default_repr_t<Rank>, bool ByteSteps = false>
    requires (arch::is(arch::Gen2) && Rank > 0)
class tensor_descriptor
{
public:
    static constexpr unsigned elems = Elems;
    using        type = T;
    using vector_type = detail::tensor_desc_type_t<T, Elems>;

    static constexpr unsigned num_levels            = std::tuple_size_v<Repr>;
    static constexpr bool has_innermost_sliding_dim = detail::utils::is_one_of_v<std::tuple_element_t<num_levels - 1, Repr>,
                                                                                 sliding_window_dim_1d, sliding_window_dim_2d, sliding_window_dim_3d>;

    static_assert(!(Mask<T> && has_innermost_sliding_dim), "Sliding windows not supported for masks");

    __aie_inline constexpr tensor_descriptor() = default;

    __aie_inline
    explicit constexpr tensor_descriptor(const std::array<tensor_dim, Rank>& dims)
        : it_desc_(to_iteration_descriptor(dims))
    {}

    __aie_inline
    explicit constexpr tensor_descriptor(const contiguous_dim& dim)
        : it_desc_(std::make_tuple(dim))
    {}

    __aie_inline
    constexpr tensor_descriptor(const tensor_descriptor& other) : it_desc_(other.it_desc_) {}

private:
    template <aie_dm_resource Resource, tbs_mode Mode, DecoratedElemBaseOrBlockType T2, typename TensorDescriptor>
        requires (arch::is(arch::Gen2))
    friend constexpr auto make_tensor_buffer_stream(T2 *base, const TensorDescriptor& dims);
    template <aie_dm_resource Resource, tbs_mode Mode, DecoratedElemBaseOrBlockType T2, typename TensorDescriptor>
        requires (arch::is(arch::Gen2))
    friend constexpr auto make_tensor_buffer_stream(const T2 *base, const TensorDescriptor& dims);
    template <aie_dm_resource Resource, tbs_mode Mode, DecoratedElemBaseOrBlockType T2, typename TensorDescriptor>
        requires (arch::is(arch::Gen2))
    friend constexpr auto make_restrict_tensor_buffer_stream(T2 *base, const TensorDescriptor& dims);

    template <typename T2, unsigned Elems2, data_layout Layout2, NativeDim... Args>
        requires (arch::is(arch::Gen2))
    friend constexpr auto make_tensor_descriptor_from_native(Args&&... args);
    template <typename T2, unsigned Elems2, data_layout Layout2, NativeOrSlidingDim... Args>
        requires (arch::is(arch::Gen2))
    friend constexpr auto make_tensor_descriptor_from_native_bytes(Args&&... args);

    template <typename ResourceType,
              ResourceType Resource,
              tbs_mode Mode,
              bool Restrict,
              bool Unaligned,
              typename T2,
              typename Desc>
    friend auto detail::make_tensor_buffer_stream(T2&& src, const Desc& desc);

    template <typename ResourceType,
              ResourceType Resource,
              tbs_mode Mode,
              bool Restrict,
              typename StreamType,
              typename MemType,
              typename Desc>
    friend auto detail::make_tensor_buffer_stream(StreamType* stream_ptr, MemType* mem_ptr, const Desc& desc);

    // For native initialization
    __aie_inline
    explicit constexpr tensor_descriptor(Repr&& dims) : it_desc_(std::move(dims))
    {}

    template <int ReprIdx = 0, int Idx = 0>
    __aie_inline
    static constexpr auto to_iteration_descriptor(const std::array<tensor_dim, Rank>& dims)
    {
        using detail::dim_2d;
        using detail::dim_3d;

        constexpr bool innermost = ReprIdx == std::tuple_size_v<Repr> - 1;
        // Implicit increments are only present when using FIFO-based operations
        // Outer level TBS will not incur an implicit increment
        constexpr int implicit_incr = [&](){
            if constexpr (innermost) {
#if __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
                if constexpr (detail::is_valid_block_type_v<T>)
                    return vector_type::memory_bytes();
#endif
            }

            return 0u;
        }();

        constexpr int scale = [&]() {
            if constexpr (ByteSteps)
                return 1;
#if __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
            else if constexpr (detail::is_valid_block_type_v<T>)
                return vector_type::memory_bytes();
#endif
            else if constexpr (Elems == 1)
                return sizeof(T);
            else if constexpr (Mask<T>)
                return Elems / 8;
            else
                return vector_type::bytes();
        }();

        if      constexpr (ReprIdx == std::tuple_size_v<Repr>) {
            return std::make_tuple();
        }
        else if constexpr (std::is_same_v<std::tuple_element_t<ReprIdx, Repr>, dim_3d>) {
            unsigned num1 = dims[Idx + 2].num - 1;
            unsigned num2 = dims[Idx + 1].num - 1;
            int inc1 = scale * (dims[Idx + 2].step) - implicit_incr;
            int inc2 = scale * (dims[Idx + 1].step - num1 * dims[Idx + 2].step) - implicit_incr;
            int inc3 = scale * (dims[Idx    ].step - num2 * dims[Idx + 1].step - num1 * dims[Idx + 2].step) - implicit_incr;
            return std::tuple_cat(std::make_tuple(dim_3d{num1, inc1,
                                                         num2, inc2,
                                                               inc3}),
                                  to_iteration_descriptor<ReprIdx + 1, Idx + 3>(dims));
        }
        else if constexpr (std::is_same_v<std::tuple_element_t<ReprIdx, Repr>, dim_2d>) {
            unsigned num1 = dims[Idx + 1].num - 1;
            int inc1 = scale * (dims[Idx + 1].step) - implicit_incr;
            int inc2 = scale * (dims[Idx    ].step - num1 * dims[Idx + 1].step) - implicit_incr;
            return std::tuple_cat(std::make_tuple(dim_2d{num1, inc1,
                                                               inc2}),
                                  to_iteration_descriptor<ReprIdx + 1, Idx + 2>(dims));
        }
        else if constexpr (std::is_same_v<std::tuple_element_t<ReprIdx, Repr>, contiguous_dim>) {
            
            // Only support a contiguous dimension as the innermost dimension for now
            static_assert(ReprIdx == std::tuple_size_v<Repr> - 1);

            // Handle contiguous_dim by ignoring the step and treating it as contiguous
            return std::tuple_cat(std::make_tuple(contiguous_dim{}), to_iteration_descriptor<ReprIdx + 1, Idx + 1>(dims));
        }
        else { // 1D
            return std::tuple_cat(std::make_tuple(dims[Idx].step * scale - implicit_incr),
                                  to_iteration_descriptor<ReprIdx + 1, Idx + 1>(dims));
        }
    }

    using tensor_iteration_descriptor = Repr;
    static constexpr data_layout layout = Layout;
    Repr it_desc_;
};

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of aie::vector<T, Elems>,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 */
template <typename T, unsigned Elems = 1u, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    return make_tensor_descriptor<T, Elems, detail::default_repr_t<Rank>, Layout>(std::forward<Args>(args)...);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of aie::vector<T, Elems>,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<T, Elems, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <typename T, unsigned Elems, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr bool byte_steps = false;
    return tensor_descriptor<Rank, T, Elems, Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor of scalar values that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of aie::vector<T, Elems>,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<T, Elems, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <typename T, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr unsigned Elems = 1u;
    constexpr bool byte_steps = false;
    return tensor_descriptor<Rank, T, Elems, Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Layout The memory order of the tensor.
 * @param  arg    A contiguous_dim object.
 *
 * The returned descriptor is intended to be used to create a tensor buffer stream
 * that tranverses memory linearly. Such a tensor buffer stream would be akin to
 * creating a basic vector iterator.
 */
template <typename T, unsigned Elems = 1u, data_layout Layout = data_layout::row_major>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(const contiguous_dim &arg)
{
    constexpr unsigned Rank = 1;
    constexpr bool byte_steps = false;
    return tensor_descriptor<Rank, T, Elems, Layout, std::tuple<contiguous_dim>, byte_steps>(arg);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of bytes,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 */
template <typename T, unsigned Elems = 1u, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_bytes(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    return make_tensor_descriptor_bytes<T, Elems, detail::default_repr_t<Rank>, Layout>(std::forward<Args>(args)...);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of bytes,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<T, Elems, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <typename T, unsigned Elems, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_bytes(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr bool byte_steps = true;
    return tensor_descriptor<Rank, T, Elems, Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor of scalar values that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of bytes,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<T, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <typename T, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_bytes(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr unsigned Elems = 1u;
    constexpr bool byte_steps = true;
    return tensor_descriptor<Rank, T, Elems, Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor from native dim types (int, dim_2d, dim_3d) that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of native dim types which describe the multidimensional increments,
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 *
 * \sa aie::dim_2d, aie::dim_3d
 */
template <typename T, unsigned Elems = 1u, data_layout Layout = data_layout::row_major, NativeDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_from_native(Args&&... args)
{
    constexpr unsigned Rank = detail::compute_rank_v<Args...>;
    constexpr bool byte_steps = false;
    using vector_type = detail::tensor_desc_type_t<T, Elems>;
    constexpr int scale = [](){
        if constexpr (Elems == 1) return sizeof(T);
        else                      return vector_type::bytes();
    }();

    constexpr auto apply_scale = []<typename U>(U&& arg, int scale){
        using U2 = std::decay_t<U>;
        if      constexpr (std::is_same_v<int,            U2>) return arg * scale;
        else if constexpr (std::is_same_v<detail::dim_2d, U2>) return detail::dim_2d(arg.num1, arg.inc1 * scale, arg.inc2 * scale);
        else if constexpr (std::is_same_v<detail::dim_3d, U2>) return detail::dim_3d(arg.num1, arg.inc1 * scale, arg.num2, arg.inc2 * scale, arg.inc3 * scale);
    };

    auto&& dims = std::make_tuple(apply_scale(std::forward<Args>(args), scale)...);
    return tensor_descriptor<Rank, T, Elems, Layout, std::decay_t<decltype(dims)>, byte_steps>(std::move(dims));
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor from native dim types (int, dim_2d, dim_3d) that is to be used to create a tensor buffer stream.
 *
 * @tparam T      Type of the elements in the described tensor.
 * @tparam Elems  Size of the vector used as part of the tensor descriptor.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of native dim types which describe the multidimensional increments, in units of bytes,
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 *
 * \sa aie::dim_2d, aie::dim_3d
 */
template <typename T, unsigned Elems = 1u, data_layout Layout = data_layout::row_major, NativeOrSlidingDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_from_native_bytes(Args&&... args)
{
    constexpr unsigned Rank = detail::compute_rank_v<Args...>;
    constexpr bool byte_steps = true;
    auto&& dims = std::make_tuple(std::forward<Args>(args)...);

    // Ensure, at maximum, one sliding_window_dim_*d is passed
    constexpr unsigned num_sliding_dims = (detail::utils::is_one_of_v<Args, sliding_window_dim_1d, sliding_window_dim_2d, sliding_window_dim_3d> + ...);
    static_assert(num_sliding_dims <= 1, "Only a single sliding_window_dim is supported");
    static_assert(!(Elems == 1 && num_sliding_dims > 0), "Sliding window not supported for scalar loads");
    // If present, ensure that the sliding_window_dim is the last argument
    if constexpr (num_sliding_dims == 1) {
        using innermost_type = std::tuple_element_t<sizeof...(Args) - 1, std::decay_t<decltype(dims)>>;
        static_assert(detail::utils::is_one_of_v<innermost_type, sliding_window_dim_1d, sliding_window_dim_2d, sliding_window_dim_3d>,
                      "sliding_window_dim is required to be the innermost dimension");
    }

    return tensor_descriptor<Rank, T, Elems, Layout, std::decay_t<decltype(dims)>, byte_steps>(std::move(dims));
}


// Mask APIs

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of aie::vector<T, Elems>,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 */
template <Mask M, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    return make_tensor_descriptor<M, detail::default_repr_t<Rank>, Layout>(std::forward<Args>(args)...);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of aie::vector<T, Elems>,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<aie::mask<32>, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <Mask M, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr bool byte_steps = false;
    return tensor_descriptor<Rank, M, M::size(), Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Layout The memory order of the tensor.
 * @param  arg    A contiguous_dim object.
 *
 * The returned descriptor is intended to be used to create a tensor buffer stream
 * that tranverses memory linearly. Such a tensor buffer stream would be akin to
 * creating a basic iterator.
 */
template <Mask M, data_layout Layout = data_layout::row_major>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor(const contiguous_dim &arg)
{
    constexpr unsigned Rank = 1;
    constexpr bool byte_steps = false;
    return tensor_descriptor<Rank, M, M::size(), Layout, std::tuple<contiguous_dim>, byte_steps>(arg);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of bytes,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 */
template <Mask M, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_bytes(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    return make_tensor_descriptor_bytes<M, detail::default_repr_t<Rank>, Layout>(std::forward<Args>(args)...);
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Repr   A std::tuple of dimension types to specify the custom partition.
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of aie::tensor_dim which describe the size and step, in units of bytes,
 *                of each dimension of the tensor from the origin. The order is expected to be outermost to innermost.
 *
 * The following snippet shows a rank 6 descriptor with a custom partition of an outermost 2D stream, an inner 1D stream,
 * which in turn has an inner 3D stream.
 *
 * @code
 * using Repr = std::tuple<aie::dim_2d, int, aie::dim_3d>;
 * auto desc = aie::make_tensor_descriptor<aie::mask<N>, Repr>(...);
 * @endcode
 *
 * \note The rank specified in Repr must match the rank specified by the args list.
 */
template <Mask M, typename Repr, data_layout Layout = data_layout::row_major, TensorDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_bytes(Args&&... args)
{
    constexpr unsigned Rank = sizeof...(Args);
    static_assert(Rank == detail::compute_rank_v<Repr>);
    constexpr bool byte_steps = true;
    return tensor_descriptor<Rank, M, M::size(), Layout, Repr, byte_steps>(std::array<tensor_dim, Rank>{std::forward<Args>(args)...});
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor from native dim types (int, dim_2d, dim_3d) that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of native dim types which describe the multidimensional increments,
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 *
 * \sa aie::dim_2d, aie::dim_3d
 */
template <Mask M, data_layout Layout = data_layout::row_major, NativeDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_from_native(Args&&... args)
{
    constexpr unsigned Rank = detail::compute_rank_v<Args...>;
    constexpr bool byte_steps = false;

    constexpr auto apply_scale = []<typename U>(U&& arg, int scale){
        using U2 = std::decay_t<U>;
        if      constexpr (std::is_same_v<int,            U2>) return arg * scale;
        else if constexpr (std::is_same_v<detail::dim_2d, U2>) return detail::dim_2d(arg.num1, arg.inc1 * scale, arg.inc2 * scale);
        else if constexpr (std::is_same_v<detail::dim_3d, U2>) return detail::dim_3d(arg.num1, arg.inc1 * scale, arg.num2, arg.inc2 * scale, arg.inc3 * scale);
    };

    auto&& dims = std::make_tuple(apply_scale(std::forward<Args>(args), M::size() / 8)...);
    return tensor_descriptor<Rank, M, M::size(), Layout, std::decay_t<decltype(dims)>, byte_steps>(std::move(dims));
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor descriptor from native dim types (int, dim_2d, dim_3d) that is to be used to create a tensor buffer stream of aie::mask values.
 *
 * @tparam M      An aie::mask<N> type
 * @tparam Layout The memory order of the tensor.
 * @param  args   A pack of native dim types which describe the multidimensional increments, in units of bytes,
 *
 * AIE hardware is limited to 3D addressing and therefore tensors of higher rank are partitioned into nested
 * tensor buffer streams. The default partitioning for a rank N tensor is (N/3)x3D + 1x(N%3)D. For example, a 5D
 * tensor buffer stream will be represented as an outer 3D tensor buffer stream with an inner 2D tensor buffer stream.
 *
 * \sa aie::dim_2d, aie::dim_3d
 */
template <Mask M, data_layout Layout = data_layout::row_major, NativeDim... Args>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_descriptor_from_native_bytes(Args&&... args)
{
    constexpr unsigned Rank = detail::compute_rank_v<Args...>;
    constexpr bool byte_steps = true;
    auto&& dims = std::make_tuple(std::forward<Args>(args)...);
    return tensor_descriptor<Rank, M, M::size(), Layout, std::decay_t<decltype(dims)>, byte_steps>(std::move(dims));
}

/**
 * @ingroup group_memory
 *
 * Creates a tensor buffer stream for accessing multidimensional data with optimized memory addressing patterns.
 *
 * Tensor buffer streams provide hardware-accelerated multidimensional memory access with automatic address
 * computation for complex access patterns. They support up to 3D addressing natively, with higher-dimensional
 * tensors automatically partitioned into nested streams.
 *
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer. Defaults to none.
 *                  If not specified, the resource will be extracted from the pointer type if available.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the elements in the tensor. Can be a base element type or block type.
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param base         Starting address for the tensor buffer stream.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A tensor buffer stream object that can be used to read vectors from the tensor.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 */
template <aie_dm_resource Resource = aie_dm_resource::none,
          tbs_mode Mode            = tbs_mode::default_mode,
          DecoratedElemBaseOrBlockType T,
          typename TensorDescriptor>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_buffer_stream(T *base, const TensorDescriptor& tensor_desc)
{
    using desc_type = typename TensorDescriptor::type;

    static_assert(Mask<desc_type> || std::is_same_v<aie_dm_resource_remove_t<T>, desc_type>,
                  "Input data type does not match tensor descriptor");

    // Use explicitly selected resource if set, otherwise try to extract resource from the pointer type.
    // If that is also none, then use none.
    constexpr aie_dm_resource _Resource = (Resource != aie_dm_resource::none) ? Resource
                                                                              : aie_dm_resource_get_v<T>;
                                        
    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, _Resource, Mode>(base, tensor_desc);
}

/**
 * @ingroup group_memory
 *
 * Creates a constant tensor buffer stream for accessing multidimensional data with optimized memory addressing patterns.
 *
 * This overload accepts a const pointer and creates a read-only tensor buffer stream. The returned stream
 * can only be used for reading data from the tensor, preventing accidental modifications.
 *
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer. Defaults to none.
 *                  If not specified, the resource will be extracted from the pointer type if available.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the elements in the tensor. Can be a base element type or block type.
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param base         Starting address for the tensor buffer stream (const).
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A constant tensor buffer stream object that can be used to read vectors from the tensor.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 */
template <aie_dm_resource Resource = aie_dm_resource::none,
          tbs_mode Mode            = tbs_mode::default_mode,
          DecoratedElemBaseOrBlockType T,
          typename TensorDescriptor>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_tensor_buffer_stream(const T *base, const TensorDescriptor& tensor_desc)
{
    using desc_type = typename TensorDescriptor::type;

    static_assert(Mask<desc_type> || std::is_same_v<aie_dm_resource_remove_t<T>, desc_type>,
                  "Input data type does not match tensor descriptor");

    // Use explicitly selected resource if set, otherwise try to extract resource from the pointer type.
    // If that is also none, then use none.
    constexpr aie_dm_resource _Resource = (Resource != aie_dm_resource::none) ? Resource
                                                                              : aie_dm_resource_get_v<T>;
                                        
    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, _Resource, Mode>(base, tensor_desc);
}

/**
 * @ingroup group_memory
 *
 * Creates a restrict tensor buffer stream for accessing multidimensional data with optimized memory addressing patterns.
 *
 * This function creates a tensor buffer stream with restrict semantics, indicating to the compiler that the
 * pointer is the only way to access the underlying data during the lifetime of the stream. This allows for
 * more aggressive optimizations by removing potential aliasing concerns.
 *
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer. Defaults to none.
 *                  If not specified, the resource will be extracted from the pointer type if available.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the elements in the tensor. Can be a base element type or block type.
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param base         Starting address for the tensor buffer stream (restrict-qualified).
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A restrict tensor buffer stream object that can be used to read/write vectors from/to the tensor
 *         with additional optimization opportunities due to restrict semantics.
 *
 * @sa make_tensor_descriptor
 * @sa make_tensor_buffer_stream
 */
template <aie_dm_resource Resource = aie_dm_resource::none,
          tbs_mode Mode            = tbs_mode::default_mode,
          DecoratedElemBaseOrBlockType T,
          typename TensorDescriptor>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_restrict_tensor_buffer_stream(T * __restrict base, const TensorDescriptor& tensor_desc)
{
    using desc_type = typename TensorDescriptor::type;

    static_assert(Mask<desc_type> || std::is_same_v<aie_dm_resource_remove_t<T>, desc_type>,
                  "Input data type does not match tensor descriptor");
    constexpr bool Restrict = true;

    // Use explicitly selected resource if set, otherwise try to extract resource from the pointer type.
    // If that is also none, then use none.
    constexpr aie_dm_resource _Resource = (Resource != aie_dm_resource::none) ? Resource
                                                                              : aie_dm_resource_get_v<T>;
                                        
    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, _Resource, Mode, Restrict>((T*)base, tensor_desc);
}

/**
 * @ingroup group_memory
 *
 * Creates a constant tensor buffer stream for accessing unaligned multidimensional data with optimized memory addressing patterns.
 *
 * This overload accepts a const pointer and creates a read-only tensor buffer stream. The returned stream
 * can only be used for reading data from the tensor, preventing accidental modifications.
 *
 * @tparam Resource Data Memory resource to be used for the access when reading from the buffer. Defaults to none.
 *                  If not specified, the resource will be extracted from the pointer type if available.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the elements in the tensor. Can be a base element type or block type.
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param base         Starting address for the tensor buffer stream (const).
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A constant tensor buffer stream object that can be used to read vectors from the tensor.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 */
template <aie_dm_resource Resource = aie_dm_resource::none,
          tbs_mode Mode            = tbs_mode::default_mode,
          DecoratedElemBaseOrBlockType T,
          typename TensorDescriptor>
    requires (arch::is(arch::Gen2))
__aie_inline
constexpr auto make_unaligned_tensor_buffer_stream(const T *base, const TensorDescriptor& tensor_desc)
{
    using desc_type = typename TensorDescriptor::type;

    static_assert(std::is_same_v<aie_dm_resource_remove_t<T>, desc_type>,
                  "Input data type does not match tensor descriptor");
    constexpr bool Restrict  = false;
    constexpr bool Unaligned = true;

    // Use explicitly selected resource if set, otherwise try to extract resource from the pointer type.
    // If that is also none, then use none.
    constexpr aie_dm_resource _Resource = (Resource != aie_dm_resource::none) ? Resource
                                                                              : aie_dm_resource_get_v<T>;
                                        
    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, _Resource, Mode, Restrict, Unaligned>(base, tensor_desc);
}


} // namespace aie

#endif // __AIE_API_ITERATOR__HPP__
