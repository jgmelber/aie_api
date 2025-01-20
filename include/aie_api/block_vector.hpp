// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_BLOCK_VECTOR__HPP__
#define __AIE_API_BLOCK_VECTOR__HPP__

#include "detail/config.hpp"

#include <type_traits>

#include "concepts.hpp"
#include "vector.hpp"

#include "detail/aie2p/block_vector_native_types.hpp"
#include "detail/aie2p/block_vector.hpp"

namespace aie {

#if AIE_API_ML_VERSION >= 210
template <BlockType T, unsigned N> class block_vector;
#endif

namespace detail {

template <typename T>
struct is_valid_block_type
{
    static constexpr bool value = false;
};

template <typename T>
struct is_block_vector
{
    static constexpr bool value = false;
};

#if AIE_API_ML_VERSION >= 210
template <BlockType T, unsigned Elems>
struct is_block_vector<block_vector<T, Elems>>
{
    static constexpr bool value = true;
};
#endif

template <typename T>
concept NativeBlockVector = requires {
    typename native_vector_traits<T>::value_type;
    requires native_vector_traits<T>::size > 0;
};

} // namespace detail

#if AIE_API_ML_VERSION >= 210
template <BlockType T, unsigned Elems>
class __AIE_API_FUNDAMENTAL_TYPE__ block_vector : private detail::block_vector_base<T, Elems>
{
    using base_type = detail::block_vector_base<T, Elems>;

    template <BlockType T2, unsigned E2> friend class block_vector;
    template <typename T2, unsigned E2> friend class detail::block_vector_base;

    // Implicit conversion constructor
    __aie_inline
    block_vector(const base_type &v) : base_type(v) {}

public:
    /** \brief Equivalent intrinsic type for the same element type and size. */
    using native_type         = typename base_type::native_type;
    /** \brief Equivalent intrinsic type for the same element type and size when it is stored in memory. */
    using native_pointer_type = typename base_type::native_pointer_type;
    /** \brief Type of the elements in the vector. */
    using value_type          = typename base_type::value_type;
    /** \brief Type that holds the actual vector's data. May be different to its native type. */
    using storage_type        = typename base_type::storage_type;

    /** \brief Returns the number of elements in the vector. */
    static constexpr unsigned size()          { return base_type::size();       }

    /** \brief Returns how many data elements in the vector share a single exponent. */
    static constexpr unsigned block_size()    { return base_type::block_size(); }
    
    /** \brief Returns the number of (exponent, data) tuples represented by the vector. */
    static constexpr unsigned blocks()        { return base_type::blocks();     }
    
    /** \brief Returns the number of mantissa bits in each data element. */
    static constexpr unsigned mantissa_bits() { return base_type::mantissa_bits(); }
    
    /** \brief Returns the number of exponent bits in each exponent. */
    static constexpr unsigned exponent_bits() { return base_type::exponent_bits(); }
    
    /** \brief Returns the total capacity of the vector in bits. */
    static constexpr unsigned bits()          { return base_type::bits(); }
    
    /** \brief Returns the total capacity of the vector in bytes. */
    static constexpr unsigned bytes()         { static_assert(bits() % 8 == 0); return bits() / 8; }
    
    /** \brief Returns the total number of bits used to store the vector in memory. */
    static constexpr unsigned memory_bits()   { return base_type::memory_bits(); }
    
    /** \brief Returns the total number of bytes used to store the vector in memory. */
    static constexpr unsigned memory_bytes()  { static_assert(memory_bits() % 8 == 0); return memory_bits() / 8; }

    /**
     * \brief Default constructor. The value of the elements is undefined.
     */
    __aie_inline
    constexpr block_vector() : base_type() {}

    /**
     * \brief Construct a vector from internal native types.
     *
     * @param v Data used to construct the vector from.
     */
    __aie_inline
    constexpr block_vector(storage_type v) : base_type(v) {}

    /**
     * \brief Casts the vector to its native type
     */
    __aie_inline
    constexpr native_type to_native() const
    {
        return base_type::to_native();
    }

    __aie_inline
    constexpr operator native_type() const
    {
        return to_native();
    }

    /**
     * \brief Returns a copy of the current vector in a larger vector.
     *
     * The value of the new elements is undefined.
     *
     * @tparam ElemsOut Size of the output vector.
     * @param idx Location of the subvector within the output vector
     */
    template <unsigned ElemsOut>
    __aie_inline
    constexpr block_vector<T, ElemsOut> grow(unsigned idx = 0) const
    {
        return {base_type::template grow<ElemsOut>(idx)};
    }

    /**
     * \brief Returns a subvector with the contents of a region of this vector.
     *
     * @tparam ElemsOut Size of the returned subvector.
     *
     * @param idx Index of the subvector to be returned.
     */
    template <unsigned ElemsOut>
    __aie_inline
    constexpr block_vector<value_type, ElemsOut> extract(unsigned idx) const
        requires(ElemsOut <= Elems)
    {
        REQUIRES_MSG(idx < Elems / ElemsOut, "idx needs to be a valid subvector index");

        return base_type::template extract<ElemsOut>(idx);
    }

    /**
     * \brief Updates the contents of a region of the vector.
     *
     * The updated region will contain the values in the given subvector.
     *
     * @param idx Index of the subvector to be replaced.
     * @param v   Subvector to be written into the region.
     *
     * @returns a reference to the updated vector.
     */
    template <unsigned ElemsIn>
    __aie_inline
    constexpr block_vector &insert(unsigned idx, const block_vector<T, ElemsIn> &v)
        requires(ElemsIn <= Elems)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");

        base_type::template insert<ElemsIn>(idx, v);

        return *this;
    }

    /**
     * \brief Updates the contents of a region of the vector.
     *
     * The updated region will contain the values in the given native subvector.
     *
     * @param idx Index of the subvector to be replaced.
     * @param v   Native subvector to be written into the region.
     *
     * @returns a reference to the updated vector.
     */
    template <unsigned ElemsIn>
    __aie_inline
    constexpr block_vector &insert(unsigned idx, typename block_vector<T, ElemsIn>::native_type v)
        requires(ElemsIn <= Elems)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");

        const block_vector<T, ElemsIn> in = v;

        return insert(idx, in);
    }

    //TODO: Implement concat and insert

    //TODO: Add element wise get via mantissa/exponent manipulation returning bfloat16

    //TODO: Support direct access to get exponent & mantissa
};

/**
 * \brief Template deduction guidelines for aie::block_vector
 */
template <detail::NativeBlockVector T>
block_vector(const T&) -> block_vector<typename detail::native_vector_traits<T>::value_type, detail::native_vector_traits<T>::size>;

template <BlockType T, unsigned Elems>
using bfp_vector [[deprecated("Use block_vector<T, Elems> instead")]] = block_vector<T, Elems>;

#endif // AIE_API_ML_VERSION

} // namespace aie

#endif // __AIE_API_BLOCK_VECTOR__HPP__
