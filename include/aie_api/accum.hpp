// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_ACCUM__HPP__
#define __AIE_API_ACCUM__HPP__

#include "aie_types.hpp"
#include "concepts.hpp"
#include "vector.hpp"

#include "detail/accum.hpp"
#include "detail/utils.hpp"

#if __AIE_ARCH__ == 10

#include "detail/aie1/accum.hpp"

#elif __AIE_ARCH__ == 20

#include "detail/aie2/accum.hpp"

#endif

namespace aie {

namespace detail {

template <typename T>
concept NativeAccumType = requires {
    typename native_accum_traits<T>::value_type;
    requires native_accum_traits<T>::size > 0;
};

} // namespace detail

template <AccumElemBaseType MinAccumTag, unsigned Elems>
class __AIE_API_FUNDAMENTAL_TYPE__ accum :
    private detail::accum_base<detail::accum_class_for_tag_v<MinAccumTag>, detail::accum_bits_for_tag_v<MinAccumTag>, Elems>
{
private:
    using base_type = detail::accum_base<detail::accum_class_for_tag_v<MinAccumTag>, detail::accum_bits_for_tag_v<MinAccumTag>, Elems>;

    template <detail::AccumClass Class, unsigned MinBits, unsigned E2> friend class accum_base;

    template <AccumElemBaseType Tag2, unsigned E2> friend class accum;

    // Conversion constructor
    __aie_inline
    explicit accum(const base_type &a) : base_type(a) {}

    template <AccumElemBaseType Tag2>
    static constexpr bool has_same_representation() {
        using other_acc = accum<Tag2, Elems>;
        return value_class() == other_acc::value_class() &&
               accum_bits() == other_acc::accum_bits();
    }

public:
    /** \brief Type of the elements in the accumulator. */
    using value_type = typename base_type::value_type;

    /** \brief Type that holds the actual accumulator's data. May be different to its native type. */
    using storage_t  = typename base_type::storage_t;

    /**
     * \brief Returns the class of the accumulator elements.
     */
    static constexpr detail::AccumClass value_class() { return base_type::value_class(); }

    /**
     * \brief Minimum required number of bits in each accumulator element.
     */
    static constexpr unsigned accum_min_bits() { return base_type::accum_min_bits(); }

    /**
     * \brief Returns the accumulation bits.
     *
     * For complex types this returns the accumulation bits for one of the components.
     */
    static constexpr unsigned accum_bits() { return base_type::accum_bits(); }

    /**
     * \brief Returns the size of each accumulator element in bits.
     *
     * For complex types this returns the combined size of both components.
     */
    static constexpr unsigned value_bits() { return base_type::value_bits(); }

    /**
     * \brief Returns the size of each accumulator element in bits when stored in memory.
     *
     * For complex types this returns the combined size of both components.
     */
    static constexpr unsigned memory_bits() { return base_type::memory_bits(); }

    /**
     * \brief Returns the number of elements in the accumulator.
     */
    static constexpr unsigned size() { return base_type::size(); }

    /**
     * \brief Returns the total size of the accumulator in bits.
     */
    static constexpr unsigned bits() { return base_type::bits(); }

    /**
     * \brief Returns true if the element class is a complex number.
     */
    static constexpr bool is_complex() { return base_type::is_complex(); }

    /**
     * \brief Returns true if the element class is a real number.
     */
    static constexpr bool is_real() { return base_type::is_real(); }

    /**
     * \brief Returns true if the element class is a floating point number.
     */
    static constexpr bool is_floating_point() { return base_type::is_floating_point(); }

    /**
     * \brief Reinterprets the current accumulator as an accumulator of the given type.
     *
     * The number of elements is automatically computed by the function
     *
     * @tparam DstTag Type the accumulator will be cast to
     */
    template <typename DstTag>
    __aie_inline
    auto cast_to() const
    {
        auto cast = base_type::template cast_to<DstTag>();
        using result_type = accum<DstTag, decltype(cast)::size()>;
        using storage_type = typename result_type::storage_t;
        return result_type(static_cast<storage_type>(cast));
    }

    /**
     * \brief Performs the conversion of a FP32/CFP32 accumulator to a vector of the same type.
     */
    template <typename T>
    __aie_inline
    operator vector<T, Elems>() const
        requires(detail::accum_class_for_type_v<T> == value_class() && is_floating_point() && size() <= 32)
    {
        return to_vector<T>();
    }

    /**
     * \brief Default constructor. The value of the elements is undefined.
     */
    __aie_inline
    accum() = default;

    /**
     * \brief Copy constructor.
     */
    accum(const accum &) = default;

    /**
     * \brief Copies another accumulator with the same underlying representation.
     */
    template <AccumElemBaseType Tag2>
        requires(has_same_representation<Tag2>())
    __aie_inline
    accum(const accum<Tag2, Elems> &acc) :
        base_type(acc)
    {
    }


    /**
     * \brief Construct from internal storage type.
     *
     * @param data Data used to construct the accumulator from.
     */
    __aie_inline
    accum(storage_t data) :
        base_type(data)
    {
        // Input is taken by value to avoid losing chess_storage qualifiers
    }

    /**
     * \brief Construct from a vector.
     *
     * The accumulator class and the vector type must be compatible.
     * The accumulator size and the vector size must be the same.
     *
     * @param v     Data used to construct the accumulator from.
     * @param shift Upshift value.
     */
    template <typename T>
    __aie_inline
    explicit accum(const vector<T, Elems> &v, int shift = 0)
    {
        this->from_vector(v, shift);
    }

    /**
     * \brief Operator for conversion to the internal underlying type.
     *
     * If the number of internal subaccumulators is 1, this type matches the native type.
     */
    __aie_inline
    operator storage_t() const
    {
        return base_type::operator storage_t();
    }

    /**
     * \brief Returns a copy of the current accumulator in a larger accumulator.
     *
     * The value of the new elements is undefined.
     *
     * @tparam ElemsOut Size of the output accumulator.
     */
    template <unsigned ElemsOut>
    __aie_inline
    accum<MinAccumTag, ElemsOut> grow() const
    {
        return accum<MinAccumTag, ElemsOut>(base_type::template grow<ElemsOut>());
    }

    /**
     * \brief Returns a copy of the current accumulator in a larger accumulator.
     *
     * The contents of the accumulator are replicated as many times as required to fill the output accumulator
     *
     * @tparam ElemsOut Size of the output accumulator.
     */
    template <unsigned ElemsOut>
    __aie_inline
    constexpr accum<MinAccumTag, ElemsOut> grow_replicate() const
    {
        return accum<MinAccumTag, ElemsOut>(base_type::template grow_replicate<ElemsOut>());
    }

    /**
     * \brief Returns the native type of the accumulator.
     *
     * Only valid when the number of subaccumulators is 1.
     */
    __aie_inline
    auto to_native() const
    {
        return base_type::to_native();
    }

    /**
     * \brief Returns a subaccumulator with the contents of a region of the accumulator.
     *
     * @tparam ElemsOut Size of the returned subaccumulator.
     * @param  idx      Index of the subaccumulator to be returned.
     */
    template <unsigned ElemsOut>
    __aie_inline
    accum<MinAccumTag, ElemsOut> extract(unsigned idx) const
    {
        return accum<MinAccumTag, ElemsOut>(base_type::template extract<ElemsOut>(idx));
    }

    /**
     * \brief Returns a copy of the accumulator with a new size.
     *
     * - Performs an `extract(idx)` operation when the new size is smaller.
     * - Performs a `grow()` operation when the new size is bigger.
     *
     * @see grow<ElemsOut>(unsigned) const
     * @see extract<ElemsOut>(unsigned) const
     */
    template <unsigned ElemsOut>
    __aie_inline
    accum<MinAccumTag, ElemsOut> grow_extract(unsigned idx) const
    {
        if constexpr (ElemsOut > Elems)
            return grow<ElemsOut>();
        else
            return extract<ElemsOut>(idx);
    }

    /**
     * \brief Returns the contents of the current accum stored in a std::tuple of subaccums.
     *
     * @tparam ElemsOut Size of each subaccum.
     */
    template <unsigned ElemsOut>
    __aie_inline
    auto split() const
    {
        auto as_tuple = [](auto &&... vs) {
            return std::make_tuple(accum<MinAccumTag, ElemsOut>(vs)...);
        };
        return detail::utils::apply_tuple(as_tuple, base_type::template split<ElemsOut>());
    }

    /**
     * \brief Updates the contents of a region of the accumulator with a native subaccumulator.
     *
     * @param idx Index of the subaccumulator to be replaced.
     * @param acc Native subaccumulator to be written into the region.
     * @returns   Reference to the updated accumulator.
     */
    template <unsigned ElemsIn, AccumElemBaseType Tag2>
    __aie_inline
    accum &insert(unsigned idx, const accum<Tag2, ElemsIn> &acc)
    {
        using other_base = typename accum<Tag2, ElemsIn>::base_type;
        base_type::insert(idx, static_cast<const other_base&>(acc));
        return *this;
    }

    /**
     * \brief Updates the contents of a region of the accumulator using the values in the given native subaccumulator.
     *
     * @param idx Index of the subaccumulator to be replaced.
     * @param acc Native subaccumulator to be written into the region.
     * @returns   Reference to the updated accumulator.
     */
    template <unsigned ElemsIn>
    __aie_inline
    accum &insert(unsigned idx, typename accum<MinAccumTag, ElemsIn>::storage_t acc)
    {
        const typename accum<value_type, ElemsIn>::base_type in = acc;

        base_type::template insert<ElemsIn>(idx, in);
        return *this;
    }

    /**
     * \brief Updates the contents of a region of the accumulator using the values in the given native subaccumulator.
     *
     * @param idx Index of the subaccumulator to be replaced.
     * @param acc Native subaccumulator to be written into the region.
     * @returns   Reference to the updated accumulator.
     */
    template <NativeAccumType T2>
    __aie_inline
    accum &insert(unsigned idx, T2 acc)
    {
        using U = typename detail::native_accum_traits<T2>::value_type;
        constexpr unsigned ElemsIn = detail::native_accum_traits<T2>::size;

        static_assert(std::is_same_v<value_type, U>, "Mismatched types cannot be inserted");

        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subaccum index");
        
        const accum<MinAccumTag, ElemsIn> in = acc;

        return insert(idx, in);
    }

    /**
     * \brief Applies shift-round-saturate to the elements of the accumulator and stores the result into a vector of the
     * requested type, keeping the original values unchanged.
     *
     * \note On AIE-ML shift values of -4, -3, and -2 are unsafe, as they will only produce correct result if truncation
     * is selected or saturation against 0 is required.
     *
     * @param shift Number of bits that need to be downshifted.
     */
    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector(int shift = 0) const
    {
        return base_type::template to_vector<T>(shift);
    }

    /**
     * \brief Applies shift-round-saturate to the elements of the accumulator and stores the result into a vector of the
     * type and signedness requested.
     *
     * The original values are left unchanged.
     *
     * \note Currently functional on AIE-ML only.
     * \note On AIE-ML shift values of -4, -3, and -2 are unsafe, as they will only produce correct result if truncation
     * is selected or saturation against 0 is required.
     *
     * @param v_sign Returned vector is signed or not, regardless of the underlying type.
     * @param shift  Number of bits that need to be downshifted.
     */
    template <typename T>
    __aie_inline
    vector<T, Elems> to_vector_sign(bool v_sign, int shift = 0) const
    {
        return base_type::template to_vector_sign<T>(v_sign, shift);
    }

    /**
     * \brief Permutes and applies shift-round-saturate to the elements of the accumulator. Then, it stores the result
     * into a vector of the requested type.
     *
     * The original values are left unchanged.
     *
     * The values are interleaved as if the accumulator were split in two parts and zipped.
     *
     * @param shift Number of bits that need to be downshifted.
     */
    template <typename T>
        requires(arch::is(arch::AIE))
    __aie_inline
    vector<T, Elems> to_vector_zip(int shift = 0) const
    {
        return base_type::template to_vector_zip<T>(shift);
    }

    /**
     * \brief Returns the values of the FP32 accumulator in a vector of the requested type.
     */
    __aie_inline
    auto to_vector() const requires(is_floating_point())
    {
        if constexpr (value_class() == detail::AccumClass::FP)
            return to_vector<float>();
        else if constexpr (value_class() == detail::AccumClass::CFP)
            return to_vector<cfloat>();
    }

    /**
     * \brief Updates the contents of the accumulator using the values in the given vector after applying the requested upshift
     * operation.
     *
     * @param v     Input vector.
     * @param shift Number of bits to be upshifted.
     */
    template <typename T>
    __aie_inline
    void from_vector(const vector<T, Elems> &v, int shift = 0)
    {
        base_type::from_vector(v, shift);
    }

    /**
     * \brief Updates the contents of the accumulator using the values in the given vector after applying the requested upshift
     * operation.
     *
     * Allows for dynamic control of whether the values are considered signed or not.
     *
     * \note Currently functional on AIE-ML only.
     *
     * @param v      Input vector.
     * @param v_sign Values are considered signed or not, regardless of the underlying type.
     * @param shift  Number of bits to be upshifted.
     */
    template <typename T>
    __aie_inline
    void from_vector_sign(const vector<T, Elems> &v, bool v_sign, int shift = 0)
    {
        return base_type::from_vector_sign(v, v_sign, shift);
    }

    /**
     * \brief Updates the contents of the accumulator using the values in the given vector. No upshift is applied.
     *
     * @param v Input vector.
     */
    template <typename T>
    __aie_inline
    accum &operator=(const vector<T, Elems> &v)
    {
        base_type::operator=(v);
        return *this;
    }

    /**
     * \brief Updates the contents of the accumulator using the values in the given subaccumulators.
     *
     * Subaccumulators must be of the same class and size.
     *
     * @param subacc    First sub-accumulator.
     * @param subaccums Remaining sub-accumulators.
     */
    template <AccumElemBaseType Tag2, unsigned E2, unsigned... Es>
        requires(Elems == (E2 + (Es + ...)) && ((Elems / E2 == Elems / Es) && ...))
    __aie_inline
    void upd_all(const accum<Tag2, E2> &subacc, const accum<Tag2, Es> & ...subaccums)
    {
        base_type::upd_all(subacc, subaccums...);
    }
};

/**
 * \brief Template deduction guidelines for aie::accum
 */
template <detail::NativeAccumType T>
accum(T) -> accum<typename detail::native_accum_traits<T>::value_type, detail::native_accum_traits<T>::size>;

} // namespace aie

#endif // __AIE_API_ACCUM__HPP__
