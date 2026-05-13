// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_MASK__HPP__
#define __AIE_API_MASK__HPP__

#include "detail/utils.hpp"

#include "detail/mask.hpp"

namespace aie {

/**
 * @ingroup group_basic_types
 *
 * Disambiguation tag type allowing mask construction when input values are
 * guaranteed to have all its padding bits set to zero.
 */
struct assume_zero_padding_t;

template <ElemBaseType T, unsigned Elems>
class vector;

template <unsigned Elems>
class mask : private detail::mask_base<Elems>
{
    using base_type = detail::mask_base<Elems>;

    template <unsigned Elems2> friend class mask;
    template <unsigned Elems2, typename Storage2> friend class detail::mask_base;

    // Implicit conversion constructor
    __aie_inline
    constexpr mask(const base_type &m) : base_type(m) {}

public:
    /** \brief Returns the number of elements in the vector. */
    static constexpr unsigned size() { return base_type::size(); }

    /**
     * Creates a mask
     *
     * @param initial_set If true, the mask is initialized with 1s, otherise it is initialized with 0s.
     */
    __aie_inline
    constexpr explicit mask(bool initial_set = false) : base_type(initial_set) {}

    /** Copy constructor */
    constexpr mask(const mask &) = default;

    /** Copy-assignment operator */
    constexpr mask& operator=(const mask &) = default;

    /**
     * Construct from unsigned (32b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    template <typename... T>
    [[deprecated("Use from_uint32 / from_uint64 / from_masks static methods instead")]]
    constexpr explicit mask(const T &... words) :
        mask(from_uint32(words...))
    {
    }

    /**
     * Construct from unsigned (32b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    template <SameAs<unsigned>... T>
        requires(sizeof...(T) == std::max(1u, Elems / 32))
    __aie_inline
    constexpr static mask<Elems> from_uint32(const T &... words)
    {
        return mask::mask_base::from_uint32(words...);
    }

    template <ConvertibleTo<uint32_t>... T>
        requires(!(std::is_same_v<T, unsigned> && ...))
    [[deprecated("Using convertible arguments instead of uint32_t is deprecated")]]
    __aie_inline
    constexpr static mask<Elems> from_uint32(const T &... words)
    {
        return from_uint32(static_cast<uint32_t>(words)...);
    }

    /**
     * Construct from an unsigned (32b) word. Insignificant or undefined elements are assumed to be zero.
     *
     * Masks larger than 32 elements have the most significant elements set to zero.
     * Masks smaller than 32 elements assume insignificant bits in `word` are already set to zero.
     */
    template <typename T>
        requires(std::is_same_v<unsigned, T>)
    __aie_inline
    constexpr static mask<Elems> from_uint32(assume_zero_padding_t tag, const T &word)
    {
        return mask::mask_base::from_uint32(tag, word);
    }

    /**
     * Construct from unsigned (64b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param words Words used to initialize the mask.
     */
    template <AnyOf<unsigned long long, uint64_t> ...T>
        requires(sizeof...(T) == std::max(1u, Elems / 64))
    __aie_inline
    constexpr static mask<Elems> from_uint64(const T &... words)
    {
        return mask::mask_base::from_uint64(words...);
    }

    /**
     * Construct from an unsigned (64b) word. Insignificant or undefined elements are assumed to be zero.
     *
     * Masks larger than 64 elements have the most significant elements set to zero.
     * Masks smaller than 64 elements assume insignificant bits in `word` are already set to zero.
     */
    template <AnyOf<unsigned long long, uint64_t> T>
    __aie_inline
    constexpr static mask<Elems> from_uint64(assume_zero_padding_t tag, const T &word)
    {
        return mask::mask_base::from_uint64(tag, word);
    }

#if __AIE_ARCH__ != 10
    /**
     * Conversion from mask64 intrinsic type. Only supported by mask<64>.
     */
    template <SameAs<mask64>... T>
        requires(sizeof...(T) == std::max(1u, Elems / 64))
    __aie_inline
    constexpr static mask<Elems> from_uint64(const T &... words)
    {
        return mask::mask_base::from_uint64(words...);
    }

    template <ConvertibleTo<mask64>... T>
        requires(!(detail::utils::is_one_of_v<T, mask64, unsigned long long, uint64_t> && ...))
    [[deprecated("Using convertible arguments instead of uint64_t/mask64 is deprecated")]]
    __aie_inline
    constexpr static mask<Elems> from_uint64(const T &... words)
    {
        return from_uint64(static_cast<mask64>(words)...);
    }
#else
    template <ConvertibleTo<uint64_t>... T>
        requires(!(detail::utils::is_one_of_v<T, unsigned long long, uint64_t> && ...))
    [[deprecated("Using convertible arguments instead of uint64_t is deprecated")]]
    __aie_inline
    constexpr static mask<Elems> from_uint64(const T &... words)
    {
        return from_uint64(static_cast<uint64_t>(words)...);
    }
#endif

    /**
     * Construct from aie::vectors. Each bit in the input vectors is used to initialize one of the elements in the
     * mask.
     *
     * @param v     First vector used to initialize the mask.
     * @param vs    Rest of vectors used to initialize the mask.
     */
    template <typename T, unsigned N, typename... Ts>
        requires((std::is_same_v<vector<T, N>, Ts> && ...))
    __aie_inline
    constexpr static mask<Elems> from_vector(const vector<T, N> &v, const Ts &... vs)
    {
        return mask::mask_base::from_vector(v, vs...);
    }

    /**
     * Construct from concatenating multiple smaller masks.
     *
     * @param m     First mask used to initialize least significant elements in the new mask.
     * @param masks Other masks used to initialize the rest of the elements.
     */
    template <unsigned E, unsigned... Es>
        requires(Elems == (Es + ... + E) && ((E == Es) && ...))
    __aie_inline
    constexpr static mask<Elems> from_masks(const mask<E> &m, const mask<Es>&... masks)
    {
        return from_masks({m, masks...});
    }

    /**
     * Construct from concatenating multiple smaller masks.
     *
     * @param masks Masks used to initialize the rest of the elements.
     */
    template <unsigned E, unsigned N>
        requires(Elems == (E * N))
    __aie_inline
    constexpr static mask<Elems> from_masks(const mask<E> (&masks)[N])
    {
        return mask::mask_base::from_masks(masks);
    }

    /**
     * Returns the value of the element in the given index.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr bool test(unsigned i) const
    {
        return base_type::test(i);
    }

    /**
     * Sets the value of the element in the given index to 1.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr void set(unsigned i)
    {
        base_type::set(i);
    }

    /**
     * Sets the value of the element in the given index to 0.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr void clear(unsigned i)
    {
        base_type::clear(i);
    }

    /**
     * Compares whether two masks are equal.
     *
     * @param a Mask to compare against.
     */
    __aie_inline
    constexpr bool operator==(const mask &a) const
    {
        return base_type::operator==(a);
    }

    /**
     * Returns the result of merging the current and the given masks using the AND operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask operator&(const mask &a) const
    {
        return base_type::operator&(a);
    }

    /**
     * Updates the mask with the result of merging the current with the given masks using the AND operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask &operator&=(const mask &a)
    {
        base_type::operator&=(a);
        return *this;
    }

    /**
     * Returns the result of merging the current and the given masks using the OR operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask operator|(const mask &a) const
    {
        return base_type::operator|(a);
    }

    /**
     * Updates the mask with the result of merging the current and the given masks using the OR operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask &operator|=(const mask &a)
    {
        base_type::operator|=(a);
        return *this;
    }

    /**
     * Returns a mask that contains the negation of the values of all the elements in the mask (0->1, 1->0).
     */
    __aie_inline
    constexpr mask operator~() const
    {
        return base_type::operator~();
    }

    /**
     * Returns the result of a binary left shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    constexpr mask &operator<<=(unsigned shift)
    {
        base_type::operator<<=(shift);
        return *this;
    }

    /**
     * Returns the result of a binary left shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    constexpr mask operator<<(unsigned shift) const
    {
        mask ret{*this};
        ret <<= shift;
        return ret;
    }

    /**
     * Returns the result of a binary right shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    constexpr mask &operator>>=(unsigned shift)
    {
        base_type::operator>>=(shift);
        return *this;
    }

    /**
     * Returns the result of a binary right shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    constexpr mask operator>>(unsigned shift) const
    {
        mask ret{*this};
        ret >>= shift;
        return ret;
    }

    /**
     * Returns the count of elements whose value is 1.
     */
    __aie_inline
    constexpr unsigned count() const
    {
        return base_type::count();
    }

    /**
     * \deprecated Use alternative functions to access the elements of a mask.
     * \sa to_uint32, to_uint64, extract
     */
    [[deprecated("Use to_uint32 / to_uint64 / extract methods instead")]]
    __aie_inline
    constexpr const auto &data() const requires(base_type::is_array_backed)
    {
        return base_type::data();
    }

    /**
     * Returns the contents of a section of the mask in a 32b unsigned integer
     *
     * @param i Index of the section within the mask (in chunks of 32 elements)
     */
    __aie_inline
    constexpr unsigned to_uint32(unsigned i) const
    {
        return base_type::to_uint32(i);
    }

    /**
     * Returns the contents of the mask in a 32b unsigned integer
     */
    template <unsigned Elems2 = Elems>
    __aie_inline
    constexpr unsigned to_uint32() const requires(Elems2 <= 32)
    {
        return base_type::to_uint32();
    }

    /**
     * Returns the contents of a section of the mask in a 64b unsigned integer
     *
     * @param i Index of the section within th mask (in chunks of 64 elements)
     */
    __aie_inline
    constexpr uint64_t to_uint64(unsigned i) const
    {
        return base_type::to_uint64(i);
    }

    /**
     * Returns the contents of the mask in a 64b unsigned integer
     */
    __aie_inline
    constexpr uint64_t to_uint64() const
        requires(Elems == 64)
    {
        return to_uint64(0);
    }

    /**
     * Returns the contents of the mask in a vector
     *
     * @tparam Elems2 number of elements in the result vector
     */
    template <unsigned Elems2>
    __aie_inline
    constexpr vector<uint32_t, Elems2> to_vector() const
    {
        return base_type::template to_vector<Elems2>();
    }

    /**
     * Returns the contents of a section of the mask into a smaller mask
     *
     * @tparam ElemsOut Size of the returned mask
     *
     * @param idx Index of the section within the mask (in chunks of ElemsOut elements)
     */
    template <unsigned ElemsOut>
    [[deprecated("Use extract<ElemsOut>(idx) instead")]]
    __aie_inline
    constexpr mask<ElemsOut> get_submask(unsigned idx) const
    {
        return this->template extract<ElemsOut>(idx);
    }

    /**
     * Returns the contents of a section of the mask into a smaller mask
     *
     * @tparam ElemsOut Size of the returned mask
     *
     * @param idx Index of the section within the mask (in chunks of ElemsOut elements)
     */
    template <unsigned ElemsOut>
    __aie_inline
    constexpr mask<ElemsOut> extract(unsigned idx) const
    {
        return base_type::template extract<ElemsOut>(idx);
    }

    /**
     * \brief Updates the contents of a region of the mask.
     *
     * The updated region will contain the values in the given submask.
     *
     * @param idx Index of the submask to be replaced. The mask is split into Elems / ElemsIn equally sized
     *            submasks, so valid indices range from 0 to Elems / ElemsIn - 1. This is a partition index,
     *            not an element offset. An invalid index may be reported at compile time if the value is known.
     * @param m Submask to be written into the region.
     * @returns a reference to the updated mask.
     */
    template <unsigned ElemsIn>
    __aie_inline
    constexpr mask& insert(unsigned idx, const mask<ElemsIn>& m)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index. Subvector indices split the mask into equal groups of the insert size, numbered from 0");

        base_type::template insert<ElemsIn>(idx, m);

        return *this;
    }

    /**
     * Returns a larger mask with matching least significant `Elems` bits and remaining bits set to zero.
     */
    template <unsigned ElemsOut>
        requires(ElemsOut >= Elems)
    __aie_inline
    constexpr mask<ElemsOut> grow() const {
        if constexpr (Elems < base_type::bits_per_word && base_type::bits_per_word < ElemsOut) {
            return grow<base_type::bits_per_word>().template grow<ElemsOut>();
        }
        else {
            mask<ElemsOut> result;
            result.insert(0, *this);
            return result;
        }
    }

    /**
     * Returns the count of consecutive elements, starting at the high part of the mask, whose value is 0.
     */
    __aie_inline
    constexpr unsigned clz() const
    {
        return base_type::clz();
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 0.
     */
    __aie_inline
    constexpr bool empty() const
    {
        return base_type::empty();
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 1.
     */
    __aie_inline
    constexpr bool full() const
    {
        return base_type::full();
    }
};

} // namespace aie

#endif // __AIE_API_MASK__HPP__
