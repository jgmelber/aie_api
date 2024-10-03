// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE_ADF_IO_BUFFER_HPP__
#define __AIE_API_AIE_ADF_IO_BUFFER_HPP__

#include <adf.h>
#include "../aie.hpp"
#include "../iterator.hpp"


namespace aie {

template <aie_dm_resource Resource, typename T>
constexpr auto begin(T *, size_t n);

template <aie_dm_resource Resource, typename T>
constexpr auto cbegin(const T *, size_t n);

/**
 * @ingroup group_adf
 *
 * Returns a foward iterator over given io buffer. Requires given io buffer
 * to have linear addressing mode.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin(adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return begin<Resource>(port.data(), 0);
}

/**
 * @ingroup group_adf
 *
 * Returns a foward const iterator over given io buffer. Requires given buffer
 * port to have linear addressing mode.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin(const adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return cbegin<Resource>(port.data(), 0);
}

/**
 * @ingroup group_adf
 *
 * Returns a foward const iterator over given io buffer. Requires given buffer
 * port to have linear addressing mode.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin<Resource>(port);
}


/**
 * @ingroup group_adf
 *
 * Returns a foward circular iterator over given io buffer.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_circular(adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return circular_iterator<T, aie::dynamic_extent, Resource>
            (port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return circular_iterator<T, sizeInclMargin, Resource>
            (port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const foward circular iterator over given io buffer.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return const_circular_iterator<T, aie::dynamic_extent, Resource>
            (port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return const_circular_iterator<T, sizeInclMargin, Resource>
            (port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const foward iterator over given io buffer.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_circular<Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a random iterator over given io buffer. If the port has circular
 * addressing the iterator is a circular random iterator.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_random_circular(adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return detail::random_circular_iterator<T, aie::dynamic_extent, 1, Resource>
            (port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return detail::random_circular_iterator<T, sizeInclMargin, 1, Resource>
            (port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a random const iterator over given io buffer. If the port has circular
 * addressing the iterator is a circular random const iterator.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_random_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return const_random_circular_iterator<T, aie::dynamic_extent, Resource>
            (port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return const_random_circular_iterator<T, sizeInclMargin, Resource>
            (port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a random const iterator over given io buffer. If the port has circular
 * addressing the iterator is a circular random const iterator.
 *
 * @tparam Resource
 * @param port The io buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_random_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_random_circular<Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a vector iterator over given io buffer. Requires given io buffer
 * to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector(adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return vector_iterator<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector iterator over given io buffer. Requires given buffer
 * port to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector(const adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return const_vector_iterator<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector iterator over given io buffer. Requires given buffer
 * port to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_vector(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_vector<Elems, Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a restrict vector iterator over given io buffer.
 * Requires given io buffer to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_restrict_vector(adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return restrict_vector_iterator<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a const restrict vector iterator over given io buffer.
 * Requires given buffer port to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_restrict_vector(const adf::io_buffer<T, Dir, Config> &port)
{
    static_assert(std::is_same<typename Config::addressing_mode, adf::addressing::linear>::value);
    return const_restrict_vector_iterator<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a const restrict vector iterator over given io buffer.
 * Requires given buffer port to have linear addressing mode.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_restrict_vector(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_restrict_vector<Elems, Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a vector circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector_circular(adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return aie::vector_circular_iterator
            <T, Elems, aie::dynamic_extent, Resource>(port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return aie::vector_circular_iterator
            <T, Elems, sizeInclMargin, Resource>(port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return aie::const_vector_circular_iterator
            <T, Elems, aie::dynamic_extent, Resource>(port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return aie::const_vector_circular_iterator
            <T, Elems, sizeInclMargin, Resource>(port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_vector_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_vector_circular<Elems, Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a vector random circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector_random_circular(adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return aie::detail::vector_random_circular_iterator
            <T, Elems, aie::dynamic_extent, 1, Resource>(port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return aie::detail::vector_random_circular_iterator
            <T, Elems, sizeInclMargin, 1, Resource>(port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector random circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_vector_random_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    if constexpr (Config::dims_type::inherited() || Config::_margin == adf::inherited_margin) {
        return aie::const_vector_random_circular_iterator
            <T, Elems, aie::dynamic_extent, Resource>(port.data(), port.base(), port.size_incl_margin());
    }
    else {
        constexpr unsigned sizeInclMargin = Config::dims_type::size() + Config::_margin;
        return aie::const_vector_random_circular_iterator
            <T, Elems, sizeInclMargin, Resource>(port.data(), port.base());
    }
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector random circular iterator over given io buffer.
 *
 * @tparam Elems The size of the vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_vector_random_circular(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_vector_random_circular<Elems, Resource>(port);
}

} // namespace aie

#endif
