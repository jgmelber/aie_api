// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_VECTOR__HPP__
#define __AIE_API_VECTOR__HPP__

#include <array>

#include "concepts.hpp"

#include "detail/utils.hpp"
#include "detail/vector.hpp"
#include "vector_elem_ref.hpp"
#if __AIE_ARCH__ == 10

#include "detail/aie1/vector_native_types.hpp"
#include "detail/aie1/vector.hpp"

#elif __AIE_ARCH__ == 20

#include "detail/aie2/vector_native_types.hpp"
#include "detail/aie2/vector.hpp"

#endif

namespace aie {
namespace detail {

template <typename T>
struct native_vector_traits;

template <typename T>
concept NativeVectorType = requires {
    typename native_vector_traits<T>::value_type;
    requires native_vector_traits<T>::size > 0;
};

template <typename Base, typename Derived>
concept BaseOf = std::is_base_of_v<Base, Derived>;

template <typename V, typename R>
concept SupportsPack  = requires (V v) {
    { v.template pack<R>() } -> BaseOf<vector_base<R, V::size()>>;
};

template <typename V, typename R>
concept SupportsPackSign = requires (V v) {
    { v.template pack_sign<R>(true) } -> BaseOf<vector_base<R, V::size()>>;
};

template <typename V, typename R>
concept SupportsUnpack  = requires (V v) {
    { v.template unpack<R>() } -> BaseOf<vector_base<R, V::size()>>;
};

template <typename V, typename R>
concept SupportsUnpackSign = requires (V v) {
    { v.template unpack_sign<R>(true) } -> BaseOf<vector_base<R, V::size()>>;
};

} // namespace aie::detail

template <ElemBaseType T, unsigned Elems>
class __AIE_API_FUNDAMENTAL_TYPE__ vector : private detail::vector_base<T, Elems>
{
private:
    using base_type = detail::vector_base<T, Elems>;

    template <ElemBaseType T2, unsigned E2> friend class vector;
    template <typename T2, unsigned E2> friend class detail::vector_base;

    // Implicit conversion constructor
    __aie_inline
    vector(const base_type &v) : base_type(v) {}

public:
    /** \brief Equivalent intrinsic type for the same element type and size. */
    using native_type = typename base_type::native_type;

    /** \brief Type of the elements in the vector. */
    using value_type  = typename base_type::value_type;

    /** \brief Type that holds the actual vector's data. May be different to its native type. */
    using storage_t   = typename base_type::storage_t;

    /**
     * \brief Returns the size of each vector element in bits.
     */
    static constexpr unsigned type_bits() { return base_type::type_bits(); }

    /**
     * \brief Returns the number of elements in the vector.
     */
    static constexpr unsigned size() { return base_type::size(); }

    /**
     * \brief Returns the total size of the vector in bits.
     */
    static constexpr unsigned bits() { return base_type::bits(); }

    /**
     * \brief Returns the total capacity of the vector in bytes.
     */
    static constexpr unsigned bytes() { return bits() / 8; }

    /**
     * \brief Returns true if the element type is signed.
     */
    static constexpr bool is_signed() { return base_type::is_signed(); }

    /**
     * \brief Returns true if the element type is a complex number.
     */
    static constexpr bool is_complex() { return base_type::is_complex(); }

    /**
     * \brief Returns true if the element type is a real number.
     */
    static constexpr bool is_real() { return !is_complex(); }

    /**
     * \brief Returns true if the element type is an integer.
     */
    static constexpr bool is_integral() { return base_type::is_integral(); }

    /**
     * \brief Returns true if the element type is floating-point.
     */
    static constexpr bool is_floating_point() { return base_type::is_floating_point(); }

    /**
     * \brief Default constructor. The value of the elements is undefined.
     */
    __aie_inline
    vector() :
        base_type()
    {
    }

    /**
     * \brief Construct a vector from internal native types.
     *
     * @param v Data used to construct the vector from.
     */
    __aie_inline
    vector(storage_t v) :
        base_type(v)
    {
        // Input is taken by value to avoid losing chess_storage qualifiers
    }

// #ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__ // FIXME: Delete in AIE1 when defined
    /**
     * \brief Construct from internal types.
     * This is a special case for 1024b internal vector types, which need to be broken into two 512b vectors
     *
     * @param v Data used to construct the vector from
     */
    __aie_inline
    vector(const native_type &v)  requires(!std::is_same_v<storage_t, native_type>)
        : base_type(v)
    {
    }
// #endif

    /**
     * \brief Construct from a list of values
     *
     * @param v First value in the list
     * @param values Rest of values in the list
     */
    template <typename... Values>
    __aie_inline
    explicit vector(value_type v, Values &&... values) :
        base_type(v, std::forward<Values>(values)...)
    {
    }

    /**
     * \brief Returns the value of the vector using its native type.
     */
    __aie_inline
    native_type to_native() const
    {
        return base_type::to_native();
    }

    /**
     * \brief Operator for conversion to the native type
     */
    __aie_inline
    operator native_type() const
    {
        return to_native();
    }

    /**
     * \brief Reinterprets the current vector as a vector of the given type. The number of elements is automatically computed
     * by the function
     *
     * @tparam DstT Type the vector will be cast to
     */
    template <typename DstT>
    __aie_inline
    auto cast_to() const
    {
        using result_base = decltype(base_type().template cast_to<DstT>());
        return vector<DstT, result_base::size()>(base_type::template cast_to<DstT>());
    }

    /**
     * \brief Adds a new element to the vector, moving existing elements one position up.
     *
     * Shifts all elements in the vector up and writes the given value into the first position of the vector.
     * The element in the last position of the vector is lost).
     *
     * @param v Value to be written into the first position of the vector
     */
    __aie_inline
    vector &push(value_type v)
    {
        base_type::push(v);
        return *this;
    }

    /**
     * \brief Returns a copy of the current vector in a larger vector.
     *
     * The value of the new elements is undefined.
     *
     * @tparam ElemsOut Size of the output vector.
     * @param idx Location of the subvector within the output vector
     */
    template <unsigned ElemsOut> constexpr __aie_inline vector<T, ElemsOut> grow(unsigned idx = 0) const
    {
        return {base_type::template grow<ElemsOut>(idx)};
    }

    /**
     * \brief Returns a copy of the current vector in a larger vector.
     *
     * The contents of the vector are replicated as many times as required to fill the output vector
     *
     * @tparam ElemsOut Size of the output vector.
     */
    template <unsigned ElemsOut> constexpr __aie_inline vector<T, ElemsOut> grow_replicate() const
    {
        return vector<T, ElemsOut>(base_type::template grow_replicate<ElemsOut>());
    }

    /**
     * \brief Updates the value of the element on the given index.
     *
     * @param v Value to write.
     * @param idx Index of the element whose value is updated.
     */
    __aie_inline
    void set(value_type v, unsigned idx)
    {
        base_type::set(v, idx);
    }

    /**
     * \brief Returns the value of the element on the given index.
     *
     * @param idx Index of the element.
     */
    __aie_inline
    value_type get(unsigned idx) const
    {
        return base_type::get(idx);
    }

    /**
     * \brief Returns a constant reference object to the element on the given index.
     *
     * @param idx Index of the element.
     */
    constexpr __aie_inline vector_elem_const_ref<value_type, Elems> operator[](unsigned idx) const
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");
        return elem_const_ref(idx);
    }

    /**
     * \brief Returns a reference object to the element on the given index.
     *
     * @param idx Index of the element.
     */
    constexpr __aie_inline vector_elem_ref<value_type, Elems> operator[](unsigned idx)
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");
        return elem_ref(idx);
    }

    /**
     * \brief Returns a constant reference object to the element on the given index.
     *
     * @param idx Index of the element.
     */
    constexpr __aie_inline vector_elem_const_ref<value_type, Elems> elem_const_ref(unsigned idx) const
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");
        return { *this, idx };
    }

    /**
     * \brief Returns a constant reference object to the element on the given index.
     *
     * @param idx Index of the element.
     */
    constexpr __aie_inline vector_elem_const_ref<value_type, Elems> elem_ref(unsigned idx) const
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");
        return { *this, idx };
    }

    /**
     * \brief Returns a reference object to the element on the given index.
     *
     * @param idx Index of the element.
     */
    constexpr __aie_inline vector_elem_ref<value_type, Elems> elem_ref(unsigned idx)
    {
        REQUIRES_MSG(idx < Elems, "idx needs to be a valid element index");
        return {*this, idx};
    }

    /**
     * \brief Updates the contents of the vector using the values in the given subvectors.
     *
     * Subvectors must be of the same type and size.
     *
     * @param subvectors List of subvectors.
     */
    template <typename... SubVectors>
    __aie_inline
    vector &upd_all(SubVectors && ...subvectors)
    {
        base_type::upd_all(std::forward<SubVectors>(subvectors)...);
        return *this;
    }

    /**
     * \brief Updates the contents of a region of the vector.
     *
     * The updated region will contain the values in the given subvector.
     *
     * @param idx Index of the subvector to be replaced.
     * @param v Subvector to be written into the region.
     * @returns a reference to the updated vector.
     */
    template <unsigned ElemsIn>
    __aie_inline
    vector &insert(unsigned idx, const vector<T, ElemsIn> &v)
    {
        base_type::template insert<ElemsIn>(idx, v);
        return *this;
    }

    /**
     * \brief Updates the contents of a region of the vector using.
     *
     * The updated region will contain the values in the given native subvector.
     *
     * @param idx Index of the subvector to be replaced.
     * @param v Native subvector to be written into the region.
     * @returns a reference to the updated vector.
     */
    template <unsigned ElemsIn>
    __aie_inline
    vector &insert(unsigned idx, typename vector<T, ElemsIn>::native_type v)
    {
        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");

        const vector<T, ElemsIn> in = v;

        return insert(idx, in);
    }

    /**
     * \brief Updates the contents of a region of the vector using.
     *
     * The updated region will contain the values in the given native subvector.
     *
     * @param idx Index of the subvector to be replaced.
     * @param v Native subvector to be written into the region.
     * @returns a reference to the updated vector.
     */
    template <NativeVectorType T2>
    __aie_inline
    vector &insert(unsigned idx, T2 v)
    {
        using U = typename detail::native_vector_traits<T2>::value_type;
        constexpr unsigned ElemsIn = detail::native_vector_traits<T2>::size;

        static_assert(std::is_same_v<T, U>, "Mismatched types cannot be inserted");

        REQUIRES_MSG(idx < Elems / ElemsIn, "idx needs to be a valid subvector index");
        
        const vector<T, ElemsIn> in = v;

        return insert(idx, in);
    }

    /**
     * \brief Returns a subvector with the contents of a region of the vector.
     *
     * @tparam ElemsOut Size of the returned subvector.
     * @param idx Index of the subvector to be returned.
     */
    template <unsigned ElemsOut>
    __aie_inline
    vector<value_type, ElemsOut> extract(unsigned idx) const
    {
        return base_type::template extract<ElemsOut>(idx);
    }

    /**
     * \brief Returns a copy of the current vector in a new size.
     *
     * - Performs an `extract(idx)` operation when the new size is smaller.
     * - Performs a `grow(idx)` operation when the new size is bigger.
     *
     * @see grow<ElemsOut>(unsigned) const
     * @see extract<ElemsOut>(unsigned) const
     */
    template <unsigned ElemsOut>
    __aie_inline
    vector<value_type, ElemsOut> grow_extract(unsigned idx) const
    {
        if constexpr (ElemsOut > Elems)
            return grow<ElemsOut>();
        else
            return extract<ElemsOut>(idx);
    }

    /**
     * \brief Returns the contents of the current vector stored in a std::tuple of subvectors.
     *
     * @tparam ElemsOut Size of each subvector.
     */
    template <unsigned ElemsOut>
    __aie_inline
    auto split() const -> auto
    {
        auto as_tuple = [](auto &&... vs) {
            return std::make_tuple(vector<T, ElemsOut>(vs)...);
        };
        return detail::utils::apply_tuple(as_tuple, base_type::template split<ElemsOut>());
    }

    /**
     * \brief Returns a copy of the vector with elements represented in a wider type.
     *
     * By default, unpacking a vector will produce a vector with twice the number of bits. For example, unpacking a
     * vector of 8b elements will produce a vector of 16b elements.
     *
     * \note Only vectors with integer elements are allowed.
     *
     * \note Complex vectors can only unpack to other complex types.
     *
     * \tparam T2 target element type. It must be wider than the vector's element type.
     */
    template <UnpackableFrom<T> T2 = typename detail::unpacked_type<T>::type>
    __aie_inline
    auto unpack() const -> vector<T2, Elems>;

    // FIXME: AIE1 vs AIE2
    /**
     * \brief Returns a copy of the vector with elements represented in a wider type, with dynamic selection of whether
     * vectors are signed or not.
     *
     * By default, unpacking a vector will produce a vector with twice the number of bits. For example, unpacking a
     * vector of 8b elements will produce a vector of 16b elements.
     *
     * \note Only vectors with integer elements are allowed.
     *
     * \note Complex vectors can only unpack to other complex types.
     *
     * \tparam T2 target element type. It must be wider than the vector's element type.
     *
     * @param v_sign Returned vector is signed or not, regardless of the underlying type.
     */
    template <UnpackableFrom<T> T2 = typename detail::unpacked_type<T>::type>
    __aie_inline
    auto unpack_sign(bool v_sign) const -> vector<T2, Elems>;

    /**
     * \brief Returns a copy of the vector with elements represented in a narrower type.
     *
     * By default, packing a vector will produce a vector with half the number of bits. For example, packing a vector
     * of 16b elements will produce a vector of 8b elements.
     *
     * \note Only vectors with integer elements are allowed.
     *
     * \note Complex vectors can only unpack to other complex types.
     *
     * \tparam T2 target element type. It must be narrower than the vector's element type.
     */
    template <PackableFrom<T> T2 = typename detail::packed_type<T>::type>
    __aie_inline
    auto pack() const -> vector<T2, Elems>;

    /**
     * \brief Returns a copy of the vector with elements represented in a narrower type, with dynamic selection of
     * whether vectors are signed or not.
     *
     * By default, packing a vector will produce a vector with half the number of bits. For example, packing a vector
     * of 16b elements will produce a vector of 8b elements.
     *
     * \note Only vectors with integer elements are allowed.
     *
     * \note Complex vectors can only unpack to other complex types.
     *
     * \tparam T2 target element type. It must be narrower than the vector's element type.
     *
     * @param v_sign Returned vector is signed or not, regardless of the underlying type.
     */
    template <PackableFrom<T> T2 = typename detail::packed_type<T>::type>
    __aie_inline
    auto pack_sign(bool v_sign) const -> vector<T2, Elems>;

    /**
     * \brief Replaces the contents of the vector with the values pointed by the given memory address.
     *
     * Address is assumed to meet the alignment requirements for vector loads (>= 32 bytes).
     *
     * @param ptr Pointer to the location of the values in memory.
     */
    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void load(const T2 *ptr)
    {
        base_type::template load<Resource, T2>(ptr);
    }

    /**
     * \brief Writes the contents of the vector into the given memory address.
     *
     * Address is assumed to meet the alignment requirements for vector stores (>= 32 bytes).
     *
     * @param ptr Pointer to the location where the vector contents must be written in memory.
     */
    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void store(T2 *ptr) const
    {
        base_type::template store<Resource, T2>(ptr);
    }

    /**
     * \brief Replaces the contents of the vector with the values pointed by the given memory address.
     * Address is assumed to be aligned to the given amount of elements.
     *
     * @param ptr Pointer to the location of the values in memory.
     * @param aligned_elems Number of elements ptr is aligned to.
     */
    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void load_unaligned(const T2 *ptr, unsigned aligned_elems)
    {
        base_type::template load_unaligned<Resource, T2>(ptr, aligned_elems);
    }

    /**
     * \brief Writes the contents of the vector into the given memory address.
     *
     * Address is assumed to be aligned to the given amount of elements.
     *
     * @param ptr Pointer to the location where the vector contents must be written in memory.
     * @param aligned_elems Number of elements ptr is aligned to.
     */
    template <aie_dm_resource Resource = aie_dm_resource::none, typename T2> requires(std::is_same_v<aie_dm_resource_remove_t<T2>, value_type>)
    __aie_inline
    void store_unaligned(T2 *ptr, unsigned aligned_elems) const
    {
        base_type::template store_unaligned<Resource, T2>(ptr, aligned_elems);
    }
};

/**
 * \brief Template deduction guidelines for aie::vector
 */
template <detail::NativeVectorType T>
vector(T) -> vector<typename detail::native_vector_traits<T>::value_type, detail::native_vector_traits<T>::size>;


template <ElemBaseType T, unsigned Elems>
vector<T, Elems> zeros();

template <ElemBaseType T, unsigned Elems>
template <UnpackableFrom<T> T2>
__aie_inline
inline auto vector<T, Elems>::unpack() const -> vector<T2, Elems>
{
    if constexpr(detail::SupportsUnpack<base_type, T2>) {
        return base_type::template unpack<T2>();
    }
    else if constexpr (detail::type_bits_v<T2> == 2 * detail::type_bits_v<T>) {
        if constexpr (detail::is_complex_v<T>) {
            using real_type1 = detail::utils::get_complex_component_type_t<T>;
            using real_type2 = detail::utils::get_complex_component_type_t<T2>;
            return (*this).template cast_to<real_type1>()
                          .template unpack<real_type2>()
                          .template cast_to<T2>();
        }
        else {
            vector<T, Elems> extend;
            if constexpr (is_signed() && vector<T2, Elems>::is_signed())
                extend = select(T(0), T(-1), lt(*this, T(0)));
            else
                extend = zeros<T, Elems>();
            auto [lo, hi] = interleave_zip(*this, extend, 1);
            return concat(lo, hi).template cast_to<T2>();
        }
    }
    else {
        using accum_tag = detail::accum_tag_for_type<T2>;
        accum<accum_tag, Elems> a{*this};
        return a.template to_vector<T2>();
    }
}

template <ElemBaseType T, unsigned Elems>
template <UnpackableFrom<T> T2>
__aie_inline
inline auto vector<T, Elems>::unpack_sign(bool sign) const -> vector<T2, Elems>
{
    if constexpr(detail::SupportsUnpackSign<base_type, T2>) {
        return base_type::template unpack_sign<T2>(sign);
    }
    else if constexpr (detail::type_bits_v<T2> == 2 * detail::type_bits_v<T>) {
        if constexpr (detail::is_complex_v<T>) {
            using real_type1 = detail::utils::get_complex_component_type_t<T>;
            using real_type2 = detail::utils::get_complex_component_type_t<T2>;
            return (*this).template cast_to<real_type1>()
                          .template unpack_sign<real_type2>(sign)
                          .template cast_to<T2>();
        }
        else {
            vector<T, Elems> extend;
            if constexpr (is_signed() && vector<T2, Elems>::is_signed())
                extend = select(T(0), T(-1 * sign), lt(*this, T(0)));
            else
                extend = zeros<T, Elems>();
            auto [lo, hi] = interleave_zip(*this, extend, 1);
            return concat(lo, hi).template cast_to<T2>();
        }
    }
    else {
        using accum_tag = detail::accum_tag_for_type<T2>;
        accum<accum_tag, Elems> a;
        a.from_vector_sign(*this, sign);
        return a.template to_vector_sign<T2>(sign);
    }
}

template <ElemBaseType T, unsigned Elems>
template <PackableFrom<T> T2>
__aie_inline
inline auto vector<T, Elems>::pack() const -> vector<T2, Elems>
{
    if constexpr(detail::SupportsPack<base_type, T2>)
        return base_type::template pack<T2>();
    else {
        using accum_tag = detail::accum_tag_for_type<T>;
        accum<accum_tag, Elems> a{*this};
        return a.template to_vector<T2>();
    }
}

template <ElemBaseType T, unsigned Elems>
template <PackableFrom<T> T2>
__aie_inline
inline auto vector<T, Elems>::pack_sign(bool sign) const -> vector<T2, Elems>
{
    if constexpr(detail::SupportsPackSign<base_type, T2>) {
        return base_type::template pack_sign<T2>(sign);
    }
    else {
        using accum_tag = detail::accum_tag_for_type<T>;
        accum<accum_tag, Elems> a = from_vector<accum_tag>(op_sign(*this, sign));
        return a.template to_vector_sign<T2>(sign);
    }
}

} // namespace aie

#endif // __AIE_API_VECTOR__HPP__
