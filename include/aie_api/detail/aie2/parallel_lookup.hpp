// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2025 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_PARALLEL_LOOKUP_HPP__
#define __AIE_API_DETAIL_AIE2_PARALLEL_LOOKUP_HPP__

#include "../broadcast.hpp"
#include "../lut.hpp"

namespace aie::detail {

namespace {

constexpr unsigned output_lanes(unsigned Words) {
    if (Words == 1) return 512 / 8 / 2;
    else            return 512 / 8 / Words;
}

constexpr unsigned mem_offset(unsigned Words) {
    if      (Words == 1) return 2;
    else if (Words == 2) return 2;
    else if (Words == 4) return 3;
    else                 return 4;
}

constexpr unsigned get_shuffle_mode(unsigned Words) {
    if      (Words == 1) return T16_16x4_lo;
    else if (Words == 2) return T16_16x4_lo;
    else if (Words == 4) return T32_16x2_lo;
    else                 return T512_1x2_lo;
}

template <unsigned Words> struct coeff_type;
template <>               struct coeff_type<1> { using type = v64int8; };
template <>               struct coeff_type<2> { using type = v64int8; };
template <>               struct coeff_type<4> { using type = v32int16; };
template <>               struct coeff_type<8> { using type = v16int32; };
template <unsigned Words> using coeff_type_t = typename coeff_type<Words>::type;

template <typename T> struct pack_type;
template <>           struct pack_type<int8>  { using type = v32int16; };
template <>           struct pack_type<uint8> { using type = v32uint16; };
template <typename T> using pack_type_t = typename pack_type<T>::type;

template <unsigned Words>
constexpr auto get_lut_x2_op() {
    if      constexpr (Words <= 2)
        return [](auto&&... params){ return ::load_lut_2x_int8(params...); };
    else if constexpr (Words == 4)
        return [](auto&&... params){ return ::load_lut_2x_int16(params...); };
    else if constexpr (Words == 8)
        return [](auto&&... params){ return ::load_lut_2x_int32(params...); };
}

} // namespace

template <typename T, typename OffsetType, lut_oor_policy oor_policy>
struct parallel_lookup<T, lut<4, OffsetType, OffsetType>, oor_policy>
{
    static constexpr unsigned KeyWords   = sizeof(T);
    static constexpr unsigned ValueWords = sizeof(OffsetType);

    using MyLUT = lut<4, OffsetType, OffsetType>;

    __aie_inline
    bool check_index_coverage(unsigned elems, unsigned step_bits, unsigned bias)
    {
        bool all_lut_elems_indexable = (1ll << (8 * KeyWords)) == ((long long)elems << step_bits);
        if constexpr (std::is_signed_v<T>)
            all_lut_elems_indexable = all_lut_elems_indexable && ((bias == 0) || (bias == elems >> 1));
        else
            all_lut_elems_indexable = all_lut_elems_indexable && (bias == 0);
        return all_lut_elems_indexable;
    }

    /*
     * Helper function to apply the specified out-of-range policy: saturation, or truncation.
     *
     * Unsigned indices:
     *   - saturation takes the minimum of the index and the maximum allowable index
     *   - truncation discards higher bits allowing the index to wrap back to 0
     * Signed indices:
     *   - saturation clamps the index to lie between the min and max allowable indices
     *   - truncation maintains the index to be in the [-bias, LUT_size-bias) range
     *     ensuring that the index wraps appropriately i.e. LUT_size-bias maps to -bias
     *
     * Note that a PAR_LOOKUP_ALL_SPACE_MANIFEST (chess_manifest) is used to optimise away
     * out-of-range checks when they are not necessary.
     */
    __aie_inline
    vector<int32, 16> apply_oor_policy(vector<int32, 16> index)
    {
        if (!PAR_LOOKUP_ALL_SPACE_MANIFEST(all_space_used_)) {
            if constexpr (std::is_signed_v<T>) {
                if        constexpr(oor_policy == lut_oor_policy::truncate) {
                    index = ::band(::add(index, idx_min_vec_), idx_max_vec_);
                } else if constexpr(oor_policy == lut_oor_policy::saturate) {
                    index = ::min(index, idx_max_vec_);
                    index = ::max(index, idx_min_vec_);
                }
            } else {
                if      constexpr(oor_policy == lut_oor_policy::truncate)
                    index = ::band(index, idx_max_vec_);
                else if constexpr(oor_policy == lut_oor_policy::saturate)
                    index = ::min(index, idx_max_vec_);
            }
        }
        return index;
    }

public:
    __aie_inline
    parallel_lookup(const MyLUT &l, unsigned step_bits = 0, unsigned bias = 0) :
        LUT_ab_((int*)l.LUT_ab_), LUT_cd_((int*)l.LUT_cd_),
        all_space_used_(check_index_coverage(l.LUT_elems_, step_bits, bias))
    {
        constexpr unsigned offset = mem_offset(ValueWords);

        int used_bias = (int)bias << offset;
        shift_addr_ = step_bits - offset;

        if constexpr (oor_policy == lut_oor_policy::truncate) {
            // If all_space_used_ is set, then no oor policy will be applied and indices will be used directly.
            // This can occur if bias == lut_size/2 and in that case we need to increment the base pointer
            if (PAR_LOOKUP_ALL_SPACE_MANIFEST(all_space_used_)) {
                LUT_ab_ = ::byte_incr(LUT_ab_, used_bias);
                LUT_cd_ = ::byte_incr(LUT_cd_, used_bias);
            }
        } else {
            if (!PAR_LOOKUP_ZERO_BIAS_MANIFEST(bias == 0)) {
                LUT_ab_ = ::byte_incr(LUT_ab_, used_bias);
                LUT_cd_ = ::byte_incr(LUT_cd_, used_bias);
            }
        }

        /*
         * For the signed truncation policy we require more than min and max allowed indices
         * diff (broadcast into idx_max_vec_) is a mask whose set bits test:
         *   - when a negative index is in range
         *   - when a positive index is out of range
         * idx_min_vec_ is a mask used to set/unset the upper bits of an index that is out of range
         */
        if (!PAR_LOOKUP_ALL_SPACE_MANIFEST(all_space_used_)) {
            int idx_max  = (l.LUT_elems_ << offset) - 1;
            if constexpr (std::is_signed_v<T> && oor_policy == lut_oor_policy::truncate) {
                idx_max_vec_ = broadcast<int32, 16>::run(idx_max);
                idx_min_vec_ = broadcast<int32, 16>::run(used_bias);
            } else {
                idx_max_vec_ = broadcast<int32, 16>::run(idx_max - used_bias);
                idx_min_vec_ = broadcast<int32, 16>::run(-used_bias);
            }
        }
    }

    template <typename Vec, unsigned N = Vec::size()>
    requires(Vec::size() <= 32 && N <= Vec::size() && N <= native_vector_length<OffsetType>::value)
    __aie_inline
    vector<OffsetType, N> fetch(const Vec &input)
    {
        // result_t does not necessarily line up with the returned type due to
        // the extract<N>() call on the return statement.
        // The reason for this is to allow lookups of 64b values with 8b keys.
        // 8b keys required a 16 lane input; however we are not able to return
        // a 16 lane 64b vector.
        // To facilitate such a fetch, we allow N <= Vec::size where N is set as
        // a template parameter: e.g. lookup.fetch<N>(input);
        // See other fetch specialisation.
        using result_t = vector<OffsetType, output_lanes(ValueWords)>;
        using coeff_t  = coeff_type_t<ValueWords>;
        // bfloat16 lookup requires a different accum_t to the other implementations
        // due to the 32b accum srs not supporting shifting
        using accum_t  = accum<acc64, Vec::size()>;

        constexpr auto mode    = get_shuffle_mode(ValueWords);
        constexpr auto load_op = get_lut_x2_op<ValueWords>();

        result_t result;
        vector<int32, 16> index;

        coeff_t coeff0, coeff1;
        accum_t acc;

        //FIXME: This implementation is not currently rounding mode agnostic,
        //would need to subtract remainder like in the linear_approx
        //Preferred option would be work directly with the configuration, pending CRVO-4425
        acc.from_vector(input);

        index = acc.template extract<16>(0).template to_vector<int32>(shift_addr_);
        index = apply_oor_policy(index);

        load_op(LUT_ab_, LUT_cd_, index, coeff0, coeff1);
        // Discard garbage resulting from 64bit read
        coeff0 = ::shuffle(coeff0, coeff1, mode);

        if constexpr (Vec::size() == 32) {
            coeff_t coeff2, coeff3;

            index = acc.template extract<16>(1).template to_vector<int32>(shift_addr_);
            index = apply_oor_policy(index);
            
            load_op(LUT_ab_, LUT_cd_, index, coeff2, coeff3);
            // Discard garbage resulting from 64bit read
            coeff2 = ::shuffle(coeff2, coeff3, mode);
            // Combines the two vectors
            coeff0 = ::shuffle(coeff0, coeff2, T256_2x2_lo);
        }

        if constexpr (ValueWords == 1)
            result = ::pack((pack_type_t<OffsetType>)coeff0);
        else
            result = (typename result_t::native_type)coeff0;

        return result.template extract<N>(0);
    }

    template <unsigned N, typename Vec>
    __aie_inline
    vector<OffsetType, N> fetch(const Vec &input)
    {
        return fetch<Vec, N>(input);
    }

private:
#if __AIE_ARCH__ == 20
    const int* LUT_ab_;
    const int* LUT_cd_;
#elif __AIE_ARCH__ == 21
    // TODO: CRVO-4468: compr and sparse functions should accept references to const pointers
    int* LUT_ab_;
    int* LUT_cd_;
#endif
    int shift_addr_;
    vector<int32, 16> idx_max_vec_;
    vector<int32, 16> idx_min_vec_;
    const bool all_space_used_; //Determine if truncate/saturate would have no impact because space is fully used, and skip via PAR_LOOKUP_ALL_SPACE_MANIFEST if possible
};

}

#endif
