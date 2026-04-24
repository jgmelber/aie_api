// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_COMMON_MASK__HPP__
#define __AIE_API_COMMON_MASK__HPP__

#include "../vector.hpp"

#include <algorithm>

namespace aie {

struct assume_zero_padding_t {};

template <ElemBaseType T, unsigned Elems>
class vector;

} // namespace aie

namespace aie::detail {

template <unsigned Elems>
struct mask_backing_storage;

template <unsigned Elems>
struct mask_backing_storage { using type = unsigned; };

template <unsigned Elems, typename Storage = typename mask_backing_storage<Elems>::type>
class mask_base;

template <typename T>
struct is_mask : std::false_type {};

template <unsigned Elems>
struct is_mask<mask<Elems>> : std::true_type {};

template <unsigned Elems, typename S>
struct is_mask<mask_base<Elems, S>> : std::true_type {};

template <typename T>
concept Mask = is_mask<T>::value;

template <unsigned Elems>
class mask_base<Elems, unsigned>
{
protected:
    template <unsigned Elems2, typename Storage2>
    friend class mask_base;

    using underlying_type = unsigned;
    static constexpr bool is_array_backed = true;

    static constexpr unsigned bits_per_word = sizeof(underlying_type) * 8;
    static constexpr unsigned num_words = (Elems + (bits_per_word - 1)) / bits_per_word;

    static constexpr unsigned bits_remainder = Elems % bits_per_word == 0?          0 : bits_per_word - Elems % bits_per_word;
    static constexpr unsigned last_word_mask = Elems % bits_per_word == 0? 0xffffffff : (1u << (bits_per_word - bits_remainder)) - 1;

    std::array<underlying_type, num_words> data_;

    __aie_inline
    static constexpr unsigned get_word_index(unsigned i)
    {
        return i / bits_per_word;
    }

    __aie_inline
    static constexpr unsigned get_bit_index(unsigned i)
    {
        return i % bits_per_word;
    }

    __aie_inline
    constexpr void init_from_words32(const unsigned (&words)[num_words])
    {
        utils::unroll_times<num_words>([&](unsigned idx) __aie_inline {
            data_[idx] = words[idx];
        });

        if constexpr (last_word_mask > 0)
            data_.back() &= last_word_mask;
    }

    __aie_inline
    constexpr void init_from_words64(const uint64_t (&words)[std::max(1u, (num_words / 2))])
    {
        constexpr unsigned num_ops = std::max(1u, (num_words / 2));

        utils::unroll_times<num_ops>([&](unsigned idx) __aie_inline {
            const auto tmp = __builtin_bit_cast(std::array<unsigned, 2>, words[idx]);
            data_[idx * 2] = tmp[0];
            if constexpr (Elems > 32)
                data_[idx * 2 + 1] = tmp[1];
        });

        if constexpr (last_word_mask > 0)
            data_.back() &= last_word_mask;
    }

    template <typename T, unsigned Elems2, unsigned N>
    __aie_inline
    constexpr void init_from_vectors(const vector<T, Elems2> (&vs)[N])
    {
        constexpr unsigned num_vectors      = std::max(1u, Elems / vector<T, Elems2>::bits());
        constexpr unsigned words_per_vector = std::min(Elems, vector<T, Elems2>::bits()) / 32u;
        utils::unroll_times_2d<num_vectors, words_per_vector>([&](unsigned idv, unsigned idw) __aie_inline {
            data_[idv * words_per_vector + idw] = vs[idv].template cast_to<uint32>().get(idw);
        });

        if constexpr (last_word_mask > 0)
            data_.back() &= last_word_mask;
    }

    __aie_inline
    static constexpr unsigned count_word(unsigned n)
    {
        unsigned count;

#if __AIE_ARCH__ == 21 || __AIE_ARCH__ == 22
        //TODO: Use other implementation if evaluating at compile time
        //      Requires if consteval from C++23
        count = ::population_count(n);
#else
        n     = n - ((n >> 1) & 0x55555555);                    // reuse input as temporary
        n     = (n & 0x33333333) + ((n >> 2) & 0x33333333);     // temp
        count = ((n + (n >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
#endif

        return count;
    }

private:
    __aie_inline
    explicit mask_base(const std::array<underlying_type, num_words> &values)
        : data_(values)
    {
    }

public:
    /**
     * Returns the number of elements in the mask
     */
    static constexpr unsigned size()
    {
        return Elems;
    }

    /**
     * Creates a mask
     *
     * @param initial_set If true, the mask is initialized with 1s, otherise it is initialized with 0s.
     */
    __aie_inline
    constexpr explicit mask_base(bool initial_set = false) : data_()
    {
        const underlying_type w = initial_set? ~underlying_type(0) : underlying_type(0);
        #pragma unroll
        for (unsigned i = 0; i < num_words - 1; ++i)
            data_[i] = w;

        data_[num_words - 1] = initial_set? last_word_mask : underlying_type(0);
    }

    constexpr mask_base(const mask_base&) = default; // Copy-constructor

    mask_base& operator=(const mask_base&) = default; // copy-assignment operator

    /**
     * Construct from unsigned (32b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    template <typename... T>
    __aie_inline
    constexpr static mask_base from_uint32(unsigned w, T &&... words)
    {
        static_assert(sizeof...(words) + 1 == num_words);

        mask_base ret;

        ret.init_from_words32({w, words...});

        return ret;
    }

    __aie_inline
    constexpr static mask_base from_uint32(assume_zero_padding_t, unsigned w)
    {
        mask_base ret;
        ret.data_[0] = w;
        #pragma unroll
        for (unsigned i = 1; i < num_words; ++i)
            ret.data_[i] = 0;
        return ret;
    }

    /**
     * Construct from unsigned (64b) words. Each bit in the input words is used to initialize one of the elements in the
     * mask.
     *
     * @param w     First word used to initialize the mask.
     * @param words Rest of words used to initialize the mask.
     */
    template <typename... T>
    __aie_inline
    constexpr static mask_base from_uint64(uint64_t w, T &&... words)
    {
        static_assert(sizeof...(words) + 1 == std::max(1u, num_words / 2));

        mask_base ret;

        ret.init_from_words64({w, words...});

        return ret;
    }

    __aie_inline
    constexpr static mask_base from_uint64(assume_zero_padding_t, uint64_t w)
    {
        mask_base ret;
        // Unsigned integer narrowing conversion
        ret.data_[0] = static_cast<underlying_type>(w);
        if constexpr (size() > 32)
            ret.data_[1] = static_cast<underlying_type>(w>>32);
        #pragma unroll
        for (unsigned i = 2; i < num_words; ++i)
            ret.data_[i] = 0;
        return ret;
    }

    /**
     * Construct from concatenating multiple smaller masks.
     *
     * @param m     First mask used to initialize least significant elements in the new mask.
     * @param masks Other masks used to initialize the rest of the elements.
     */
    template <Mask M, unsigned N>
        requires(Elems == (M::size() * N))
    __aie_inline
    constexpr static mask_base from_masks(const M (&masks)[N])
    {
        constexpr unsigned in_elems = M::size();
        static_assert(Elems == (in_elems * N));

        std::array<underlying_type, num_words> word_masks;
        if constexpr (in_elems < 32) {
            auto in_it = std::begin(masks);
            #pragma unroll
            for (auto &out_chunk : word_masks) {
                out_chunk = (*in_it++).to_uint32(0);
                #pragma unroll
                for (unsigned i = in_elems; i < std::min(Elems, bits_per_word); i += in_elems) {
                    underlying_type shifted = (*in_it++).to_uint32(0) << i;
                    out_chunk |= shifted;
                }
            }
        }
        else {
            auto it = word_masks.begin();
            #pragma unroll
            for (auto &in : masks) {
                #pragma unroll
                for (unsigned i = 0; i < in_elems / 32; ++i) {
                    *it++ = in.to_uint32(i);
                }
            }
        }
        return mask_base(word_masks);
    }

    template <typename T, unsigned E>
    __aie_inline
    constexpr static mask_base from_vector(const vector<T, E> (&v)[1])
    {
        mask_base ret;
        ret.init_from_vectors(v);
        return ret;
    }

    template <typename T, unsigned E>
    __aie_inline
    constexpr static mask_base from_vector(const vector<T, E> &v)
    {
        return from_vector({v});
    }

    /**
     * Returns the value of the element in the given index.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr bool test(unsigned i) const
    {
        return (data_[get_word_index(i)] & (1 << get_bit_index(i))) != 0;
    }

    /**
     * Sets the value of the element in the given index to 1.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr void set(unsigned i)
    {
        data_[get_word_index(i)] |= (1 << get_bit_index(i));
    }

    /**
     * Sets the value of the element in the given index to 0.
     *
     * @param i Element index.
     */
    __aie_inline
    constexpr void clear(unsigned i)
    {
        data_[get_word_index(i)] &= ~(1 << get_bit_index(i));
    }

    /**
     * Compares whether two masks are equal.
     *
     * @param a Mask to compare against.
     */
    __aie_inline
    constexpr bool operator==(const mask_base &a) const
    {
        bool ret = true;

        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            ret = ret && (data_[i] == a.data_[i]);

        return ret;
    }

    /**
     * Returns the result of merging the current and the given masks using the AND operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask_base operator&(const mask_base &a) const
    {
        mask_base ret;

        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            ret.data_[i] = data_[i] & a.data_[i];

        return ret;
    }

    /**
     * Updates the mask with the result of merging the current with the given masks using the AND operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask_base &operator&=(const mask_base &a)
    {
        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            data_[i] &= a.data_[i];

        return *this;
    }

    /**
     * Returns the result of merging the current and the given masks using the OR operation.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask_base operator|(const mask_base &a) const
    {
        mask_base ret;

        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            ret.data_[i] = data_[i] | a.data_[i];

        return ret;
    }

    /**
     * Updates the mask with the result of merging the current and the given masks using the OR operation. Returns
     * a reference to the updated mask.
     *
     * @param a Mask to merge with.
     */
    __aie_inline
    constexpr mask_base &operator|=(const mask_base &a)
    {
        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            data_[i] |= a.data_[i];

        return *this;
    }

    /**
     * Returns a mask that contains the negation of the values of all the elements in the mask (0->1, 1->0).
     */
    __aie_inline
    constexpr mask_base operator~() const
    {
        mask_base ret;

        for (unsigned i = 0; i < num_words - 1; ++i) chess_unroll_loop()
            ret.data_[i] = ~data_[i];

        ret.data_[num_words - 1] = data_[num_words - 1] ^ last_word_mask;

        return ret;
    }

    /**
     * Returns the result of a binary left shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    constexpr mask_base &operator<<=(unsigned shift)
    {
        if constexpr (Elems <= bits_per_word) {
            data_[0] = shift >= Elems ? 0 : data_[0] << shift;
        } else {
            const int split_index = (shift >= Elems) ? num_words : shift / bits_per_word;
            const unsigned bits_from_low_word  = shift % bits_per_word;
            const unsigned bits_from_high_word = bits_per_word - bits_from_low_word;
                  unsigned high_word_index     = get_word_index(num_words * bits_per_word - 1 - shift);
            for (int i = num_words - 1; i >= split_index; --i, --high_word_index) {
                const unsigned high_word    = data_[high_word_index];
                const unsigned next_index   = high_word_index + (high_word_index == 0);
                const unsigned low_word_tmp = data_[next_index - 1];
                const unsigned low_word     = high_word_index ? low_word_tmp : 0;
                data_[i] = (bits_from_high_word == bits_per_word)
                                ? high_word
                                : (high_word << bits_from_low_word) |
                                        (low_word >> bits_from_high_word);
            }
            for (int i = split_index - 1; i >= 0; --i) {
                data_[i] = 0;
            }
        }
        // Unset any bits shifted outside the Elems-sized window on the underlying type
        if constexpr (bits_remainder > 0) {
            data_[num_words - 1] &= last_word_mask;
        }
        return *this;
    }

    /**
     * Returns the result of a binary left shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    constexpr mask_base operator<<(unsigned shift) const
    {
        mask_base ret{*this};
        ret <<= shift;
        return ret;
    }

    /**
     * Returns the result of a binary right shift of the mask. Returns a reference to the updated mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    constexpr mask_base &operator>>=(unsigned shift)
    {
        if constexpr (Elems <= bits_per_word) {
            if (shift >= Elems)
                data_[0] = 0;
            else
                data_[0] >>= shift;
        } else {
            const int split_index = (shift >= Elems) ? 0 : (Elems - shift - 1) / bits_per_word + 1;
            const unsigned bits_from_high_word = shift % bits_per_word;
            const unsigned bits_from_low_word  = bits_per_word - bits_from_high_word;
                  unsigned low_word_index      = get_word_index(shift);
            for (int i = 0; i < split_index; ++i, ++low_word_index) {
                const unsigned low_word      = data_[low_word_index];
                const unsigned next_index    = low_word_index - (low_word_index == (num_words - 1));
                const unsigned high_word_tmp = data_[next_index + 1];
                const unsigned high_word     = ((low_word_index + 1) < num_words) ? high_word_tmp : 0;
                data_[i] = (bits_from_low_word == bits_per_word)
                                ? low_word
                                : (low_word >> bits_from_high_word) |
                                        (high_word << bits_from_low_word);
            }
            for (unsigned i = split_index; i < num_words; ++i) {
                data_[i] = 0;
            }
        }
        return *this;
    }

    /**
     * Returns the result of a binary right shift of the mask.
     *
     * @param shift Number of positions to shift the mask.
     */
    __aie_inline
    constexpr mask_base operator>>(unsigned shift) const
    {
        mask_base ret{*this};
        ret >>= shift;
        return ret;
    }

    /**
     * Returns the count of elements whose value is 1.
     */
    __aie_inline
    constexpr unsigned count() const
    {
        unsigned count = 0;

        utils::unroll_times<num_words>([&](unsigned idx) __aie_inline {
            count += count_word(data_[idx]);
        });

        return count;
    }

    /**
     * Returns the contents of a section of the mask in a 32b unsigned integer
     *
     * @param i Index of the section within the mask (in chunks of 32 elements)
     */
    __aie_inline
    constexpr unsigned to_uint32(unsigned i) const
    {
        return data_[i];
    }

    /**
     * Returns the contents of the mask in a 32b unsigned integer
     */
    template <unsigned Elems2 = Elems>
    __aie_inline
    constexpr unsigned to_uint32() const requires(Elems2 <= 32)
    {
        return data_[0];
    }

    /**
     * Returns the contents of a section of the mask in a 64b unsigned integer
     *
     * @param i Index of the section within th mask (in chunks of 64 elements)
     */
    __aie_inline
    constexpr uint64_t to_uint64(unsigned i) const
    {
        if constexpr (sizeof(data_) < sizeof(uint64_t)) {
            REQUIRES(i == 0);
            return uint64_t(data_[i]);
        }
        else {
            const uint64_t *ptr = reinterpret_cast<const uint64_t *>(data_.data());
            return ptr[i];
        }
    }

    /**
     * Returns the contents of the mask in a vector
     */
    template <unsigned Elems2>
    __aie_inline
    vector<uint32_t, Elems2> to_vector() const
    {
        auto construct = [this]<unsigned... Idx>(std::integer_sequence<unsigned, Idx...>) __aie_inline {
            return vector<uint32_t, Elems2>{data_[Idx]...};
        };
        return construct(std::make_integer_sequence<unsigned, num_words>());
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
        static_assert(ElemsOut <= Elems);
        REQUIRES(idx * ElemsOut < Elems);

        mask_base<ElemsOut> ret;

        if constexpr (ElemsOut < bits_per_word) {
            constexpr unsigned ratio = bits_per_word / ElemsOut;
            ret.data_[0] = (data_[idx / ratio] >> (ElemsOut * (idx % ratio))) & ((1 << ElemsOut) - 1);
        }
        else {
            constexpr unsigned out_words = std::max(1u, ElemsOut / mask_base::bits_per_word);
            for (unsigned j = 0; j < out_words; ++j) {
                auto chunk = mask_base<32>::from_uint32(to_uint32(out_words * idx + j));
                ret.insert(j, chunk);
            }
        }

        return ret;
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
    template <unsigned ElemsIn>
    __aie_inline
    constexpr mask_base& insert(unsigned idx, const mask_base<ElemsIn>& m)
    {
        static_assert(ElemsIn <= Elems);
        REQUIRES(idx * ElemsIn < Elems);

        if constexpr (ElemsIn < bits_per_word) {
            constexpr unsigned ratio = bits_per_word / ElemsIn;
            constexpr unsigned zero_mask = (1 << ElemsIn) - 1;
            data_[idx / ratio] = (data_[idx / ratio] & (~(zero_mask << (idx * ElemsIn)))) | (m.to_uint32(0) << (idx * ElemsIn));
        }
        else {
            // Note that the word size is for the destination, not the input
            constexpr unsigned in_words = std::max(1u, ElemsIn / mask_base::bits_per_word);
            for (unsigned j = 0; j < in_words; ++j)
                data_[in_words * idx + j] = m.to_uint32(j);
        }

        return *this;
    }

    /**
     * Returns the count of consecutive elements, starting at the high part of the mask, whose value is 0.
     */
    __aie_inline
    constexpr unsigned clz() const
    {
        unsigned ret = ::clb(data_[num_words - 1]);
        bool counting = (ret == bits_per_word);

        for (int i = num_words - 2; i >= 0; --i) chess_unroll_loop() {
            unsigned local = ::clb(data_[i]);
            unsigned inc = counting ? local : 0;
            ret += inc;
            counting &= (local == bits_per_word);
        }

        if constexpr (bits_remainder > 0)
            ret -= bits_remainder;

        return ret;
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 0.
     */
    __aie_inline
    constexpr bool empty() const
    {
        bool ret = true;
        for (unsigned i = 0; i < num_words; ++i) chess_unroll_loop()
            ret = ret && (data_[i] == 0);

        return ret;
    }

    /**
     * Returns a bool that says whether the value of all the elements in the mask is 1.
     */
    __aie_inline
    constexpr bool full() const
    {
        bool ret = true;
        for (unsigned i = 0; i < num_words - 1; ++i) chess_unroll_loop()
            ret = ret && (data_[i] == 0xffffffff);
        ret = ret && data_.back() == last_word_mask;

        return ret;
    }
};

} // namespace aie::detail

#if __AIE_ARCH__ != 10
#include "aie2/mask.hpp"
#endif
#endif // __AIE_API_COMMON_MASK__HPP__
