// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE1_ACCUM__HPP__

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
        static_assert(MinBits <= 80);

        if constexpr (MinBits <= 48)
            return 48;
        else if constexpr (MinBits <= 80)
            return 80;
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
  if constexpr (accum_class_for_tag_v<AccumTag> == AccumClass::CFP || accum_class_for_tag_v<AccumTag> == AccumClass::CInt)
      return 2 * to_native_accum_bits<AccumTag>();
  else
      return to_native_accum_bits<AccumTag>();
}

template <AccumClass Class, unsigned Bits, unsigned DstElems, typename T>
static __aie_inline accum_storage_t<Class, Bits, DstElems> accum_cast_helper(T &&from)
{
    if constexpr (Class == AccumClass::Int) {
        if constexpr (Bits == 48 && DstElems == 8)   return as_v8acc48(from);
        if constexpr (Bits == 48 && DstElems == 16)  return as_v16acc48(from);
        if constexpr (Bits == 48 && DstElems == 32)  return { as_v16acc48(from[0]), as_v16acc48(from[1]) };
        if constexpr (Bits == 48 && DstElems == 64)  return { as_v16acc48(from[0]), as_v16acc48(from[1]), as_v16acc48(from[2]), as_v16acc48(from[3]) };
        if constexpr (Bits == 48 && DstElems == 128) return { as_v16acc48(from[0]), as_v16acc48(from[1]), as_v16acc48(from[2]), as_v16acc48(from[3]),
                                                              as_v16acc48(from[4]), as_v16acc48(from[5]), as_v16acc48(from[6]), as_v16acc48(from[7]) };

        if constexpr (Bits == 80 && DstElems == 4)   return as_v4acc80(from);
        if constexpr (Bits == 80 && DstElems == 8)   return as_v8acc80(from);
        if constexpr (Bits == 80 && DstElems == 16)  return { as_v8acc80(from[0]), as_v8acc80(from[1]) };
        if constexpr (Bits == 80 && DstElems == 32)  return { as_v8acc80(from[0]), as_v8acc80(from[1]), as_v8acc80(from[2]), as_v8acc80(from[3]) };
        if constexpr (Bits == 80 && DstElems == 64)  return { as_v8acc80(from[0]), as_v8acc80(from[1]), as_v8acc80(from[2]), as_v8acc80(from[3]),
                                                              as_v8acc80(from[4]), as_v8acc80(from[5]), as_v8acc80(from[6]), as_v8acc80(from[7]) };
    }
    else if constexpr (Class == AccumClass::CInt) {
        if constexpr (Bits == 48 && DstElems == 4)   return as_v4cacc48(from);
        if constexpr (Bits == 48 && DstElems == 8)   return as_v8cacc48(from);
        if constexpr (Bits == 48 && DstElems == 16)  return { as_v8cacc48(from[0]), as_v8cacc48(from[1]) };
        if constexpr (Bits == 48 && DstElems == 32)  return { as_v8cacc48(from[0]), as_v8cacc48(from[1]), as_v8cacc48(from[2]), as_v8cacc48(from[3]) };
        if constexpr (Bits == 48 && DstElems == 64)  return { as_v8cacc48(from[0]), as_v8cacc48(from[1]), as_v8cacc48(from[2]), as_v8cacc48(from[3]),
                                                              as_v8cacc48(from[4]), as_v8cacc48(from[5]), as_v8cacc48(from[6]), as_v8cacc48(from[7]) };

        if constexpr (Bits == 80 && DstElems == 2)   return as_v2cacc80(from);
        if constexpr (Bits == 80 && DstElems == 4)   return as_v4cacc80(from);
        if constexpr (Bits == 80 && DstElems == 8)   return { as_v4cacc80(from[0]), as_v4cacc80(from[1]) };
        if constexpr (Bits == 80 && DstElems == 16)  return { as_v4cacc80(from[0]), as_v4cacc80(from[1]), as_v4cacc80(from[2]), as_v4cacc80(from[3]) };
        if constexpr (Bits == 80 && DstElems == 32)  return { as_v4cacc80(from[0]), as_v4cacc80(from[1]), as_v4cacc80(from[2]), as_v4cacc80(from[3]),
                                                              as_v4cacc80(from[4]), as_v4cacc80(from[5]), as_v4cacc80(from[6]), as_v4cacc80(from[7]) };
    }
}

//Helper function to align accumulator sizes to multiples of 40b so they are easily comparable regardless of containing 48b/80b elements or being complex
template <unsigned Size>
static constexpr unsigned get_size_aligned_helper()
{
    if      constexpr(Size == 48)  return 40;
    else if constexpr(Size == 80)  return 80;
    else if constexpr(Size == 96)  return 80;
    else if constexpr(Size == 160) return 160;
}

//Helper function to get number of elements in accumulator resulting from a cast from starting elements and element size, for destination element size.
template <unsigned StartSize, unsigned DstSize>
static constexpr unsigned accum_cast_elems_helper(unsigned Elems)
{
    constexpr unsigned align_DstSize   = get_size_aligned_helper<DstSize>();
    constexpr unsigned align_StartSize = get_size_aligned_helper<StartSize>();

    if constexpr (align_DstSize <= align_StartSize) {
       return Elems * (align_StartSize / align_DstSize);
    }
    else {
       return Elems / (align_DstSize   / align_StartSize);
    }
}

template <AccumClass Class, unsigned MinBits, unsigned Elems>
class accum_base
{
    static constexpr unsigned Bits = to_native_accum_bits<Class, MinBits>();

    template <AccumClass C2, unsigned M2, unsigned E2> friend class accum_base;

    template <AccumElemBaseType Tag, unsigned E2> friend class accum;

protected:
    using storage_type = accum_storage<Class, Bits, Elems>;
    using storage_t    = accum_storage_t<Class, Bits, Elems>;

public:
    using value_type = accum_tag_t<Class, MinBits>;

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
        return (Class == AccumClass::CInt || Class == AccumClass::CFP);
    }

    static constexpr bool is_real()
    {
        return !is_complex();
    }

    static constexpr bool is_floating_point()
    {
        return (Class == AccumClass::FP || Class == AccumClass::CFP);
    }

    template <typename DstTag>
    __aie_inline
    auto cast_to() const
    {
        constexpr unsigned DstSize    = to_native_value_bits<DstTag>();
        constexpr AccumClass DstClass = accum_class_for_tag_v<DstTag>;
        constexpr unsigned StartSize  = value_bits();

        constexpr unsigned DstElems = accum_cast_elems_helper<StartSize, DstSize>(Elems);

        constexpr unsigned RealDstSize = to_native_accum_bits<DstTag>();

        accum_base<DstClass, RealDstSize, DstElems> ret(accum_cast_helper<DstClass, RealDstSize, DstElems>(data));
        return ret;
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

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow() const
    {
        static_assert(ElemsOut >= size());

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        constexpr unsigned num_accums_out = utils::num_elems_v<storage_out_t>;

        if constexpr (ElemsOut == size())
            return *this;

        accum_base<Class, MinBits, ElemsOut> ret;

        if constexpr (Class == AccumClass::FP) {
            const vector<float, Elems> tmp = data;

            ret.data = tmp.template grow<ElemsOut>();

            return ret;
        }
        else if constexpr (Class == AccumClass::CFP) {
            const vector<cfloat, Elems> tmp = data;

            ret.data = tmp.template grow<ElemsOut>();

            return ret;
        }
        else if constexpr (num_accums_out == 1) {
            if constexpr (ElemsOut / Elems == 2) {
                ret.data = ::upd_lo(ret.data, data);

                return ret;
            }
        }
        else if constexpr (num_accums_out == 2) {
            if constexpr (ElemsOut / Elems == 2) {
                ret.data[0] = data;

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 4) {
                ret.data[0] = ::upd_lo(data[0], data);

                return ret;
            }
        }
        else if constexpr (num_accums_out == 4) {
            if constexpr (ElemsOut / Elems == 2) {
                ret.data[0] = data[0];
                ret.data[1] = data[1];

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 4) {
                ret.data[0] = data[0];

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 8) {
                ret.data[0] = ::upd_lo(data[0], data);

                return ret;
            }
        }
        else if constexpr (num_accums_out == 8) {
            if constexpr (ElemsOut / Elems == 2) {
                ret.data[0] = data[0];
                ret.data[1] = data[1];
                ret.data[2] = data[2];
                ret.data[3] = data[3];

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 4) {
                ret.data[0] = data[0];
                ret.data[1] = data[1];

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 8) {
                ret.data[0] = data[0];

                return ret;
            }
            else if constexpr (ElemsOut / Elems == 16) {
                ret.data[0] = ::upd_lo(data[0], data);

                return ret;
            }
        }
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow_replicate() const
    {
        static_assert(ElemsOut >= size());

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        constexpr unsigned num_subaccums     = utils::num_elems_v<storage_t>;
        constexpr unsigned num_subaccums_out = utils::num_elems_v<storage_out_t>;
        constexpr unsigned growth_ratio      = ElemsOut / Elems;

        if constexpr (growth_ratio == 1) {
            return *this;
        }
        else if constexpr (num_subaccums == 1) {
            accum_base<Class, MinBits, ElemsOut> res;

            constexpr unsigned native_elems = native_accum_length_v<Class, MinBits>;

            if constexpr (Elems == native_elems) {
                utils::unroll_times<growth_ratio>([&](unsigned idx) __aie_inline {
                    res.data[idx] = data;
                });
            }
            else {
                constexpr unsigned tmp_elems         = std::min(ElemsOut, native_elems);
                constexpr unsigned native_ratio      = tmp_elems / Elems;
                using tmp_t                          = accum_base<Class, MinBits, tmp_elems>;

                tmp_t tmp;

                if      constexpr (native_ratio == 2) tmp.data = ::concat(data, data);
                else if constexpr (native_ratio == 4) tmp.data = ::concat(data, data, data, data);

                if constexpr (num_subaccums_out == 1) {
                    res = tmp;
                }
                else {
                    utils::unroll_times<growth_ratio / native_ratio>([&](unsigned idx) __aie_inline {
                        res.data[idx] = tmp.data;
                    });
                }
            }

            return res;
        }
        else if constexpr (num_subaccums > 1) {
            accum_base<Class, MinBits, Elems * growth_ratio> res;

            utils::unroll_times_2d<growth_ratio, num_subaccums>([&](unsigned idx, unsigned idy) __aie_inline {
                res.data[idx * num_subaccums + idy] = data[idy];
            });

            return res;
        }
    }

    __aie_inline
    auto to_native() const
    {
        static_assert(utils::num_elems_v<storage_t> == 1);

        return data;
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> extract(unsigned idx) const
    {
        static_assert(ElemsOut <= Elems);

        constexpr unsigned     num_subaccums = utils::num_elems_v<accum_storage_t<Class, Bits, Elems>>;
        constexpr unsigned out_num_subaccums = utils::num_elems_v<accum_storage_t<Class, Bits, ElemsOut>>;

        static_assert(out_num_subaccums <= num_subaccums);

        if constexpr (ElemsOut == Elems) {
            return *this;
        }
        else if constexpr (Class == AccumClass::FP) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            // TODO: CRVO-3213
            vector<float, Elems> tmp;

            if constexpr (vector<float, Elems>::bits() == 1024) {
                tmp.template insert<Elems / 2>(0, data[0]);
                tmp.template insert<Elems / 2>(1, data[1]);
            }
            else {
                tmp.template insert<Elems>(0, data);
            }
#else
            const vector<float, Elems> tmp(data);
#endif
            accum_base<Class, MinBits, ElemsOut> ret;

            ret.data = tmp.template extract<ElemsOut>(idx);

            return ret;
        }
        else if constexpr (Class == AccumClass::CFP) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            // TODO: CRVO-3213
            vector<cfloat, Elems> tmp;

            if constexpr (vector<cfloat, Elems>::bits() == 1024) {
                tmp.template insert<Elems / 2>(0, data[0]);
                tmp.template insert<Elems / 2>(1, data[1]);
            }
            else {
                tmp.template insert<Elems>(0, data);
            }
#else
            const vector<cfloat, Elems> tmp(data);
#endif
            accum_base<Class, MinBits, ElemsOut> ret;

            ret.data = tmp.template extract<ElemsOut>(idx);

            return ret;
        }
        else {
            constexpr unsigned     elems_per_subaccum = Elems  / num_subaccums;
            constexpr unsigned out_elems_per_subaccum = ElemsOut / out_num_subaccums;
            static_assert(out_elems_per_subaccum <= elems_per_subaccum);

            accum_base<Class, MinBits, ElemsOut> ret;

            if constexpr (out_num_subaccums == 1) {
                if constexpr (elems_per_subaccum == out_elems_per_subaccum) {
                    ret.data = data[idx];

                    return ret;
                }
                else if constexpr (num_subaccums == 1) {
                    if constexpr (elems_per_subaccum / out_elems_per_subaccum == 2) {
                        if (idx % 2 == 0)
                            ret.data = ::ext_lo(data);
                        else
                            ret.data = ::ext_hi(data);

                        return ret;
                    }
                }
                else {
                    if (idx % 2 == 0)
                        ret.data = ::ext_lo(data[idx / 2]);
                    else
                        ret.data = ::ext_hi(data[idx / 2]);

                    return ret;
                }
            }
            else {
                #pragma unroll
                for (unsigned i = 0; i < out_num_subaccums; ++i)
                    ret.data[i] = data[i + out_num_subaccums * idx];

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
        else if constexpr (Class == AccumClass::FP) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            // TODO: CRVO-3213
            vector<float, Elems> tmp;

            if constexpr (vector<float, Elems>::bits() == 1024) {
                tmp.template insert<Elems / 2>(0, data[0]);
                tmp.template insert<Elems / 2>(1, data[1]);
            }
            else {
                tmp.template insert<Elems>(0, data);
            }
#else
            vector<float, Elems> tmp(data);
#endif
            const vector<float, ElemsIn> tmp_in = acc.data;

            const accum_base acc_tmp(tmp.template insert(idx, tmp_in));

            data = acc_tmp.data;

            return *this;
        }
        else if constexpr (Class == AccumClass::CFP) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            // TODO: CRVO-3213
            vector<cfloat, Elems> tmp;

            if constexpr (vector<cfloat, Elems>::bits() == 1024) {
                tmp.template insert<Elems / 2>(0, data[0]);
                tmp.template insert<Elems / 2>(1, data[1]);
            }
            else {
                tmp.template insert<Elems>(0, data);
            }
#else
            vector<cfloat, Elems> tmp(data);
#endif
            const vector<cfloat, ElemsIn> tmp_in = acc.data;

            const accum_base acc_tmp(tmp.template insert(idx, tmp_in));

            data = acc_tmp.data;

            return *this;
        }
        else {
            constexpr unsigned in_elems_per_subaccum = ElemsIn / in_num_subaccums;
            constexpr unsigned    elems_per_subaccum = Elems  / num_subaccums;
            static_assert(in_elems_per_subaccum <= elems_per_subaccum);

            if constexpr (in_num_subaccums == 1) {
                if constexpr (in_elems_per_subaccum == elems_per_subaccum) {
                    data[idx] = acc.data;

                    return *this;
                }
                else if constexpr (num_subaccums == 1) {
                    if constexpr (elems_per_subaccum / in_elems_per_subaccum == 2) {
                        if (idx % 2 == 0)
                            data = ::upd_lo(data, acc.data);
                        else
                            data = ::upd_hi(data, acc.data);

                        return *this;
                    }
                }
                else {
                    if (idx % 2 == 0)
                        data[idx / 2] = ::upd_lo(data[idx / 2], acc.data);
                    else
                        data[idx / 2] = ::upd_hi(data[idx / 2], acc.data);

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
    vector<T, Elems> to_vector(int shift = 0) const
    {
        vector<T, Elems> ret;

        if constexpr (utils::is_one_of_v<T, float, cfloat>) {
#ifdef __AIE_API_USE_NATIVE_1024B_VECTOR__
            if constexpr (vector<T, Elems>::bits() == 1024)
                ret = ::concat(data[0], data[1]);
            else
                ret = data;
#else
            ret = data;
#endif
        }
        else if constexpr(utils::is_one_of_v<T, int8, uint8> and Bits == 80) {

            constexpr auto fn = get_srs<T>();
            if constexpr (utils::num_elems_v<storage_t> == 2) {
                ret                             = fn(::concat(::ups(::srs(extract<Elems / 2>(0), shift), 0),
                                                              ::ups(::srs(extract<Elems / 2>(1), shift), 0)), 0);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 4) {
                using sub_vector = vector<T, Elems / 2>;
                const sub_vector tmp1 = fn(::concat(::ups(::srs(extract<Elems / 4>(0), shift), 0),
                                                    ::ups(::srs(extract<Elems / 4>(1), shift), 0)), 0);
                ret.insert(0, tmp1);
                const sub_vector tmp2 = fn(::concat(::ups(::srs(extract<Elems / 4>(2), shift), 0),
                                                    ::ups(::srs(extract<Elems / 4>(3), shift), 0)), 0);
                ret.insert(1, tmp2);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 8) {
                using sub_vector = vector<T, Elems / 4>;
                const sub_vector tmp1 = fn(::concat(::ups(::srs(extract<Elems / 8>(0), shift), 0),
                                                    ::ups(::srs(extract<Elems / 8>(1), shift), 0)), 0);
                ret.insert(0, tmp1);
                const sub_vector tmp2 = fn(::concat(::ups(::srs(extract<Elems / 8>(2), shift), 0),
                                                    ::ups(::srs(extract<Elems / 8>(3), shift), 0)), 0);
                ret.insert(1, tmp2);
                const sub_vector tmp3 = fn(::concat(::ups(::srs(extract<Elems / 8>(4), shift), 0),
                                                    ::ups(::srs(extract<Elems / 8>(5), shift), 0)), 0);
                ret.insert(2, tmp3);
                const sub_vector tmp4 = fn(::concat(::ups(::srs(extract<Elems / 8>(6), shift), 0),
                                                    ::ups(::srs(extract<Elems / 8>(7), shift), 0)), 0);
                ret.insert(3, tmp4);
            }
        }
        else {
            constexpr auto fn = get_srs<T>();

            if constexpr (utils::num_elems_v<storage_t> == 1) {
                ret = fn(data, shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 2) {
                using sub_vector = vector<T, Elems / 2>;
                const sub_vector tmp1 = fn(extract<Elems / 2>(0), shift); ret.insert(0, tmp1);
                const sub_vector tmp2 = fn(extract<Elems / 2>(1), shift); ret.insert(1, tmp2);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 4) {
                using sub_vector = vector<T, Elems / 4>;
                const sub_vector tmp1 = fn(extract<Elems / 4>(0), shift); ret.insert(0, tmp1);
                const sub_vector tmp2 = fn(extract<Elems / 4>(1), shift); ret.insert(1, tmp2);
                const sub_vector tmp3 = fn(extract<Elems / 4>(2), shift); ret.insert(2, tmp3);
                const sub_vector tmp4 = fn(extract<Elems / 4>(3), shift); ret.insert(3, tmp4);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 8) {
                using sub_vector = vector<T, Elems / 8>;
                const sub_vector tmp1 = fn(extract<Elems / 8>(0), shift); ret.insert(0, tmp1);
                const sub_vector tmp2 = fn(extract<Elems / 8>(1), shift); ret.insert(1, tmp2);
                const sub_vector tmp3 = fn(extract<Elems / 8>(2), shift); ret.insert(2, tmp3);
                const sub_vector tmp4 = fn(extract<Elems / 8>(3), shift); ret.insert(3, tmp4);
                const sub_vector tmp5 = fn(extract<Elems / 8>(4), shift); ret.insert(4, tmp5);
                const sub_vector tmp6 = fn(extract<Elems / 8>(5), shift); ret.insert(5, tmp6);
                const sub_vector tmp7 = fn(extract<Elems / 8>(6), shift); ret.insert(6, tmp7);
                const sub_vector tmp8 = fn(extract<Elems / 8>(7), shift); ret.insert(7, tmp8);
            }
        }

        return ret;
    }

    /* Note: Not currently functional on this architecture */
    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return to_vector<T>(shift);
    }

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_zip(int shift = 0) const
    {
        vector<T, Elems> ret;

        static_assert(!utils::is_one_of_v<T, float, cfloat>);

        constexpr auto fn = get_srs_ilv<T>();

        if constexpr (utils::num_elems_v<storage_t> == 1) {
            ret = fn(data, shift);
        }
        else if constexpr (utils::num_elems_v<storage_t> == 2) {
            const vector<T, Elems / 2> tmp1 = fn(::concat(extract<Elems / 4>(0), extract<Elems / 4>(2)), shift); ret.insert(0, tmp1);
            const vector<T, Elems / 2> tmp2 = fn(::concat(extract<Elems / 4>(1), extract<Elems / 4>(3)), shift); ret.insert(1, tmp2);
        }
        else if constexpr (utils::num_elems_v<storage_t> == 4) {
            const vector<T, Elems / 4> tmp1 = fn(::concat(extract<Elems / 8>(0), extract<Elems / 8>(4)), shift); ret.insert(0, tmp1);
            const vector<T, Elems / 4> tmp2 = fn(::concat(extract<Elems / 8>(1), extract<Elems / 8>(5)), shift); ret.insert(1, tmp2);
            const vector<T, Elems / 4> tmp3 = fn(::concat(extract<Elems / 8>(2), extract<Elems / 8>(6)), shift); ret.insert(2, tmp3);
            const vector<T, Elems / 4> tmp4 = fn(::concat(extract<Elems / 8>(3), extract<Elems / 8>(7)), shift); ret.insert(3, tmp4);
        }
        else if constexpr (utils::num_elems_v<storage_t> == 8) {
            const vector<T, Elems / 8> tmp1 = fn(::concat(extract<Elems / 16>(0), extract<Elems / 16>(8)),  shift); ret.insert(0, tmp1);
            const vector<T, Elems / 8> tmp2 = fn(::concat(extract<Elems / 16>(1), extract<Elems / 16>(9)),  shift); ret.insert(1, tmp2);
            const vector<T, Elems / 8> tmp3 = fn(::concat(extract<Elems / 16>(2), extract<Elems / 16>(10)), shift); ret.insert(2, tmp3);
            const vector<T, Elems / 8> tmp4 = fn(::concat(extract<Elems / 16>(3), extract<Elems / 16>(11)), shift); ret.insert(3, tmp4);
            const vector<T, Elems / 8> tmp5 = fn(::concat(extract<Elems / 16>(4), extract<Elems / 16>(12)), shift); ret.insert(4, tmp5);
            const vector<T, Elems / 8> tmp6 = fn(::concat(extract<Elems / 16>(5), extract<Elems / 16>(13)), shift); ret.insert(5, tmp6);
            const vector<T, Elems / 8> tmp7 = fn(::concat(extract<Elems / 16>(6), extract<Elems / 16>(14)), shift); ret.insert(6, tmp7);
            const vector<T, Elems / 8> tmp8 = fn(::concat(extract<Elems / 16>(7), extract<Elems / 16>(15)), shift); ret.insert(7, tmp8);
        }

        return ret;
    }

    __aie_inline
    auto to_vector() const requires(Class == AccumClass::FP || Class == AccumClass::CFP)
    {
        if constexpr (Class == AccumClass::FP)
            return to_vector<float>();
        else if constexpr (Class == AccumClass::CFP)
            return to_vector<cfloat>();
    }

    template <typename T>
    __aie_inline
    void from_vector(const vector<T, Elems> &v, int shift = 0)
    {
        constexpr unsigned subaccum_elems = Elems / utils::num_elems_v<storage_t>;

        if constexpr (utils::is_one_of_v<T, int8, uint8> && Bits==80) {
            if constexpr (utils::num_elems_v<storage_t> == 2) {
                vector<int32, 16> temp = ::lsrs(::ups(v, 0), 0);
                data[0] = ::lups(temp.template extract<8>(0), shift);
                data[1] = ::lups(temp.template extract<8>(1), shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 4) {
                vector<int32, 16> temp0 = ::lsrs(::ups(v.template extract<16>(0), 0), 0);
                data[0] = ::lups(temp0.template extract<8>(0), shift);
                data[1] = ::lups(temp0.template extract<8>(1), shift);
                vector<int32, 16> temp1 = ::lsrs(::ups(v.template extract<16>(1), 0), 0);
                data[2] = ::lups(temp1.template extract<8>(0), shift);
                data[3] = ::lups(temp1.template extract<8>(1), shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 8) {
                vector<int32, 16> temp0 = ::lsrs(::ups(v.template extract<16>(0), 0), 0);
                data[0] = ::lups(temp0.template extract<8>(0), shift);
                data[1] = ::lups(temp0.template extract<8>(1), shift);
                vector<int32, 16> temp1 = ::lsrs(::ups(v.template extract<16>(1), 0), 0);
                data[2] = ::lups(temp1.template extract<8>(0), shift);
                data[3] = ::lups(temp1.template extract<8>(1), shift);
                vector<int32, 16> temp2 = ::lsrs(::ups(v.template extract<16>(2), 0), 0);
                data[4] = ::lups(temp2.template extract<8>(0), shift);
                data[5] = ::lups(temp2.template extract<8>(1), shift);
                vector<int32, 16> temp3 = ::lsrs(::ups(v.template extract<16>(3), 0), 0);
                data[6] = ::lups(temp3.template extract<8>(0), shift);
                data[7] = ::lups(temp3.template extract<8>(1), shift);
            }
        }
        else {
            constexpr auto fn = get_ups<T, subaccum_elems>();

            if constexpr (utils::num_elems_v<storage_t> == 1) {
                data = fn(v, shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 2) {
                data[0] = fn(v.template extract<Elems / 2>(0), shift);
                data[1] = fn(v.template extract<Elems / 2>(1), shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 4) {
                data[0] = fn(v.template extract<Elems / 4>(0), shift);
                data[1] = fn(v.template extract<Elems / 4>(1), shift);
                data[2] = fn(v.template extract<Elems / 4>(2), shift);
                data[3] = fn(v.template extract<Elems / 4>(3), shift);
            }
            else if constexpr (utils::num_elems_v<storage_t> == 8) {
                data[0] = fn(v.template extract<Elems / 8>(0), shift);
                data[1] = fn(v.template extract<Elems / 8>(1), shift);
                data[2] = fn(v.template extract<Elems / 8>(2), shift);
                data[3] = fn(v.template extract<Elems / 8>(3), shift);
                data[4] = fn(v.template extract<Elems / 8>(4), shift);
                data[5] = fn(v.template extract<Elems / 8>(5), shift);
                data[6] = fn(v.template extract<Elems / 8>(6), shift);
                data[7] = fn(v.template extract<Elems / 8>(7), shift);
            }
        }
    }

    /* Note: Not currently functional on this architecture */
    template <typename T>
    __aie_inline
    void from_vector_sign(const vector<T, Elems> &v, bool v_sign, int shift = 0)
    {
        return from_vector(v, shift);
    }

    template <typename T>
    __aie_inline
    accum_base &operator=(const vector<T, Elems> &v)
    {
        from_vector(v);
        return *this;
    }

    template <unsigned E2, unsigned... Es>
        requires(Elems == (E2 + (Es + ...)) && ((Elems / E2 == Elems / Es) && ...))
    __aie_inline
    void upd_all(const accum_base<Class, MinBits, E2> &subacc, const accum_base<Class, MinBits, Es> & ...subaccums)
    {
        constexpr unsigned num_subaccums  = 1 + sizeof...(Es);
        constexpr unsigned num_storage_slots = utils::num_elems_v<storage_t>;

        if constexpr (num_storage_slots == 1) {
            data = ::concat(subacc, subaccums...);
        }
        else if constexpr (num_storage_slots == num_subaccums) {
            data = { subacc, subaccums... };
        }
        else if constexpr (num_storage_slots > num_subaccums) {
            constexpr unsigned ratio = num_storage_slots / num_subaccums;

            const std::array t = {subacc, subaccums...};
            utils::unroll_times<num_storage_slots>([&](auto i) __aie_inline {
                unsigned sub_idx = i / ratio;
                unsigned sub_chunk = i % ratio;
                data[i] = t[sub_idx].template extract<Elems / num_storage_slots>(sub_chunk);
            });
        } else { // num_storage_slots < num_subaccums
            static_assert(num_subaccums % num_storage_slots == 0);

            const std::array t = {subacc, subaccums...};
            if constexpr (num_subaccums / num_storage_slots == 2) {
                utils::unroll_times<num_storage_slots>([&](auto i) __aie_inline {
                    data[i] = ::concat(t[i * 2], t[i * 2 + 1]);
                });
            }
            else if constexpr (num_subaccums / num_storage_slots == 4) {
                utils::unroll_times<num_storage_slots>([&](auto i) __aie_inline {
                    data[i] = ::concat(t[i * 4], t[i * 4 + 1], t[i * 4 + 2], t[i * 4 + 3]);
                });
            }
            else {
                UNREACHABLE_MSG("Unhandled case\n");
            }
        }
    }

private:
    template <typename T, unsigned Elems2>
    static constexpr auto get_ups()
    {
        if constexpr (utils::is_one_of_v<T, float, cfloat>) {
            return [](const auto &acc, auto shift_dummy) __aie_inline { return acc; };
        }
        else {
            if constexpr (Bits == 48) {
                using vector_type = vector<T, Elems2>;
                if constexpr (utils::is_one_of_v<T, int32, cint32> && vector_type::bits() >= 512) {
                    return [](const auto &v, int shift) __aie_inline {
                        accum_base<Class, MinBits, Elems2> ret;

                        ret.template insert<Elems2 / 2>(0, ::ups(v.template extract<Elems2 / 2>(0), shift));
                        ret.template insert<Elems2 / 2>(1, ::ups(v.template extract<Elems2 / 2>(1), shift));

                        return ret;
                    };
                }
                else {
                    return [](const auto &v, int shift) __aie_inline { return ::ups(v, shift);  };
                }
            }
            else if constexpr (Bits == 80) {
                if constexpr (utils::is_one_of_v<T, int32, cint32>)
                    return [](const auto &v, int shift) __aie_inline { return ::lups(v, shift);  };
                else if constexpr(utils::is_one_of_v<T, int16, cint16>)
                    return [](const auto &v, int shift) __aie_inline { return ::lups(::lsrs(::ups(v, 0), 0), shift);  };
            }
        }
    }

    template <typename T>
    static constexpr auto get_srs()
    {
        if constexpr (utils::is_one_of_v<T, float, cfloat>) {
            return [](const auto &acc, auto shift_dummy) __aie_inline { return acc; };
        }
        else {
            if constexpr (std::is_same_v<T, int8>) {
                return [](const auto &acc, int shift) __aie_inline { return ::bsrs(acc, shift);  };
            }
            else if constexpr (std::is_same_v<T, uint8>) {
                return [](const auto &acc, int shift) __aie_inline { return ::ubsrs(acc, shift);  };
            }
            else if constexpr (utils::is_one_of_v<T, int16, cint16>) {
                if constexpr (Bits == 48)
                    return [](const auto &acc, int shift) __aie_inline { return ::srs(acc, shift);  };
                else if constexpr (Bits == 80) //NOTE: This could take less cycles with srs + interleave but we would lose saturation functionality
                    return [](const auto &acc, int shift) __aie_inline { return ::srs(::ups(::srs(acc, shift), 0), 0);  };
            }
            else if constexpr (utils::is_one_of_v<T, int32, cint32>) {
                if constexpr (Bits == 48)
                    return [](const auto &acc, int shift) __aie_inline { return ::lsrs(acc, shift);  };
                else if constexpr (Bits == 80)
                    return [](const auto &acc, int shift) __aie_inline { return ::srs(acc, shift);  };
            }
        }
    }

    template <typename T>
    static constexpr auto get_srs_ilv()
    {
        return [](const auto &acc, int shift) __aie_inline { return ::srs_ilv(acc, shift);  };
    }

#ifdef AIE_API_EMULATION
    // TODO: floating point
    std::array<uint64_t, Elems> data;
#else
    storage_t data;
#endif
};

}

#endif
