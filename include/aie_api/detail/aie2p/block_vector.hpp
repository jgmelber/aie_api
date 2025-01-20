// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__
#define __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__

#include <algorithm>

#include "../ld_st.hpp"

namespace aie::detail {

template <typename T>
struct native_block_vector_length
{
    static constexpr unsigned value = 64;
};

template <typename T, unsigned Elems> struct block_vector_set;
template <> struct block_vector_set<bfp16ebs8,  128> { static v128bfp16ebs8   run(const v64bfp16ebs8  &v, unsigned idx) { return ::set_v128bfp16ebs8(idx, v);  } };
template <> struct block_vector_set<bfp16ebs16, 128> { static v128bfp16ebs16  run(const v64bfp16ebs16 &v, unsigned idx) { return ::set_v128bfp16ebs16(idx, v); } };

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
public:
    using native_type         = typename vector_storage_type::type;
    using native_pointer_type = typename vector_storage_type::pointer_type;
    using value_type          = T;
    using storage_type        = typename vector_storage_type::type;

    /**
     * \brief Returns the number of elements in the vector.
     */
    static constexpr unsigned size() { return Elems; }

    /**
     * \brief Returns how many data elements in the vector share a single exponent.
     */
    static constexpr unsigned block_size()
    {
        if      constexpr(std::is_same_v<T, bfp16ebs8>)
            return 8;
        else if constexpr(std::is_same_v<T, bfp16ebs16>)
            return 16;
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
        return 8;
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

        return res;
    }

    /**
     * \brief Returns the total number of bytes used to store the vector in memory.
     */
    static constexpr unsigned memory_bytes()
    {
        return bytes();
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
    auto to_native() const
    {
        return data;
    }

    __aie_inline
    operator native_type() const
    {
        return to_native();
    }

private:
    template <unsigned N>
    __aie_inline
    block_vector_base<value_type, N> extract_helper(unsigned idx) const
    {
        static_assert(N <= Elems);
        static_assert(N == 64 || N == 128);

        if constexpr (N == Elems) {
            return *this;
        }
        else {
#if __AIE_API_HAS_EXTRACT_V64BFP16__
            return detail::vector_extract<N>(data, idx);
#else
            return block_vector_base<value_type, N>();
#endif
        }
    }

    template <unsigned N>
    __aie_inline
    void insert_helper(unsigned idx, const block_vector_base<T, N> &v)
    {
        static_assert(N <= Elems);
        static_assert(N == 64 || N == 128);

        if constexpr (N == Elems)
            data = v;
        else
            data = ::insert(data, idx, v);
    }

    static constexpr unsigned native_elems      = native_block_vector_length<T>::value;
    static constexpr unsigned num_storage_elems = utils::num_elems_v<storage_type>;
    static constexpr bool is_compound_storage   = num_storage_elems > 1;

    storage_type data;
};

} // namespace aie::detail

#endif // __AIE_API_DETAIL_AIE2P_BLOCK_VECTOR__HPP__
