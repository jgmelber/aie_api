// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__
#define __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__

#include <algorithm>

#include "../ld_st.hpp"

namespace aie::detail {

/**
 * Architecture-specific implementation of the block_vector data type
 *
 * @tparam T Type of the elements contained in the vector.
 * @tparam Elems Number of elements in the vector.
 */
template <typename T, unsigned Elems>
class block_vector_base
{
    template <typename T2, unsigned E2> friend class block_vector_base;

    using vector_storage_type = block_vector_storage<T, Elems>;

    static constexpr unsigned native_elems      = native_block_vector_length<T>::value;
    static constexpr unsigned num_storage_elems = utils::num_elems_v<typename vector_storage_type::type>;
    static constexpr bool is_compound_storage   = num_storage_elems > 1;
public:
    using native_type         = typename vector_storage_type::type;
    using native_pointer_type = typename vector_storage_type::pointer_type;
    using value_type          = T;
    using storage_type        = typename vector_storage_type::type;
    using chunk_type          = native_block_vector_type_t<T, Elems>;

    /**
     * \brief Returns the number of elements in the vector.
     */
    static constexpr unsigned size() { return Elems; }

    /**
     * \brief Returns how many data elements in the vector share a single exponent.
     */
    static constexpr unsigned block_size()
    {
#if __AIE_ARCH__ == 21
        if      constexpr(std::is_same_v<T, bfp16ebs8>)  return 8;
        else if constexpr(std::is_same_v<T, bfp16ebs16>) return 16;
#elif __AIE_ARCH__ == 22
        return 16;
#else
#if __AIE_API_MX6_SUPPORT__
        if      constexpr(std::is_same_v<T, mx6>)  return 16;
#endif
        return 0;
#endif
    }

    /**
     * \brief Returns the number of (exponent, data) tuples represented by the vector.
     */
    static constexpr unsigned blocks()
    {
        return Elems / block_size();
    }

    /**
     * \brief Returns the number of mantissa bits in each data element.
     */
    static constexpr unsigned mantissa_bits()
    {
#if __AIE_ARCH__ == 21
        return 8;
#elif __AIE_ARCH__ == 22
        if      constexpr(std::is_same_v<T, mx4>) return 3;
        else if constexpr(std::is_same_v<T, mx6>) return 5;
        else if constexpr(std::is_same_v<T, mx9>) return 8;
#else
        return 0;
#endif
    }

    /**
     * \brief Returns the number of exponent bits in each exponent.
     */
    static constexpr unsigned exponent_bits()
    {
        return 8;
    }

    /**
     * \brief Returns the total capacity of the vector in bytes.
     */
    static constexpr unsigned bytes()
    {
        static_assert(bits() % 8 == 0);

        return bits() / 8;
    }

    /**
     * \brief Returns the total capacity of the vector in bits.
     */
    static constexpr unsigned bits()
    {
        unsigned res = (Elems * mantissa_bits());

        res += blocks() * exponent_bits();

        // Account for prime bits - each two mantissas share a prime bit
#if __AIE_ARCH__ == 22
        res += size() / 2u;
#endif

        return res;
    }

    /**
     * \brief Returns the total number of bytes used to store the vector in memory.
     */
    static constexpr unsigned memory_bytes()
    {
        static_assert(memory_bits() % 8 == 0);

        return memory_bits() / 8;
    }

    /**
     * \brief Returns the total number of bits used to store the vector in memory.
     */
    static constexpr unsigned memory_bits()
    {
        return bits();
    }

    __aie_inline
    block_vector_base() : data(vector_storage_type::undef()) {}

    __aie_inline
    block_vector_base(storage_type data) : data(data) {}

    template <unsigned ElemsOut>
    __aie_inline
    block_vector_base<T, ElemsOut> grow(unsigned idx = 0) const
    {
        static_assert(ElemsOut >= Elems);

        constexpr unsigned growth_ratio = ElemsOut / Elems;

        block_vector_base<T, ElemsOut> ret;

        constexpr unsigned in_storage_elems  = num_storage_elems;
        constexpr unsigned out_storage_elems = ret.num_storage_elems;

        if constexpr (growth_ratio == 1) {
            ret = data;
        }
        else if constexpr (in_storage_elems == 1 && out_storage_elems == 1) {
            ret = block_vector_set<value_type, ElemsOut>::run(data, idx);
        }
        else if constexpr (in_storage_elems == 1 && out_storage_elems > 1) {
            constexpr unsigned to_native_ratio = native_elems / Elems;

            // Other elements are default initialized to undef() already
            if constexpr (to_native_ratio == 1)
                ret.data[idx / to_native_ratio] = data;
            else
                ret.data[idx / to_native_ratio] = block_vector_set<value_type, native_elems>::run(data, idx % to_native_ratio);
        }
        else {
            // Other elements are default initialized to undef() already
            utils::unroll_times<in_storage_elems>([&](unsigned elem) __aie_inline {
                ret.data[idx * in_storage_elems + elem] = data[elem];
            });
        }

        return ret;
    }

    template <unsigned ElemsOut>
    __aie_inline
    block_vector_base<T, ElemsOut> extract(unsigned idx) const
        requires(ElemsOut <= Elems)
    {
        return extract_helper<ElemsOut>(idx);
    }

    template <unsigned ElemsIn>
    __aie_inline
    block_vector_base &insert(unsigned idx, const block_vector_base<T, ElemsIn> &v)
        requires(ElemsIn <= Elems)
    {
        insert_helper<ElemsIn>(idx, v);

        return *this;
    }

    __aie_inline
    auto to_native() const requires (!is_compound_storage)
    {
        return data;
    }

    __aie_inline
    operator native_type() const requires (!is_compound_storage)
    {
        return to_native();
    }

private:
    template <unsigned ElemsOut>
    __aie_inline
    block_vector_base<value_type, ElemsOut> extract_helper(unsigned idx) const
    {
        static_assert(ElemsOut <= Elems && ElemsOut >= 64);

        using ret_type = block_vector_base<value_type, ElemsOut>;
        constexpr unsigned num_out_storage_elems = ret_type::num_storage_elems;

        if constexpr (ElemsOut == Elems) {
            return *this;
        }
        else if constexpr (num_storage_elems > 1) {
            ret_type res;

            utils::unroll_times<num_out_storage_elems>([&](unsigned idy) __aie_inline {
                res.data[idy] = data[idx * num_out_storage_elems + idy];
            });

            return res;
        }
        else {
            if constexpr (num_storage_elems > 1) {
                constexpr unsigned ratio = native_vector_traits<chunk_type>::size / ElemsOut;
                return detail::vector_extract<ElemsOut>(data[idx / ratio], idx % ratio);
            }
            else {
                return detail::vector_extract<ElemsOut>(data, idx);
            }
        }
    }

    template <unsigned ElemsIn>
    __aie_inline
    void insert_helper(unsigned idx, const block_vector_base<T, ElemsIn> &v)
    {
        static_assert(ElemsIn <= Elems);

        constexpr unsigned num_in_storage_elems = block_vector_base<T, ElemsIn>::num_storage_elems;

        if constexpr (ElemsIn == Elems) {
            data = v;
        }
        else if constexpr (is_compound_storage) {
            if constexpr (num_in_storage_elems > 1) {
                utils::unroll_times<num_in_storage_elems>([&](unsigned idy) __aie_inline {
                    data[idx * num_in_storage_elems + idy] = v.data[idy];
                });
            }
            else if constexpr (ElemsIn == native_vector_traits<chunk_type>::size) {
                data[idx] = v;
            }
            else {
                constexpr unsigned ratio = native_vector_traits<chunk_type>::size / ElemsIn;
                data[idx / ratio] = ::insert(data[idx / ratio], idx % ratio, v);
            }
        }
        else {
#if __AIE_API_USE_MX_UPDATE__
            if constexpr ((std::is_same_v<T, mx4> || std::is_same_v<T, mx6> || std::is_same_v<T, mx9>) && (Elems / native_elems == 2)) {
                data = ::update(data, idx, v);
            } else
#endif
            {
                data = ::insert(data, idx, v);
            }
        }
    }

    storage_type data;
};

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__
