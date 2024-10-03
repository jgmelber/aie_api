// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

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

} // namespace aie

#endif // __AIE_API_ITERATOR__HPP__
