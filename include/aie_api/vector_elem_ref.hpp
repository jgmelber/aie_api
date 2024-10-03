// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_VECTOR_ELEM_REF__HPP__
#define __AIE_API_VECTOR_ELEM_REF__HPP__

#include "detail/utils.hpp"
#include "concepts.hpp"

#include <tuple>

namespace aie {

template <ElemBaseType T, unsigned N>
class vector;

template <ElemBaseType T, unsigned N>
class vector_elem_ref;

// Note: Find types' documentation in aie_types.hpp
template <ElemBaseType T, unsigned N>
class vector_elem_const_ref
{
public:
    using vector_type = vector<T, N>;
    using parent_type = vector_type;
    using  value_type = T;

    /**
     * Converts a mutable reference into an immutable reference
     */
    constexpr vector_elem_const_ref(const vector_elem_ref<T, N> &ref) :
        parent(ref.parent),
        offset(ref.offset)
    {
    }

    /**
     * \brief Access the element value.
     */
    value_type get() const
    {
        return parent.get(offset);
    }

    /**
     * \brief Cast operator. Access the element value.
     */
    operator value_type() const
    {
        return get();
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator==(T2 v) const
    {
        return (value_type)*this == (value_type)v;
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator!=(T2 v) const
    {
        return (value_type)*this != (value_type)v;
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator<=(T v) const
    {
        return (value_type)*this <= (value_type)v;
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator>=(T2 v) const
    {
        return (value_type)*this >= (value_type)v;
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator<(T2 v) const
    {
        return (value_type)*this < (value_type)v;
    }

    /**
     * \brief Compares element value with another
     */
    template <typename T2>
    constexpr bool operator>(T2 v) const
    {
        return (value_type)*this > (value_type)v;
    }

    /**
     * \brief The vector where the referenced element is located
     */
    const vector_type &parent;

    /**
     * \brief The position within a vector where the element is located
     */
    unsigned offset;

private:
    constexpr vector_elem_const_ref(const vector_type &v, unsigned idx) :
        parent(v),
        offset(idx)
    {
    }

    friend vector_type;
    friend vector_elem_ref<T, N>;
};

template <ElemBaseType T, unsigned N>
class vector_elem_ref
{
public:
    using vector_type = vector<T, N>;
    using parent_type = vector_type;
    using  value_type = T;

    /**
     * \brief Access the element value.
     */
    value_type get() const
    {
        return parent.get(offset);
    }

    /**
     * \brief Cast operator. Access the element value.
     */
    operator value_type() const
    {
        return get();
    }

    /**
     * \brief Assignment operator. Overwrites the referenced value.
     */
    vector_elem_ref &operator=(const value_type &v)
    {
        parent.set(v, offset);
        return *this;
    }

    /**
     * \brief Assignment operator. Overwrites the referenced value.
     */
    vector_elem_ref &operator=(const vector_elem_ref<T, N> &v)
    {
        parent.set(v.get(), offset);
        return *this;
    }

    /**
     * \brief Assignment operator. Overwrites the referenced value.
     */
    vector_elem_ref &operator=(const vector_elem_const_ref<T, N> &v)
    {
        parent.set(v.get(), offset);
        return *this;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator==(T2 v) const
    {
        return (value_type)*this == (value_type)v;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator!=(T2 v) const
    {
        return (value_type)*this != (value_type)v;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator<=(T v) const
    {
        return (value_type)*this <= (value_type)v;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator>=(T2 v) const
    {
        return (value_type)*this >= (value_type)v;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator<(T2 v) const
    {
        return (value_type)*this < (value_type)v;
    }

    /**
     * \brief Compares element value with another.
     */
    template <typename T2>
    constexpr bool operator>(T2 v) const
    {
        return (value_type)*this > (value_type)v;
    }

    vector_type &parent;
    unsigned offset;

private:
    constexpr vector_elem_ref(vector_type &v, unsigned idx) :
        parent(v),
        offset(idx)
    {
    }

    friend vector_type;
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
class vector_ref
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<std::remove_const_t<elem_type>, Elems>;

    using        value_type = typename vector_type::value_type;

    static constexpr unsigned size()
    {
        return vector_type::size();
    }

    __aie_inline
    constexpr vector_ref(T *ptr) :
        ptr_(ptr)
    {}

    __aie_inline
    constexpr operator vector_type() const
    {
        vector_type ret;

        ret.template load<Elems, Resource>(ptr_);

        return ret;
    }

    __aie_inline
    constexpr vector_ref &operator=(const vector_type &v) requires(std::is_const_v<T>)
    {
        vector_type dst(v);

        dst.template store<Elems, Resource>(ptr_);

        return *this;
    }

private:
    T *ptr_;
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
class unaligned_vector_ref
{
public:
    using         elem_type = aie_dm_resource_remove_t<T>;
    using       vector_type = vector<std::remove_const_t<elem_type>, Elems>;

    using        value_type = typename vector_type::value_type;

    static constexpr unsigned size()
    {
        return vector_type::size();
    }

    __aie_inline
    constexpr unaligned_vector_ref(T *ptr, unsigned alignment = 1) :
        ptr_(ptr),
        alignment_(alignment)
    {}

    __aie_inline
    constexpr operator vector_type() const
    {
        vector_type ret;

        ret.template load_unaligned<Resource>(ptr_, alignment_);

        return ret;
    }

    __aie_inline
    constexpr unaligned_vector_ref &operator=(const vector_type &v) requires(!std::is_const_v<T>)
    {
        vector_type dst(v);

        dst.template store_unaligned<Resource>(ptr_, alignment_);

        return *this;
    }

private:
    T *ptr_;
    unsigned alignment_;
};

namespace detail {

template <typename DstT, typename SrcT, unsigned SrcElems>
auto vector_cast(const vector<SrcT, SrcElems> &v)
{
    return v.template cast_to<DstT>();
}

template <typename T, unsigned Elems>
struct concat_vector_helper
{
    template <typename... Vectors>
    static auto run(Vectors && ...vectors)
    {
        static_assert(sizeof...(vectors) > 1);
        static_assert((std::is_same_v<vector<T, Elems>, utils::remove_all_t<Vectors>> && ...));

        using out_vector_type = vector<T, Elems * sizeof...(vectors)>;

        out_vector_type ret;

        ret.upd_all(std::forward<Vectors>(vectors)...);

        return ret;
    }
};

template <typename... Vectors>
auto concat_vector(Vectors && ...vectors)
{
    using first_type = std::tuple_element_t<0, std::tuple<utils::remove_all_t<Vectors>...>>;

    return concat_vector_helper<typename first_type::value_type, first_type::size()>::run(std::forward<Vectors>(vectors)...);
}

template <typename T>
struct is_vector_elem_ref
{
    static constexpr bool value = false;
};

template <ElemBaseType T, unsigned Elems>
struct is_vector_elem_ref<vector_elem_const_ref<T, Elems>>
{
    static constexpr bool value = true;
};

template <ElemBaseType T, unsigned Elems>
struct is_vector_elem_ref<vector_elem_ref<T, Elems>>
{
    static constexpr bool value = true;
};

template <typename T>
struct is_vector
{
    static constexpr bool value = false;
};

template <typename T, unsigned Elems>
struct is_vector<vector<T, Elems>>
{
    static constexpr bool value = true;
};

template <typename T>
struct is_vector_ref
{
    static constexpr bool value = false;
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct is_vector_ref<vector_ref<T, Elems, Resource>>
{
    static constexpr bool value = true;
};


template <typename T, unsigned Elems, aie_dm_resource Resource>
struct is_vector_ref<unaligned_vector_ref<T, Elems, Resource>>
{
    static constexpr bool value = true;
};

template <typename T>
constexpr auto remove_elem_ref(T t) {
    if constexpr (is_vector_elem_ref_v<T>)
        return t.get();
    else
        return t;
}

} // namespace detail

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator+(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) +
           detail::remove_elem_ref(v2);
}

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator*(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) *
           detail::remove_elem_ref(v2);
}

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator-(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) -
           detail::remove_elem_ref(v2);
}

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator&(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) &
           detail::remove_elem_ref(v2);
}

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator|(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) |
           detail::remove_elem_ref(v2);
}

template <typename T1, typename T2> requires(detail::is_vector_elem_ref_v<T1> || detail::is_vector_elem_ref_v<T2>)
constexpr auto operator^(T1 v1, T2 v2)
{
    return detail::remove_elem_ref(v1) ^
           detail::remove_elem_ref(v2);
}

template <typename T> requires(detail::is_vector_elem_ref_v<T>)
constexpr auto operator<<(T v, int n) -> typename T::value_type
{
    return v.get() << n;
}

template <typename T> requires(detail::is_vector_elem_ref_v<T>)
constexpr auto operator<<(T v, unsigned n) -> typename T::value_type
{
    return v.get() << n;
}

template <typename T> requires(detail::is_vector_elem_ref_v<T>)
constexpr auto operator>>(T v, int n) -> typename T::value_type
{
    return v.get() >> n;
}

template <typename T> requires(detail::is_vector_elem_ref_v<T>)
constexpr auto operator>>(T v, unsigned n) -> typename T::value_type
{
    return v.get() >> n;
}

template <typename T> requires(detail::is_vector_elem_ref_v<T>)
constexpr auto operator~(T v)
{
    return ~v.get();
}

} // namespace aie

#endif // __AIE_API_VECTOR_ELEM_REF__HPP__
