// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE2_MASK__HPP__
#define __AIE_API_AIE2_MASK__HPP__

// Specializes mask<64> to avoid using 'uint64_t' as member value
// This prevents inefficient scalar register manipulation when operating 64 bit masks
// This is only necessary when building with Chess compiler
#if !__AIECC__

#include <algorithm>

#include "../../vector.hpp"

namespace aie {

template <unsigned E>
class mask;

} // namespace aie

namespace aie::detail {

template <unsigned Elems>
struct mask_backing_storage;

template <>
struct mask_backing_storage<64> {
    using type = v2uint32;
};

template <>
class mask_base<64, v2uint32>
{
protected:
    template <unsigned Elems2, typename Storage2>
    friend class mask_base;

    using underlying_type = v2uint32;
    static constexpr bool is_array_backed = false;

    static constexpr unsigned bits_per_word = 64;

private:
    constexpr mask_base(underlying_type v) : data_{v} {}

    alignas(8) underlying_type data_;

public:
    /**
     * Returns the number of elements in the mask
     */
    static constexpr unsigned size() { return 64; }

    /**
     * Creates a mask
     *
     * @param initial_set If true, the mask is initialized with 1s, otherise it is initialized with 0s.
     */
    __aie_inline
    explicit mask_base(bool initial_set = false) : mask_base(from_uint32(initial_set? ~0u : 0,
                                                                         initial_set? ~0u : 0))
    {
    }

    __aie_inline
    constexpr mask_base(const mask_base &) = default;

    /**
     * Construct from unsigned (32b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    __aie_inline
    static constexpr mask_base from_uint32(unsigned a, unsigned b)
    {
        underlying_type v;
        v = ::set_v2uint32(0, a);
        v = ::insert(v, 1, b);
        return mask_base{v};
    }

    __aie_inline
    constexpr static mask_base from_uint32(assume_zero_padding_t, unsigned w)
    {
        return from_uint32(w, 0);
    }

    __aie_inline
    static mask_base from_vector(vector<uint32, 16> v)
    {
        return mask_base{::extract_v2uint32(v, 0)};
    }

    /**
     * Construct from unsigned (64b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    __aie_inline
    static mask_base from_uint64(unsigned long long w)
    {
        underlying_type v;
        v = ::set_v2uint32(0, ::extract_uint32(w, 0));
        v = ::insert(v, 1, ::extract_uint32(w, 1));
        return mask_base{v};
    }

    __aie_inline
    static mask_base from_uint64(mask64 w)
    {
        // XXX: CRVO-12998
        // Processor internal ::sel implementation for 64 element vectors uses w64 storage
        // which models L scalar register pairs (e.g. l0 maps to r1:r0)
        // However, unsigned long long, the interface type to ::sel, has a problem in its implementation where it
        // can produce redundant copies, inefficient scalar loads and mess up register allocation.
        // Until that is fixed, provide a means to construct masks with mask64 type and make sure there are no
        // intermediate conversions to uint64_t/unsigned long long when using ::sel intrinsic.
        mask_base ret;
        ret.data_ = __builtin_bit_cast(v2uint32, w);
        return ret;
    }

    template <typename T>
    __aie_inline
    constexpr static mask_base from_uint64(assume_zero_padding_t, const T &w)
    {
        return from_uint64(w);
    }

    /**
     * Construct from concatenating multiple smaller masks.
     *
     * @param m     First mask used to initialize least significant elements in the new mask.
     * @param masks Other masks used to initialize the rest of the elements.
     */
    template <Mask M, unsigned N>
    __aie_inline
    static mask_base from_masks(const M (&masks)[N])
    {
        // Masks are default constructed to 0
        // Insignificant bits in a mask are set to zero or assumed to be so
        mask_base ret;

        constexpr unsigned elems_in = M::size();
        if constexpr (elems_in < 32) {
            mask_base<32> word_masks[size() / 32];

            #pragma unroll
            for (unsigned i = 0; i < size(); i += elems_in) {
                uint32_t shifted = masks[i / elems_in].to_uint32() << (i % 32);
                word_masks[i / 32] |= mask_base<32>::from_uint32(shifted);
            }

            ret = mask_base::from_masks(word_masks);
        }
        else {
            #pragma unroll
            for (unsigned i = 0; i < N; ++i)
                ret.insert(i, masks[i]);
        }
        return ret;
    }

    /**
     * Returns the value of the element in the given index.
     *
     * @param i Element index.
     */
    __aie_inline
    bool test(unsigned i) const
    {
        uint32_t w = to_uint32(i / 32);
        return (w & (uint32_t(1)<<(i%32))) != 0;
    }

    /**
     * Sets the value of the element in the given index to 1.
     *
     * @param i Element index.
     */
    __aie_inline
    void set(unsigned i)
    {
        uint32_t bit = uint32_t(1)<<(i%32);
        uint32_t w = to_uint32(i / 32) | bit;
        data_ = ::insert(data_, i / 32, w);
    }

    /**
     * Sets the value of the element in the given index to 0.
     *
     * @param i Element index.
     */
    __aie_inline
    void clear(unsigned i)
    {
        uint32_t bit = uint32_t(1)<<(i%32);
        uint32_t w = to_uint32(i / 32) & (~bit);
        data_ = ::insert(data_, i / 32, w);
    }

    /**
     * Compares whether two masks are equal.
     *
     * @param a Mask to compare against.
     */
    __aie_inline
    bool operator==(const mask_base &a) const
    {
        return to_uint32(0) == a.to_uint32(0)
            && to_uint32(1) == a.to_uint32(1);
    }

    /**
     * Returns the result of merging the current and the given masks using the AND operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    mask_base operator&(const mask_base &a) const
    {
        return mask_base::from_uint32(
                to_uint32(0) & a.to_uint32(0),
                to_uint32(1) & a.to_uint32(1));
    }

    /**
     * Updates the mask with the result of merging the current with the given masks using the AND operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    mask_base &operator&=(const mask_base &a)
    {
        return *this = operator&(a);
    }

    /**
     * Returns the result of merging the current and the given masks using the OR operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    mask_base operator|(const mask_base &a) const
    {
        return mask_base::from_uint32(
                to_uint32(0) | a.to_uint32(0),
                to_uint32(1) | a.to_uint32(1));
    }

    /**
     * Updates the mask with the result of merging the current and the given masks using the OR operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    mask_base &operator|=(const mask_base &a)
    {
        return *this = operator|(a);
    }

    /**
     * Returns a mask that contains the negation of the values of all the elements in the mask (0->1, 1->0).
     */
    __aie_inline
    mask_base operator~() const
    {
        return mask_base::from_uint32(
                ~to_uint32(0),
                ~to_uint32(1));
    }

    /**
     * Returns the result of a binary left shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    mask_base operator<<(unsigned shift) const
    {
        using mask_impl = mask_base<64, unsigned>;
        mask_impl v = mask_impl::from_uint32(to_uint32(0),
                                             to_uint32(1));
        v = v << shift;
        return mask_base::from_uint32(v.to_uint32(0),
                                      v.to_uint32(1));
    }

    /**
     * Returns the result of a binary left shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    mask_base &operator<<=(unsigned shift)
    {
        return *this = operator<<(shift);
    }

    /**
     * Returns the result of a binary right shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    mask_base operator>>(unsigned shift) const
    {
        using mask_impl = mask_base<64, unsigned>;
        mask_impl v = mask_impl::from_uint32(to_uint32(0),
                                             to_uint32(1));
        v = v >> shift;
        return mask_base::from_uint32(v.to_uint32(0),
                                      v.to_uint32(1));
    }

    /**
     * Returns the result of a binary right shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    mask_base &operator>>=(unsigned shift)
    {
        return *this = operator>>(shift);
    }

    /**
     * Returns the count of elements whose value is 1.
     */
    __aie_inline
    unsigned count() const
    {
        return __builtin_popcount(to_uint32(0))
            + __builtin_popcount(to_uint32(1));
    }

    /**
     * Returns the contents of a section of the mask in a 32b unsigned integer
     *
     * @param i Index of the section within the mask (in chunks of 32 elements)
     */
    __aie_inline
    unsigned to_uint32(unsigned i) const
    {
        return ::extract_elem(data_, i);
    }

    /**
     * Returns the contents of a section of the mask in a 64b unsigned integer
     *
     * @param i Index of the section within th mask (in chunks of 64 elements)
     */
    __aie_inline
    unsigned long long to_uint64(unsigned i = 0) const
    {
        REQUIRES(i == 0);
        unsigned int lo = ::extract_elem(data_, 0);
        unsigned int hi = ::extract_elem(data_, 1);

#if __AIE_ARCH__ == 20
        unsigned long long r = ::set_u64(0, lo);
#else
        unsigned long long r = ::set_uint64(0, lo);
#endif
        return ::insert(r, 1, hi);
    }

    /**
     * Returns the contents of the mask in a vector
     */
    template <unsigned Elems2>
    __aie_inline
    vector<uint32_t, Elems2> to_vector() const
    {
        return vector<uint32_t, Elems2>{to_uint32(0),
                                        to_uint32(1)};
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
    constexpr mask_base<ElemsOut> extract(unsigned idx) const
    {
        constexpr unsigned Elems = size();
        static_assert(ElemsOut <= Elems);
        REQUIRES(idx * ElemsOut < Elems);

        if constexpr (ElemsOut == Elems)
            return *this;
        if constexpr (ElemsOut == 32)
            return mask_base<ElemsOut>::from_uint32(to_uint32(idx));
        else if constexpr (ElemsOut < 32)
            return extract<32>(idx / (32 / ElemsOut)).template extract<ElemsOut>(idx % (32 / ElemsOut));
    }

    /**
     * \brief Updates the contents of a region of the mask.
     *
     * The updated region will contain the values in the given submask.
     *
     * @param idx Index of the submask to be replaced.
     * @param m Submask to be written into the region.
     * @returns a reference to the updated mask.
     */
    template <unsigned ElemsIn, typename Storage2>
    __aie_inline
    constexpr mask_base& insert(unsigned idx, const mask_base<ElemsIn, Storage2>& m)
    {
        constexpr unsigned Elems = size();
        static_assert(ElemsIn <= Elems);
        REQUIRES(idx * ElemsIn < Elems);

        if constexpr (ElemsIn < 32) {

            using slot_ratio = std::ratio<ElemsIn, 32>;
            unsigned slot  = idx / slot_ratio::den;
            unsigned shift = (idx % slot_ratio::den) * ElemsIn;

            const uint32_t mask = ((INT32_C(1u) << ElemsIn) - 1) << shift;
            if (chess_const(idx)) {
                uint32_t tmp = (to_uint32(slot) & mask)
                             | m.to_uint32() << shift;
                data_ = ::insert(data_, slot, tmp);
            }
            else {
                uint32_t value = m.to_uint32() << shift;
                uint32_t m_lo = slot == 0? value : this->to_uint32(0);
                uint32_t m_hi = slot != 0? value : this->to_uint32(1);
                data_ = ::set_v2uint32(0,  (to_uint32(0) & mask) | m_lo);
                data_ = ::insert(data_, 1, (to_uint32(1) & mask) | m_hi);
            }
        }
        else if constexpr (ElemsIn == 32) {
            data_ = ::insert(data_, idx, m.to_uint32());
        }
        else if constexpr (ElemsIn == Elems) {
            *this = m;
        }
        else {
            auto chunk = extract<32>(idx / 32);
            chunk.insert(idx % 32, m);
            insert(idx, chunk);
        }
        return *this;
    }

    /**
     * Returns the count of consecutive elements, starting at the high part of the mask, whose value is 0.
     */
    __aie_inline
    constexpr unsigned clz() const
    {
        using mask_impl = mask_base<64, unsigned>;
        return mask_impl::from_uint32(to_uint32(0), to_uint32(1)).clz();
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 0.
     */
    __aie_inline
    constexpr bool empty() const
    {
        return to_uint32(0) == 0
            && to_uint32(1) == 0;
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 1.
     */
    __aie_inline
    constexpr bool full() const
    {
        constexpr auto all_set = ~uint32_t(0);
        return to_uint32(0) == all_set
            && to_uint32(1) == all_set;
    }
};

} // namespace aie::detail

#endif // !__AIECC__

#endif // __AIE_API_AIE2_MASK__HPP__
