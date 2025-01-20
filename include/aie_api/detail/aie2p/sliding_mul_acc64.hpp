// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC64_HPP__
#define __AIE_API_DETAIL_AIE2P_SLIDING_MUL_ACC64_HPP__

#include "../mul.hpp"
#include "../vector.hpp"

namespace aie::detail {

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_op()
{
    if constexpr (utils::is_one_of_v<CoeffType, int16, uint16> && utils::is_one_of_v<DataType, int16, uint16>) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x4(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x4(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x4(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x4(args...); };
    }
    else {
        return [](auto &&... args) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(args...); };
    }
}

template <MulMacroOp MulOp, unsigned Lanes, int DataStep, typename CoeffType, typename DataType>
static constexpr auto sliding_mul_acc48_get_mul_conf_op()
{
    if constexpr (utils::is_one_of_v<CoeffType, int16, uint16> && utils::is_one_of_v<DataType, int16, uint16>) {
        if constexpr (MulOp == MulMacroOp::Mul)     return [](auto &&... args) __aie_inline { return ::mul_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::NegMul)  return [](auto &&... args) __aie_inline { return ::negmul_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Add_Mul) return [](auto &&... args) __aie_inline { return ::mac_conv_32x4_conf(args...); };
        if constexpr (MulOp == MulMacroOp::Sub_Mul) return [](auto &&... args) __aie_inline { return ::msc_conv_32x4_conf(args...); };
    }
    else {
        return [](auto &&... args) __aie_inline { return mul<MulOp, 64, DataType, CoeffType>::run(args...); };
    }
}

// 16b coeff * 16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <typename T, unsigned Elems>
    static auto prepare_coeff(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            return shuffle_down_rotate<T, native_coeff_elems>::run(input.template grow_replicate<native_coeff_elems>(), offset);
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<native_coeff_elems>(0);
        }
    };


    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned coeff_start,
                          bool coeff_sign,
                          const vector<data_type, N_Data> &data,
                          unsigned data_start,
                          bool data_sign,
                          const Acc &... acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;
            accum_type32 tmp;

            auto  data_tmp = prepare_data<data_type,   N_Data,  64>(data, data_start_local);
            auto coeff_tmp = prepare_coeff<coeff_type, N_Coeff>(coeff, coeff_start_local);
            
            tmp = mul_op(data_tmp,  data_sign,
                         coeff_tmp, coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned  data_start_local =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;
                
                auto  data_tmp = prepare_data<data_type,   N_Data,  64>(data, data_start_local);
                auto coeff_tmp = prepare_coeff<coeff_type, N_Coeff>(coeff, coeff_start_local);

                tmp = mac_op(data_tmp,  data_sign,
                             coeff_tmp, coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

// 32b coeff * 16b data
template <unsigned Lanes, unsigned Points, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = native_vector_length_v<data_type>;

    template <typename T, unsigned Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= 32) {
            return shuffle_down_rotate<T, 32>::run(input.template grow_replicate<32>(), offset);
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<32>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        data_start = data_start % N_Data;

        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % data_elems;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto  data_local = prepare_data(data, data_start_local);
            auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

            tmp = mul_op(data_local,   data_sign,
                         coeff_local, coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % data_elems;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto  data_local = prepare_data(data, data_start_local);
                auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

                tmp = mac_op(data_local,   data_sign,
                             coeff_local, coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

// 16b coeff * 32b data
// 32b coeff * 32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && !is_complex_v<DataType> && (CoeffBits == 16 || CoeffBits == 32))
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, 32, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type32 = accum<accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems  = native_vector_length_v<data_type>;

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto data_local  = shuffle_down_rotate<data_type, N_Data>::run(data, data_start_local);
            auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

            tmp = mul_op(data_local.template grow_replicate<lanes_per_mul>(),  data_sign,
                         coeff_local,                                         coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto data_local  = shuffle_down_rotate<data_type, N_Data>::run(data, data_start_local);
                auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

                tmp = mac_op(data_local.template grow_replicate<lanes_per_mul>(),  data_sign,
                             coeff_local,                                         coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

// 16b coeff * c16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 16, 32, int16, cint16>
{
    using coeff_type      = int16;
    using data_type       = cint16;
    using real_data_type  = int16;
    using accum_tag       = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using real_accum_tag  = accum_tag_for_mul_types<real_data_type, coeff_type, 64>;
    using accum_type      = accum<accum_tag, Lanes>;
    using accum_type32    = accum<accum_tag, 32>;
    using real_accum_type = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        data_start = data_start % N_Data;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp,                   lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(),      lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp,              lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), lanes_per_mul, 1, coeff_type, real_data_type>();

        constexpr unsigned num_conj = has_conj1<MulOp>()? 1 : 0;

        // Unzipping data
        vector<int16, N_Data> data_re, data_im;
        std::tie(data_re, data_im) = unzip_complex(data);

        // Unzipping input accumulator
        real_accum_type acc_re, acc_im;
        if constexpr (sizeof...(Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<real_accum_type::size()>(unzip_complex(acc)...);

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto coeff_shuffle = shuffle_down_rotate<int16, native_coeff_elems>::run(coeff.template grow_replicate<native_coeff_elems>(), coeff_start_local);

        acc_re = mul_op(prepare_data<int16, N_Data, native_data_elems>(data_re, data_start),
                        true,
                        coeff_shuffle,
                        coeff_sign,
                        utils::get_nth<0>(acc_re, acc)...);

        acc_im = mul_conf_op(prepare_data<int16, N_Data, native_data_elems>(data_im, data_start),
                             true,
                             coeff_shuffle,
                             coeff_sign,
                             utils::get_nth<0>(acc_im, acc)...,
                             utils::get_nth<0>(0, acc)...,       // zero_acc
                             utils::get_nth<0>(0, acc)...,       // shift16
                             num_conj,                           // sub_mul - negation of mul result for  -(coef.im*data.im)
                             utils::get_nth<0>(0, acc)...);      // sub_acc

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % N_Data;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            coeff_shuffle = shuffle_down_rotate<int16, native_coeff_elems>::run(coeff.template grow_replicate<native_coeff_elems>(), coeff_start_local);

            acc_re = mac_op(prepare_data<int16, N_Data, native_data_elems>(data_re, data_start_local),
                            true,
                            coeff_shuffle,
                            coeff_sign,
                            acc_re);

            acc_im = mac_conf_op(prepare_data<int16, N_Data, native_data_elems>(data_im, data_start_local),
                                 true,
                                 coeff_shuffle,
                                 coeff_sign,
                                 acc_im,
                                 0,
                                 0,
                                 num_conj, //negation of mul result for  -(coef.im*data.im)
                                 0);
        });

        // Re-shuffling real and imaginary
        return combine_into_complex(acc_re, acc_im).template extract<Lanes>(0);
    }
};

// 16b coeff * c32b data
// 32b coeff * c16b data
// 32b coeff * c32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(!is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type        = CoeffType;
    using data_type         = DataType;
    using real_data_type    = std::conditional_t<std::is_same_v<data_type, cint16>, int16, int32>;
    using accum_tag         = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type        = accum<accum_tag, Lanes>;
    using real_accum_tag    = accum_tag_for_mul_types<real_data_type, coeff_type, 64>;
    using real_accum_type32 = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        constexpr unsigned native_elems = native_vector_length_v<T>;
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_vector_length_v<T>) {
            auto tmp = shuffle_down_rotate<T, native_elems>::run(input.template grow_replicate<native_elems>(), offset);
            return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<OutElems>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 2 * lanes_per_mul, 1, coeff_type, real_data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              2 * lanes_per_mul, 1, coeff_type, real_data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            real_accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto data_local  = prepare_data<data_type, N_Data, lanes_per_mul>(data, data_start_local);
            auto coeff_val   = coeff.get(coeff_start_local);
            auto coeff_local = broadcast<coeff_type, 2 * lanes_per_mul>::run(coeff_val);

            tmp = mul_op(data_local.template cast_to<real_data_type>(),  data_sign,
                         coeff_local,                                   coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y).template cast_to<real_accum_tag>(), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto data_local  = prepare_data<data_type, N_Data, lanes_per_mul>(data, data_start_local);
                auto coeff_val   = coeff.get(coeff_start_local);
                auto coeff_local = broadcast<coeff_type, 2 * lanes_per_mul>::run(coeff_val);

                tmp = mac_op(data_local.template cast_to<real_data_type>(),  data_sign,
                             coeff_local,                                   coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template cast_to<accum_tag>().template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp.template cast_to<accum_tag>());
        });

        return ret;
    }
};

// c16b coeff * 32b data
// c32b coeff * 16b data
// c32b coeff * 32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(is_complex_v<CoeffType> && !is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type        = CoeffType;
    using data_type         = DataType;
    using real_coeff_type   = std::conditional_t<std::is_same_v<coeff_type, cint16>, int16, int32>;
    using accum_tag         = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type        = accum<accum_tag, Lanes>;
    using real_accum_tag    = accum_tag_for_mul_types<data_type, real_coeff_type, 64>;
    using real_accum_type32 = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        constexpr unsigned native_elems = native_vector_length_v<T>;
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_vector_length_v<T>) {
            auto tmp = shuffle_down_rotate<T, native_elems>::run(input.template grow_replicate<native_elems>(), offset);
            return concat(interleave_zip<T, native_elems>::run(tmp, tmp, 1)).template extract<OutElems>(0);
        }
        else {
            auto tmp = shuffle_down_rotate<T, OutElems>::run(input, offset).template extract<native_elems>(0);
            return concat(interleave_zip<T, native_elems>::run(tmp, tmp, 1)).template extract<OutElems>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), 2 * lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              2 * lanes_per_mul, 1, real_coeff_type, data_type>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            real_accum_type32 tmp;

            const unsigned  data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto data_local  = prepare_data<data_type, N_Data, 2 * lanes_per_mul>(data, data_start_local);
            auto coeff_val   = coeff.get(coeff_start_local);
            auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff_val);

            tmp = mul_op(data_local,                                       data_sign,
                         coeff_local.template cast_to<real_coeff_type>(), coeff_sign,
                         utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y).template cast_to<real_accum_tag>(), acc)...);

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto data_local  = prepare_data<data_type, N_Data, 2 * lanes_per_mul>(data, data_start_local);
                auto coeff_val   = coeff.get(coeff_start_local);
                auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff_val);

                tmp = mac_op(data_local,                                       data_sign,
                             coeff_local.template cast_to<real_coeff_type>(), coeff_sign,
                             tmp);
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template cast_to<accum_tag>().template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp.template cast_to<accum_tag>());
        });

        return ret;
    }
};

// c16b coeff * 16b data - leverage convolution mode
template <unsigned Lanes, unsigned Points>
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, 32, 16, cint16, int16>
{
    using coeff_type      = cint16;
    using real_coeff_type = int16;
    using data_type       = int16;
    using accum_tag       = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using real_accum_tag  = accum_tag_for_mul_types<data_type, real_coeff_type, 64>;
    using accum_type      = accum<accum_tag, Lanes>;
    using accum_type32    = accum<accum_tag, 32>;
    using real_accum_type = accum<real_accum_tag, 32>;

    static constexpr unsigned   lanes_per_mul = 32;
    static constexpr unsigned columns_per_mul = 4;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul  == 0);

    static constexpr unsigned native_data_elems  = 64;
    static constexpr unsigned native_coeff_elems = native_vector_length_v<real_coeff_type>;

    template <typename T, unsigned Elems, unsigned OutElems = Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset)
    {
        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= lanes_per_mul) {
            auto tmp = shuffle_down_rotate<T, lanes_per_mul>::run(input.template grow_replicate<lanes_per_mul>(), offset);
            if constexpr (Lanes <= 16)
                return tmp.template grow<OutElems>();
            else
                return tmp.template grow_replicate<OutElems>();
        }
        else {
            return shuffle_down_rotate<T, OutElems>::run(input.template grow_replicate<OutElems>(), offset);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr unsigned data_elems = std::max(N_Data, native_data_elems);

        data_start = data_start % N_Data;

        constexpr auto mul_op      = sliding_mul_acc48_get_mul_op<MulOp,                   lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mac_op      = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(),      lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mul_conf_op = sliding_mul_acc48_get_mul_conf_op<MulOp,              lanes_per_mul, 1, real_coeff_type, data_type>();
        constexpr auto mac_conf_op = sliding_mul_acc48_get_mul_conf_op<add_to_op<MulOp>(), lanes_per_mul, 1, real_coeff_type, data_type>();

        constexpr unsigned num_conj = has_conj2<MulOp>()? 1 : 0;

        // Unzipping coeff
        vector<int16, native_coeff_elems> coeff_re, coeff_im;
        std::tie(coeff_re, coeff_im) = grow_all<native_coeff_elems>(unzip_complex(coeff));

        // Unzipping input accumulator
        real_accum_type acc_re, acc_im;
        if constexpr (sizeof...(Acc) > 0)
            std::tie(acc_re, acc_im) = grow_all<real_accum_type::size()>(unzip_complex(acc)...);

        const unsigned coeff_start_local = coeff_start % N_Coeff;

        auto data_shuffle = prepare_data<int16, N_Data, native_data_elems>(data, data_start);

        acc_re = mul_op(data_shuffle,
                        true,
                        shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_re, coeff_start_local),
                        coeff_sign,
                        utils::get_nth<0>(acc_re, acc)...);

        acc_im = mul_conf_op(data_shuffle,
                             true,
                             shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_im, coeff_start_local),
                             coeff_sign,
                             utils::get_nth<0>(acc_im, acc)...,
                             utils::get_nth<0>(0, acc)...,       // zero_acc
                             utils::get_nth<0>(0, acc)...,       // shift16
                             num_conj,                           // sub_mul - negation of mul result for  -(coef.im*data.im)
                             utils::get_nth<0>(0, acc)...);      // sub_acc

        utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
            const unsigned data_start_local  = ( data_start + columns_per_mul * idx) % data_elems;
            const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

            auto data_shuffle = prepare_data<int16, N_Data, native_data_elems>(data, data_start_local);

            acc_re = mac_op(data_shuffle,
                            true,
                            shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_re, coeff_start_local),
                            coeff_sign,
                            acc_re);

            acc_im = mac_conf_op(data_shuffle,
                                 true,
                                 shuffle_down_rotate<int16, native_coeff_elems>::run(coeff_im, coeff_start_local),
                                 coeff_sign,
                                 acc_im,
                                 0,
                                 0,
                                 num_conj, //negation of mul result for  -(coef.im*data.im)
                                 0);
        });

        // Re-shuffling real and imaginary
        return combine_into_complex(acc_re, acc_im).template extract<Lanes>(0);
    }
};

// c16b coeff * c16b data
// c16b coeff * c32b data
// c32b coeff * c16b data
// c32b coeff * c32b data
template <unsigned Lanes, unsigned Points, unsigned CoeffBits, unsigned DataBits, typename CoeffType, typename DataType>
    requires(is_complex_v<CoeffType> && is_complex_v<DataType>)
struct sliding_mul_bits_impl<Lanes, Points, 1, 1, 1, 64, CoeffBits, DataBits, CoeffType, DataType>
{
    using coeff_type   = CoeffType;
    using data_type    = DataType;
    using accum_tag    = accum_tag_for_mul_types<data_type, coeff_type, 64>;
    using accum_type   = accum<accum_tag, Lanes>;
    using accum_type16 = accum<accum_tag, 16>;

    static constexpr unsigned   lanes_per_mul = 16;
    static constexpr unsigned columns_per_mul = 1;
    static constexpr unsigned         num_mul = Points / columns_per_mul;
    static_assert(Points >= columns_per_mul);
    static_assert(Points % columns_per_mul == 0);

    static constexpr unsigned native_data_elems = native_vector_length_v<data_type>;

    template <typename T, unsigned Elems>
    static auto prepare_data(vector<T, Elems> input, unsigned offset) {
        constexpr unsigned native_elems = native_vector_length_v<T>;

        // Avoid 1024b shuffle_down_rotate where possible
        if constexpr (Elems <= native_elems) {
            //FIXME: hoist grow_replicate
            return shuffle_down_rotate<T, native_elems>::run(input.template grow_replicate<native_elems>(), offset)
                        .template grow_replicate<lanes_per_mul>();
        }
        else {
            return shuffle_down_rotate<T, Elems>::run(input, offset).template extract<lanes_per_mul>(0);
        }
    };

    template <MulMacroOp MulOp, unsigned N_Coeff, unsigned N_Data, typename... Acc> requires(is_accum_v<Acc> && ...)
    __aie_inline
    static accum_type run(const vector<coeff_type, N_Coeff> &coeff,
                          unsigned                           coeff_start,
                          bool                               coeff_sign,
                          const vector<data_type, N_Data>   &data,
                          unsigned                           data_start,
                          bool                               data_sign,
                          const Acc &...                     acc)
    {
        constexpr bool flip_args = (DataBits < 64);
        using Type1 = std::conditional_t<flip_args, data_type, coeff_type>;
        using Type2 = std::conditional_t<flip_args, coeff_type, data_type>;
        constexpr auto mac_op = sliding_mul_acc48_get_mul_op<add_to_op<MulOp>(), lanes_per_mul, 1, Type1, Type2>();
        constexpr auto mul_op = sliding_mul_acc48_get_mul_op<MulOp,              lanes_per_mul, 1, Type1, Type2>();

        accum_type ret(acc...);

        utils::unroll_for<unsigned, 0, std::max(1u, Lanes / lanes_per_mul)>([&](auto idx_y) __aie_inline {
            accum_type16 tmp;

            const unsigned data_start_local = (data_start + idx_y * lanes_per_mul) % N_Data;
            const unsigned coeff_start_local = coeff_start % N_Coeff;

            auto  data_local = prepare_data(data, data_start_local);
            auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

            if constexpr (flip_args) {
                tmp = mul_op(coeff_local, true, data_local, true, utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...);
            }
            else {
                tmp = mul_op(data_local, true, coeff_local, true, utils::get_nth<0>(ret.template grow_extract<lanes_per_mul>(idx_y), acc)...);
            }

            utils::unroll_for<unsigned, 1, num_mul>([&](auto idx) __aie_inline {
                const unsigned data_start_local  =  (data_start + columns_per_mul * idx + idx_y * lanes_per_mul) % N_Data;
                const unsigned coeff_start_local = (coeff_start + columns_per_mul * idx) % N_Coeff;

                auto  data_local = prepare_data(data, data_start_local);
                auto coeff_local = broadcast<coeff_type, lanes_per_mul>::run(coeff[coeff_start_local]);

                if constexpr (flip_args) {
                    tmp = mac_op(coeff_local, true, data_local, true, tmp);
                }
                else {
                    tmp = mac_op(data_local, true, coeff_local, true, tmp);
                }
            });

            if constexpr (Lanes <= lanes_per_mul)
                ret = tmp.template extract<Lanes>(0);
            else
                ret.insert(idx_y, tmp);
        });

        return ret;
    }
};

}

#endif
