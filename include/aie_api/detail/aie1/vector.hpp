// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_VECTOR__HPP__
#define __AIE_API_DETAIL_AIE1_VECTOR__HPP__

#include <array>

#include "../ld_st.hpp"
#include "../utils.hpp"

namespace aie::detail {

template <typename T>
struct native_vector_length
{
    static constexpr unsigned value = 512 / type_bits_v<T>;
};

template <typename DstT, unsigned DstElems, typename T>
__aie_inline
static vector_storage_t<DstT, DstElems> vector_cast_helper(T &&from)
{
    if constexpr (std::is_same_v<DstT, int8>   && DstElems == 16) return ::as_v16int8(from);
    if constexpr (std::is_same_v<DstT, int8>   && DstElems == 32) return ::as_v32int8(from);
    if constexpr (std::is_same_v<DstT, int8>   && DstElems == 64) return ::as_v64int8(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, int8>   && DstElems ==128) return ::as_v128int8(from);
#else
    if constexpr (std::is_same_v<DstT, int8>   && DstElems ==128) return { ::as_v64int8(from[0]), ::as_v64int8(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, uint8>  && DstElems == 16) return ::as_v16uint8(from);
    if constexpr (std::is_same_v<DstT, uint8>  && DstElems == 32) return ::as_v32uint8(from);
    if constexpr (std::is_same_v<DstT, uint8>  && DstElems == 64) return ::as_v64uint8(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, uint8>  && DstElems ==128) return ::as_v128uint8(from);
#else
    if constexpr (std::is_same_v<DstT, uint8>  && DstElems ==128) return { ::as_v64uint8(from[0]), ::as_v64uint8(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, int16>  && DstElems ==  8) return ::as_v8int16(from);
    if constexpr (std::is_same_v<DstT, int16>  && DstElems == 16) return ::as_v16int16(from);
    if constexpr (std::is_same_v<DstT, int16>  && DstElems == 32) return ::as_v32int16(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, int16>  && DstElems == 64) return ::as_v64int16(from);
#else
    if constexpr (std::is_same_v<DstT, int16>  && DstElems == 64) return { ::as_v32int16(from[0]), ::as_v32int16(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, int32>  && DstElems ==  4) return ::as_v4int32(from);
    if constexpr (std::is_same_v<DstT, int32>  && DstElems ==  8) return ::as_v8int32(from);
    if constexpr (std::is_same_v<DstT, int32>  && DstElems == 16) return ::as_v16int32(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, int32>  && DstElems == 32) return ::as_v32int32(from);
#else
    if constexpr (std::is_same_v<DstT, int32>  && DstElems == 32) return { ::as_v16int32(from[0]), ::as_v16int32(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, float>  && DstElems ==  4) return ::as_v4float(from);
    if constexpr (std::is_same_v<DstT, float>  && DstElems ==  8) return ::as_v8float(from);
    if constexpr (std::is_same_v<DstT, float>  && DstElems == 16) return ::as_v16float(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, float>  && DstElems == 32) return ::as_v32float(from);
#else
    if constexpr (std::is_same_v<DstT, float>  && DstElems == 32) return { ::as_v16float(from[0]), ::as_v16float(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, cint16> && DstElems ==  4) return ::as_v4cint16(from);
    if constexpr (std::is_same_v<DstT, cint16> && DstElems ==  8) return ::as_v8cint16(from);
    if constexpr (std::is_same_v<DstT, cint16> && DstElems == 16) return ::as_v16cint16(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, cint16> && DstElems == 32) return ::as_v32cint16(from);
#else
    if constexpr (std::is_same_v<DstT, cint16> && DstElems == 32) return { ::as_v16cint16(from[0]), ::as_v16cint16(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, cint32> && DstElems ==  2) return ::as_v2cint32(from);
    if constexpr (std::is_same_v<DstT, cint32> && DstElems ==  4) return ::as_v4cint32(from);
    if constexpr (std::is_same_v<DstT, cint32> && DstElems ==  8) return ::as_v8cint32(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, cint32> && DstElems == 16) return ::as_v16cint32(from);
#else
    if constexpr (std::is_same_v<DstT, cint32> && DstElems == 16) return { ::as_v8cint32(from[0]), ::as_v8cint32(from[1]) };
#endif

    if constexpr (std::is_same_v<DstT, cfloat> && DstElems ==  2) return ::as_v2cfloat(from);
    if constexpr (std::is_same_v<DstT, cfloat> && DstElems ==  4) return ::as_v4cfloat(from);
    if constexpr (std::is_same_v<DstT, cfloat> && DstElems ==  8) return ::as_v8cfloat(from);
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
    if constexpr (std::is_same_v<DstT, cfloat> && DstElems == 16) return ::as_v16cfloat(from);
#else
    if constexpr (std::is_same_v<DstT, cfloat> && DstElems == 16) return { ::as_v8cfloat(from[0]), ::as_v8cfloat(from[1]) };
#endif
}

/** 
 * Provides compile time access to the vector element type resulting from vector<T>::unpack operation.
 */
template <typename T>
struct unpacked_type {
    using type = utils::get_next_integer_type_t<T>;
};

// AIE1 does not support uint16 vector types, so unpacking uint8 vectors always yields an int16 vector.
template <>
struct unpacked_type<uint8> {
    using type = int16; 
};

/** Helper type alias for unpacked_type<T> */
template <typename T>
using unpacked_type_t = typename unpacked_type<T>::type;

/** 
 * Provides compile time access to the vector element type resulting from vector<T>::pack operation.
 */
template <typename T>
struct packed_type {
    using type = utils::get_prev_integer_type_t<T>;
};

/** Helper type alias for packed_type<T> */
template <typename T>
using packed_type_t = typename packed_type<T>::type;

/**
 * Architecture-specific implementation of the vector data type
 *
 * @tparam T Type of the elements contained in the vector.
 * @tparam Elems Number of elements in the vector.
 */
template <typename T, unsigned Elems>
class vector_base
{
private:
    template <typename T2, unsigned E2>     friend class vector_base;

    using vector_storage_type = vector_storage<T, Elems>;

public:
    using         native_type = native_vector_type_t<T, Elems>;
    using          value_type = T;
    using           storage_t = typename vector_storage_type::type;

    static constexpr unsigned type_bits()
    {
        return type_bits_v<T>;
    }

    static constexpr unsigned size()
    {
        return Elems;
    }

    static constexpr unsigned bits()
    {
        return type_bits() * Elems;
    }

    static constexpr bool is_signed()
    {
        return is_signed_v<T>;
    }

    static constexpr bool is_complex()
    {
        return is_complex_v<T>;
    }

    static constexpr bool is_real()
    {
        return !is_complex();
    }

    static constexpr bool is_integral()
    {
        return is_integral_v<T>;
    }

    static constexpr bool is_floating_point()
    {
        return is_floating_point_v<T>;
    }

    __aie_inline
    vector_base() :
        data(vector_storage_type::undef())
    {
    }

    __aie_inline
    vector_base(storage_t v) :
        data(v)
    {
        // Input is taken by value to avoid losing chess_storage qualifiers
    }

#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
    /**
     * Construct from internal types. This is a special case for 1024b internal vector types, which need to be broken
     * into two 512b vectors
     *
     * @param v Data used to construct the vector from
     */
    __aie_inline
    vector_base(const native_type &v) requires(bits() == 1024)
    {
        data[0] = ext_x(v, 0);
        data[1] = ext_x(v, 1);
    }
#endif

    template <typename... Values>
    __aie_inline
    explicit vector_base(value_type v, Values &&... values) :
        data(vector_storage_type::undef())
    {
        constexpr size_t num_params = sizeof...(values) + 1;

        if constexpr (type_bits() == 8) {
            // TODO: use unpack / pack
            init_from_values_complex(std::make_integer_sequence<unsigned, num_params>{},
                                     v,
                                     std::forward<Values>(values)...);
        }
        else if constexpr (bits() == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            init_from_values_complex(std::make_integer_sequence<unsigned, num_params>{},
                                     v,
                                     std::forward<Values>(values)...);
#else
            if constexpr (num_params <= Elems / 2) {
                init_from_values_simple(data[0], v, std::forward<Values>(values)...);
            }
            else {
                init_from_values_complex(std::make_integer_sequence<unsigned, num_params>{},
                                         v,
                                         std::forward<Values>(values)...);
            }
#endif
        }
        else {
            init_from_values_simple(data, v, std::forward<Values>(values)...);
        }
    }

    __aie_inline
    native_type to_native() const
    {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
        return data;
#else
        if constexpr (bits() == 1024)
            return ::concat(data[0], data[1]);
        else
            return data;
#endif
    }

    __aie_inline
    operator native_type() const
    {
        return to_native();
    }

    template <typename DstT>
    __aie_inline
    auto cast_to() const
    {
        constexpr unsigned DstSize  = type_bits_v<DstT>;
        constexpr unsigned DstElems = Elems * type_bits() / DstSize;

        vector_base<DstT, DstElems> ret; ret.data = vector_cast_helper<DstT, DstElems>(data);

        return ret;
    }

    __aie_inline
    vector_base &push(value_type v)
    {
        if constexpr (type_bits() == 8) {
            if constexpr (bits() == 1024) {
                vector_base<int16, Elems / 2> tmp1 = extract<Elems / 2>(0).unpack();
                vector_base<int16, Elems / 2> tmp2 = extract<Elems / 2>(1).unpack();

                tmp2 = tmp2.push(tmp1.get(Elems / 2 - 1));
                tmp1 = tmp1.push(v);

                data[0] = tmp1.template pack<value_type>().data;
                data[1] = tmp2.template pack<value_type>().data;
            }
            else {
                const vector_base<int16, Elems> tmp = unpack().push(v);

                data = tmp.template pack<value_type>().data;
            }
        }
        else {
            if constexpr (bits() == 1024) {
                const T m = get(Elems / 2 - 1);
                data[0] = ::shft_elem(data[0], v);
                data[1] = ::shft_elem(data[1], m);
            }
            else {
                data = ::shft_elem(data, v);
            }
        }

        return *this;
    }

    template <unsigned ElemsOut>
    __aie_inline
    constexpr vector_base<T, ElemsOut> grow(unsigned idx = 0) const
    {
        constexpr unsigned output_bits = type_bits() * ElemsOut;

        static_assert(output_bits >= bits());
        static_assert(output_bits == 128 || output_bits == 256 || output_bits == 512 || output_bits == 1024);

        vector_base<T, ElemsOut> ret;

#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        using output_vector_storage_type_half = vector_storage<T, ElemsOut / 2>;
#endif

        if constexpr (ElemsOut / Elems == 1) {
            ret.data = data;
        }
        else if constexpr (ElemsOut / Elems == 2) {
            if constexpr (output_bits == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                ret.data = ::yset_x(idx, data);
#else
                ret.data[idx]     = data;
                ret.data[1 - idx] = output_vector_storage_type_half::undef();
#endif
            }
            else if constexpr (output_bits == 512) {
                ret.data = ::xset_w(idx, data);
            }
            else if constexpr (output_bits == 256) {
                ret.data = ::wset_v(idx, data);
            }
        }
        else if constexpr (ElemsOut / Elems == 4) {
            if constexpr (output_bits == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                ret.data = ::yset_w(idx, data);
#else
                ret.data[idx / 2]     = ::xset_w(idx % 2, data);
                ret.data[1 - idx / 2] = output_vector_storage_type_half::undef();
#endif
            }
            else if constexpr (output_bits == 512) {
                ret.data = ::xset_v(idx, data);
            }
        }
        else if constexpr (ElemsOut / Elems == 8) {
            if constexpr (output_bits == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                ret.data = ::yset_v(idx, data);
#else
                ret.data[idx / 4]     = ::xset_v(idx % 4, data);
                ret.data[1 - idx / 4] = output_vector_storage_type_half::undef();
#endif
            }
        }

        return ret;
    }

    template <unsigned ElemsOut>
    __aie_inline
    constexpr vector_base<T, ElemsOut> grow_replicate() const
    {
        constexpr unsigned output_bits = type_bits() * ElemsOut;

        static_assert(output_bits >= bits());
        static_assert(output_bits == 128 || output_bits == 256 || output_bits == 512 || output_bits == 1024);

        vector_base<T, ElemsOut> ret;
        if constexpr (ElemsOut / Elems == 1) {
            return *this;
        }
        else if constexpr (ElemsOut / Elems == 2) {
            return ret.upd_all(*this, *this);
        }
        else if constexpr (ElemsOut / Elems == 4) {
            return ret.upd_all(*this, *this, *this, *this);
        }
        else if constexpr (ElemsOut / Elems == 8) {
            return ret.upd_all(*this, *this, *this, *this, *this, *this, *this, *this);
        }
    }

    __aie_inline
    void set(value_type v, unsigned idx)
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");

        if constexpr (utils::is_one_of_v<value_type, int8, uint8>) {
            if constexpr (Elems <= 64) {
                const bool odd = idx % 2;
                vector_base<int16, Elems / 2> tmp_vec = this->template cast_to<int16>();
                int16 tmp = tmp_vec.get(idx / 2);

                if (odd)
                    tmp = (tmp & 0x00ff) | (v << 8);
                else
                    tmp = (tmp & 0xff00) | (uint8)v;

                tmp_vec = ::upd_elem(tmp_vec, idx / 2, tmp);
                data = tmp_vec.template cast_to<value_type>();
            }
            else if constexpr (Elems == 128) {
                const bool odd = idx % 2;
                vector_base<int16, 32> tmp_vec = this->template extract<64>(idx / 64).template cast_to<int16>();
                int16 tmp = ::ext_elem(tmp_vec, (idx % 64) / 2);

                if (odd)
                    tmp = (tmp & 0x00ff) | (v << 8);
                else
                    tmp = (tmp & 0xff00) | (uint8)v;

                tmp_vec = ::upd_elem(tmp_vec, (idx % 64) / 2, tmp);

#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                const vector_base<value_type, 64> tmp1 = tmp_vec.template cast_to<value_type>();
                this->insert(idx / 64, tmp1);
#else
                data[idx / 64] = tmp_vec.template cast_to<value_type>();
#endif
            }
        }
        else {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            data = ::upd_elem(data, idx, v);
#else
            if constexpr (bits() == 1024) {
                if (idx < Elems / 2)
                    data[0] = ::upd_elem(data[0], idx, v);
                else
                    data[1] = ::upd_elem(data[1], idx - Elems / 2, v);
            }
            else {
                data = ::upd_elem(data, idx, v);
            }
#endif
        }
    }

    __aie_inline
    value_type get(unsigned idx) const
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");

        if constexpr (utils::is_one_of_v<T, int8, uint8>) {
            if constexpr (Elems <= 64) {
                const bool odd = idx % 2;
                const int16 tmp = cast_to<int16>().get(idx / 2);
                value_type ret;

                if (odd)
                    ret = tmp >> 8;
                else
                    ret = 0xff & tmp;

                return ret;
            }
            else if constexpr (Elems == 128) {
                const bool odd = idx % 2;
                const int16 tmp = extract<64>(idx / 64).template cast_to<int16>().get((idx % 64) / 2);
                value_type ret;

                if (odd)
                    ret = tmp >> 8;
                else
                    ret = 0xff & tmp;

                return ret;
            }
        }
        else {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            return ::ext_elem(data, idx);
#else
            if constexpr (bits() == 1024) {
                if (idx < Elems / 2)
                    return ::ext_elem(data[0], idx);
                else
                    return ::ext_elem(data[1], idx - Elems / 2);
            }
            else {
                return ::ext_elem(data, idx);
            }
#endif
        }
    }

    template <typename... SubVectors>
    __aie_inline
    vector_base &upd_all(SubVectors && ...subvectors)
    {
        constexpr unsigned num_subvectors  = sizeof...(subvectors);
        constexpr unsigned subvector_bits  = bits() / num_subvectors;
        constexpr unsigned subvector_elems = Elems / num_subvectors;

        static_assert(sizeof...(subvectors) > 1);
        static_assert((std::is_base_of_v<vector_base<T, subvector_elems>, utils::remove_all_t<SubVectors>> && ...));

        static_assert(bits() == subvector_bits * num_subvectors);

        static_assert(subvector_bits >= 128);

        if constexpr (bits() == 1024 && utils::num_elems_v<vector_storage_t<T, Elems>> == 2) {
            const auto t = std::make_tuple(std::forward<SubVectors>(subvectors)...);

            if constexpr (num_subvectors == 2) {
                data[0] = std::get<0>(t).data;
                data[1] = std::get<1>(t).data;
            }
            else if constexpr (num_subvectors == 4) {
                data[0] = ::concat(std::get<0>(t), std::get<1>(t));
                data[1] = ::concat(std::get<2>(t), std::get<3>(t));
            }
            else if constexpr (num_subvectors == 8) {
                data[0] = ::concat(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t));
                data[1] = ::concat(std::get<4>(t), std::get<5>(t), std::get<6>(t), std::get<7>(t));
            }
        }
        else {
            data = ::concat(std::forward<SubVectors>(subvectors).data...);
        }

        return *this;
    }

    template <unsigned ElemsIn>
    __aie_inline
    vector_base &insert(unsigned idx, const vector_base<T, ElemsIn> &v)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");

        constexpr unsigned input_bits = type_bits() * ElemsIn;

        static_assert(input_bits <= bits());
        static_assert(input_bits == 128 || input_bits == 256 || input_bits == 512 || input_bits == 1024);

        if constexpr (input_bits == bits()) {
            data = v.data;
        }
        else {
            if (__builtin_constant_p(idx)) {
                if constexpr (bits() == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                    if constexpr (input_bits == 512) {
                        // TODO: investigate why this is needed as it is a constant
                        if (idx == 0)
                            data = ::upd_x(data, 0, v);
                        else
                            data = ::upd_x(data, 1, v);
                    }
                    else if constexpr (input_bits == 256)
                        data = ::upd_w(data, idx, v);
                    else if constexpr (input_bits == 128)
                        data = ::upd_v(data, idx, v);
#else
                    if constexpr (input_bits == 512)
                        data[idx] = v.data;
                    else if constexpr (input_bits == 256)
                        data[idx / 2] = ::upd_w(data[idx / 2], idx % 2, v);
                    else if constexpr (input_bits == 128)
                        data[idx / 4] = ::upd_v(data[idx / 4], idx % 4, v);
#endif
                }
                else {
                    if constexpr (input_bits == 256)
                        data = ::upd_w(data, idx, v);
                    else if constexpr (input_bits == 128)
                        data = ::upd_v(data, idx, v);
                }
            }
            else if constexpr (bits() == 1024 && input_bits == 512) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                if (idx == 0)
                    data = ::upd_x(data, 0, v);
                else
                    data = ::upd_x(data, 1, v);
#else
                data[idx] = v.data;
#endif
            }
            else {
                constexpr unsigned mask_base = input_bits == 256? (1 << 4) - 1 : (1 << 2) - 1;
                constexpr unsigned shift_subvector = input_bits / type_bits_v<cint32>;
                vector_base<cint32, 8> tmp;

                if constexpr (bits() == 1024) {
                    const vector_base<cint32, 8> input = v.template grow<Elems / 2>().template cast_to<cint32>();
                    unsigned word_idx;
                    unsigned local_idx;

                    if constexpr (input_bits == 256) {
                        word_idx  = idx < 2?   0 : 1;
                        local_idx = idx < 2? idx : idx - 2;
                    }
                    else {
                        word_idx  = idx < 4?   0 : 1;
                        local_idx = idx < 4? idx : idx - 4;
                    }

                    const unsigned mask = mask_base << (local_idx * shift_subvector);

                    tmp = ::select8(mask, this->cast_to<cint32>().template extract<8>(word_idx), 0, 0x76543210, input, -(local_idx * shift_subvector), 0x76543210);

#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                    if (word_idx == 0)
                        data = ::upd_x(data, 0, tmp.cast_to<T>());
                    else
                        data = ::upd_x(data, 1, tmp.cast_to<T>());
#else
                    data[word_idx] = tmp.cast_to<T>();
#endif
                }
                else if constexpr (bits() == 512) {
                    const unsigned mask = mask_base << (idx * shift_subvector);
                    const vector_base<cint32, 8> input = v.template grow<Elems>().template cast_to<cint32>();

                    tmp = ::select8(mask, this->cast_to<cint32>(), 0, 0x76543210, input, -(idx * shift_subvector), 0x76543210);

                    data = vector_cast_helper<value_type, Elems>(tmp.data);
                }
                else if constexpr (bits() == 256) {
                    const unsigned mask = mask_base << (idx * shift_subvector);
                    const vector_base<cint32, 8> input = v.template grow<Elems * 2>().template cast_to<cint32>();

                    tmp = ::select8(mask, this->grow<Elems * 2>().template cast_to<cint32>(), 0, 0x76543210, input, -(idx * shift_subvector), 0x76543210);

                    data = vector_cast_helper<value_type, Elems>(::ext_w(tmp, 0));
                }
            }
        }

        return *this;
    }

    template <unsigned ElemsIn>
    __aie_inline
    vector_base &insert(unsigned idx, typename vector_base<T, ElemsIn>::native_type v)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");

        const vector_base<T, ElemsIn> in = v;

        return insert(idx, in);
    }

    template <unsigned ElemsOut>
    __aie_inline
    vector_base<value_type, ElemsOut> extract(unsigned idx) const
    {
        REQUIRES_MSG(idx < Elems / ElemsOut, "idx needs to be a valid subvector index");

        constexpr unsigned output_bits = type_bits() * ElemsOut;

        static_assert(output_bits <= bits());
        static_assert(output_bits == 128 || output_bits == 256 || output_bits == 512 || output_bits == 1024);

        if constexpr (output_bits == bits()) {
            return *this;
        }
        else {
            vector_base<value_type, ElemsOut> ret;

            if (__builtin_constant_p(idx)) {
                if constexpr (bits() == 1024) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                    if constexpr (output_bits == 512) {
                        // TODO: investigate why this is needed as it is a constant
                        if (idx == 0)
                            ret.data = ::ext_x(data, 0);
                        else
                            ret.data = ::ext_x(data, 1);
                    }
                    else if constexpr (output_bits == 256)
                        ret.data = ::ext_w(data, idx);
                    else if constexpr (output_bits == 128)
                        ret.data = ::ext_v(data, idx);
#else
                    if constexpr (output_bits == 512)
                        ret.data = data[idx];
                    else if constexpr (output_bits == 256)
                        ret.data = ::ext_w(data[idx / 2], idx % 2);
                    else if constexpr (output_bits == 128)
                        ret.data = ::ext_v(data[idx / 4], idx % 4);
#endif
                }
                else {
                    if constexpr (output_bits == 256)
                        ret.data = ::ext_w(data, idx);
                    else if constexpr (output_bits == 128)
                        ret.data = ::ext_v(data, idx);
                }

                return ret;
            }
            else {
                constexpr unsigned shift_subvector = output_bits / type_bits_v<cint32>;

                if constexpr (output_bits == 512) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
                    const vector_base<cint32, 8> tmp = ::shuffle8(cast_to<cint32>(), idx * shift_subvector, 0x76543210);
                    ret = vector_cast_helper<value_type, ElemsOut>(tmp);
#else
                    ret = data[idx];
#endif
                }
                else if constexpr (output_bits == 256) {
                    const vector_base<cint32, 8> tmp = ::shuffle8(cast_to<cint32>(), idx * shift_subvector, 0x3210);
                    ret = vector_cast_helper<value_type, ElemsOut>(::ext_w(tmp, 0));
                }
                else if constexpr (output_bits == 128) {
                    constexpr unsigned cint32_elems = std::max(bits() / type_bits_v<cint32>, 8u);
                    const vector_base<cint32, 8> tmp = ::shuffle8(cast_to<cint32>().template grow<cint32_elems>(), idx * shift_subvector, 0x10);
                    ret = vector_cast_helper<value_type, ElemsOut>(::ext_v(tmp, 0));
                }
            }

            return ret;
        }
    }

    template <unsigned ElemsOut>
    __aie_inline
    vector_base<value_type, ElemsOut> grow_extract(unsigned idx) const
    {
        if constexpr (ElemsOut > Elems)
            return grow<ElemsOut>();
        else
            return extract<ElemsOut>(idx);
    }

    template <unsigned ElemsOut>
    __aie_inline
    auto split() const
    {
        constexpr unsigned output_bits = type_bits() * ElemsOut;

        static_assert(output_bits < bits());
        static_assert(output_bits == 128 || output_bits == 256 || output_bits == 512);

        if constexpr (output_bits == bits())
            *this;
        else
            return split_helper<ElemsOut>(std::make_integer_sequence<unsigned, Elems / ElemsOut>{});
    }

    template <typename T2 = utils::get_next_integer_type_t<T>>
    __aie_inline
    vector_base<int16, Elems> unpack() const requires(utils::is_one_of_v<T,  int8,  uint8>)
    {
        vector_base<int16, Elems> ret;

        if constexpr (Elems <= 32) {
            ret.data = ::unpack(data);
        }
        else if constexpr (Elems == 64) {
            auto [v1, v2] = split<32>();

            ret.upd_all(v1.unpack(), v2.unpack());
        }

        return ret;
    }

    // Dynamic sign is not supported in this architecture
    template <typename T2 = utils::get_next_integer_type_t<T>>
    __aie_inline
    vector_base<int16, Elems> unpack_sign(bool v_sign) const requires(utils::is_one_of_v<T, int8, uint8>
                                                                      && std::is_same_v<T2, int16>)
    {
        return unpack();
    }

    template <typename T2 = utils::get_prev_integer_type_t<T>>
    __aie_inline
    vector_base<T2, Elems> pack() const requires(utils::is_one_of_v<T2, int8, uint8> &&
                                                 utils::is_one_of_v<T,  int16, uint16>)
    {
        vector_base<T2, Elems> ret;
        if constexpr (Elems <= 32) {
            if constexpr (is_signed_v<T2>)
                ret.data = ::pack(data);
            else
                ret.data = ::upack(data);
        }
        else if constexpr (Elems == 64) {
            auto [v1, v2] = split<32>();

            ret.upd_all(v1.template pack<T2>(), v2.template pack<T2>());
        }

        return ret;
    }

    template <typename T2>
    __aie_inline
    vector_base<T2, Elems> pack_sign(bool v_sign) const requires(std::is_same_v<T, int16>)
    {
        return pack<T2>();
    }

    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void load(const T2 *ptr)
    {
        if constexpr (bits() == 1024) {
            using native_type = native_vector_type_t<value_type, Elems / 2>;
            using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<native_type, aie_dm_resource_get_v<T2>>>;

            vector_base<T, Elems / 2> tmp1; tmp1.data = ((const aliased_type *) ptr)[0]; this->insert(0, tmp1);
            vector_base<T, Elems / 2> tmp2; tmp2.data = ((const aliased_type *) ptr)[1]; this->insert(1, tmp2);
        }
        else {
            using native_type = native_vector_type_t<value_type, Elems>;
            using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<native_type, aie_dm_resource_get_v<T2>>>;

            data = *(const aliased_type *)ptr;
        }
    }

    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void store(T2 *ptr) const
    {
        if constexpr (bits() == 1024) {
            using native_type = native_vector_type_t<value_type, Elems / 2>;
            using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<native_type, aie_dm_resource_get_v<T2>>>;

            ((aliased_type *)ptr)[0] = this->extract<Elems / 2>(0).data;
            ((aliased_type *)ptr)[1] = this->extract<Elems / 2>(1).data;
        }
        else {
            using native_type = native_vector_type_t<value_type, Elems>;
            using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<native_type, aie_dm_resource_get_v<T2>>>;

            *(aliased_type *)ptr = data;
        }
    }

    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void load_unaligned(const T2 *ptr, unsigned aligned_elems)
    {
        constexpr unsigned native_elems = 256 / type_bits();

        using  native_type = native_vector_type_t<T, native_elems>;
        using aliased_type = const add_memory_bank_t<Resource, aie_dm_resource_set_t<native_type, aie_dm_resource_get_v<T2>>>;

        const unsigned aligned_bits = aligned_elems * type_bits();

        if (chess_manifest(aligned_bits >= 128)) {
            load<Resource>(ptr);
        }
        else if (type_bits() >= 32 || chess_manifest(aligned_bits >= 32)) {
            // Align frac to int32
            const uintptr_t addr = uintptr_t(ptr);
            const unsigned frac = (addr & 15) >> 2;

            aliased_type *ptr2 = (aliased_type *)utils::floor_ptr(ptr);

            if constexpr (bits() == 128) {
                vector_base<int32, 16> tmp;

                tmp = ::xset_w(0,      ::as_v8int32(*ptr2));
                tmp = ::shuffle16(tmp, frac, 0x00003210, 0x00000000);

                data = tmp.template cast_to<T>().template extract<Elems>(0);
            }
            else if constexpr (bits() == 256) {
                vector_base<int32, 16> tmp;

                tmp = ::xset_w(0,     ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v8int32(*ptr2));
                tmp = ::shuffle16(tmp, frac, 0x76543210, 0x00000000);

                data = tmp.template cast_to<T>().template extract<Elems>(0);
            }
            else if constexpr (bits() == 512) {
                // Use low-level type to ensure Y register utilization
                v32int32 tmp;
                vector_base<int32, 16> res;

                tmp = ::yset_w(0,     ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 2, ::as_v8int32(*ptr2));
                res = ::shuffle16(tmp, frac, 0x76543210, 0xfedcba98);

                data = res.template cast_to<T>();
            }
            else if constexpr (bits() == 1024) {
                // Use low-level type to ensure Y register utilization
                v32int32 tmp;
                vector_base<int32, 16> res;

                tmp = ::yset_w(0,     ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 2, ::as_v8int32(*ptr2++));
                res = ::shuffle16(tmp, frac, 0x76543210, 0xfedcba98);
                this->insert(0, res.cast_to<T>());

                tmp = ::upd_w(tmp, 3, ::as_v8int32(*ptr2++));
                tmp = ::upd_w(tmp, 0, ::as_v8int32(*ptr2));
                res = ::shuffle16(tmp, frac + 16, 0x76543210, 0xfedcba98);
                this->insert(1, res.cast_to<T>());
            }
        }
        else if (type_bits() >= 16 || chess_manifest(aligned_bits >= 16)) {
            // Align frac to int16
            const uintptr_t addr = uintptr_t(ptr);

            const unsigned frac = (addr & 15) >> 1;
            const unsigned odd = addr & 0x2;

            const unsigned mask   = odd? 0x88888888 : 0x0;
            const unsigned square = odd? 0x0321 : 0x3210;

            aliased_type *ptr2 = (aliased_type *)utils::floor_ptr(ptr);

            // The implementation takes advantage of the HW automatically rounding start to 2 for X/Y buffers

            if constexpr (bits() == 128) {
                vector_base<int16, 32> tmp;

                tmp = ::xset_w(0,      ::as_v16int16(*ptr2));
                tmp = ::select32(mask, tmp, frac,     0x06040200, 0x0e0c0a08, square,
                                            frac + 2, 0x06040200, 0x0e0c0a08, 0x2000);

                data = tmp.template cast_to<T>().template extract<Elems>(0);
            }
            else if constexpr (bits() == 256) {
                vector_base<int16, 32> tmp;

                tmp = ::xset_w(0,     ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v16int16(*ptr2));
                tmp = ::select32(mask, tmp, frac,     0x06040200, 0x0e0c0a08, square,
                                            frac + 2, 0x06040200, 0x0e0c0a08, 0x2000);

                data = tmp.template cast_to<T>().template extract<Elems>(0);
            }
            else if constexpr (bits() == 512) {
                // Use low-level type to ensure Y register utilization
                v64int16 tmp;
                vector_base<int16, 32> res;

                tmp = ::yset_w(0,     ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 2, ::as_v16int16(*ptr2));
                res = ::select32(mask, tmp, frac,     0x06040200, 0x0e0c0a08, square,
                                            frac + 2, 0x06040200, 0x0e0c0a08, 0x2000);

                data = res.template cast_to<T>();
            }
            else if constexpr (bits() == 1024) {
                // Use low-level type to ensure Y register utilization
                v64int16 tmp;
                vector_base<int16, 32> res;

                tmp = ::yset_w(0,     ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 1, ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 2, ::as_v16int16(*ptr2++));
                res = ::select32(mask, tmp, frac,     0x06040200, 0x0e0c0a08, square,
                                            frac + 2, 0x06040200, 0x0e0c0a08, 0x2000);
                this->insert(0, res.cast_to<T>());

                tmp = ::upd_w(tmp, 3, ::as_v16int16(*ptr2++));
                tmp = ::upd_w(tmp, 0, ::as_v16int16(*ptr2));
                res = ::select32(mask, tmp, (frac + 32),     0x06040200, 0x0e0c0a08, square,
                                            (frac + 32) + 2, 0x06040200, 0x0e0c0a08, 0x2000);
                this->insert(1, res.cast_to<T>());
            }
        }
        else {
            load_unaligned_scalar<int8, Resource>(ptr);
        }
    }

    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void store_unaligned(T2 *ptr, unsigned aligned_elems) const
    {
        const unsigned aligned_bits = aligned_elems * type_bits();

        if (aligned_bits >= 128)
            store<Resource>(ptr);
        else if (type_bits() >= 64 || chess_manifest(aligned_bits >= 64))
            store_unaligned_scalar<cint32, Resource>(ptr);
        else if (type_bits() >= 32 || chess_manifest(aligned_bits >= 32))
            store_unaligned_scalar<int32, Resource>(ptr);
        else if (type_bits() >= 16 || chess_manifest(aligned_bits >= 16))
            store_unaligned_scalar<int16, Resource>(ptr);
        else
            store_unaligned_scalar<int8, Resource>(ptr);
    }

private:
    template <typename Data, typename... Values, typename T2 = T> requires(type_bits_v<T2> > 8)
    __aie_inline
    void init_from_values_simple(Data &d, const value_type &v, Values &&... values)
    {
        if constexpr (sizeof...(values) > 0)
            init_from_values_simple(d, std::forward<Values>(values)...);

        d = ::shft_elem(d, v);
    }

    template <unsigned... Indices, typename... Values>
    __aie_inline
    void init_from_values_complex(std::integer_sequence<unsigned, Indices...>, Values &&... values)
    {
        (set(std::forward<Values>(values), Indices), ...);
    }

    template <unsigned ElemsOut, unsigned... Indices>
    __aie_inline
    std::array<vector_base<T, ElemsOut>, Elems/ElemsOut> split_helper(std::integer_sequence<unsigned, Indices...> &&) const
    {
        return {extract<ElemsOut>(Indices)...};
    }

    template <typename ScalarType, aie_dm_resource Resource, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    void load_unaligned_scalar(const T2 *ptr)
    {
        constexpr unsigned num_elems = bits() / type_bits_v<ScalarType>;
        vector_base<ScalarType, num_elems> tmp;

        using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<ScalarType, aie_dm_resource_get_v<T2>>>;

        if constexpr (type_bits_v<ScalarType> > 8) {
            for (unsigned i = 0; i < num_elems; ++i) chess_unroll_loop()
                tmp.set(((const aliased_type *)ptr)[i], i);
        }
        else {
            for (unsigned i = 0; i < num_elems; ++i) chess_prepare_for_pipelining
                tmp.set(((const aliased_type *)ptr)[i], i);
        }

        data = vector_cast_helper<value_type, Elems>(tmp.data);
    }

    template <typename ScalarType, aie_dm_resource Resource, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    void store_unaligned_scalar(T2 *ptr) const
    {
        constexpr unsigned num_elems = bits() / type_bits_v<ScalarType>;
        const vector_base<ScalarType, num_elems> tmp = vector_cast_helper<ScalarType, num_elems>(data);

        using aliased_type = add_memory_bank_t<Resource, aie_dm_resource_set_t<ScalarType, aie_dm_resource_get_v<T2>>>;

        aliased_type *ptr2 = (aliased_type *)ptr;

        if constexpr (type_bits_v<ScalarType> > 8) {
            for (unsigned i = 0; i < num_elems; ++i) chess_unroll_loop()
                ptr2[i] = tmp.get(i);
        }
        else {
            for (unsigned i = 0; i < num_elems; ++i) chess_prepare_for_pipelining
                ptr2[i] = tmp.get(i);
        }
    }

#ifdef AIE_API_EMULATION
    std::array<T, Elems> data;
#else
    vector_storage_t<T, Elems> data;
#endif
};

}

#endif // __AIE_API_DETAIL_AIE1_VECTOR__HPP__
