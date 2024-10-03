// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_AIE_ADF_STREAM_HPP__
#define __AIE_API_AIE_ADF_STREAM_HPP__

#include <adf.h>
#include "../aie.hpp"

namespace aie::detail::adf {

template <unsigned N, typename T>
struct stream_helper_common
{
    using type = aie::vector<T, N>;

#if __AIE_ARCH__ == 10
    static constexpr unsigned stream_width = 128;
#elif __AIE_ARCH__ == 20
    static constexpr unsigned stream_width = std::min(512u, type::bits());
#endif

    static_assert(type::bits() >= stream_width);
    static_assert(type::bits() % stream_width == 0);

    static constexpr unsigned num_ops = type::bits() / stream_width;
    static constexpr unsigned elems_per_op = N / num_ops;
};

template <unsigned N, typename T, aie_stream_resource_in Resource>
struct stream_in_helper : public stream_helper_common<N, T>
{
    using type = typename stream_in_helper::type;

    static constexpr auto get_op()
    {
        if      constexpr (stream_in_helper::elems_per_op ==   2) return [](auto&&... args) __aie_inline { return ::readincr_v2<Resource>  (std::forward<decltype(args)>(args)...); };
        else if constexpr (stream_in_helper::elems_per_op ==   4) return [](auto&&... args) __aie_inline { return ::readincr_v4<Resource>  (std::forward<decltype(args)>(args)...); };
        else if constexpr (stream_in_helper::elems_per_op ==   8) return [](auto&&... args) __aie_inline { return ::readincr_v8<Resource>  (std::forward<decltype(args)>(args)...); };
        else if constexpr (stream_in_helper::elems_per_op ==  16) return [](auto&&... args) __aie_inline { return ::readincr_v16<Resource> (std::forward<decltype(args)>(args)...); };
#if __AIE_ARCH__ == 20
        else if constexpr (stream_in_helper::elems_per_op ==  32) return [](auto&&... args) __aie_inline { return ::readincr_v32<Resource> (std::forward<decltype(args)>(args)...); };
        else if constexpr (stream_in_helper::elems_per_op ==  64) return [](auto&&... args) __aie_inline { return ::readincr_v64<Resource> (std::forward<decltype(args)>(args)...); };
        else if constexpr (stream_in_helper::elems_per_op == 128) return [](auto&&... args) __aie_inline { return ::readincr_v128<Resource>(std::forward<decltype(args)>(args)...); };
#endif
        else UNREACHABLE_MSG("Unhandled case\n");
    }

    __aie_inline
    static type readincr(input_stream<T> *w)
    {
        type ret;

#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<T, cfloat>) {
            aie::vector<float, N * 2> tmp = stream_in_helper<N * 2, float, Resource>::readincr((input_stream<float> *)w);

            ret = tmp.template cast_to<cfloat>();
        }
        else
#endif
        {
            utils::unroll_times<stream_in_helper::num_ops>([&](auto idx) __aie_inline {
                aie::vector<T, stream_in_helper::elems_per_op> tmp;

                tmp = get_op()(w);

                ret.insert(idx, tmp);
            });
        }

        return ret;
    }

    __aie_inline
    static type readincr(input_stream<T> *w, bool &tlast)
    {
        type ret;

#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<T, cfloat>) {
            aie::vector<float, N * 2> tmp = stream_in_helper<N * 2, float, Resource>::readincr((input_stream<float> *)w, tlast);

            ret = tmp.template cast_to<cfloat>();
        }
        else
#endif
        {
            utils::unroll_times<stream_in_helper::num_ops>([&](auto idx) __aie_inline {
                aie::vector<T, stream_in_helper::elems_per_op> tmp;

                if (idx < stream_in_helper::num_ops - 1)
                    tmp = get_op()(w);
                else
                    tmp = get_op()(w, tlast);

                ret.insert(idx, tmp);
            });
        }

        return ret;
    }
};

template <unsigned N, typename T, aie_stream_resource_out Resource>
struct stream_out_helper : public stream_helper_common<N, T>
{
    using type = typename stream_out_helper::type;

    __aie_inline
    static void writeincr(output_stream<T> *w, type value)
    {
#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<T, cfloat>) {
            stream_out_helper<N * 2, float, Resource>::writeincr((output_stream<float> *)w, value.template cast_to<float>());
        }
        else
#endif
        {
            #pragma unroll
            for (unsigned i = 0; i < stream_out_helper::num_ops; ++i)
                ::writeincr<Resource>(w, value.template extract<stream_out_helper::elems_per_op>(i));
        }
    }

    __aie_inline
    static void writeincr(output_stream<T> *w, type value, bool tlast)
    {
#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<T, cfloat>) {
            stream_out_helper<N * 2, float, Resource>::writeincr((output_stream<float> *)w, value.template cast_to<float>(), tlast);
        }
        else
#endif
        {
            #pragma unroll
            for (unsigned i = 0; i < stream_out_helper::num_ops; ++i) {
                if (i < stream_out_helper::num_ops - 1)
                    ::writeincr<Resource>(w, value.template extract<stream_out_helper::elems_per_op>(i));
                else
                    ::writeincr<Resource>(w, value.template extract<stream_out_helper::elems_per_op>(i), tlast);
            }
        }
    }
};

template <typename AccumTag, unsigned N>
struct cascade_stream_helper
{
    using native_tag = aie::detail::accum_native_type_t<AccumTag>;
    using       type = aie::accum<native_tag, N>;

#if __AIE_ARCH__ == 10
    static constexpr unsigned compute_num_ops()
    {
        if      constexpr (type::accum_bits() % 48 == 0) // acc48/cacc48
            return type::bits() / 384;
        else if constexpr (type::accum_bits() % 80 == 0) // acc80/cacc80
            return type::bits() / 320;
    }

    static constexpr unsigned             num_ops = compute_num_ops();
    static constexpr unsigned        elems_per_op = N / num_ops;
    static constexpr unsigned native_elems_per_op = elems_per_op;
#elif __AIE_ARCH__ == 20
    static constexpr unsigned compute_native_elems_per_op()
    {
        if      constexpr (type::value_bits() == 32)
            return 16;
        else if constexpr (type::value_bits() == 64)
            return 8;
        else if constexpr (type::value_bits() == 128)
            return 4;
    }

    static constexpr unsigned       cascade_width = 512;
    static constexpr unsigned             num_ops = cascade_width > type::bits()? 1 : type::bits() / cascade_width;
    static constexpr unsigned        elems_per_op = N / num_ops;
    static constexpr unsigned native_elems_per_op = compute_native_elems_per_op();
#endif

    __aie_inline
    static void writeincr(output_stream<AccumTag> *_w, type value)
    {
        output_cascade<native_tag> *w = (output_cascade<native_tag> *) _w;
        cascade_stream_helper<native_tag, N>::writeincr(w, value);
    }

    __aie_inline
    static void writeincr(output_cascade<AccumTag> *_w, type value)
    {
        output_cascade<native_tag> *w = (output_cascade<native_tag> *) _w;

        if constexpr (native_elems_per_op > elems_per_op) {
            ::writeincr(w, value.template grow<native_elems_per_op>());
        }
        else {
            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, value.template extract<elems_per_op>(i));
        }
    }

    __aie_inline
    static type readincr(input_stream<AccumTag> *_w)
    {
        input_cascade<native_tag> *w = (input_cascade<native_tag> *) _w;
        return cascade_stream_helper<native_tag, N>::readincr(w);
    }

    __aie_inline
    static type readincr(input_cascade<AccumTag> *_w)
    {
        input_cascade<native_tag> *w = (input_cascade<native_tag> *) _w;
        type ret;

#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<AccumTag, caccfloat>) {
            aie::accum<accfloat, N * 2> tmp = cascade_stream_helper<accfloat, N * 2>::readincr((input_cascade<accfloat> *)w);

            ret = tmp.template cast_to<caccfloat>();
        }
        else
#endif
        {
            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i) {
#if __AIE_ARCH__ == 10
                if      constexpr (elems_per_op == 2)
                    ret.template insert<2>(i, readincr_v2(w));
                else if constexpr (elems_per_op == 4)
                    ret.template insert<4>(i, readincr_v4(w));
                else if constexpr (elems_per_op == 8)
                    ret.template insert<8>(i, readincr_v8(w));
#elif __AIE_ARCH__ == 20
                if      constexpr (native_elems_per_op == 4) {
                    const aie::accum<native_tag, 4> tmp = readincr_v4(w);

                    if constexpr (native_elems_per_op > elems_per_op)
                        ret.template insert<N>(i, tmp.template extract<N>(0));
                    else
                        ret.template insert<4>(i, tmp);
                }
                else if      constexpr (native_elems_per_op == 8) {
                    const aie::accum<native_tag, 8> tmp = readincr_v8(w);

                    if constexpr (native_elems_per_op > elems_per_op)
                        ret.template insert<N>(i, tmp.template extract<N>(0));
                    else
                        ret.template insert<8>(i, tmp);
                }
                else if constexpr (native_elems_per_op == 16) {
                    const aie::accum<native_tag, 16> tmp = readincr_v16(w);

                    if constexpr (native_elems_per_op > elems_per_op)
                        ret.template insert<N>(i, tmp.template extract<N>(0));
                    else
                        ret.template insert<16>(i, tmp);
                }
#endif
            }
        }

        return ret;
    }
};

#if __AIE_ARCH__ == 10
template <unsigned N>
struct cascade_stream_helper<accfloat, N>
{
    using AccumTag = accfloat;
    using     type = aie::accum<AccumTag, N>;

    static constexpr unsigned       num_ops = (type::bits() < 256)? 1 : type::bits() / 256;
    static constexpr unsigned  elems_per_op = N / num_ops;

    __aie_inline
    static void writeincr(output_stream<AccumTag> *_w, type value)
    {
        output_cascade<accfloat> *w = (output_cascade<accfloat> *) _w;
        writeincr(w, value);
    }

    __aie_inline
    static type readincr(input_stream<AccumTag> *_w)
    {
        input_cascade<accfloat> *w = (input_cascade<accfloat> *) _w;
        return readincr(w);
    }

    __aie_inline
    static void writeincr(output_cascade<AccumTag> *_w, type value)
    {
        if constexpr (type::bits() < 256) {
            cascade_stream_helper<AccumTag, N * 2>::writeincr(_w, value.template grow<N * 2>());
        }
        else {
            // TODO: use accfloat when the corresponding writeincr overload is added
            output_cascade<float> *w = (output_cascade<float> *) _w;

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, value.template extract<elems_per_op>(i));
        }
    }

    __aie_inline
    static type readincr(input_cascade<AccumTag> *_w)
    {
        if constexpr (type::bits() < 256) {
            return cascade_stream_helper<AccumTag, N * 2>::readincr(_w).template extract<N>(0);
        }
        else {
            // TODO: use accfloat when the corresponding readincr overload is added
            input_cascade<float> *w = (input_cascade<float> *) _w;
            type ret;

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ret.template insert<8>(i, ::readincr_v8(w));

            return ret;
        }
    }
};

template <unsigned N>
struct cascade_stream_helper<caccfloat, N>
{
    using AccumTag = caccfloat;
    using     type = aie::accum<AccumTag, N>;

    static constexpr unsigned       num_ops = (type::bits() < 256)? 1 : type::bits() / 256;
    static constexpr unsigned  elems_per_op = N / num_ops;

    __aie_inline
    static void writeincr(output_stream<AccumTag> *_w, type value)
    {
        output_cascade<caccfloat> *w = (output_cascade<caccfloat> *) _w;
        writeincr(w, value);
    }

    __aie_inline
    static type readincr(input_stream<AccumTag> *_w)
    {
        input_cascade<caccfloat> *w = (input_cascade<caccfloat> *) _w;
        return readincr(w);
    }

    __aie_inline
    static void writeincr(output_cascade<AccumTag> *_w, type value)
    {
        if constexpr (type::bits() < 256) {
            cascade_stream_helper<AccumTag, N * 2>::writeincr(_w, value.template grow<N * 2>());
        }
        else {
            // TODO: use caccfloat when the corresponding writeincr overload is added
#if AIE_API_NATIVE == 1
            // TODO: error in cfloat overload on Native
            output_cascade<float> *w = (output_cascade<float> *) _w;

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, value.template extract<elems_per_op>(i).template to_vector<cfloat>().template cast_to<float>().to_native());
#else
            output_cascade<cfloat> *w = (output_cascade<cfloat> *) _w;

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, value.template extract<elems_per_op>(i));
#endif
        }
    }

    __aie_inline
    static type readincr(input_cascade<AccumTag> *_w)
    {
        if constexpr (type::bits() < 256) {
            return cascade_stream_helper<AccumTag, N * 2>::readincr(_w).template extract<N>(0);
        }
        else {
            // TODO: use caccfloat when the corresponding readincr overload is added
            input_cascade<cfloat> *w = (input_cascade<cfloat> *) _w;
            type ret;

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ret.template insert<4>(i, ::readincr_v4(w));

            return ret;
        }
    }
};

#endif

template <typename T, unsigned N>
struct vector_cascade_stream_helper
{
    using type = aie::vector<T, N>;

#if __AIE_ARCH__ == 10
    static constexpr unsigned cascade_width = 256;
#elif __AIE_ARCH__ == 20
    static constexpr unsigned cascade_width = 512;
#endif

    static constexpr unsigned             num_ops = cascade_width > type::bits()? 1 : type::bits() / cascade_width;
    static constexpr unsigned        elems_per_op = N / num_ops;
    static constexpr unsigned native_elems_per_op = cascade_width / type_bits_v<T>;

    __aie_inline
    static void writeincr(output_stream<T> *_w, type value)
    {
        output_cascade<T> *w = (output_cascade<T> *) _w;
        vector_cascade_stream_helper<T, N>::writeincr(w, value);
    }

    __aie_inline
    static void writeincr(output_cascade<T> *w, type value)
    {
        if constexpr (native_elems_per_op > elems_per_op) {
            ::writeincr(w, value.template grow<native_elems_per_op>());
        }
        else {
            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, value.template extract<elems_per_op>(i));
        }
    }

    __aie_inline
    static type readincr(input_stream<T> *_w)
    {
        input_cascade<T> *w = (input_cascade<T> *) _w;
        return vector_cascade_stream_helper<T, N>::readincr(w);
    }

    __aie_inline
    static type readincr(input_cascade<T> *w)
    {
        type ret;

#if __AIE_API_COMPLEX_FP32_EMULATION__
        if constexpr (std::is_same_v<T, cfloat>) {
            aie::vector<float, N * 2> tmp = vector_cascade_stream_helper<float, N * 2>::readincr((input_cascade<float> *)w);

            ret = tmp.template cast_to<cfloat>();
        }
        else
#endif
        {
            auto read_op = [&](){
#if __AIE_ARCH__ == 10
                if      constexpr (native_elems_per_op ==   4) { return [](auto* w){ return readincr_v4(w);   }; }
                else if constexpr (native_elems_per_op ==   8) { return [](auto* w){ return readincr_v8(w);   }; }
                else if constexpr (native_elems_per_op ==  16) { return [](auto* w){ return readincr_v16(w);  }; }
                else if constexpr (native_elems_per_op ==  32) { return [](auto* w){ return readincr_v32(w);  }; }
#elif __AIE_ARCH__ == 20 
                if      constexpr (native_elems_per_op ==   8) { return [](auto* w){ return readincr_v8(w);   }; }
                else if constexpr (native_elems_per_op ==  16) { return [](auto* w){ return readincr_v16(w);  }; }
                else if constexpr (native_elems_per_op ==  32) { return [](auto* w){ return readincr_v32(w);  }; }
                else if constexpr (native_elems_per_op ==  64) { return [](auto* w){ return readincr_v64(w);  }; }
                else if constexpr (native_elems_per_op == 128) { return [](auto* w){ return readincr_v128(w); }; }
                else if constexpr (native_elems_per_op == 256) { return [](auto* w){ return readincr_v256(w); }; }
#endif
            }();

            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i) {
                const aie::vector<T, native_elems_per_op> tmp = read_op(w);

                if constexpr (native_elems_per_op > elems_per_op)
                    ret.template insert<N>(i, tmp.template extract<N>(0));
                else
                    ret.template insert<native_elems_per_op>(i, tmp);
            }
        }

        return ret;
    }
};

#if __AIE_API_FP32_EMULATION__
template <unsigned N>
struct vector_cascade_stream_helper<float, N>
{
    using          T = float;
    using       type = aie::vector<T, N>;
    using accum_type = aie::accum<accfloat, N>;

    static constexpr unsigned       cascade_width = 512;
    static constexpr unsigned             num_ops = cascade_width > type::bits()? 1 : type::bits() / cascade_width;
    static constexpr unsigned        elems_per_op = N / num_ops;
    static constexpr unsigned native_elems_per_op = cascade_width / type_bits_v<T>;

    __aie_inline
    static void writeincr(output_stream<T> *_w, type value)
    {
        output_cascade<T> *w = (output_cascade<T> *) _w;
        vector_cascade_stream_helper<T, N>::writeincr(w, value);
    }

    __aie_inline
    static void writeincr(output_cascade<T> *_w, type value)
    {
        output_cascade<accfloat> *w = (output_cascade<accfloat> *) _w;
        accum_type acc(value);

        if constexpr (native_elems_per_op > elems_per_op) {
            ::writeincr(w, acc.template grow<native_elems_per_op>());
        }
        else {
            #pragma unroll
            for (unsigned i = 0; i < num_ops; ++i)
                ::writeincr(w, acc.template extract<elems_per_op>(i));
        }
    }

    __aie_inline
    static type readincr(input_stream<T> *_w)
    {
        input_cascade<T> *w = (input_cascade<T> *) _w;
        return vector_cascade_stream_helper<T, N>::readincr(w);
    }

    __aie_inline
    static type readincr(input_cascade<T> *w)
    {
        type ret;

        #pragma unroll
        for (unsigned i = 0; i < num_ops; ++i) {
            const aie::accum<accfloat, native_elems_per_op> tmp_acc = ::readincr_v16((input_cascade<accfloat> *)w);
            const aie::vector<float, native_elems_per_op> tmp = tmp_acc.template to_vector<float>();

            if constexpr (native_elems_per_op > elems_per_op)
                ret.template insert<N>(i, tmp.template extract<N>(0));
            else
                ret.template insert<native_elems_per_op>(i, tmp);
        }

        return ret;
    }
};

#if __AIE_API_COMPLEX_FP32_EMULATION__
template <unsigned N>
struct vector_cascade_stream_helper<cfloat, N>
{
    using    T = cfloat;
    using type = aie::vector<T, N>;

    static constexpr unsigned       cascade_width = 512;
    static constexpr unsigned             num_ops = cascade_width > type::bits()? 1 : type::bits() / cascade_width;
    static constexpr unsigned        elems_per_op = N / num_ops;
    static constexpr unsigned native_elems_per_op = cascade_width / type_bits_v<T>;

    __aie_inline
    static void writeincr(output_stream<T> *_w, type value)
    {
        output_cascade<T> *w = (output_cascade<T> *) _w;
        vector_cascade_stream_helper<T, N>::writeincr(w, value);
    }

    __aie_inline
    static void writeincr(output_cascade<T> *w, type value)
    {
        vector_cascade_stream_helper<float, N * 2>::writeincr((output_cascade<float> *)w, value.template cast_to<float>());
    }

    __aie_inline
    static type readincr(input_stream<T> *_w)
    {
        input_cascade<T> *w = (input_cascade<T> *) _w;
        return vector_cascade_stream_helper<T, N>::readincr(w);
    }

    __aie_inline
    static type readincr(input_cascade<T> *w)
    {
        aie::vector<float, N * 2> tmp = vector_cascade_stream_helper<float, N * 2>::readincr((input_cascade<float> *)w);

        return tmp.template cast_to<cfloat>();
    }
};
#endif

#endif
}

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output AXI stream. TLAST is not asserted.
 *
 * @tparam Resource Stream out resource. This information can be leveraged by the compiler to schedule stream writes in parallel when possible
 * @param w Output stream
 * @param value Vector to be written
 */
template <aie_stream_resource_out Resource = aie_stream_resource_out::none, typename T, unsigned N>
__aie_inline
void              writeincr (output_stream<T> *w, const aie::vector<T, N> &value)             {        aie::detail::adf::stream_out_helper<N, T, Resource>::writeincr(w, value);        }
/**
 * @ingroup group_adf
 *
 * Writes a vector into the output AXI stream. An additional argument says whether TLAST is asserted.
 *
 * @tparam Resource Stream out resource. This information can be leveraged by the compiler to schedule stream writes in parallel when possible
 * @param w Output stream
 * @param value Vector to be written
 * @param tlast TLAST is asserted if true
 */
template <aie_stream_resource_out Resource = aie_stream_resource_out::none, typename T, unsigned N>
__aie_inline
void              writeincr (output_stream<T> *w, const aie::vector<T, N> &value, bool tlast) {        aie::detail::adf::stream_out_helper<N, T, Resource>::writeincr(w, value, tlast); }

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input AXI stream.
 *
 * @tparam N Size of the vector to be read
 * @tparam Resource Stream in resource. This information can be leveraged by the compiler to schedule stream reads in parallel when possible
 * @param w Input stream
 */
template <unsigned N, aie_stream_resource_in Resource = aie_stream_resource_in::none, typename T>
__aie_inline
aie::vector<T, N> readincr_v( input_stream<T> *w)                                             { return aie::detail::adf::stream_in_helper<N, T, Resource>::readincr(w);                 }

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input AXI stream and returns whether TLAST has been asserted.
 *
 * @tparam N        Size of the vector to be read
 * @tparam Resource Stream in resource. This information can be leveraged by the compiler to schedule stream reads in parallel when possible
 * @param w     Input stream
 * @param tlast Output argument that tells whether TLAST has been asserted.
 */
template <unsigned N, aie_stream_resource_in Resource = aie_stream_resource_in::none, typename T>
__aie_inline
aie::vector<T, N> readincr_v( input_stream<T> *w, bool &tlast)                                { return aie::detail::adf::stream_in_helper<N, T, Resource>::readincr(w, tlast);          }

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream.
 *
 * @param w Output stream
 * @param value Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
[[deprecated("Use output_cascade instead")]]
__aie_inline
void              writeincr(output_stream<T> *w,  const aie::accum<T, N> &value)              {        aie::detail::adf::cascade_stream_helper< T, N>::writeincr(w, value);   }

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w Input stream
 */
template <unsigned N, aie::AccumElemBaseType T>
[[deprecated("Use input_cascade instead")]]
__aie_inline
aie::accum<T, N>  readincr_v( input_stream<T> *w)                                             { return aie::detail::adf::cascade_stream_helper<T, N>::readincr(w);            }

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output cascade stream.
 *
 * @param w Output cascade stream
 * @param value Vector to be written
 */
template <aie::ElemBaseType T, unsigned N>
__aie_inline
void              writeincr(output_cascade<T> *w,  const aie::vector<T, N> &value)             {        aie::detail::adf::vector_cascade_stream_helper< T, N>::writeincr(w, value);   }

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream.
 *
 * @param w Output cascade stream
 * @param value Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
__aie_inline
void              writeincr(output_cascade<T> *w,  const aie::accum<T, N> &value)             {        aie::detail::adf::cascade_stream_helper< T, N>::writeincr(w, value);   }

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input cascade stream.
 *
 * @param w Input cascade stream
 */
template <unsigned N, aie::ElemBaseType T>
__aie_inline
aie::vector<T, N>  readincr_v( input_cascade<T> *w)                                            { return aie::detail::adf::vector_cascade_stream_helper<T, N>::readincr(w);            }

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w Input cascade stream
 */
template <unsigned N, aie::AccumElemBaseType T>
__aie_inline
aie::accum<T, N>  readincr_v( input_cascade<T> *w)                                            { return aie::detail::adf::cascade_stream_helper<T, N>::readincr(w);            }

template <unsigned N, typename T>
[[deprecated("Use writeincr instead")]]
__aie_inline
void writeincr_v(output_stream<T> *w, const aie::vector<T, N> &value)                         { writeincr(w, value); }

template <unsigned N, typename T>
[[deprecated("Use writeincr instead")]]
__aie_inline
void writeincr_v(output_stream<T> *w, const aie::vector<T, N> &value, bool tlast)             { writeincr(w, value); }

template <aie::AccumElemBaseType T, unsigned N> [[deprecated("Use writeincr instead")]]
__aie_inline
void writeincr_v(output_stream<T> *w,  const aie::accum<T, N> &value)                         { writeincr(w, value); }

namespace aie {

/**
 * @ingroup group_adf
 *
 * Type used to wrap objects read/written from/to streams that also encapsulates TLAST information. When passed into
 * output AXI streams, this object will say whether TLAST is asserted after the write operation. When read from input
 * AXI streams, the object will say whether TLAST was asserted by the writer.
 */
template <typename T, typename Bool>
struct tlast
{
    using value_type = T;

    T value;
    Bool t_last;

    tlast(const tlast &) = delete;
    tlast &operator=(const tlast &) = delete;

    /**
     * Constructor used to initialize the object. For input AXI streams, both arguments are references to the variables
     * where to store the read value and TLAST. For output AXI streams, these are the value and TLAST to be written.
     */
    __aie_inline
    constexpr tlast(T value, Bool t_last) : value(value), t_last(t_last)
    {
    }
};

// Constructor deduction guides

// For input AXI streams, return an object that stores the references to the variables where to store the read value
// and TLAST
template <typename T>
tlast(T &, bool &) -> tlast<T &, bool &>;

// For output AXI streams, return an object that stores the value and TLAST to be written on the stream
template <typename T>
tlast(const T &, bool) -> tlast<const T &, bool>;

namespace detail {

// Helpers to determine whether a type is a tlast object

template <typename T>
struct is_tlast
{
    static constexpr bool value = false;
};

template <typename T, typename Bool>
struct is_tlast<tlast<T, Bool>>
{
    static constexpr bool value = true;
};

// Determine whether T is a tlast object
template <typename T>
static constexpr bool is_tlast_v = is_tlast<T>::value;

template <typename T, typename T2>
struct is_tlast_type
{
    static constexpr bool value = false;
};

template <typename T, typename Bool, typename T2>
struct is_tlast_type<tlast<T, Bool>, T2>
{
    static constexpr bool value = std::is_same_v<detail::utils::remove_all_t<T>, T2>;
};

template <typename T, unsigned N, typename Bool, typename T2>
struct is_tlast_type<tlast<vector<T, N> &, Bool>, T2>
{
    static constexpr bool value = std::is_same_v<T, T2>;
};

template <typename T, unsigned N, typename Bool, typename T2>
struct is_tlast_type<tlast<const vector<T, N> &, Bool>, T2>
{
    static constexpr bool value = std::is_same_v<T, T2>;
};

// Determine whether T is a tlast object of values of type T2. This includes both T2 values or vectors of T2
template <typename T, typename T2>
static constexpr bool is_tlast_type_v = is_tlast_type<T, T2>::value;

}

/**
 * @ingroup group_adf
 *
 * @concept aie::TLast
 * Concept for tlast objects passed to << and >> operators with streams.
 */
template <typename T, typename T2>
concept TLast = detail::is_tlast_v<T> && detail::is_tlast_type_v<detail::utils::remove_all_t<T>, T2>;

}

/**
 * @ingroup group_adf
 *
 * Writes a value into the output AXI stream. TLAST is not asserted.
 *
 * @param w Output stream
 * @param v Value to be written
 */
template <typename T>
__aie_inline
constexpr output_stream<T> &operator<<(output_stream<T> &w, const T &v)
{
    if constexpr (aie::detail::type_bits_v<T> < 32) {
        if constexpr (aie::detail::is_floating_point_v<T>) {
            float tmp = v;
            writeincr((output_stream<float> *)&w, tmp);
        }
        else {
            int tmp = v;
            writeincr((output_stream<int> *)&w, tmp);
        }
    }
    else {
        writeincr(&w, v);
    }

    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a value or vector into the output AXI stream. The tlast object says whether TLAST is asserted.
 *
 * @param w  Output stream
 * @param tl Object that contains the value or vector to be written and whether TLAST is asserted.
 */
template <typename T, aie::TLast<T> TL>
__aie_inline
constexpr output_stream<T> &operator<<(output_stream<T> &w, const TL &tl)
{
    using value_type = aie::detail::utils::remove_all_t<typename TL::value_type>;

    if constexpr (aie::detail::is_vector_v<value_type>) {
        writeincr(&w, tl.value, tl.t_last);
    }
    else {
        if constexpr (aie::detail::type_bits_v<T> < 32) {
            if constexpr (aie::detail::is_floating_point_v<T>) {
                float tmp = tl.value;
                writeincr((output_stream<float> *)&w, tmp, tl.t_last);
            }
            else {
                int tmp = tl.value;
                writeincr((output_stream<int> *)&w, tmp, tl.t_last);
            }
        }
        else {
            writeincr(&w, tl.value, tl.t_last);
        }
    }

    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a value into the output AXI stream. TLAST is not asserted.
 *
 * @param w Output stream
 * @param v Value to be written
 */
template <typename T>
__aie_inline
constexpr output_stream<T> &operator<<(output_stream<T> *w, const T &v)
{
    *w << v;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a value into the output AXI stream. The tlast object says whether TLAST is asserted.
 *
 * @param w  Output stream
 * @param tl Object that contains the value to be written and whether TLAST is asserted.
 */
template <typename T, aie::TLast<T> TL>
__aie_inline
constexpr output_stream<T> *operator<<(output_stream<T> *w, const TL &tl)
{
    *w << tl;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output AXI stream. TLAST is not asserted.
 *
 * @param w Output stream
 * @param v Vector to be written
 */
template <typename T, unsigned N>
__aie_inline
constexpr output_stream<T> &operator<<(output_stream<T> &w, const aie::vector<T, N> &v)
{
    writeincr(&w, v);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output AXI stream. TLAST is not asserted.
 *
 * @param w Output stream
 * @param v Vector to be written
 */
template <typename T, unsigned N>
__aie_inline
constexpr output_stream<T> *operator<<(output_stream<T> *w, const aie::vector<T, N> &v)
{
    *w << v;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream
 *
 * @param w   Output stream
 * @param acc Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
[[deprecated("Use output_cascade instead")]]
__aie_inline
constexpr output_stream<T> *operator<<(output_stream<T> *w, const aie::accum<T, N> &acc)
{
    writeincr(w, acc);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream
 *
 * @param w   Output stream
 * @param acc Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
[[deprecated("Use output_cascade instead")]]
__aie_inline
constexpr output_stream<T> &operator<<(output_stream<T> &w, const aie::accum<T, N> &acc)
{
    writeincr(&w, acc);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output cascade stream
 *
 * @param w   Output cascade stream
 * @param v   Vector to be written
 */
template <aie::ElemBaseType T, unsigned N>
__aie_inline
constexpr output_cascade<T> *operator<<(output_cascade<T> *w, const aie::vector<T, N> &v)
{
    writeincr(w, v);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes a vector into the output cascade stream
 *
 * @param w   Output cascade stream
 * @param v   Vector to be written
 */
template <aie::ElemBaseType T, unsigned N>
__aie_inline
constexpr output_cascade<T> &operator<<(output_cascade<T> &w, const aie::vector<T, N> &v)
{
    writeincr(&w, v);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream
 *
 * @param w   Output cascade stream
 * @param acc Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
__aie_inline
constexpr output_cascade<T> *operator<<(output_cascade<T> *w, const aie::accum<T, N> &acc)
{
    writeincr(w, acc);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Writes an accumulator into the output cascade stream
 *
 * @param w   Output cascade stream
 * @param acc Accumulator to be written
 */
template <aie::AccumElemBaseType T, unsigned N>
__aie_inline
constexpr output_cascade<T> &operator<<(output_cascade<T> &w, const aie::accum<T, N> &acc)
{
    writeincr(&w, acc);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a value from the input AXI stream.
 *
 * @param w Input stream
 * @param v Value read from stream. Output argument.
 */
template <typename T>
__aie_inline
constexpr input_stream<T> &operator>>(input_stream<T> &w, T &v)
{
    // AXI stream width is 32b. Scalar reads for types smaller than 32b are promoted to 32b
    if constexpr (aie::detail::type_bits_v<T> < 32) {
        if constexpr (aie::detail::is_floating_point_v<T>) {
            float tmp = readincr((input_stream<float> *)&w);

            v = tmp;
        }
        else {
            int tmp = readincr((input_stream<int> *)&w);

            v = tmp;
        }
    }
    else {
        v = readincr(&w);
    }

    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a value or vector from the input AXI stream. The tlast object says whether TLAST is asserted.
 *
 * @param w  Input stream
 * @param tl Object that contains the value or vector read from stream. Output argument.
 */
template <typename T, aie::TLast<T> TL>
__aie_inline
constexpr input_stream<T> &operator>>(input_stream<T> &w, const TL &tl)
{
    using value_type = aie::detail::utils::remove_all_t<typename TL::value_type>;

    if constexpr (aie::detail::is_vector_v<value_type>) {
        tl.value = readincr_v<value_type::size()>(&w, tl.t_last);
    }
    else {
        // AXI stream width is 32b. Scalar writes for types smaller than 32b are promoted to 32b
        if constexpr (aie::detail::type_bits_v<T> < 32) {
            if constexpr (aie::detail::is_floating_point_v<T>) {
                const float tmp = readincr((input_stream<float> *)&w, tl.t_last);

                tl.value = tmp;
            }
            else {
                const int tmp = readincr((input_stream<int> *)&w, tl.t_last);

                tl.value = tmp;
            }
        }
        else {
            tl.value = readincr(&w, tl.t_last);
        }
    }

    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a value from the input AXI stream.
 *
 * @param w Input stream
 * @param v Value read from stream. Output argument.
 */
template <typename T>
__aie_inline
constexpr input_stream<T> &operator>>(input_stream<T> *w, T &v)
{
    *w >> v;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a value or vector from the input AXI stream. The tlast object says whether TLAST is asserted.
 *
 * @param w  Input stream
 * @param tl Object that contains the value or vector read from stream. Output argument.
 */
template <typename T, aie::TLast<T> TL>
__aie_inline
constexpr input_stream<T> *operator>>(input_stream<T> *w, const TL &tl)
{
    *w >> tl;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input AXI stream.
 *
 * @param w Input stream
 * @param v Vector read from stream. Output argument.
 */
template <unsigned N, typename T>
__aie_inline
constexpr input_stream<T> &operator>>(input_stream<T> &w, aie::vector<T, N> &v)
{
    v = readincr_v<N>(&w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input AXI stream.
 *
 * @param w Input stream
 * @param v Vector read from stream. Output argument.
 */
template <unsigned N, typename T>
__aie_inline
constexpr input_stream<T> *operator>>(input_stream<T> *w, aie::vector<T, N> &v)
{
    *w >> v;
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w   Input stream
 * @param acc Accumulator read from stream. Output argument.
 */
template <unsigned N, aie::AccumElemBaseType T>
[[deprecated("Use input_cascade instead")]]
__aie_inline
constexpr input_stream<T> *operator>>(input_stream<T> *w, aie::accum<T, N> &acc)
{
    acc = readincr_v<N>(w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w   Input stream
 * @param acc Accumulator read from stream. Output argument.
 */
template <unsigned N, aie::AccumElemBaseType T>
[[deprecated("Use input_cascade instead")]]
__aie_inline
constexpr input_stream<T> &operator>>(input_stream<T> &w, aie::accum<T, N> &acc)
{
    acc = readincr_v<N>(&w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input cascade stream.
 *
 * @param w   Input cascade stream
 * @param v   Vector read from stream. Output argument.
 */
template <unsigned N, aie::ElemBaseType T>
__aie_inline
constexpr input_cascade<T> *operator>>(input_cascade<T> *w, aie::vector<T, N> &v)
{
    v = readincr_v<N>(w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads a vector from the input cascade stream.
 *
 * @param w   Input cascade stream
 * @param v   Vector read from stream. Output argument.
 */
template <unsigned N, aie::ElemBaseType T>
__aie_inline
constexpr input_cascade<T> &operator>>(input_cascade<T> &w, aie::vector<T, N> &v)
{
    v = readincr_v<N>(&w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w   Input cascade stream
 * @param acc Accumulator read from stream. Output argument.
 */
template <unsigned N, aie::AccumElemBaseType T>
__aie_inline
constexpr input_cascade<T> *operator>>(input_cascade<T> *w, aie::accum<T, N> &acc)
{
    acc = readincr_v<N>(w);
    return w;
}

/**
 * @ingroup group_adf
 *
 * Reads an accumulator from the input cascade stream.
 *
 * @param w   Input cascade stream
 * @param acc Accumulator read from stream. Output argument.
 */
template <unsigned N, aie::AccumElemBaseType T>
__aie_inline
constexpr input_cascade<T> &operator>>(input_cascade<T> &w, aie::accum<T, N> &acc)
{
    acc = readincr_v<N>(&w);
    return w;
}

#endif
