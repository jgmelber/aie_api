// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

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

/**
 * @ingroup group_adf
 *
 * Returns a const vector input stream over given io buffer.
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
constexpr auto begin_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return make_vector_input_buffer_stream<Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a const vector input stream over given io buffer.
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
constexpr auto cbegin_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_vector_input_buffer_stream(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a vector output stream over given io buffer.
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
constexpr auto begin_vector_output_buffer_stream(adf::io_buffer<T, Dir, Config> &port)
{
    return make_vector_output_buffer_stream<Elems, Resource>(port.data());
}

#if AIE_API_ML_VERSION >= 200
/**
 * @ingroup group_adf
 *
 * Returns a sparse vector input stream over given io buffer.
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
constexpr auto begin_sparse_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return sparse_vector_input_buffer_stream<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a sparse vector input stream over given io buffer.
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
constexpr auto cbegin_sparse_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_sparse_vector_input_buffer_stream<Elems, Resource>(port);
}
#endif

#if AIE_API_ML_VERSION >= 210
/**
 * @ingroup group_adf
 *
 * Returns a block vector input stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_block_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return block_vector_input_buffer_stream<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a block vector input stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_block_vector_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_block_vector_input_buffer_stream<Elems, Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a block vector output stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_block_vector_output_buffer_stream(adf::io_buffer<T, Dir, Config> &port)
{
    return block_vector_output_buffer_stream<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a restrict block vector input stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_block_vector_restrict_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return block_vector_restrict_input_buffer_stream<T, Elems, Resource>(port.data());
}

/**
 * @ingroup group_adf
 *
 * Returns a restrict block vector input stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto cbegin_block_vector_restrict_input_buffer_stream(const adf::io_buffer<T, Dir, Config> &port)
{
    return begin_block_vector_restrict_input_buffer_stream<Elems, Resource>(port);
}

/**
 * @ingroup group_adf
 *
 * Returns a restrict block vector output stream over given io buffer.
 *
 * @tparam Elems The size of the block vector
 * @tparam Resource Bank annotation
 * @param port The io buffer
 */
template<unsigned Elems,
         aie_dm_resource Resource = aie_dm_resource::none,
         typename T,
         typename Dir,
         typename Config>
__aie_inline
constexpr auto begin_block_vector_restrict_output_buffer_stream(adf::io_buffer<T, Dir, Config> &port)
{
    return block_vector_restrict_output_buffer_stream<T, Elems, Resource>(port.data());
}
#endif


namespace detail {

template <typename T>
struct addressing_mode { using type = ::adf::addressing::linear; };

template <typename T, typename Dir, typename Config>
struct addressing_mode<::adf::io_buffer<T, Dir, Config>>
{
    using type = typename Config::addressing_mode;
};

template <typename T>
using addressing_mode_t = typename addressing_mode<T>::type;

} // detail

/**
 * @ingroup group_adf
 *
 * Creates a tensor buffer stream for accessing multidimensional data from an ADF I/O buffer port.
 *
 * This function wraps an ADF I/O buffer port (input_buffer or output_buffer) with tensor buffer stream semantics,
 * enabling hardware-accelerated multidimensional memory access patterns. The stream accesses data using the
 * addressing pattern defined by the tensor descriptor.
 *
 * @tparam Resource Data Memory resource to be used for the access. Defaults to none.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the I/O buffer (must satisfy IOBuffer concept).
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param port         Reference to the I/O buffer port to wrap.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A tensor buffer stream object that accesses the I/O buffer according to the tensor descriptor.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 * @sa input_buffer
 * @sa output_buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         aie::tbs_mode Mode       = aie::tbs_mode::default_mode,
         IOBuffer T,
         typename TensorDescriptor>
__aie_inline
constexpr auto make_tensor_buffer_stream(T &port, const TensorDescriptor& tensor_desc)
{
    using type      = std::remove_pointer_t<decltype(port.data())>;
    using desc_type = typename TensorDescriptor::type;
    static_assert(std::is_same_v<detail::addressing_mode_t<T>, adf::addressing::linear>);
    static_assert(Mask<desc_type> || std::is_same_v<type, desc_type>,
                  "Input data type does not match tensor descriptor");
    static_assert(Mode == aie::tbs_mode::default_mode,                   "Unsupported tbs_mode selected");

    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, Resource, Mode>(port, tensor_desc);
}

/**
 * @ingroup group_adf
 *
 * Creates a constant tensor buffer stream for accessing multidimensional data from an ADF I/O buffer port.
 *
 * This overload accepts a const reference to an I/O buffer port and creates a read-only tensor buffer stream.
 * The returned stream can only be used for reading data from the buffer, preventing accidental modifications.
 *
 * @tparam Resource Data Memory resource to be used for the access. Defaults to none.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the I/O buffer (must satisfy IOBuffer concept).
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param port         Const reference to the I/O buffer port to wrap.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A constant tensor buffer stream object that can be used to read from the I/O buffer.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 * @sa input_buffer
 * @sa output_buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         aie::tbs_mode Mode       = aie::tbs_mode::default_mode,
         IOBuffer T,
         typename TensorDescriptor>
__aie_inline
constexpr auto make_tensor_buffer_stream(const T &port, const TensorDescriptor& tensor_desc)
{
    using type      = std::remove_pointer_t<decltype(port.data())>;
    using desc_type = typename TensorDescriptor::type;
    static_assert(std::is_same_v<detail::addressing_mode_t<T>, adf::addressing::linear>);
    static_assert(Mask<desc_type> || std::is_same_v<type, desc_type>,
                  "Input data type does not match tensor descriptor");
    static_assert(Mode == aie::tbs_mode::default_mode,                   "Unsupported tbs_mode selected");

    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, Resource, Mode>(port, tensor_desc);
}

/**
 * @ingroup group_adf
 *
 * Creates a restrict tensor buffer stream for accessing multidimensional data from an ADF I/O buffer port.
 *
 * This function creates a tensor buffer stream with restrict semantics, indicating to the compiler that the
 * I/O buffer is the only way to access the underlying data during the lifetime of the stream. This allows for
 * more aggressive optimizations by removing potential aliasing concerns.
 *
 * @tparam Resource Data Memory resource to be used for the access. Defaults to none.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the I/O buffer (must satisfy IOBuffer concept).
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param port         Reference to the I/O buffer port to wrap.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A restrict tensor buffer stream object with additional optimization opportunities due to restrict semantics.
 *
 * @sa make_tensor_descriptor
 * @sa make_tensor_buffer_stream
 * @sa input_buffer
 * @sa output_buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         aie::tbs_mode Mode       = aie::tbs_mode::default_mode,
         IOBuffer T,
         typename TensorDescriptor>
__aie_inline
constexpr auto make_restrict_tensor_buffer_stream(T &port, const TensorDescriptor& tensor_desc)
{
    using type      = std::remove_pointer_t<decltype(port.data())>;
    using desc_type = typename TensorDescriptor::type;
    static_assert(std::is_same_v<detail::addressing_mode_t<T>, adf::addressing::linear>);
    static_assert(std::is_same_v<type, typename TensorDescriptor::type>, "Input data type does not match tensor descriptor");
    static_assert(Mask<desc_type> || std::is_same_v<type, desc_type>,
                  "Input data type does not match tensor descriptor");
    static_assert(Mode == aie::tbs_mode::default_mode,                   "Unsupported tbs_mode selected");

    using ResourceType = decltype(Resource);
    constexpr bool Restrict = true;
    return detail::make_tensor_buffer_stream<ResourceType, Resource, Mode, Restrict>(port, tensor_desc);
}

/**
 * @ingroup group_adf
 *
 * Creates a tensor buffer stream for accessing unaligned multidimensional data from an ADF I/O buffer port.
 *
 * This function wraps an ADF I/O buffer port (input_buffer or output_buffer) with tensor buffer stream semantics,
 * enabling hardware-accelerated multidimensional memory access patterns. The stream accesses data using the
 * addressing pattern defined by the tensor descriptor.
 *
 * @tparam Resource Data Memory resource to be used for the access. Defaults to none.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the I/O buffer (must satisfy IOBuffer concept).
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param port         Reference to the I/O buffer port to wrap.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A tensor buffer stream object that accesses the I/O buffer according to the tensor descriptor.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 * @sa input_buffer
 * @sa output_buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         aie::tbs_mode Mode       = aie::tbs_mode::default_mode,
         IOBuffer T,
         typename TensorDescriptor>
__aie_inline
constexpr auto make_unaligned_tensor_buffer_stream(T &port, const TensorDescriptor& tensor_desc)
{
    using type      = std::remove_pointer_t<decltype(port.data())>;
    using desc_type = typename TensorDescriptor::type;
    static_assert(std::is_same_v<detail::addressing_mode_t<T>, adf::addressing::linear>);
    static_assert(Mask<desc_type> || std::is_same_v<type, desc_type>,
                  "Input data type does not match tensor descriptor");
    static_assert(Mode == aie::tbs_mode::default_mode,                   "Unsupported tbs_mode selected");

    constexpr bool Restrict  = false;
    constexpr bool Unaligned = true;

    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, Resource, Mode, Restrict, Unaligned>(port, tensor_desc);
}

/**
 * @ingroup group_adf
 *
 * Creates a constant tensor buffer stream for accessing unaligned multidimensional data from an ADF I/O buffer port.
 *
 * This overload accepts a const reference to an I/O buffer port and creates a read-only tensor buffer stream.
 * The returned stream can only be used for reading data from the buffer, preventing accidental modifications.
 *
 * @tparam Resource Data Memory resource to be used for the access. Defaults to none.
 * @tparam Mode     Tensor buffer stream mode. Defaults to default_mode.
 * @tparam T        Type of the I/O buffer (must satisfy IOBuffer concept).
 * @tparam TensorDescriptor Type of the tensor descriptor (automatically deduced).
 *
 * @param port         Const reference to the I/O buffer port to wrap.
 * @param tensor_desc  Tensor descriptor created with make_tensor_descriptor that describes the shape and
 *                     stride of the multidimensional tensor.
 *
 * @return A constant tensor buffer stream object that can be used to read from the I/O buffer.
 *
 * @sa make_tensor_descriptor
 * @sa make_restrict_tensor_buffer_stream
 * @sa input_buffer
 * @sa output_buffer
 */
template<aie_dm_resource Resource = aie_dm_resource::none,
         aie::tbs_mode Mode       = aie::tbs_mode::default_mode,
         IOBuffer T,
         typename TensorDescriptor>
__aie_inline
constexpr auto make_unaligned_tensor_buffer_stream(const T &port, const TensorDescriptor& tensor_desc)
{
    using type      = std::remove_pointer_t<decltype(port.data())>;
    using desc_type = typename TensorDescriptor::type;
    static_assert(std::is_same_v<detail::addressing_mode_t<T>, adf::addressing::linear>);
    static_assert(Mask<desc_type> || std::is_same_v<type, desc_type>,
                  "Input data type does not match tensor descriptor");
    static_assert(Mode == aie::tbs_mode::default_mode,                   "Unsupported tbs_mode selected");

    constexpr bool Restrict  = false;
    constexpr bool Unaligned = true;

    using ResourceType = decltype(Resource);
    return detail::make_tensor_buffer_stream<ResourceType, Resource, Mode, Restrict, Unaligned>(port, tensor_desc);
}

} // namespace aie

#endif
