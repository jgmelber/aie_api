// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE2_ACCUM__HPP__

#include "accum_native_types.hpp"

#include "../utils.hpp"
#include "../../vector.hpp"

namespace aie::detail {

template <AccumClass Class, unsigned MinBits>
static constexpr unsigned to_native_accum_bits()
{
    if constexpr (Class == AccumClass::FP || Class == AccumClass::CFP) {
        static_assert(MinBits <= 32);

        return 32;
    }
    else {
        static_assert(MinBits <= 64);

        if constexpr (MinBits <= 32 && Class == AccumClass::Int)
            return 32;
        else if constexpr (MinBits <= 64)
            return 64;
    }
}

template <typename AccumTag>
static constexpr unsigned to_native_accum_bits()
{
    return to_native_accum_bits<accum_class_for_tag_v<AccumTag>, accum_bits_for_tag_v<AccumTag>>();
}

template <typename AccumTag>
static constexpr unsigned to_native_value_bits()
{
  if constexpr (accum_class_for_tag_v<AccumTag> == AccumClass::CInt || accum_class_for_tag_v<AccumTag> == AccumClass::CFP)
      return 2 * to_native_accum_bits<AccumTag>();
  else
      return to_native_accum_bits<AccumTag>();
}

template <AccumClass Class, unsigned Bits, unsigned DstElems, typename T>
static __aie_inline accum_storage_t<Class, Bits, DstElems> accum_cast_helper(T &&from)
{
    static constexpr bool       is_complex = Class == AccumClass::CInt
#if __AIE_API_COMPLEX_FP32_EMULATION__
                                          || Class == AccumClass::CFP
#endif
                                             ;
    static constexpr unsigned bits_per_elem = is_complex ? 2 * Bits : Bits;
    static constexpr unsigned native_elems  = 1024 / bits_per_elem;
    static constexpr unsigned Blocks        = DstElems / native_elems;

    if constexpr (Class == AccumClass::Int) {
        if constexpr (Bits == 32 && DstElems == 8)   return v8acc32(from);
        if constexpr (Bits == 32 && DstElems == 16)  return v16acc32(from);
        if constexpr (Bits == 32 && DstElems == 32)  return v32acc32(from);
        if constexpr (Bits == 32 && DstElems >= 64)  return utils::make_array<Blocks>([](auto f) __aie_inline { return v32acc32(f); }, from);

        if constexpr (Bits == 64 && DstElems == 4)   return v4acc64(from);
        if constexpr (Bits == 64 && DstElems == 8)   return v8acc64(from);
        if constexpr (Bits == 64 && DstElems == 16)  return v16acc64(from);
        if constexpr (Bits == 64 && DstElems >= 32)  return utils::make_array<Blocks>([](auto f) __aie_inline { return v16acc64(f); }, from);
    }
    else if constexpr (Class == AccumClass::FP) {
        if constexpr (DstElems == 4)                 return v8accfloat(from);
        if constexpr (DstElems == 8)                 return v8accfloat(from);
        if constexpr (DstElems == 16)                return v16accfloat(from);
        if constexpr (DstElems == 32)                return v32accfloat(from);
        if constexpr (DstElems >= 64)                return utils::make_array<Blocks>([](auto f) __aie_inline { return v32accfloat(f); }, from);
    }
    else if constexpr (Class == AccumClass::CInt) {
        if constexpr (DstElems == 2)                 return v2cacc64(from);
        if constexpr (DstElems == 4)                 return v4cacc64(from);
        if constexpr (DstElems == 8)                 return v8cacc64(from);
        if constexpr (DstElems >= 16)                return utils::make_array<Blocks>([](auto f) __aie_inline { return v8cacc64(f); }, from);
    }
#if __AIE_API_COMPLEX_FP32_EMULATION__
    else if constexpr (Class == AccumClass::CFP) {
        if constexpr (DstElems == 2)                 return v4caccfloat(from);
        if constexpr (DstElems == 4)                 return v4caccfloat(from);
        if constexpr (DstElems == 8)                 return v8caccfloat(from);
        if constexpr (DstElems == 16)                return v16caccfloat(from);
        if constexpr (DstElems >= 32)                return utils::make_array<Blocks>([](auto f) __aie_inline { return v16caccfloat(f); }, from);
    }
#endif
}

template <unsigned Elems, typename T> static auto accum_extract(const T& acc, unsigned idx);

template <> inline __aie_inline auto accum_extract<16, v32acc32>(const v32acc32& acc, unsigned idx) { return ::extract_v16acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32acc32>(const v32acc32& acc, unsigned idx) { return ::extract_v8acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v16acc32>(const v16acc32& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16acc32>(const v16acc32& acc, unsigned idx) { return ::extract_v8acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8acc32>(const  v8acc32& acc, unsigned idx) { return acc; }

template <> inline __aie_inline auto accum_extract<16, v16acc64>(const v16acc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16acc64>(const v16acc64& acc, unsigned idx) { return ::extract_v8acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v16acc64>(const v16acc64& acc, unsigned idx) { return ::extract_v4acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8acc64>(const  v8acc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8acc64>(const  v8acc64& acc, unsigned idx) { return ::extract_v4acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4acc64>(const  v4acc64& acc, unsigned idx) { return acc; }

template <> inline __aie_inline auto accum_extract<8,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return ::extract_v4cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<2,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return ::extract_v2cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4cacc64>(const  v4cacc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<2,   v4cacc64>(const  v4cacc64& acc, unsigned idx) { return ::extract_v2cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<2,   v2cacc64>(const  v2cacc64& acc, unsigned idx) { return acc; }

template <> inline __aie_inline auto accum_extract<32, v32accfloat>(const v32accfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<16, v32accfloat>(const v32accfloat& acc, unsigned idx) { return ::extract_v16accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32accfloat>(const v32accfloat& acc, unsigned idx) { return ::extract_v8accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v16accfloat>(const v16accfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16accfloat>(const v16accfloat& acc, unsigned idx) { return ::extract_v8accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8accfloat>(const  v8accfloat& acc, unsigned idx) { return acc; }
// Special cases as accum<accfloat, 4> is backed by v8accfloat
template <> inline __aie_inline auto accum_extract<4,  v32accfloat>(const v32accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<4,  v16accfloat>(const v16accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<4,   v8accfloat>(const  v8accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <> inline __aie_inline auto accum_extract<16, v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return ::extract_v8caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return ::extract_v4caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return ::extract_v4caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4caccfloat>(const  v4caccfloat& acc, unsigned idx) { return acc; }
// Special cases as accum<caccfloat, 2> is backed by v4caccfloat
template <> inline __aie_inline auto accum_extract<2,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<2,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<2,   v4caccfloat>(const  v4caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
#endif

template <AccumClass Class, unsigned MinBits, unsigned Elems>
class accum_base;

template <AccumClass Class, unsigned MinBits, unsigned... Es>
__aie_inline
accum_base<Class, MinBits, (Es + ...)> concat_helper(const accum_base<Class, MinBits, Es>&... inputs);

template <AccumClass Class, unsigned MinBits, unsigned Elems>
class accum_base
{
    static constexpr unsigned Bits = to_native_accum_bits<Class, MinBits>();

    using storage_type = accum_storage<Class, Bits, Elems>;

    template <AccumClass C2, unsigned MN2, unsigned E2>
    friend class accum_base;

public:
    using value_type = accum_tag_t<Class, MinBits>;
    using storage_t  = accum_storage_t<Class, Bits, Elems>;

    static constexpr AccumClass value_class()
    {
        return Class;
    }

    static constexpr unsigned accum_bits()
    {
        return Bits;
    }

    static constexpr unsigned accum_min_bits()
    {
        return MinBits;
    }

    static constexpr unsigned value_bits()
    {
        if constexpr (is_complex())
            return accum_bits() * 2;
        else
            return accum_bits();
    }

    static constexpr unsigned memory_bits()
    {
        constexpr unsigned v = value_bits();

        if constexpr (utils::is_powerof2(v)) {
            return v;
        }
        else {
            return 1u << (utils::fls(v) + 1);
        }
    }

    static constexpr unsigned size()
    {
        return Elems;
    }

    static constexpr unsigned bits()
    {
        if constexpr (is_complex())
            return 2 * Bits * size();
        else
            return Bits * size();
    }

    static constexpr bool is_complex()
    {
        return (Class == AccumClass::CInt)
#if __AIE_API_COMPLEX_FP32_EMULATION__
            || (Class == AccumClass::CFP)
#endif
            ;
    }

    static constexpr bool is_real()
    {
        return !is_complex();
    }

    static constexpr bool is_floating_point()
    {
        return (Class == AccumClass::FP)
#if __AIE_API_COMPLEX_FP32_EMULATION__
            || (Class == AccumClass::CFP)
#endif
            ;
    }

    __aie_inline
    accum_base() :
        data(storage_type::undef())
    {
    }

    template <unsigned MinBits2> requires(accum_base<Class, MinBits2, Elems>::accum_bits() == accum_bits())
    __aie_inline
    accum_base(const accum_base<Class, MinBits2, Elems> &acc) :
        data(acc.data)
    {
    }

    __aie_inline
    accum_base(storage_t data) :
        data(data)
    {
        // Input is taken by value to avoid losing chess_storage qualifiers
    }

    template <typename T>
    __aie_inline
    explicit accum_base(const vector<T, Elems> &v, int shift = 0)
    {
        from_vector(v, shift);
    }

    __aie_inline
    operator storage_t() const
    {
        return data;
    }

    __aie_inline
    auto to_native() const requires(utils::num_elems_v<storage_t> == 1)
    {
        return data;
    }

    template <typename DstTag>
    __aie_inline
    auto cast_to() const
    {
        constexpr unsigned DstSize  = to_native_value_bits<DstTag>();
        constexpr unsigned DstElems = (DstSize <= value_bits())? Elems * (value_bits() / DstSize) :
                                                                 Elems / (     DstSize / value_bits());

        constexpr AccumClass DstClass  = accum_class_for_tag_v<DstTag>;

        constexpr unsigned RealDstSize = to_native_accum_bits<DstTag>();

        accum_base<DstClass, RealDstSize, DstElems> ret;
        ret = accum_cast_helper<DstClass, RealDstSize, DstElems>(data);

        return ret;
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow() const
    {
        static_assert(ElemsOut >= size());

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        constexpr unsigned in_num_subaccums  = utils::num_elems_v<storage_t>;
        constexpr unsigned out_num_subaccums = utils::num_elems_v<storage_out_t>;
        accum_base<Class, MinBits, ElemsOut> ret;

        //Special case with no v4accfloat, which we represent with a v8accfloat internally for the storage type
        constexpr unsigned growth_ratio = []() { if      constexpr (Class == AccumClass::FP  && Elems == 4) return ElemsOut / (2 * Elems) ;
#if __AIE_API_COMPLEX_FP32_EMULATION__
                                                 else if constexpr (Class == AccumClass::CFP && Elems == 2) return ElemsOut / (2 * Elems) ;
#endif
                                                 else                                                       return ElemsOut / Elems;
                                               }();

        if constexpr (growth_ratio == 1) {
            ret = data;
        }
        else if constexpr (in_num_subaccums == 1 && out_num_subaccums == 1) {
            if constexpr (growth_ratio == 2)
                ret = ::concat(data, storage_type::undef());
            else if constexpr (growth_ratio == 4)
                ret = ::concat(data, storage_type::undef(), storage_type::undef(), storage_type::undef());
        }
        else if constexpr (in_num_subaccums == 1 && out_num_subaccums > 1) {
            // Other elements are default initialized to undef() already
            ret.data[0] = this->grow<native_elems>();
        }
        else {
            // Other elements are default initialized to undef() already
            utils::unroll_times<out_num_subaccums>([&](unsigned elem) __aie_inline {
                ret.data[elem] = data[elem];
            });
        }

        return ret;
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow_replicate() const
    {
        static_assert(ElemsOut >= size());

        constexpr unsigned growth_ratio = ElemsOut / Elems;

        if constexpr (growth_ratio == 1) {
            return *this;
        }
        else {
            accum_base<Class, MinBits, Elems * growth_ratio> tmp;

            if constexpr (growth_ratio >=  2) {
                if constexpr (bits() == 256) {
                    using return_type        = accum_base<Class, Bits, Elems * 2>;
                    using native_return_type = typename return_type::storage_t;
                    using shuffle_type = v16int32;
                    tmp.insert(0, return_type((native_return_type)::shuffle((shuffle_type)this->template grow<Elems * 2>(),
                                                                            (shuffle_type)this->template grow<Elems * 2>(),
                                                                            T256_2x2_lo)));
                }
                else {
                    tmp.insert(0, *this);
                    tmp.insert(1, tmp.template extract<1 * Elems>(0));
                }
            }
            if constexpr (growth_ratio >=  4) tmp.insert(1, tmp.template extract< 2 * Elems>(0));
            if constexpr (growth_ratio >=  8) tmp.insert(1, tmp.template extract< 4 * Elems>(0));
            if constexpr (growth_ratio >= 16) tmp.insert(1, tmp.template extract< 8 * Elems>(0));
            if constexpr (growth_ratio >= 32) tmp.insert(1, tmp.template extract<16 * Elems>(0));

            return tmp;
        }
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> extract(unsigned idx) const
    {
        static_assert(ElemsOut <= size() && utils::is_powerof2(ElemsOut));

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        // Handle special case for 128b float accums that are backed by 256b native accums
        constexpr bool fpacc_128b = (Class == AccumClass::FP  && ElemsOut == 4)
#if __AIE_API_COMPLEX_FP32_EMULATION__
                                    || (Class == AccumClass::CFP && ElemsOut == 2)
#endif
                                    ;

        constexpr unsigned num_subaccums     = utils::num_elems_v<storage_t>;
        constexpr unsigned num_subaccums_out = utils::num_elems_v<storage_out_t>;

        if constexpr (ElemsOut == size()) {
            return *this;
        }
        else {
            constexpr unsigned elems_per_subaccum     = Elems  / num_subaccums;
            constexpr unsigned out_elems_per_subaccum = ElemsOut / num_subaccums_out;
            constexpr unsigned ratio                  = elems_per_subaccum / out_elems_per_subaccum;

            accum_base<Class, MinBits, ElemsOut> ret;

            if constexpr (num_subaccums_out == 1) {
                if constexpr (elems_per_subaccum == out_elems_per_subaccum) {
                    ret = data[idx];
                    return ret;
                }
                else if constexpr (num_subaccums == 1) {
                    if constexpr (fpacc_128b) {
#if __AIE_API_COMPLEX_FP32_EMULATION__
                        using vector_elem_type = std::conditional_t<Class == AccumClass::CFP, cfloat, float>;
#else
                        using vector_elem_type = float;
#endif
                        accum_base<Class, MinBits, 2 * ElemsOut> tmp = accum_extract<2 * ElemsOut>(data, idx / 2);
                        auto v = tmp.template cast_to<accfloat>().template to_vector<float>();
                        v.insert(0, v.template extract<4>(idx % 2));
                        ret.from_vector(v.template cast_to<vector_elem_type>().template extract<ElemsOut>(0));
                    }
                    else {
                        ret = accum_extract<ElemsOut>(data, idx);
                    }

                    return ret;
                }
                else {
                    if constexpr (fpacc_128b) {
                        constexpr unsigned ratio = elems_per_subaccum / ElemsOut;
                        return accum_base<Class, MinBits, elems_per_subaccum>(data[idx / ratio]).template extract<ElemsOut>(idx % ratio);
                    }
                    else {
                        ret = accum_extract<ElemsOut>(data[idx / ratio], idx % ratio);
                        return ret;
                    }
                }
            }
            else {
                utils::unroll_times<num_subaccums_out>([&](unsigned j) __aie_inline {
                    ret.insert(j, accum_base<Class, Bits, elems_per_subaccum>(data[idx * num_subaccums_out + j]));
                });

                return ret;
            }
        }
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow_extract(unsigned idx) const
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
        constexpr unsigned output_bits = value_bits() * ElemsOut;

        static_assert(output_bits <= bits() && utils::is_powerof2(output_bits));

        if constexpr (output_bits == bits())
            *this;
        else
            return split_helper<ElemsOut>(std::make_index_sequence<Elems / ElemsOut>{});
    }

    template <unsigned ElemsIn, unsigned Bits2>
    __aie_inline
    accum_base &insert(unsigned idx, const accum_base<Class, Bits2, ElemsIn> &acc)
    {
        static_assert(ElemsIn <= Elems);
        static_assert(to_native_accum_bits<Class, Bits2>() == Bits);

        constexpr unsigned in_num_subaccums = utils::num_elems_v<accum_storage_t<Class, Bits, ElemsIn>>;
        constexpr unsigned    num_subaccums = utils::num_elems_v<accum_storage_t<Class, Bits, Elems>>;

        static_assert(in_num_subaccums <= num_subaccums);

        if constexpr (ElemsIn == Elems) {
            data = acc.data;

            return *this;
        }
        else {
            constexpr unsigned in_elems_per_subaccum = ElemsIn / in_num_subaccums;
            constexpr unsigned    elems_per_subaccum = Elems  / num_subaccums;
            static_assert(in_elems_per_subaccum <= elems_per_subaccum);

            auto insert_op = [&](auto acc, unsigned idx, auto in_acc) __aie_inline {
                // Handle special case for 128b float accums that are backed by 256b native accums
                constexpr bool fpacc_128b = (Class == AccumClass::FP  && ElemsIn == 4)
#if __AIE_API_COMPLEX_FP32_EMULATION__
                                            || (Class == AccumClass::CFP && ElemsIn == 2)
#endif
                                            ;

                if constexpr (fpacc_128b) {
#if __AIE_API_COMPLEX_FP32_EMULATION__
                    using vector_elem_type = std::conditional_t<Class == AccumClass::CFP, cfloat, float>;
#else
                    using vector_elem_type = float;
#endif
                    accum_base<Class, Bits, 2 * ElemsIn> tmp_acc;

                    if constexpr (Class == AccumClass::FP && Elems == 8) {
                        tmp_acc = acc;
                    }
#if __AIE_API_COMPLEX_FP32_EMULATION__
                    else if constexpr (Class == AccumClass::CFP && Elems == 4) {
                        tmp_acc = acc;
                    }
#endif
                    else {
                        tmp_acc = accum_extract<2 * ElemsIn>(acc, idx / 2);
                    }

                    auto v    = tmp_acc.template cast_to<accfloat>().template to_vector<float>();
                    auto v_in = accum_base<Class, Bits, ElemsIn>(in_acc).template cast_to<accfloat>().template to_vector<float>();
                    v.insert(idx % 2, v_in);
                    tmp_acc.from_vector(v.template cast_to<vector_elem_type>());
                    if constexpr (Elems == 2 * ElemsIn) {
                        return tmp_acc;
                    } else {
                        return ::insert(acc, idx / 2, tmp_acc);
                    }
                }
                else {
                    return ::insert(acc, idx, in_acc);
                }
            };

            if constexpr (in_num_subaccums == 1) {
                if constexpr (in_elems_per_subaccum == elems_per_subaccum) {
                    data[idx] = acc.data;

                    return *this;
                }
                else if constexpr (num_subaccums == 1) {
                    data = insert_op(data, idx, acc.data);

                    return *this;
                }
                else {
                    constexpr unsigned ratio = elems_per_subaccum / in_elems_per_subaccum;
                    data[idx / ratio] = insert_op(data[idx / ratio], idx % ratio, acc.data);

                    return *this;
                }
            }
            else {
                #pragma unroll
                for (unsigned i = 0; i < in_num_subaccums; ++i)
                    data[i + in_num_subaccums * idx] = acc.data[i];

                return *this;
            }
        }
    }

    template <unsigned ElemsIn>
    __aie_inline
    accum_base &insert(unsigned idx, typename accum_base<Class, MinBits, ElemsIn>::storage_t acc)
    {
        const accum_base<Class, MinBits, ElemsIn> in = acc;

        return insert(idx, in);
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const //TODO: What order of params do we want with the shift having a default?
    {
        if constexpr (utils::is_one_of_v<T, int4, uint4>) {
            return to_vector_sign<utils::get_next_integer_type_t<T>>(v_sign, shift).pack_sign(v_sign);
        }
        else {
            constexpr auto fn = get_srs<T>();

#if 1
            constexpr unsigned native_vector_elems = native_vector_length_v<T>;
            constexpr unsigned max_elems_per_srs   = 1024 / value_bits();
            constexpr unsigned native_elems        = std::min(native_vector_elems, max_elems_per_srs);
            constexpr unsigned elems_per_srs       = std::min(native_elems, Elems);

            vector<T, Elems> ret;

            utils::unroll_times<Elems / elems_per_srs>([&](unsigned idx) __aie_inline {
                ret.insert(idx, fn(extract<elems_per_srs>(idx), shift, v_sign));
            });

            return ret;
#else
            constexpr unsigned native_vector_elems = native_vector_length_v<T>;
            constexpr unsigned num_acc_elems       = utils::num_elems_v<storage_t>;
            constexpr unsigned num_vector_elems    = utils::num_elems_v<vector_storage_t<T, Elems>>;

            vector<T, Elems> ret;

            if constexpr (num_vector_elems == 1) {
                if constexpr (num_acc_elems == 1) {
                    ret = fn(data, shift, v_sign);
                    return ret;
                }
                else if constexpr (num_acc_elems == 2) {
                    ret = ::concat(fn(data[0], shift, v_sign),
                                   fn(data[1], shift, v_sign));
                    return ret;
                }
                else if constexpr (num_acc_elems == 4) {
                    ret = ::concat(fn(data[0], shift, v_sign),
                                   fn(data[1], shift, v_sign),
                                   fn(data[2], shift, v_sign),
                                   fn(data[3], shift, v_sign));
                    return ret;
                }
            }
            else {
                utils::unroll_times<num_vector_elems>([&](unsigned idx) __aie_inline {
                    ret.insert(idx, extract<native_vector_elems>(idx).template to_vector_sign<T>(v_sign, shift));
                });

                return ret;
            }
#endif
        }
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(is_signed_v<T>, shift);
    }

    template <typename T>
    __aie_inline
    void from_vector_sign(const vector<T, Elems> &v, bool v_sign, int shift = 0)
    {
        if constexpr (utils::is_one_of_v<T, int4, uint4>) {
            // Elems maximum value is 128, so 1024b int4/uint4 vectors are not allowed
            from_vector_sign(v.unpack_sign(v_sign), v_sign, shift);
        }
        else if constexpr (utils::is_one_of_v<T, int8, uint8> && Bits > 32) {
            from_vector_sign(v.unpack_sign(v_sign), v_sign, shift);
        }
        else {
            if constexpr (utils::num_elems_v<storage_t> == 1) {
                if constexpr (utils::num_elems_v<typename vector<T, Elems>::storage_t> == 1) {
                    constexpr auto fn = get_ups<T, Elems>();

                    if constexpr (vector<T, Elems>::bits() == 128)
                        data = fn(v.template grow<Elems * 2>(), shift, v_sign);
                    else
                        data = fn(v, shift, v_sign);
                }
                else if constexpr (utils::num_elems_v<typename vector<T, Elems>::storage_t> == 2) {
                    constexpr auto fn = get_ups<T, Elems / 2>();

                    this->insert<Elems / 2>(0, fn(v.template extract<Elems / 2>(0), shift, v_sign));
                    this->insert<Elems / 2>(1, fn(v.template extract<Elems / 2>(1), shift, v_sign));
                }
            }
            else {
                constexpr unsigned native_accum_elems  = 1024 / value_bits();
                constexpr unsigned chunks              = utils::num_elems_v<storage_t>;

                constexpr unsigned native_vector_elems = native_vector_length_v<T>;
                constexpr unsigned elems_per_srs       = std::min(native_vector_elems, native_accum_elems);

                constexpr unsigned elems_per_chunk = std::max(1u, native_accum_elems / native_vector_elems);

                constexpr auto fn = get_ups<T, elems_per_srs>();

                utils::unroll_times<chunks>([&](unsigned idx) __aie_inline {
                    if constexpr (elems_per_chunk == 1)
                        data[idx] = fn(v.template extract<elems_per_srs>(idx), shift, v_sign);
                    else if constexpr (elems_per_chunk == 2)
                        data[idx] = ::concat(fn(v.template extract<elems_per_srs>(2 * idx + 0), shift, v_sign),
                                             fn(v.template extract<elems_per_srs>(2 * idx + 1), shift, v_sign));
                    else
                        UNREACHABLE_MSG("Unreachable");
                });
            }
        }
    }

    template <typename T>
    __aie_inline
    void from_vector(const vector<T, Elems> &v, int shift = 0)
    {
        from_vector_sign(v, is_signed_v<T>, shift);
    }

    template <unsigned E2, unsigned... Es>
        requires(Elems == (E2 + (Es + ...)) && ((E2 == Es) && ...))
    __aie_inline
    void upd_all(const accum_base<Class, MinBits, E2> &subacc, const accum_base<Class, MinBits, Es> & ...subaccums)
    {
        using subaccum_t                       = accum_base<Class, MinBits, E2>;
        constexpr unsigned num_subaccums       = 1 + sizeof...(Es);
        constexpr unsigned subaccum_elems      = subaccum_t::size();
        constexpr unsigned num_chunks          = utils::num_elems_v<storage_t>;
        constexpr unsigned chunk_elems         = size() / num_chunks;
        constexpr unsigned subaccums_per_chunk = chunk_elems / subaccum_elems;

        const std::array arr = {subacc, subaccums...};
        if constexpr (num_chunks > 1 && chunk_elems > subaccum_elems) {
            auto update_chunk = [&] <size_t... Is> (unsigned start, std::index_sequence<Is...>) __aie_inline {
                return concat_helper(arr[start + Is]...);
            };

            utils::unroll_times<num_chunks>([&] (unsigned c) __aie_inline {
                insert(c, update_chunk(c * subaccums_per_chunk,
                                       std::make_index_sequence<subaccums_per_chunk>()));
            });
        }
        else if constexpr (chunk_elems < subaccum_elems) {
            constexpr unsigned chunks_per_subaccum = subaccum_elems / chunk_elems;

            utils::unroll_times<num_chunks>([&] (unsigned c) __aie_inline {
                const unsigned s = c / chunks_per_subaccum;
                const unsigned s_chunk = c % chunks_per_subaccum;
                insert(c, arr[s].template extract<chunk_elems>(s_chunk));
            });
        }
        else if constexpr (chunk_elems > 1 && chunk_elems == subaccum_elems) {
            utils::unroll_times<num_chunks>([&] (unsigned c) __aie_inline {
                insert(c, arr[c]);
            });
        }
        else if constexpr (num_subaccums == 8) {
            data = ::concat(::concat(arr[0], arr[1], arr[2], arr[3]),
                            ::concat(arr[4], arr[5], arr[6], arr[7]));
        }
        else if constexpr (num_subaccums < 8) {
            data = ::concat(subacc, subaccums...);
        }
        else {
            utils::unroll_times<num_subaccums>([&] (unsigned idx) __aie_inline {
                insert(idx, arr[idx]);
            });
        }
    }

private:
    template <typename T, unsigned Elems2>
    static constexpr auto get_ups()
    {
        if constexpr (std::is_same_v<T, bfloat16>) {
            if constexpr      (Elems2 == 8)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v8accfloat(::ups_to_v16accfloat(v), 0); };
            else if constexpr (Elems2 == 16)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::ups_to_v16accfloat(v); };
            else
#if __AIE_API_32ELEM_FLOAT_SRS_UPS__
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::ups_to_v32accfloat(v); };
#else
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::concat(::ups_to_v16accfloat(::extract_v16bfloat16(v, 0)), ::ups_to_v16accfloat(::extract_v16bfloat16(v, 1))); };
#endif
        }
        else if constexpr (std::is_same_v<T, float>) {
            if constexpr      (Elems == 4 || Elems2 == 8)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v8accfloat)(v); };
            else if constexpr (Elems2 == 16)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v16accfloat)(v); };
            else
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v32accfloat)(v); };
        }
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
        else if constexpr (std::is_same_v<T, cbfloat16>) {
            if constexpr      (Elems2 == 4)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v4caccfloat(::ups_to_v8caccfloat(v), 0); };
            else if constexpr (Elems2 == 8)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::ups_to_v8caccfloat(v); };
            else
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return ::ups_to_v16caccfloat(v); };
        }
#endif
        else if constexpr (std::is_same_v<T, cfloat>) {
            if constexpr      (Elems == 2 || Elems2 == 4)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v4caccfloat)(v);  };
            else if constexpr (Elems2 == 8)
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v8caccfloat)(v);  };
            else
                return [](const auto &v, auto shift_dummy, auto sign_dummy) __aie_inline { return (v16caccfloat)(v); };
        }
#endif
        else if constexpr (utils::is_one_of_v<T, int8, uint8>) {
            if constexpr (Elems2 == 16)
                return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v16acc32(::ups_to_v32acc32(v.template grow<32>(), shift, sign), 0); };
            else
                return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v32acc32(v, shift, sign); };
        }
        else if constexpr (utils::is_one_of_v<T, int16, uint16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems2 == 8)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v8acc32(::ups_to_v16acc32(v, shift, sign), 0); };
                else if constexpr (Elems2 == 16)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v16acc32(v, shift, sign); };
                else
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v32acc32(v, shift, sign); };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems2 == 8)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v8acc64(::ups_to_v16acc64(v, shift, sign), 0); };
                else
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v16acc64(v, shift, sign); };
            }
        }
        else if constexpr (utils::is_one_of_v<T, int32, uint32>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems2 == 4)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v4acc64(::ups_to_v8acc64(v, shift, sign), 0); };
                else if constexpr (Elems2 == 8)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v8acc64(v, shift, sign); };
                else
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v16acc64(v, shift, sign); };
            }
            else if constexpr (Bits == 32) {
                // TODO: should we add support for shift != 0 in this scenario?
                if      constexpr (Elems2 == 8)
                    return [](const auto &v, int shift, bool sign) __aie_inline { REQUIRES(shift == 0); return (v8acc32)v; };
                else //if constexpr (Elems2 == 16)
                    return [](const auto &v, int shift, bool sign) __aie_inline { REQUIRES(shift == 0); return (v16acc32)v; };
            }
        }
        else if constexpr (utils::is_one_of_v<T, cint16>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems2 == 4)
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v4cacc64(::ups_to_v8cacc64(v, shift, sign), 0); };
                else
                    return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v8cacc64(v, shift, sign);  };
            }
        }
        else if constexpr (utils::is_one_of_v<T, cint32>) {
            if constexpr (Elems2 == 2)
                return [](const auto &v, int shift, bool sign) __aie_inline { return ::extract_v2cacc64(::ups_to_v4cacc64(v, shift, sign), 0); };
            else if constexpr (Elems2 == 4)
                return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v4cacc64(v, shift, sign);  };
            else
                return [](const auto &v, int shift, bool sign) __aie_inline { return ::ups_to_v8cacc64(v, shift, sign);  };
        }
    }

    template <typename T>
    static constexpr auto get_srs()
    {
        if constexpr (std::is_same_v<T, bfloat16>) {
            if constexpr (Elems == 8)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v8bfloat16(::to_v16bfloat16(::set_v16accfloat(0, acc)), 0); };
            else if constexpr (Elems == 16)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::to_v16bfloat16(acc); };
            else
#if __AIE_API_32ELEM_FLOAT_SRS_UPS__
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::to_v32bfloat16(acc); };
#else
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::concat(::to_v16bfloat16(::extract_v16accfloat(acc, 0)), ::to_v16bfloat16(::extract_v16accfloat(acc, 1))); };
#endif
        }
        else if constexpr (std::is_same_v<T, float>) {
            if constexpr (Elems == 4)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v4float((v8float) acc, 0); };
            else if constexpr (Elems == 8)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return (v8float) acc;  };
            else
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return (v16float) acc; };
        }
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
        else if constexpr (std::is_same_v<T, cbfloat16>) {
            if constexpr (Elems == 4)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v4cbfloat16(::to_v8cbfloat16(::set_v8caccfloat(0, acc)), 0); };
            else if constexpr (Elems == 8)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::to_v8cbfloat16(acc); };
            else
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::to_v16cbfloat16(acc); };
        }
#endif
        else if constexpr (std::is_same_v<T, cfloat>) {
            if constexpr (Elems == 2)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return ::extract_v2cfloat((v4cfloat) acc, 0); };
            else if constexpr (Elems == 4)
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return (v4cfloat) acc;  };
            else
                return [](const auto &acc, auto shift_dummy, auto sign_dummy) __aie_inline { return (v8cfloat) acc;  };
        }
#endif
        else if constexpr (std::is_same_v<T, int8>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 16)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<16>(::srs_to_v32int8(::set_v32acc32(0, acc), shift, sign), 0); };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v32int8(acc, shift, sign); };
            }
            else if constexpr (Bits == 64) {
                return [](const auto &acc, int shift, bool sign) __aie_inline { return ::extract_v16int8(::pack(::set_v32int16(0,::srs_to_v16int16(acc, shift, sign)), sign), 0); };
            }
        }
        else if constexpr (std::is_same_v<T, uint8>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 16)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<16>(::srs_to_v32uint8(::set_v32acc32(0, acc), shift, sign), 0); };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v32uint8(acc, shift, sign);  };
            }
            else if constexpr (Bits == 64) {
                return [](const auto &acc, int shift, bool sign) __aie_inline { return ::extract_v16uint8(::pack(::set_v32uint16(0,::srs_to_v16uint16(acc, shift, sign)), sign), 0); };
            }
        }
        else if constexpr (std::is_same_v<T, int16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<8>(::srs_to_v16int16(::set_v16acc32(0, acc), shift, sign), 0); };
                else if constexpr (Elems == 16)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16int16(acc, shift, sign);  };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v32int16(acc, shift, sign);  };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<8>(::srs_to_v16int16(::set_v16acc64(0, acc), shift, sign), 0); };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16int16(acc, shift, sign); };
            }
        }
        else if constexpr (std::is_same_v<T, uint16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<8>(::srs_to_v16uint16(::set_v16acc32(0, acc), shift, sign), 0); };
                else if constexpr (Elems == 16)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16uint16(acc, shift, sign);  };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v32uint16(acc, shift, sign);  };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<8>(::srs_to_v16uint16(::set_v16acc64(0, acc), shift, sign), 0); };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16uint16(acc, shift, sign); };
            }
        }
        else if constexpr (std::is_same_v<T, int32>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return (v8int32) acc; };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return (v16int32) acc; };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems == 4)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<4>(::srs_to_v8int32(::set_v8acc64(0, acc), shift, sign), 0);  };
                else if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v8int32(acc, shift, sign);  };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16int32(acc, shift, sign);  };
            }
        }
        else if constexpr (std::is_same_v<T, uint32>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return (v8uint32) acc; };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return (v16uint32) acc; };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems == 4)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<4>(::srs_to_v8uint32(::set_v8acc64(0, acc), shift, sign), 0);  };
                else if constexpr (Elems == 8)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v8uint32(acc, shift, sign);  };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v16uint32(acc, shift, sign);  };
            }
        }
        else if constexpr (std::is_same_v<T, cint16>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems == 4)
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<4>(::srs_to_v8cint16(::set_v8cacc64(0, acc), shift, sign), 0); };
                else
                    return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v8cint16(acc, shift, sign); };
            }
        }
        else if constexpr (std::is_same_v<T, cint32>) {
            if constexpr (Elems == 2)
                return [](const auto &acc, int shift, bool sign) __aie_inline { return vector_extract<2>(::srs_to_v4cint32(::set_v4cacc64(0, acc), shift, sign), 0);  };
            else if constexpr (Elems == 4)
                return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v4cint32(acc, shift, sign);  };
            else
                return [](const auto &acc, int shift, bool sign) __aie_inline { return ::srs_to_v8cint32(acc, shift, sign);  };
        }
    }

    template <unsigned ElemsOut, std::size_t... Indices>
    __aie_inline
    std::array<accum_base<Class, MinBits, ElemsOut>, Elems/ElemsOut> split_helper(std::index_sequence<Indices...> &&) const
    {
        return {extract<ElemsOut>(Indices)...};
    }

    static constexpr unsigned native_elems  = 1024 / value_bits();

#ifdef AIE_API_EMULATION
    // TODO: floating point
    std::array<uint64_t, Elems> data;
#else
    storage_t data;
#endif
};

template <AccumClass Class, unsigned MinBits, unsigned... Es>
__aie_inline
inline accum_base<Class, MinBits, (Es + ...)> concat_helper(const accum_base<Class, MinBits, Es>&... inputs)
{
    accum_base<Class, MinBits, (Es + ...)> result;
    result.upd_all(inputs...);
    return result;
}

} // namespace aie::detail

#endif
