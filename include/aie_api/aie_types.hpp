// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

/**
 * @file
 * @brief Types exposed to users.
 */

#pragma once

#ifndef __AIE_API_AIE_TYPES__HPP__
#define __AIE_API_AIE_TYPES__HPP__

#include "types.hpp"

#include "detail/accum.hpp"
#include "detail/lock.hpp"
#include "detail/mdspan.hpp"
#include "detail/sync_buffer.hpp"
#include "detail/tiled_mdspan.hpp"
#include "detail/vector.hpp"

/**
 * Base namespace for AIE operations and types.
 */
namespace aie {

enum class saturation_mode : unsigned {
    none      = 0, //!< Allows values to overflow.
    truncate  [[deprecated("Use saturate instead")]] = 1, //!< Deprecated: use saturate instead.
    saturate  = 1, //!< Retain maximum/minimum value on positive/negative overflow. Limits depend on type bit-width and if it has sign.
    symmetric = 3  //!< Retain maximum/minimum value on positive/negative overflow (signed types only). Max and min limits have exactly the same absolute value.
};

enum class rounding_mode : unsigned {
  floor           = rnd_floor,     /**< Always round towards negative infinity. */
  ceil            = rnd_ceil,      /**< Always round towards positive infinity. */
  positive_inf    = rnd_pos_inf,   /**< Round to nearest integer, with preference to positive infinity at half-way. */
  negative_inf    = rnd_neg_inf,   /**< Round to nearest integer, with preference to negative infinity at half-way. */
  symmetric_inf   = rnd_sym_inf,   /**< Round to nearest integer, with preference away from zero at half-way. */
  symmetric_zero  = rnd_sym_zero,  /**< Round to nearest integer, with preference towards zero at half-way. */
  conv_even       = rnd_conv_even, /**< Round to nearest integer, with preference to even numbers. */
  conv_odd        = rnd_conv_odd,  /**< Round to nearest integer, with preference to odd  numbers. */
#if AIE_API_PLATFORM_VERSION >= 200
  symmetric_floor = rnd_sym_floor, /**< Always round towards zero. */
  symmetric_ceil  = rnd_sym_ceil,  /**< Always round away from zero. */
#endif
};

/** Identifies a particular tile within the AIE array */
struct tile_id
{
    uint16_t row;
    uint16_t col;
};

/**
 * Retrieves and manipulates the current tile's state.
 */
class tile;

using AccumClass = detail::AccumClass;

/**
 * @ingroup group_basic_types
 *
 * @brief Type for vector accumulators.
 *
 * @tparam MinAccumTag Type tag that specifies the accumulator class and the required number of accumulation bits.
 * Internally, this is rounded up to the smallest native accumulator size that meets the accuracy requirements. It must
 * meet @ref aie::AccumElemBaseType.
 * @tparam Elems Number of elements in the accumulator.
 */
template <AccumElemBaseType MinAccumTag, unsigned Elems>
class accum;

template <unsigned MinAccumBits, unsigned Elems>
using  int_accum [[deprecated("Use aie::accum directly")]]
                 = accum<detail::accum_tag_t<AccumClass::Int,  detail::to_native_accum_bits<AccumClass::Int,  MinAccumBits>()>, Elems>;

template <unsigned MinAccumBits, unsigned Elems>
using cint_accum [[deprecated("Use aie::accum directly")]]
                 = accum<detail::accum_tag_t<AccumClass::CInt, detail::to_native_accum_bits<AccumClass::CInt, MinAccumBits>()>, Elems>;

template <unsigned MinAccumBits, unsigned Elems>
using   fp_accum [[deprecated("Use aie::accum directly")]]
                 = accum<detail::accum_tag_t<AccumClass::FP,   detail::to_native_accum_bits<AccumClass::FP,   MinAccumBits>()>, Elems>;

template <unsigned MinAccumBits, unsigned Elems>
using  cfp_accum [[deprecated("Use aie::accum directly")]]
                 = accum<detail::accum_tag_t<AccumClass::CFP,  detail::to_native_accum_bits<AccumClass::CFP,  MinAccumBits>()>, Elems>;

/**
 * @ingroup group_basic_types
 *
 * Type for vector element masks.
 *
 * This type is returned by logical operations and used as input in selection operations.
 *
 * @tparam Elems Number of elements represented in the mask.
 */
template <unsigned Elems>
class mask;

template <typename T>
static constexpr unsigned native_vector_length_v = detail::native_vector_length_v<T>;

/**
 * @ingroup group_basic_types
 *
 * @brief Type for vector registers.
 *
 * @tparam T Type of the elements contained in the vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the vector.
 */
template <ElemBaseType T, unsigned Elems = detail::native_vector_length_v<T>>
class vector;

/**
 * Type for references to vector registers.
 *
 * @tparam T Type of the elements contained in the vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the vector.
 */
template <typename T, unsigned Elems = native_vector_length_v<T>, aie_dm_resource Resource = aie_dm_resource::none>
class vector_ref;

/**
 * Type for references to vector registers.
 *
 * @tparam T Type of the elements contained in the vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the vector.
 */
template <typename T, unsigned Elems = native_vector_length_v<T>, aie_dm_resource Resource = aie_dm_resource::none>
class unaligned_vector_ref;

/**
 * @ingroup group_basic_types
 *
 * Type for sparse vector registers.
 *
 * @tparam T     Type of the elements contained in the sparse vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the sparse vector.
 */
template <ElemBaseType T, unsigned Elems = native_vector_length_v<T>>
        requires(arch::is(arch::Gen2))
class sparse_vector;

#if AIE_API_ML_VERSION >= 210

/**
 * @ingroup group_basic_types
 *
 * Type for block floating point vector registers.
 *
 * @tparam T     Type of the elements contained in the block vector. It must meet @ref aie::BlockType "BlockType".
 * @tparam Elems Number of elements in the block vector.
 */
template <BlockType T, unsigned Elems>
class block_vector;

template <BlockType T, unsigned Elems>
using bfp_vector [[deprecated("Use block_vector<T, Elems> instead")]] = block_vector<T, Elems>;

#endif
/**
 * @ingroup group_basic_types
 *
 * Reference to vector element. This type is returned by element access functions and represents a proxy to the actual
 * element, which allows for optimizations in the API such as accessing the element from its vector without extracting
 * it. This is common in element / vector operations. Otherwise, the element value is extracted when assigned to a
 * variable of type T.
 *
 * @tparam T Type of the elements contained in the referenced vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the referenced vector.
 *
 * @sa @ref aie::vector_elem_const_ref
 */
template <ElemBaseType T, unsigned N>
class vector_elem_ref;

/**
 * @ingroup group_basic_types
 *
 * Reference to immutable vector element.
 *
 * @tparam T Type of the elements contained in the referenced vector. It must meet @ref aie::ElemBaseType.
 * @tparam Elems Number of elements in the referenced vector.
 *
 * @sa @ref aie::vector_elem_ref
 */
template <ElemBaseType T, unsigned N>
class vector_elem_const_ref;

template <size_t... Extents>
using extents = detail::extents<Extents...>;

template <typename T, size_t... Extents>
using mdspan  = detail::basic_mdspan<T, extents<Extents...>, void, detail::accessor_basic<T>>;

template <typename T, size_t Extents1 = dynamic_extent>
using span_1d = mdspan<T, Extents1>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent>
using span_2d = mdspan<T, Extents1, Extents2>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent>
using span_3d = mdspan<T, Extents1, Extents2, Extents3>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent, size_t Extents4 = dynamic_extent>
using span_4d = mdspan<T, Extents1, Extents2, Extents3, Extents4>;

template <typename T, size_t Extents1 = dynamic_extent>
using span    = span_1d<T, Extents1>;

template <typename T, size_t... Extents>
using restrict_mdspan  = detail::basic_mdspan<T, extents<Extents...>, void, detail::accessor_restrict<T>>;

template <typename T, size_t Extents1 = dynamic_extent>
using restrict_span_1d = restrict_mdspan<T, Extents1>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent>
using restrict_span_2d = restrict_mdspan<T, Extents1, Extents2>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent>
using restrict_span_3d = restrict_mdspan<T, Extents1, Extents2, Extents3>;

template <typename T, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent, size_t Extents4 = dynamic_extent>
using restrict_span_4d = restrict_mdspan<T, Extents1, Extents2, Extents3, Extents4>;

template <typename T, size_t Extents1 = dynamic_extent>
using restrict_span    = restrict_span_1d<T, Extents1>;

template <typename T, typename TileExtents, size_t... Extents>
using tiled_mdspan  = detail::basic_tiled_mdspan<T, TileExtents, extents<Extents...>, void, detail::accessor_basic<T>>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent>
using tiled_span_1d = tiled_mdspan<T, TileExtents, Extents1>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent>
using tiled_span_2d = tiled_mdspan<T, TileExtents, Extents1, Extents2>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent>
using tiled_span_3d = tiled_mdspan<T, TileExtents, Extents1, Extents2, Extents3>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent, size_t Extents4 = dynamic_extent>
using tiled_span_4d = tiled_mdspan<T, TileExtents, Extents1, Extents2, Extents3, Extents4>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent>
using tiled_span    = tiled_span_1d<T, TileExtents, Extents1>;

template <typename T, typename TileExtents, size_t... Extents>
using restrict_tiled_mdspan  = detail::basic_tiled_mdspan<T, TileExtents, extents<Extents...>, void, detail::accessor_restrict<T>>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent>
using restrict_tiled_span_1d = tiled_mdspan<T, TileExtents, Extents1>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent>
using restrict_tiled_span_2d = tiled_mdspan<T, TileExtents, Extents1, Extents2>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent>
using restrict_tiled_span_3d = tiled_mdspan<T, TileExtents, Extents1, Extents2, Extents3>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent, size_t Extents2 = dynamic_extent, size_t Extents3 = dynamic_extent, size_t Extents4 = dynamic_extent>
using restrict_tiled_span_4d = tiled_mdspan<T, TileExtents, Extents1, Extents2, Extents3, Extents4>;

template <typename T, typename TileExtents, size_t Extents1 = dynamic_extent>
using restrict_tiled_span    = tiled_span_1d<T, TileExtents, Extents1>;

namespace sync {

using scoped_lock   = detail::scoped_lock;
template <unsigned NumReaders = 1, unsigned NumWriters = 1>
using producer_lock = detail::producer_lock<NumReaders, NumWriters>;
template <unsigned NumReaders = 1, unsigned NumWriters = 1>
using consumer_lock = detail::consumer_lock<NumReaders, NumWriters>;

using adopt_lock_t  = detail::adopt_lock_t;

template <unsigned NumReaders = 1, unsigned NumWriters = 1>
using producer_sem  = detail::producer_sem<NumReaders, NumWriters>;
template <unsigned NumReaders = 1, unsigned NumWriters = 1>
using consumer_sem  = detail::consumer_sem<NumReaders, NumWriters>;
using mutex         = detail::mutex;

using detail::locked;

template <typename Span, unsigned NumBuffers = 2, unsigned NumReaders = 1, unsigned NumWriters = 1>
using buffered_input  = detail::sync::input<Span, NumBuffers>;

template <typename Span, unsigned NumBuffers = 2, unsigned NumReaders = 1, unsigned NumWriters = 1>
using buffered_output = detail::sync::output<Span, NumBuffers>;

}

}

#endif
