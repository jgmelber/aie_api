// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE_ADF_WINDOW_HPP__
#define __AIE_API_AIE_ADF_WINDOW_HPP__

#include <adf.h>
#include "../aie.hpp"

namespace aie::detail::adf {

template <typename T, unsigned N, aie_dm_resource Resource>
struct window_helper
{
    using vector_type = aie::vector<T, N>;

    __aie_inline
    static void window_write(output_window<T> *w, vector_type value)
    {
#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        if constexpr (vector_type::bits() == 1024) {
            ::window_write<Resource>(w, value.template extract<N / 2>(0));
            ::window_write<Resource>(w, value.template extract<N / 2>(1));

            return;
        }
#endif // __AIE_API_USE_NATIVE_1024B_VECTOR__

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21
        // TODO: CRVO-3196: implement this more efficiently if possible
        if constexpr (vector_type::bits() == 128)
            value.template store<Resource>((typename vector_type::value_type *)w->ptr);
        else
#endif
        ::window_write<Resource>(w, value);
    }

    __aie_inline
    static void window_writeincr(output_window<T> *w, vector_type value)
    {
#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        if constexpr (vector_type::bits() == 1024) {
            ::window_writeincr<Resource>(w, value.template extract<N / 2>(0));
            ::window_writeincr<Resource>(w, value.template extract<N / 2>(1));

            return;
        }
#endif // __AIE_API_USE_NATIVE_1024B_VECTOR__

#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21
        // TODO: CRVO-3196: implement this more efficiently if possible
        if constexpr (vector_type::bits() == 128) {
            value.template store<Resource>((typename vector_type::value_type *)w->ptr);
            ::window_incr(w, value.size());
        }
        else
#endif

        ::window_writeincr<Resource>(w, value);
    }

    __aie_inline
    static vector_type window_read(input_window<T> *w)
    {
#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        if constexpr (vector_type::bits() == 1024) {
            vector_type ret;

            ret.insert(0, window_helper<T, N / 2, Resource>::window_read(w));
            ret.insert(1, window_helper<T, N / 2, Resource>::window_read(w));

            return ret;
        }
        else
#endif // __AIE_API_USE_NATIVE_1024B_VECTOR__
        {
#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21
            // TODO: CRVO-3196: implement this more efficiently if possible
            if constexpr (vector_type::bits() == 128) {
                vector_type ret;

                ret.template load<Resource>((const typename vector_type::value_type *)w->ptr);

                return ret;
            }
            else
#endif
            {
                if constexpr (N == 2)
#if AIE_API_PLATFORM_VERSION == 100
                    return ::window_read_v2<Resource>(w);
#else
                { // TODO: CRVO-3196
                    return vector_type();
                }
#endif
                else if constexpr (N == 4)
                    return ::window_read_v4<Resource>(w);
                else if constexpr (N == 8)
                    return ::window_read_v8<Resource>(w);
                else if constexpr (N == 16)
                    return ::window_read_v16<Resource>(w);
                else if constexpr (N == 32)
                    return ::window_read_v32<Resource>(w);
                else if constexpr (N == 64)
                    return ::window_read_v64<Resource>(w);
                else if constexpr (N == 128)
                    return ::window_read_v128<Resource>(w);
            }
        }
    }

    __aie_inline
    static vector_type window_readincr(input_window<T> *w)
    {
#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        if constexpr (vector_type::bits() == 1024) {
            vector_type ret;

            ret.insert(0, window_helper<T, N / 2, Resource>::window_readincr(w));
            ret.insert(1, window_helper<T, N / 2, Resource>::window_readincr(w));

            return ret;
        }
        else
#endif // __AIE_API_USE_NATIVE_1024B_VECTOR__
        {
#if __AIE_ARCH__ == 20 || __AIE_ARCH__ == 21
            // TODO: CRVO-3196: implement this more efficiently if possible
            if constexpr (vector_type::bits() == 128) {
                vector_type ret;

                ret.template load<Resource>((const typename vector_type::value_type *)w->ptr);
                ::window_incr(w, ret.size());

                return ret;
            }
            else
#endif
            {
                if constexpr (N == 2)
#if AIE_API_PLATFORM_VERSION == 100
                    return ::window_readincr_v2<Resource>(w);
#else
                { // TODO: CRVO-3196
                    ::window_incr(w, 2);
                    return vector_type();
                }
#endif
                else if constexpr (N == 4)
                    return ::window_readincr_v4<Resource>(w);
                else if constexpr (N == 8)
                    return ::window_readincr_v8<Resource>(w);
                else if constexpr (N == 16)
                    return ::window_readincr_v16<Resource>(w);
                else if constexpr (N == 32)
                    return ::window_readincr_v32<Resource>(w);
                else if constexpr (N == 64)
                    return ::window_readincr_v64<Resource>(w);
                else if constexpr (N == 128)
                    return ::window_readincr_v128<Resource>(w);
            }
        }
    }

    __aie_inline
    static vector_type window_readdecr(input_window<T> *w)
    {
#ifndef __AIE_API_USE_NATIVE_1024B_VECTOR__
        if constexpr (vector_type::bits() == 1024) {
            vector_type ret;

            ret.insert(0, window_helper<T, N / 2, Resource>::window_readdecr(w));
            ret.insert(1, window_helper<T, N / 2, Resource>::window_readdecr(w));

            return ret;
        }
        else
#endif // __AIE_API_USE_NATIVE_1024B_VECTOR__
        {
            if constexpr (N == 2)
#if AIE_API_PLATFORM_VERSION == 100
                return ::window_readdecr_v2<Resource>(w);
#else
            { // TODO: CRVO-3196
                ::window_decr(w, 2);
                return vector_type();
            }
#endif
            else if constexpr (N == 4)
                return ::window_readdecr_v4<Resource>(w);
            else if constexpr (N == 8)
                return ::window_readdecr_v8<Resource>(w);
            else if constexpr (N == 16)
                return ::window_readdecr_v16<Resource>(w);
            else if constexpr (N == 32)
                return ::window_readdecr_v32<Resource>(w);
            else if constexpr (N == 64)
                return ::window_readdecr_v64<Resource>(w);
            else if constexpr (N == 128)
                return ::window_readdecr_v128<Resource>(w);
        }
    }

    __aie_inline
    static void window_read(input_window<T> *w, vector_type & value)     { value = window_helper::window_read(w);     }
    __aie_inline
    static void window_readincr(input_window<T> *w, vector_type & value) { value = window_helper::window_readincr(w); }
    __aie_inline
    static void window_readdecr(input_window<T> *w, vector_type & value) { value = window_helper::window_readdecr(w); }
};

}

/**
 * @ingroup group_adf
 *
 * Write a vector into an output window.
 *
 * @param w Output window
 * @param value Vector to be written
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, unsigned N>
__aie_inline
void              window_write      (output_window<T> *w, const aie::vector<T, N> &value) {        aie::detail::adf::window_helper<T, N, Resource>::window_write(w, value);     }

/**
 * @ingroup group_adf
 *
 * Write a vector into an output window and increment the window pointer.
 *
 * @param w Output window
 * @param value Vector to be written
 */
template <aie_dm_resource Resource = aie_dm_resource::none, typename T, unsigned N>
__aie_inline
void              window_writeincr  (output_window<T> *w, const aie::vector<T, N> &value) {        aie::detail::adf::window_helper<T, N, Resource>::window_writeincr(w, value); }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window.
 *
 * @tparam N Size of the vector to be read
 * @param w Input window
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
aie::vector<T, N> window_read_v     ( input_window<T> *w)                                 { return aie::detail::adf::window_helper<T, N, Resource>::window_read(w);             }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window and increment the window pointer.
 *
 * @tparam N Size of the vector to be read
 * @param w Input window
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
aie::vector<T, N> window_readincr_v ( input_window<T> *w)                                 { return aie::detail::adf::window_helper<T, N, Resource>::window_readincr(w);         }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window and decrement the window pointer.
 *
 * @tparam N Size of the vector to be read
 * @param w Input window
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
aie::vector<T, N> window_readdecr_v ( input_window<T> *w)                                 { return aie::detail::adf::window_helper<T, N, Resource>::window_readdecr(w);         }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window.
 *
 * @param w Input window
 * @param value Output parameter that contains the read vector
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
void              window_read_v     ( input_window<T> *w, aie::vector<T, N> &value)       {        aie::detail::adf::window_helper<T, N, Resource>::window_read(w, value);      }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window and increment the window pointer.
 *
 * @param w Input window
 * @param value Output parameter that contains the read vector
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
void              window_readincr_v ( input_window<T> *w, aie::vector<T, N> &value)       {        aie::detail::adf::window_helper<T, N, Resource>::window_readincr(w, value);  }

/**
 * @ingroup group_adf
 *
 * Read a vector from an input window and decrement the window pointer.
 *
 * @param w Input window
 * @param value Output parameter that contains the read vector
 */
template <unsigned N, aie_dm_resource Resource = aie_dm_resource::none, typename T>
__aie_inline
void              window_readdecr_v ( input_window<T> *w, aie::vector<T, N> &value)       {        aie::detail::adf::window_helper<T, N, Resource>::window_readdecr(w, value);  }

#endif
