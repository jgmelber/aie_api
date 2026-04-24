// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_ACCUM__HPP__
#define __AIE_API_DETAIL_AIE2P_ACCUM__HPP__

#include "accum_native_types.hpp"

#include "../utils.hpp"
#include "../traits.hpp"
#include "../../vector.hpp"
#if AIE_API_ML_VERSION >= 210
#include "../../block_vector.hpp"
#endif

namespace aie::detail {

template <AccumClass Class, unsigned MinBits>
static constexpr unsigned to_native_accum_bits()
{
    if constexpr (is_floating_point_class(Class)) {
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

template <AccumClass Class, unsigned MinBits>
static constexpr unsigned to_native_value_bits()
{
    if constexpr (is_complex_class(Class))
        return 2 * to_native_accum_bits<Class, MinBits>();
    else
        return to_native_accum_bits<Class, MinBits>();
}

template <typename AccumTag>
static constexpr unsigned to_native_value_bits()
{
    return to_native_value_bits<accum_class_for_tag_v<AccumTag>, accum_bits_for_tag_v<AccumTag>>();
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
    static constexpr unsigned native_elems  = 2048 / bits_per_elem;
    static constexpr unsigned Chunks        = DstElems / native_elems;

    if constexpr (Class == AccumClass::Int) {
        if constexpr (Bits == 32 && DstElems == 8)   return v8acc32(from);
        if constexpr (Bits == 32 && DstElems == 16)  return v16acc32(from);
        if constexpr (Bits == 32 && DstElems == 32)  return v32acc32(from);
        if constexpr (Bits == 32 && DstElems == 64)  return v64acc32(from);
        if constexpr (Bits == 32 && DstElems >= 128) return utils::make_array<Chunks>([](auto f) __aie_inline { return v64acc32(f); }, from);

        if constexpr (Bits == 64 && DstElems == 4)   return v4acc64(from);
        if constexpr (Bits == 64 && DstElems == 8)   return v8acc64(from);
        if constexpr (Bits == 64 && DstElems == 16)  return v16acc64(from);
        if constexpr (Bits == 64 && DstElems == 32)  return v32acc64(from);
        if constexpr (Bits == 64 && DstElems >= 64)  return utils::make_array<Chunks>([](auto f) __aie_inline { return v32acc64(f); }, from);
    }
    else if constexpr (Class == AccumClass::FP) {
        if constexpr (DstElems == 4)                 return v8accfloat(from);
        if constexpr (DstElems == 8)                 return v8accfloat(from);
        if constexpr (DstElems == 16)                return v16accfloat(from);
        if constexpr (DstElems == 32)                return v32accfloat(from);
        if constexpr (DstElems == 64)                return v64accfloat(from);
        if constexpr (DstElems >= 128)               return utils::make_array<Chunks>([](auto f) __aie_inline { return v64accfloat(f); }, from);
    }
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
    else if constexpr (Class == AccumClass::CInt) {
        if constexpr (DstElems == 2)                 return v2cacc64(from);
        if constexpr (DstElems == 4)                 return v4cacc64(from);
        if constexpr (DstElems == 8)                 return v8cacc64(from);
        if constexpr (DstElems == 16)                return v16cacc64(from);
        if constexpr (DstElems >= 32)                return utils::make_array<Chunks>([](auto f) __aie_inline { return v16cacc64(f); }, from);
    }
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
    else if constexpr (Class == AccumClass::CFP) {
        if constexpr (DstElems == 2)                 return v4caccfloat(from);
        if constexpr (DstElems == 4)                 return v4caccfloat(from);
        if constexpr (DstElems == 8)                 return v8caccfloat(from);
        if constexpr (DstElems == 16)                return v16caccfloat(from);
        if constexpr (DstElems == 32)                return v32caccfloat(from);
        if constexpr (DstElems >= 64)                return utils::make_array<Chunks>([](auto f) __aie_inline { return v32caccfloat(f); }, from);
    }
#endif
}

template <unsigned Elems, typename T> static auto accum_extract(const T& acc, unsigned idx);

template <> inline __aie_inline auto accum_extract<64, v64acc32>(const v64acc32& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<32, v64acc32>(const v64acc32& acc, unsigned idx) { return ::extract_v32acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v64acc32>(const v64acc32& acc, unsigned idx) { return ::extract_v16acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v64acc32>(const v64acc32& acc, unsigned idx) { return ::extract_v8acc32(::extract_v16acc32(acc, idx / 2), idx % 2); }
template <> inline __aie_inline auto accum_extract<32, v32acc32>(const v32acc32& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<16, v32acc32>(const v32acc32& acc, unsigned idx) { return ::extract_v16acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32acc32>(const v32acc32& acc, unsigned idx) { return ::extract_v8acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v16acc32>(const v16acc32& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16acc32>(const v16acc32& acc, unsigned idx) { return ::extract_v8acc32(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8acc32>(const  v8acc32& acc, unsigned idx) { return acc; }

template <> inline __aie_inline auto accum_extract<32, v32acc64>(const v32acc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<16, v32acc64>(const v32acc64& acc, unsigned idx) { return ::extract_v16acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32acc64>(const v32acc64& acc, unsigned idx) { return ::extract_v8acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v32acc64>(const v32acc64& acc, unsigned idx) { return ::extract_v4acc64(::extract_v8acc64(acc, idx / 2), idx % 2); }
template <> inline __aie_inline auto accum_extract<16, v16acc64>(const v16acc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16acc64>(const v16acc64& acc, unsigned idx) { return ::extract_v8acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v16acc64>(const v16acc64& acc, unsigned idx) { return ::extract_v4acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8acc64>(const  v8acc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8acc64>(const  v8acc64& acc, unsigned idx) { return ::extract_v4acc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4acc64>(const  v4acc64& acc, unsigned idx) { return acc; }

#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <> inline __aie_inline auto accum_extract<16, v16cacc64>(const v16cacc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8 , v16cacc64>(const v16cacc64& acc, unsigned idx) { return ::extract_v8cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v16cacc64>(const v16cacc64& acc, unsigned idx) { return ::extract_v4cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<2,  v16cacc64>(const v16cacc64& acc, unsigned idx) { return ::extract_v2cacc64(::extract_v4cacc64(acc, idx / 2), idx % 2); }
template <> inline __aie_inline auto accum_extract<8,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return ::extract_v4cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<2,   v8cacc64>(const  v8cacc64& acc, unsigned idx) { return ::extract_v2cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4cacc64>(const  v4cacc64& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<2,   v4cacc64>(const  v4cacc64& acc, unsigned idx) { return ::extract_v2cacc64(acc, idx); }
template <> inline __aie_inline auto accum_extract<2,   v2cacc64>(const  v2cacc64& acc, unsigned idx) { return acc; }
#endif

template <> inline __aie_inline auto accum_extract<64, v64accfloat>(const v64accfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<32, v64accfloat>(const v64accfloat& acc, unsigned idx) { return ::extract_v32accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v64accfloat>(const v64accfloat& acc, unsigned idx) { return ::extract_v16accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v64accfloat>(const v64accfloat& acc, unsigned idx) { return ::extract_v8accfloat(::extract_v16accfloat(acc, idx / 2), idx % 2); }
template <> inline __aie_inline auto accum_extract<32, v32accfloat>(const v32accfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<16, v32accfloat>(const v32accfloat& acc, unsigned idx) { return ::extract_v16accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32accfloat>(const v32accfloat& acc, unsigned idx) { return ::extract_v8accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<16, v16accfloat>(const v16accfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16accfloat>(const v16accfloat& acc, unsigned idx) { return ::extract_v8accfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8accfloat>(const  v8accfloat& acc, unsigned idx) { return acc; }
// Special cases as accum<accfloat, 4> is backed by v8accfloat
template <> inline __aie_inline auto accum_extract<4,  v64accfloat>(const v64accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<4,  v32accfloat>(const v32accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<4,  v16accfloat>(const v16accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<4,   v8accfloat>(const  v8accfloat& acc, unsigned idx) { return accum_extract<8>(acc, idx / 2); }

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <> inline __aie_inline auto accum_extract<32, v32caccfloat>(const v32caccfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<16, v32caccfloat>(const v32caccfloat& acc, unsigned idx) { return ::extract_v16caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,  v32caccfloat>(const v32caccfloat& acc, unsigned idx) { return ::extract_v8caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v32caccfloat>(const v32caccfloat& acc, unsigned idx) { return ::extract_v4caccfloat(::extract_v8caccfloat(acc, idx / 2), idx % 2); }
template <> inline __aie_inline auto accum_extract<16, v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<8,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return ::extract_v8caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return ::extract_v4caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<8,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return acc; }
template <> inline __aie_inline auto accum_extract<4,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return ::extract_v4caccfloat(acc, idx); }
template <> inline __aie_inline auto accum_extract<4,   v4caccfloat>(const  v4caccfloat& acc, unsigned idx) { return acc; }
// Special cases as accum<caccfloat, 2> is backed by v4caccfloat
template <> inline __aie_inline auto accum_extract<2,  v32caccfloat>(const v32caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<2,  v16caccfloat>(const v16caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<2,   v8caccfloat>(const  v8caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
template <> inline __aie_inline auto accum_extract<2,   v4caccfloat>(const  v4caccfloat& acc, unsigned idx) { return accum_extract<4>(acc, idx / 2); }
#endif


template <AccumClass Class, unsigned Bits, unsigned Elems> struct accum_set {};
template <> struct accum_set<AccumClass::Int,  32, 8>  { static v8acc32      run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::Int,  32, 16> { static v16acc32     run(const auto &acc) __aie_inline { return ::set_v16acc32(0, acc);     } };
template <> struct accum_set<AccumClass::Int,  32, 32> { static v32acc32     run(const auto &acc) __aie_inline { return ::set_v32acc32(0, acc);     } };
template <> struct accum_set<AccumClass::Int,  32, 64> { static v64acc32     run(const auto &acc) __aie_inline { return ::set_v64acc32(0, acc);     } };
template <> struct accum_set<AccumClass::Int,  64,  4> { static v8acc64      run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::Int,  64,  8> { static v8acc64      run(const auto &acc) __aie_inline { return ::set_v8acc64(0, acc);      } };
template <> struct accum_set<AccumClass::Int,  64, 16> { static v16acc64     run(const auto &acc) __aie_inline { return ::set_v16acc64(0, acc);     } };
template <> struct accum_set<AccumClass::Int,  64, 32> { static v32acc64     run(const auto &acc) __aie_inline { return ::set_v32acc64(0, acc);     } };
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
template <> struct accum_set<AccumClass::CInt, 64,  2> { static v2cacc64     run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::CInt, 64,  4> { static v4cacc64     run(const auto &acc) __aie_inline { return ::set_v4cacc64(0, acc);     } };
template <> struct accum_set<AccumClass::CInt, 64,  8> { static v8cacc64     run(const auto &acc) __aie_inline { return ::set_v8cacc64(0, acc);     } };
template <> struct accum_set<AccumClass::CInt, 64, 16> { static v16cacc64    run(const auto &acc) __aie_inline { return ::set_v16cacc64(0, acc);    } };
#endif
template <> struct accum_set<AccumClass::FP,   32,  8> { static v8accfloat   run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::FP,   32, 16> { static v16accfloat  run(const auto &acc) __aie_inline { return ::set_v16accfloat(0, acc);  } };
template <> struct accum_set<AccumClass::FP,   32, 32> { static v32accfloat  run(const auto &acc) __aie_inline { return ::set_v32accfloat(0, acc);  } };
template <> struct accum_set<AccumClass::FP,   32, 64> { static v64accfloat  run(const auto &acc) __aie_inline { return ::set_v64accfloat(0, acc);  } };

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <> struct accum_set<AccumClass::CFP,  32,  2> { static v4caccfloat  run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::CFP,  32,  4> { static v4caccfloat  run(const auto &acc) __aie_inline { return acc;                        } };
template <> struct accum_set<AccumClass::CFP,  32,  8> { static v8caccfloat  run(const auto &acc) __aie_inline { return ::set_v8caccfloat(0, acc);  } };
template <> struct accum_set<AccumClass::CFP,  32, 16> { static v16caccfloat run(const auto &acc) __aie_inline { return ::set_v16caccfloat(0, acc); } };
template <> struct accum_set<AccumClass::CFP,  32, 32> { static v32caccfloat run(const auto &acc) __aie_inline { return ::set_v32caccfloat(0, acc); } };
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

    // Returned when UPS/SRS argument type is not supported (e.g. acc32 for input complex vectors)
    struct unsupported_ups {};
    struct unsupported_srs {};
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
        return is_complex_class(Class);
    }

    static constexpr bool is_real()
    {
        return !is_complex();
    }

    static constexpr bool is_floating_point()
    {
        return is_floating_point_class(Class);
    }

    __aie_inline
    accum_base() :
        data(storage_type::undef())
    {
    }

    /**
     * Copy constructor.
     */
    template <unsigned MinBits2> requires(accum_base<Class, MinBits2, Elems>::accum_bits() == accum_bits())
    __aie_inline
    accum_base(const accum_base<Class, MinBits2, Elems> &acc) :
        data(acc.data)
    {
    }

    __aie_inline
    accum_base(const storage_t &data) :
        data(data)
    {
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

    /**
     * Reinterprets the current accumulator as an accumulator of the given type. The number of elements is automatically computed
     * by the function
     *
     * @tparam DstTag Type the accumulator will be cast to
     */
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
        ret.data = accum_cast_helper<DstClass, RealDstSize, DstElems>(data);

        return ret;
    }

    template <unsigned ElemsOut>
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> grow() const
    {
        static_assert(ElemsOut >= size());

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        constexpr unsigned in_num_chunks  = utils::num_elems_v<storage_t>;
        constexpr unsigned out_num_chunks = utils::num_elems_v<storage_out_t>;
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
        else if constexpr (in_num_chunks == 1 && out_num_chunks == 1) {
            if constexpr (growth_ratio < 8) {
                ret.data = accum_set<Class, Bits, ElemsOut>::run(data);
            }
            else { //TODO: Remove if 256b in 2048b accums intrinsics are added (CRVO-7549)
                auto tmp = accum_set<Class, Bits, ElemsOut / 2>::run(data);
                ret.data = accum_set<Class, Bits, ElemsOut>::run(tmp);
            }
        }
        else if constexpr (in_num_chunks == 1 && out_num_chunks > 1) {
            // Other elements are default initialized to undef() already
            ret.data[0] = this->grow<native_elems>();
        }
        else {
            // Other elements are default initialized to undef() already
            utils::unroll_times<out_num_chunks>([&](unsigned elem) __aie_inline {
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
#if __AIE_ARCH__ == 21
                    using shuffle_type = v16acc32;
#else
                    using shuffle_type = v16int32;
#endif
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
        requires(ElemsOut <= Elems)
    __aie_inline
    accum_base<Class, MinBits, ElemsOut> extract(unsigned idx) const
    {
        if constexpr (ElemsOut == Elems) {
            return *this;
        }

        using storage_out_t = accum_storage_t<Class, Bits, ElemsOut>;

        // Handle special case for 128b float accums that are backed by 256b native accums
        constexpr bool fpacc_128b = is_floating_point() && (value_bits() * ElemsOut == 128);

        constexpr unsigned num_chunks     = utils::num_elems_v<storage_t>;
        constexpr unsigned num_chunks_out = utils::num_elems_v<storage_out_t>;

        constexpr unsigned chunk_elems     = Elems  / num_chunks;
        constexpr unsigned chunk_elems_out = ElemsOut / num_chunks_out;

        // The output accumulator is always smaller
        // Both this and output accumulators are backed by the same storage when both of them are compound
        static_assert(num_chunks_out <= num_chunks);
        static_assert(num_chunks_out > 1 ? chunk_elems_out == chunk_elems
                                         : chunk_elems_out <= chunk_elems);

        accum_base<Class, MinBits, ElemsOut> ret;

        if constexpr (num_chunks_out > 1) {
            utils::unroll_times<num_chunks_out>([&](unsigned j) __aie_inline {
                accum_base<Class, MinBits, chunk_elems> chunk = data[idx * num_chunks_out + j];
                ret.insert(j, chunk);
            });

            return ret;
        }
        // num_chunks_out == 1
        else if constexpr (num_chunks > 1) {
            constexpr unsigned result_ratio = chunk_elems / ElemsOut;
            accum_base<Class, MinBits, chunk_elems> chunk = data[idx / result_ratio];
            return chunk.template extract<ElemsOut>(idx % result_ratio);
        }
        // num_chunks_out == 1 && num_chunks == 1
        else if constexpr (chunk_elems_out == chunk_elems) {
            ret = data;
            return ret;
        }
        // chunk_elems_out < chunk_elems
        else if constexpr (fpacc_128b) {
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
        requires(ElemsIn <= Elems)
    __aie_inline
    accum_base &insert(unsigned idx, const accum_base<Class, Bits2, ElemsIn> &acc)
    {
        constexpr unsigned in_chunks = utils::num_elems_v<accum_storage_t<Class, Bits, ElemsIn>>;
        constexpr unsigned    chunks = utils::num_elems_v<accum_storage_t<Class, Bits, Elems>>;

        static_assert(to_native_accum_bits<Class, Bits2>() == Bits);

        if constexpr (ElemsIn == Elems) {
            data = acc.data;
            return *this;
        }
        else {
            constexpr unsigned in_chunk_elems = ElemsIn / in_chunks;
            constexpr unsigned    chunk_elems = Elems  / chunks;
            static_assert(in_chunk_elems <= chunk_elems);

            auto insert_op = [&]<unsigned E1, unsigned E2>(accum_base<Class, MinBits, E1> self,
                                                           unsigned idx,
                                                           const accum_base<Class, Bits2, E2> &in) __aie_inline {
                // Handle special case for 128b float accums that are backed by 256b native accums
                if constexpr (is_floating_point() && (value_bits() * ElemsIn == 128)) {
                    using vector_elem_type = std::conditional_t<is_complex(), cfloat, float>;

                    auto tmp  = self.template grow_extract<2 * ElemsIn>(idx / 2);
                    auto v    = tmp.template cast_to<accfloat>().template to_vector<float>();
                    auto v_in = acc.template cast_to<accfloat>().template to_vector<float>();

                    v.insert(idx % 2, v_in);
                    tmp = v.template cast_to<vector_elem_type>();

                    self.insert(idx / 2, tmp);
                    return self;
                }
                else
#if __AIE_ARCH__ == 22
                // Note: 256b insertion into 2k accum is now emulated inside the intrinsic (except AIE2ps)
                if constexpr (value_bits() * E1 == 2048 && value_bits() * E2 == 256) {
                    constexpr unsigned ratio = E1 / E2 / 2;
                    auto tmp = self.template extract<E1 / 2>(idx / ratio);
                    tmp = ::insert(tmp, idx % ratio, in);
                    self.insert(idx / ratio, tmp);
                    return self;
                }
                else
#endif
                {
                    self = ::insert(self, idx, in);
                    return self;
                }
            };

            if constexpr (in_chunks == 1) {
                if constexpr (chunk_elems == in_chunk_elems) {
                    data[idx] = acc.data;

                    return *this;
                }
                else if constexpr (chunks == 1) {
                    *this = insert_op(*this, idx, acc);
                    return *this;
                }
                else {
                    constexpr unsigned ratio = chunk_elems / in_chunk_elems;

                    auto chunk = extract<chunk_elems>(idx / ratio);
                    chunk = insert_op(chunk, idx % ratio, acc);
                    return insert(idx / ratio, chunk);
                }
            }
            else {
                #pragma unroll
                for (unsigned i = 0; i < in_chunks; ++i)
                    data[i + in_chunks * idx] = acc.data[i];

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

#if AIE_API_ML_VERSION >= 210
    template <typename T> requires(is_valid_block_type_v<T>)
    __aie_inline
    block_vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        constexpr auto fn = get_srs<T>();

        block_vector<T, Elems> ret;

        utils::unroll_times<Elems / 64>([&](unsigned idx) __aie_inline {
            block_vector<T, 64> tmp = fn(extract<64>(idx), shift, v_sign);
            ret.insert(idx, tmp);
        });

        return ret;
    }
#endif

    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        constexpr auto srs_fn        = get_srs<T>();
        using srs_fn_t               = std::remove_cvref_t<decltype(srs_fn)>;
        constexpr bool requires_pack = std::is_same_v<srs_fn_t, unsupported_srs>
                                       && utils::is_one_of_v<T, int4, uint4, int8, uint8>;

        if constexpr (requires_pack) {
            using promoted_type = utils::get_next_integer_type_t<T>;
            return to_vector_sign<promoted_type>(v_sign, shift).pack_sign(v_sign);
        }
        else {
            using srs_fn_t  = std::remove_cvref_t<decltype(srs_fn)>;
            using srs_acc_t = std::remove_cvref_t<invoke_arg_t<0, srs_fn_t>>;

            constexpr unsigned elems_per_srs = srs_acc_t::size();
            using chunk_vector_t = vector<T, elems_per_srs>;

            vector<T, Elems> ret;
            if constexpr (Elems <= elems_per_srs) {
                ret = chunk_vector_t(srs_fn(grow<elems_per_srs>(), shift, v_sign)).template extract<Elems>(0);
            } else {
                utils::unroll_times<Elems / elems_per_srs>([&](unsigned idx) __aie_inline {
                    ret.insert(idx, chunk_vector_t(srs_fn(extract<elems_per_srs>(idx), shift, v_sign)));
                });
            }
            return ret;
        }
    }

#if AIE_API_ML_VERSION >= 210
    template <typename T> requires(is_valid_block_type_v<T>)
    __aie_inline
    block_vector<T, Elems> to_vector(int shift = 0) const
    {
        return to_vector_sign<T>(true, shift);
    }
#endif

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
        constexpr unsigned chunks      = utils::num_elems_v<storage_t>;
        constexpr auto ups_fn          = get_ups<T, Elems>();
        using ups_fn_t                 = std::remove_cvref_t<decltype(ups_fn)>;
        constexpr bool requires_unpack = std::is_same_v<ups_fn_t, unsupported_ups>
                                         && utils::is_one_of_v<T,
                                                               int4, uint4, int8, uint8>;

        if constexpr (requires_unpack) {
            from_vector_sign(v.unpack_sign(v_sign), v_sign, shift);
        }
        else if constexpr (chunks > 1) {
            utils::unroll_times<chunks>([&](unsigned idx) __aie_inline {
                constexpr unsigned chunk_elems = Elems / chunks;
                accum_base<Class, MinBits, chunk_elems> acc;
                acc.from_vector_sign(v.template extract<chunk_elems>(idx), v_sign, shift);
                insert(idx, acc);
            });
        }
        else {
            using ups_vec_t                  = std::remove_cvref_t<invoke_arg_t<0, ups_fn_t>>;
            constexpr unsigned elems_per_ups = ups_vec_t::size();
            using ups_acc_t                  = accum_base<Class, MinBits, elems_per_ups>;

            if constexpr (Elems <= elems_per_ups) {
                ups_acc_t acc = ups_fn(v.template grow<elems_per_ups>(), shift, v_sign);
                *this = acc.template extract<Elems>(0);
            }
            else {
                std::array<ups_acc_t, Elems / elems_per_ups> chunks;
                utils::unroll_times<Elems / elems_per_ups>([&](unsigned idx) __aie_inline {
                    chunks[idx] = ups_fn(v.template extract<elems_per_ups>(idx), shift, v_sign);
                });
                auto apply_array = [this, &chunks]<unsigned... Is>(std::integer_sequence<unsigned, Is...>) __aie_inline {
                    *this = concat_helper(chunks[Is]...);
                };
                apply_array(std::make_integer_sequence<unsigned, Elems / elems_per_ups>());
            }
        }
    }

    template <typename T>
    __aie_inline
    void from_vector(const vector<T, Elems> &v, int shift = 0)
    {
        from_vector_sign(v, is_signed_v<T>, shift);
    }

#if __AIE_ARCH__ == 21
    template <typename T> requires(detail::is_valid_block_type_v<T> &&
                                   std::is_same_v<T, bfp16ebs8>) //FIXME: CRVO-9745 support bfp16ebs16
    __aie_inline
    void from_vector(const block_vector<T, Elems> &v, int shift = 0)
    {
        using native_type = native_block_vector_type_t<T, 64>;

        // MMUL with identity to convert
        block_vector<T, 64> eye;
        eye = ::insert(eye, 0, 0x01010101 * (127 + shift));
        eye = ::insert(eye, 1, 0x01010101 * (127 + shift));
        eye = ::insert(eye, ::sel(::broadcast_zero_to_v64int8(),
                                  ::broadcast_to_v64int8(0x40),
                                  0x8040201008040201ll));

        if constexpr (Elems == 64) {
            data = ::mul_8x8_8x8T((native_type)v, eye);
        }
        else {
            data[0] = ::mul_8x8_8x8T((native_type)v.template extract<64>(0), eye);
            data[1] = ::mul_8x8_8x8T((native_type)v.template extract<64>(1), eye);
        }
    }
#elif __AIE_ARCH__ == 22
    template <typename T> requires(detail::is_valid_block_type_v<T>)
    __aie_inline
    void from_vector(const block_vector<T, Elems> &v, int shift = 0)
    {
        constexpr unsigned native_elems = 64;

        if constexpr (std::is_same_v<T, mx9>) {
            block_vector<T, 256> eye;
            block_vector<T, 128> eye_tmp;

            // MMUL with identity to convert
            eye_tmp = ::insert(eye_tmp, 0, 0x01010101 * (127 + shift)); // Exponent
            eye_tmp = ::insert(eye_tmp, 1, 0x01010101 * (127 + shift)); // Exponent
            eye_tmp = ::insert(eye_tmp, 2, 0x01010101 * (127 + shift)); // Exponent
            eye_tmp = ::insert(eye_tmp, 3, 0x01010101 * (127 + shift)); // Exponent
            eye_tmp = ::insert(eye_tmp, 0, ::set_v4int8(0, 0)); // Prime
            eye_tmp = ::insert(eye_tmp, 1, ::set_v4int8(0, 0)); // Prime
            eye_tmp = ::insert(eye_tmp, 2, ::set_v4int8(0, 0)); // Prime
            eye_tmp = ::insert(eye_tmp, 3, ::set_v4int8(0, 0)); // Prime

            eye_tmp = ::insert(eye_tmp, 0, ::sel(::broadcast_zero_to_v64int8(), // Data
                                                 ::broadcast_to_v64int8(0x40),
                                                 0x0008000400020001ll));
            eye_tmp = ::insert(eye_tmp, 1, ::sel(::broadcast_zero_to_v64int8(), // Data
                                                 ::broadcast_to_v64int8(0x40),
                                                 0x0080004000200010ll));
            //TODO: Update eye_tmp definition to remove shuffles
            eye = ::insert(eye, 0, ::shuffle(::extract_v64mx9(eye_tmp, 0), ::extract_v64mx9(eye_tmp, 1), T64_8x2_lo));
            eye = ::insert(eye, 2, ::shuffle(::extract_v64mx9(eye_tmp, 0), ::extract_v64mx9(eye_tmp, 1), T64_8x2_hi));

            eye_tmp = ::insert(eye_tmp, 0, ::sel(::broadcast_zero_to_v64int8(), // Data
                                                 ::broadcast_to_v64int8(0x40),
                                                 0x0800040002000100ll));
            eye_tmp = ::insert(eye_tmp, 1, ::sel(::broadcast_zero_to_v64int8(), // Data
                                                 ::broadcast_to_v64int8(0x40),
                                                 0x8000400020001000ll));
            //TODO: Update eye_tmp definition to remove shuffles
            eye = ::insert(eye, 1, ::shuffle(::extract_v64mx9(eye_tmp, 0), ::extract_v64mx9(eye_tmp, 1), T64_8x2_lo));
            eye = ::insert(eye, 3, ::shuffle(::extract_v64mx9(eye_tmp, 0), ::extract_v64mx9(eye_tmp, 1), T64_8x2_hi));

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                insert<native_elems>(idx, ::mul_4x16_16x16T(::shuffle(v.template extract<native_elems>(idx), T64_4x2), eye));
            });
        }
        else {
            block_vector<T, 128> eye_lower;
            block_vector<T, 128> eye_upper;
            block_vector<T, 64> eye_tmp;

            eye_tmp = ::insert(eye_tmp, 0, 0x01010101 * (127 + shift)); // Exponent
            eye_tmp = ::insert(eye_tmp, 1, 0); // Prime
            eye_tmp = ::insert(eye_tmp, ::set_v16int4(0, 0)); // Sign

            v64int8 tmp1, tmp2;

            tmp1 = ::sel(::broadcast_to_v64int8(0x80), ::broadcast_to_v64int8(0x08), 0x0008000400020001ll);
            tmp2 = ::sel(::broadcast_zero_to_v64int8(), tmp1, 0x0808040402020101ll);

            eye_tmp = ::insert(eye_tmp, (v64int4)::extract_v32int8(tmp2, 0)); // Data
            eye_lower = ::insert(eye_lower, 0, eye_tmp);

            eye_tmp = ::insert(eye_tmp, (v64int4)::extract_v32int8(tmp2, 1)); // Data
            eye_lower = ::insert(eye_lower, 1, eye_tmp);

            tmp1 = ::sel(::broadcast_to_v64int8(0x80), ::broadcast_to_v64int8(0x08), 0x0080004000200010ll);
            tmp2 = ::sel(::broadcast_zero_to_v64int8(), tmp1, 0x8080404020201010ll);

            eye_tmp = ::insert(eye_tmp, (v64int4)::extract_v32int8(tmp2, 0)); // Data
            eye_upper = ::insert(eye_upper, 0, eye_tmp);

            eye_tmp = ::insert(eye_tmp, (v64int4)::extract_v32int8(tmp2, 1)); // Data
            eye_upper = ::insert(eye_upper, 1, eye_tmp);

            utils::unroll_times<Elems / native_elems>([&](unsigned idx) __aie_inline {
                insert<native_elems>(idx, ::mul_4x16_16x16T(v.template extract<native_elems>(idx).template grow<128>(),
                                                            ::concat(eye_lower, eye_upper)));
            });
        }
    }
#endif

    template <typename T>
    __aie_inline
    accum_base &operator=(const vector<T, Elems> &v)
    {
        from_vector(v);
        return *this;
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
#if !__AIE_API_WORKAROUND_CR_1223259__ // AIE2p Error: no mappings for bundle b16 containing: functional opn 'concat_cm_bm_1_S2'
        else if constexpr (num_subaccums == 8) {
            data = ::concat(::concat(arr[0], arr[1], arr[2], arr[3]),
                            ::concat(arr[4], arr[5], arr[6], arr[7]));
        }
        else if constexpr (num_subaccums < 8) {
            data = ::concat(subacc, subaccums...);
        }
#endif
        else {
            utils::unroll_times<num_subaccums>([&] (unsigned idx) __aie_inline {
                insert(idx, arr[idx]);
            });
        }
    }

    __aie_inline
    auto to_native() const
    {
        static_assert(utils::num_elems_v<storage_t> == 1);

        return data;
    }

private:
    template <typename T, unsigned Elems2 = Elems>
    static constexpr auto get_ups()
    {
#if __AIE_API_USE_UPS_TO__
#define UPSHIFT_FN(acc_type) ::ups_to_##acc_type
#else
#define UPSHIFT_FN(acc_type) ::to_##acc_type
#endif
        // Conversions using large intrinsic types may not be appropriate if 1k vector backend is disabled.
        constexpr bool disabled_1k_storage = max_intrinsic_vector_bits::value < 1024;

        if constexpr (std::is_same_v<T, bfloat16>) {
            if constexpr (Elems2 <= 16)
                return [](const vector<T, 16> &v, int, bool) __aie_inline { return UPSHIFT_FN(v16accfloat)(v); };
            else
                return [](const vector<T, 32> &v, int, bool) __aie_inline { return UPSHIFT_FN(v32accfloat)(v); };
        }
#if __AIE_API_BF8_SUPPORT__
        else if constexpr (std::is_same_v<T, bfloat8>) {
            return [](const vector<T, 64> &v, int, bool) __aie_inline {
                // Create 8x8 identity matrix
                // There is no ups instruction for bfloat8 -> float conversion. Instead multiply by identity.
                vector<bfloat8, 64> ident = ::sel(broadcast_zero_to_v64bfloat8(),
                                                  broadcast_one_to_v64bfloat8(),
                                                  0x80402010'08040201ull);

                accum_base<AccumClass::FP, 32, 64> ret = ::mul_8x8_8x8(v, ident);
                return ret;
            };
        }
#endif
#if __AIE_API_FP8_SUPPORT__
        else if constexpr (std::is_same_v<T, float8>) {
            // There is no ups instruction for float8 -> float conversion. Instead multiply by 1.
            return [](const vector<T, 64> &v, int, bool) __aie_inline {
                // Create a 8x8 identity matrix
                vector<float8, 64> ident = ::sel(broadcast_zero_to_v64float8(),
                                                 broadcast_one_to_v64float8(),
                                                 0x80402010'08040201ull);

                accum_base<AccumClass::FP, 32, 64> ret =::mul_8x8_8x8(v, ident);
                return ret;
            };
        }
#endif

#if __AIE_API_FP16_SUPPORT__
        else if constexpr (std::is_same_v<T, float16>) {
            return [](const vector<T, 32> &v, int, bool) __aie_inline { return ::to_v32accfloat(v); };
        }
#endif
#if __AIE_API_FP32_EMULATION__ || __AIE_API_FP32_SUPPORT__
        else if constexpr (std::is_same_v<T, float>) {
            if constexpr      (Elems2 <= 8) // v4accfloat is represented with v8accfloat
                return [](const vector<T, Elems2> &v, int, bool) __aie_inline { return (v8accfloat)(v.template grow<8>()); };
            else if constexpr (Elems2 == 16 || disabled_1k_storage)
                return [](const vector<T, 16> &v, int, bool) __aie_inline { return (v16accfloat)(v); };
            else
                return [](const vector<T, 32> &v, int, bool) __aie_inline { return (v32accfloat)(v); };
        }
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
        else if constexpr (std::is_same_v<T, cbfloat16>) {
            if constexpr      (Elems2 == 4)
                return [](const vector<T, 4> &v, int, bool) __aie_inline { return ::extract_v4caccfloat(::to_v8caccfloat(v.template grow<8>()), 0); };
            else if constexpr (Elems2 == 8)
                return [](const vector<T, 8> &v, int, bool) __aie_inline { return ::to_v8caccfloat(v); };
            else
                return [](const vector<T, 16> &v, int, bool) __aie_inline { return ::to_v16caccfloat(v); };
        }
#endif
        else if constexpr (std::is_same_v<T, cfloat>) {
            if constexpr      (Elems2 <= 4) // v2caccfloat is represented with v4caccfloat
                return [](const vector<T, Elems2> &v, int, bool) __aie_inline { return (v4caccfloat)(v.template grow<4>());  };
            else if constexpr (Elems2 == 8)
                return [](const vector<T, 8> &v, int, bool) __aie_inline { return (v8caccfloat)(v.to_native());  };
            else
                return [](const vector<T, 16> &v, int, bool) __aie_inline { return (v16caccfloat)(v.to_native()); };
            // Note: v32caccfloat exists but vector<cfloat, 32>::storage_t is always an std::array, so we need to
            // perform upshift operation one array element at a time.
        }
#endif
        else if constexpr (utils::is_one_of_v<T, int8, uint8>) {
            if constexpr (Bits != 32 )
                return unsupported_ups{};
            else if constexpr (Elems2 <= 32)
                return [](const vector<T, 32> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v32acc32)(v, shift, sign); };
            else
                return [](const vector<T, 64> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v64acc32)(v, shift, sign); };
        }
        else if constexpr (utils::is_one_of_v<T, int16, uint16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems2 <= 16)
                    return [](const vector<T, 16> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v16acc32)(v, shift, sign); };
                else
                    return [](const vector<T, 32> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v32acc32)(v, shift, sign); };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems2 <= 16)
                    return [](const vector<T, 16> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v16acc64)(v, shift, sign); };
                else
                    return [](const vector<T, 32> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v32acc64)(v, shift, sign); };
            }
            else {
                return unsupported_ups{};
            }
        }
        else if constexpr (utils::is_one_of_v<T, int32, uint32>) {
            if constexpr (Bits == 32) {
                // TODO: should we add support for shift != 0 in this scenario?
                if constexpr (Elems2 <= 8)
                    return [](const vector<T, 8> &v, int shift, bool) __aie_inline { REQUIRES(shift == 0); return (v8acc32)v; };
                else if constexpr (Elems2 == 16 || disabled_1k_storage)
                    return [](const vector<T, 16> &v, int shift, bool) __aie_inline { REQUIRES(shift == 0); return (v16acc32)v; };
                else
                    return [](const vector<T, 32> &v, int shift, bool) __aie_inline { REQUIRES(shift == 0); return (v32acc32)v; };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems2 <= 8)
                    return [](const vector<T, 8> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v8acc64)(v, shift, sign); };
                else
                    return [](const vector<T, 16> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v16acc64)(v, shift, sign); };
            }
            else {
                return unsupported_ups{};
            }
        }
#if __AIE_API_COMPLEX_VECTOR_SUPPORT__
        else if constexpr (utils::is_one_of_v<T, cint16>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems2 <= 8)
                    return [](const vector<T, 8> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v8cacc64)(v, shift, sign);  };
                else
                    return [](const vector<T, 16> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v16cacc64)(v, shift, sign);  };
            }
            else {
                return unsupported_ups{};
            }
        }
        else if constexpr (utils::is_one_of_v<T, cint32>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems2 <= 4)
                    return [](const vector<T, 4> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v4cacc64)(v, shift, sign);  };
                else
                    return [](const vector<T, 8> &v, int shift, bool sign) __aie_inline { return UPSHIFT_FN(v8cacc64)(v, shift, sign);  };
            }
            else {
                return unsupported_ups{};
            }
        }
#endif
        else {
            return unsupported_ups{};
        }
#undef UPSHIFT_FN
    }

    template <typename T>
    static constexpr auto get_srs()
    {
#if __AIE_API_USE_SRS_TO__
#define SRS_FN(vec_type) ::srs_to_##vec_type
#else
#define SRS_FN(vec_type) ::to_##vec_type
#endif
        // Conversions using large intrinsic types may not be appropriate if 1k vector backend is disabled.
        constexpr bool disabled_1k_storage = max_intrinsic_vector_bits::value < 1024;

        if constexpr (std::is_same_v<T, bfloat16>) {
            if constexpr (Elems <= 16)
                return [](const accum_base<Class, MinBits, 16> &acc,  int, bool) __aie_inline { return ::to_v16bfloat16(acc); };
            else
                return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return ::to_v32bfloat16(acc); };
        }
#if __AIE_API_BF8_SUPPORT__
        else if constexpr (std::is_same_v<T, bfloat8>) {
            return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return ::to_v32bfloat8(acc); };
        }
#endif
#if __AIE_API_FP8_SUPPORT__
        else if constexpr (std::is_same_v<T, float8>) {
            return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return ::to_v32float8(acc); };
        }
#endif

#if __AIE_API_FP16_SUPPORT__
        else if constexpr (std::is_same_v<T, float16>) {
            if constexpr (Elems <= 16)
                return [](const accum_base<Class, MinBits, 16> &acc, int, bool) __aie_inline { return ::to_v16float16(acc); };
            else
                return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return ::to_v32float16(acc); };
        }
#endif
#if __AIE_API_FP32_EMULATION__|| __AIE_API_FP32_SUPPORT__
        else if constexpr (std::is_same_v<T, float>) {
            if constexpr (Elems <= 8)
                return [](const accum_base<Class, MinBits, 8> &acc, int, bool) __aie_inline { return  (v8float) acc; };
            else if constexpr (Elems == 16 || disabled_1k_storage)
                return [](const accum_base<Class, MinBits, 16> &acc, int, bool) __aie_inline { return (v16float) acc; };
            else
                return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return (v32float) acc; };
        }
#endif
#if __AIE_API_COMPLEX_FP32_EMULATION__
#if __AIE_API_CBF16_SUPPORT__
        else if constexpr (std::is_same_v<T, cbfloat16>) {
            if constexpr (Elems == 4)
                return [](const accum_base<Class, MinBits, 4> &acc, int, bool) __aie_inline { return ::extract_v4cbfloat16(::to_v8cbfloat16(::set_v8caccfloat(0, acc)), 0); };
            else if constexpr (Elems == 8)
                return [](const accum_base<Class, MinBits, 8> &acc, int, bool) __aie_inline { return ::to_v8cbfloat16(acc); };
            else
                return [](const accum_base<Class, MinBits, 16> &acc, int, bool) __aie_inline { return ::to_v16cbfloat16(acc); };
        }
#endif
        else if constexpr (std::is_same_v<T, cfloat>) {
            if constexpr (Elems <= 4)
                return [](const accum_base<Class, MinBits, 4> &acc, int, bool) __aie_inline { return (v4cfloat) acc;  };
            else
                return [](const accum_base<Class, MinBits, 8> &acc, int, bool) __aie_inline { return (v8cfloat) acc;  };
        }
#endif
#if __AIE_ARCH__ == 21
        else if constexpr (std::is_same_v<T, bfp16ebs8>) {
            return [](const accum_base<Class, MinBits, 64> &acc, int, bool) __aie_inline { return ::to_v64bfp16ebs8(acc); };
        }
        else if constexpr (std::is_same_v<T, bfp16ebs16>) {
            return [](const accum_base<Class, MinBits, 64> &acc, int, bool) __aie_inline { return ::to_v64bfp16ebs16(acc); };
        }
#endif
#if __AIE_ARCH__ == 22
        else if constexpr (std::is_same_v<T, mx4>) {
            return [](const accum_base<Class, MinBits, 64> &acc, int, bool) __aie_inline { return ::to_v64mx4(acc); };
        }
        else if constexpr (std::is_same_v<T, mx6>) {
            return [](const accum_base<Class, MinBits, 64> &acc, int, bool) __aie_inline { return ::to_v64mx6(acc); };
        }
        else if constexpr (std::is_same_v<T, mx9>) {
            return [](const accum_base<Class, MinBits, 64> &acc, int, bool) __aie_inline { return ::to_v64mx9(acc); };
        }
#endif
        else if constexpr (std::is_same_v<T, int8>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 32)
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32int8)(acc, shift, sign); };
                else
                    return [](const accum_base<Class, MinBits, 64> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v64int8)(acc, shift, sign); };
            }
            else if constexpr (Bits == 64) {
                return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return ::pack(SRS_FN(v32int16)(acc, shift, sign), sign); };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, uint8>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 32)
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32uint8)(acc, shift, sign); };
                else
                    return [](const accum_base<Class, MinBits, 64> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v64uint8)(acc, shift, sign); };
            }
            else if constexpr (Bits == 64) {
                return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return ::pack(SRS_FN(v32uint16)(acc, shift, sign), sign); };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, int16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 16)
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16int16)(acc, shift, sign);  };
                else
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32int16)(acc, shift, sign);  };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems <= 16)
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16int16)(acc, shift, sign); };
                else
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32int16)(acc, shift, sign); };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, uint16>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 16)
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16uint16)(acc, shift, sign);  };
                else
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32uint16)(acc, shift, sign);  };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems <= 16)
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16uint16)(acc, shift, sign); };
                else
                    return [](const accum_base<Class, MinBits, 32> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v32uint16)(acc, shift, sign); };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, int32>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 8)
                    return [](const accum_base<Class, MinBits, 8> &acc, int, bool) __aie_inline { return (v8int32) acc; };
                else if constexpr (Elems == 16 || disabled_1k_storage)
                    return [](const accum_base<Class, MinBits, 16> &acc, int, bool) __aie_inline { return (v16int32) acc; };
                else
                    return [](const accum_base<Class, MinBits, 32> &acc, int, bool) __aie_inline { return (v32int32) acc; };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems <= 8)
                    return [](const accum_base<Class, MinBits, 8> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v8int32)(acc, shift, sign);  };
                else
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16int32)(acc, shift, sign);  };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, uint32>) {
            if constexpr (Bits == 32) {
                if constexpr (Elems <= 8)
                    return [](const accum_base<Class, MinBits, 8> &acc, int, bool) __aie_inline { return (v8uint32) acc; };
                else
                    return [](const accum_base<Class, MinBits, 16> &acc, int, bool) __aie_inline { return (v16uint32) acc; };
            }
            else if constexpr (Bits == 64) {
                if constexpr (Elems <= 8)
                    return [](const accum_base<Class, MinBits, 8> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v8uint32)(acc, shift, sign);  };
                else
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16uint32)(acc, shift, sign);  };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, cint16>) {
            if constexpr (Bits == 64) {
                if constexpr (Elems <= 8)
                    return [](const accum_base<Class, MinBits, 8> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v8cint16)(acc, shift, sign); };
                else
                    return [](const accum_base<Class, MinBits, 16> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v16cint16)(acc, shift, sign); };
            }
            else
                return unsupported_srs{};
        }
        else if constexpr (std::is_same_v<T, cint32>) {
            if constexpr (Elems <= 4)
                return [](const accum_base<Class, MinBits, 4> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v4cint32)(acc, shift, sign);  };
            else
                return [](const accum_base<Class, MinBits, 8> &acc, int shift, bool sign) __aie_inline { return SRS_FN(v8cint32)(acc, shift, sign);  };
        }
        else
            return unsupported_srs{};
#undef SRS_FN
    }

    template <unsigned ElemsOut, std::size_t... Indices>
    __aie_inline
    std::array<accum_base<Class, MinBits, ElemsOut>, Elems/ElemsOut> split_helper(std::index_sequence<Indices...> &&) const
    {
        return {extract<ElemsOut>(Indices)...};
    }

    static constexpr unsigned native_elems = 2048 / value_bits();

#ifdef AIE_API_EMULATION
    // TODO: floating point
    std::array<uint64_t, Elems> data;
#else
    storage_t data;
#endif
};

template <AccumClass Class, unsigned MinBits, unsigned... Es>
inline __aie_inline accum_base<Class, MinBits, (Es + ...)>
concat_helper(const accum_base<Class, MinBits, Es> &...inputs)
{
    accum_base<Class, MinBits, (Es + ...)> result;
    result.upd_all(inputs...);
    return result;
}

} // namespace aie::detail

#endif
