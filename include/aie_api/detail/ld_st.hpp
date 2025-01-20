// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_LD_ST__HPP__
#define __AIE_API_DETAIL_LD_ST__HPP__

#include "accum.hpp"
#include "vector.hpp"

namespace aie::detail {

static constexpr unsigned vector_decl_align = AIE_VECTOR_LDST_ALIGN;

#if __AIE_ARCH__ == 10

template <typename T, unsigned Elems>
struct vector_ldst_align
{
    static constexpr unsigned value = 16;
};

#elif __AIE_ARCH__ == 20

template <typename T, unsigned Elems>
struct vector_ldst_align
{
    static constexpr unsigned value = (detail::type_bits_v<T> * Elems == 128)? 16 : 32;
};

#elif __AIE_ARCH__ == 21

template <typename T, unsigned Elems>
struct vector_ldst_align
{
    static constexpr unsigned value = (detail::type_bits_v<T> * Elems == 128)? 16 :
                                      (detail::type_bits_v<T> * Elems == 256)? 32 :
                                      64;
};

#endif

template <typename T, unsigned Elems>
static constexpr unsigned vector_ldst_align_v = vector_ldst_align<T, Elems>::value;

namespace utils {

// This function is used to mimic hardware pointer semantics on Native
// i.e. truncate the pointer to perform an aligned load.
//
// Default behaviour is to truncate the pointer to the native load size.
// Passing a number of elements as a template parameter can be used to
// truncate the pointer as required for an aligned load of the given size.
template <unsigned Elems, typename T>
__aie_inline
constexpr T *floor_ptr(T *ptr)
{
#if AIE_API_NATIVE == 1
    constexpr uintptr_t mask = ~(vector_ldst_align_v<T, Elems> - 1);

    return (T *)((uintptr_t)ptr & mask);
#else
    // Rely on automatic HW truncation for enhanced performance
    return ptr;
#endif
}

template <typename T>
__aie_inline
constexpr T *floor_ptr(T *ptr)
{
    return floor_ptr<native_vector_length_v<T>, T>(ptr);
}

}

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct load_vector_helper
{
    using vector_type = vector<aie_dm_resource_remove_t<T>, Elems>;

    __aie_inline
    static vector_type run(const T *ptr)
    {
        vector_type ret;

#ifdef AIE_API_EMULATION
        for (unsigned i = 0; i < Elems; ++i)
            ret.data[i] = ptr[i];
#else
        ret.template load<Resource>(ptr);
#endif

        return ret;
    }
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct store_vector_helper
{
    using vector_type = vector<aie_dm_resource_remove_t<T>, Elems>;

    __aie_inline
    static T *run(T *ptr, const vector_type &v)
    {
#ifdef AIE_API_EMULATION
        for (unsigned i = 0; i < Elems; ++i)
            ptr[i] = v.data[i];
#else
        v.template store<Resource>(ptr);
#endif

        return ptr;
    }
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct load_unaligned_vector_helper
{
    using vector_type = vector<aie_dm_resource_remove_t<T>, Elems>;

    __aie_inline
    static vector_type run(const T *ptr, unsigned aligned_elems)
    {
        vector_type ret;

#ifdef AIE_API_EMULATION
        for (unsigned i = 0; i < Elems; ++i)
            ret.data[i] = ptr[i];
#else
        ret.template load_unaligned<Resource>(ptr, aligned_elems);
#endif

        return ret;
    }
};

template <typename T, unsigned Elems, aie_dm_resource Resource>
struct store_unaligned_vector_helper
{
    using vector_type = vector<aie_dm_resource_remove_t<T>, Elems>;

    __aie_inline
    static T *run(T *ptr, const vector_type &v, unsigned aligned_elems)
    {
#ifdef AIE_API_EMULATION
        for (unsigned i = 0; i < Elems; ++i)
            ptr[i] = v.data[i];
#else
        v.template store_unaligned<Resource>(ptr, aligned_elems);
#endif

        return ptr;
    }
};

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
auto load_vector(const T *ptr)
{
    return load_vector_helper<T, Elems, Resource>::run(ptr);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
auto load_unaligned_vector(const T *ptr, unsigned aligned_elems = 1)
{
    return load_unaligned_vector_helper<T, Elems, Resource>::run(ptr, aligned_elems);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T1, typename T2>
__aie_inline
T1 *store_vector(T1 *ptr, const vector<T2, Elems> &v)
{
    return store_vector_helper<T1, Elems, Resource>::run(ptr, v);
}

template <unsigned Elems, aie_dm_resource Resource = aie_dm_resource::none, typename T1, typename T2>
__aie_inline
T1 *store_unaligned_vector(T1 *ptr, const vector<T2, Elems> &v, unsigned aligned_elems = 1)
{
    return store_unaligned_vector_helper<T1, Elems, Resource>::run(ptr, v, aligned_elems);
}

template <unsigned Elems, typename T>
__attribute__((pure))
constexpr bool check_vector_alignment(const T *ptr)
{
    return (uintptr_t(ptr) % vector_ldst_align_v<aie_dm_resource_remove_t<T>, Elems>) == 0;
}

template <typename T>
__attribute__((pure))
constexpr bool check_vector_alignment(const T *ptr)
{
    return check_vector_alignment<native_vector_length_v<T>, T>(ptr);
}

template <typename T>
__attribute__((pure))
constexpr bool check_alignment(const T *ptr, unsigned aligned_elems)
{
    return (uintptr_t(ptr) % ((detail::type_bits_v<aie_dm_resource_remove_t<T>> * aligned_elems) / 8)) == 0;
}

}

#endif
