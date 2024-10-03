// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE2_LINEAR_APPROX_HPP__
#define __AIE_API_DETAIL_AIE2_LINEAR_APPROX_HPP__

#include "../blend.hpp"
#include "../broadcast.hpp"
#include "../lut.hpp"
#include "../../mask.hpp"

namespace aie::detail {

/* Scratchpad usage in linear_approx implementations
 *
 * Recall that the logical steps of a linear approximation are defined to be
 *   1. index = (input >> step_bits) + bias
 *   2. slope/offset pair read from LUT based on index
 *   3. output = slope * (input & ((1 << step_bits) - 1)) + (offset << shift_offset)
 *
 * Here, we refer to 'input & ((1 << step_bits) - 1)' as 'remainder'.
 * This remainder is calculated very early in the compute member function and keeping its value
 * in a register would prevent pipelining due to register pressure, To overcome this, we force
 * spilling the value to memory via a scratchpad.
 * The scratchpad is implemented as a cyclic buffer due to the fact pipelining will result in
 * a number of values being written to the scratchpad before one is read - a value of 8 words is used below.
 * The cyclic buffer implementation uses two pointers and a 2d pointer addition to achieve wrap around.
 * At construction, both pointers are set to the scratchpad but note that pDout_ is restrict.
 * This is to inform the compiler that pDout_ carries no memory dependencies. While this is obviously not the case,
 * it is required to pipeline writes to the buffer. Consistency is handled by a chess_separator_scheduler_local
 * call between the reads and writes which is acting as a memory fence enforcing correct ordering on reads and
 * writes to the scratchpad via pDin_ and pDout_.
 * 
 */

template <>
struct linear_approx<int8, lut<4, int8, int8>>
{
    using OffsetType = int8;
    using MyLUT = lut<4, OffsetType, OffsetType>;
    template <unsigned Lanes>
    using accum_type = accum<acc32, Lanes>;

    static constexpr unsigned acc_lanes = 16;
    using bias_type = accum<acc64, acc_lanes>;

public:
    linear_approx(const MyLUT &l, unsigned step_bits, int bias = 0, int shift_offset = 0):
        step_bits_(step_bits),
        shift_offset_(shift_offset),
        cntDin_(0),
        cntDout_(0)
    {
        //TODO: Pending CRVO-4122 change to use const pointers of the correct types
        LUT_ab_ = (int *) l.LUT_ab_;
        LUT_cd_ = (int *) l.LUT_cd_;

        // equal to int(np.log2(self.actv.bits/8))) = log2(sizeof(int8) which is 0 for int8
        constexpr int LUT_type_shift = 0;
        constexpr int LUT_duplication_shift = 2;
        constexpr int min_step_bits = LUT_type_shift + LUT_duplication_shift;

        int used_bias = (bias << LUT_duplication_shift);

#if AIE_API_NATIVE
        REQUIRES_MSG(step_bits >= min_step_bits, "step_bits is below minimum threshold");
#endif

        shift_addr_ = step_bits_ - min_step_bits; //Sizeof scalar elements read in the LUT

        int idx_max  = (l.LUT_elems_ << (LUT_duplication_shift+LUT_type_shift)) - 1;
        int rem_mask = (1 << step_bits_) - 1;

        pDin_  = (v32int8 chess_storage(DM_bankD) * __restrict) scratch_pad_;
        pDout_ = (v32int8 chess_storage(DM_bankD) * __restrict) scratch_pad_;

        numD_  = scratch_size_ / 32 - 1;
        incD1_ = 32;
        incD2_ = -incD1_ * numD_;

        bias_vec_.from_vector(broadcast<int32, acc_lanes>::run(used_bias), shift_addr_);

        idx_max_vec_  = broadcast<uint32, 16>::run(idx_max);
        rem_mask_vec_ = select<uint8, 64>::run(broadcast<uint8, 64>::run(rem_mask),
                                               zeros<uint8, 64>::run(),
                                               mask<64>::from_uint64(0xFFFFFFFF00000000));
    }

    template <typename Vec>
    __aie_inline
    accum_type<Vec::size()> compute(const Vec &input)
    {
        vector<int8, 32> input_ = input.template grow<32>();

        //Mask at the start in this implementation, before storing through the scratchpad
        vector<uint8, 64> remainder = ::band(input_.cast_to<uint8>().grow<64>(), rem_mask_vec_);
        *pDout_ = input_;       pDout_ = ::add_2d_byte(pDout_, incD2_, numD_, cntDout_, incD1_);
        input_ = ::extract_v32int8((set_v64int8(0, input_) - (v64int8)remainder), 0);

        v64int8 coeff0, coeff1, coeff2, coeff3;

        vector<int16, 32> input16 = input_.unpack();

        accum<acc64, 16> acc64;
        vector<uint32, 16> index;

        //First load
        acc64.from_vector(input16.extract<16>(0));
        acc64 = ::add(acc64, bias_vec_);
        index = acc64.to_vector<uint32>(shift_addr_);
        index = ::min(index, idx_max_vec_);
        chess_separator_scheduler_local(); //Needed to enforce read/write order on pDin/pDout in all cases (CRVO-3520)
        ::load_lut_2x_int8(LUT_ab_, LUT_cd_, index, coeff0, coeff1);

        //Second load
        acc64.from_vector(input16.extract<16>(1));
        acc64 = ::add(acc64, bias_vec_);
        index = acc64.to_vector<uint32>(shift_addr_);
        index = ::min(index, idx_max_vec_);
        ::load_lut_2x_int8(LUT_ab_, LUT_cd_, index, coeff2, coeff3);

        //Discards unused 48b gaps
        coeff0 = ::shuffle(coeff0, coeff1, T16_16x4_lo);
        //Discards unused 48b gaps
        coeff2 = ::shuffle(coeff2, coeff3, T16_16x4_lo);
        //combines the two v16int16
        coeff0 = ::shuffle(coeff0, coeff2, T256_2x2_lo);
        //Selects from the two previous
        vector<int8, 64> slope  = ::shuffle(coeff0, coeff0, T8_64x2_lo);
        vector<int8, 32> offset = ::extract_v32int8(::shuffle(coeff0, coeff0, T8_64x2_hi), 0);

        accum_type<32> result(offset, shift_offset_);
        remainder = ::band(vector<int8,32>(*pDin_).cast_to<uint8>().grow<64>(), rem_mask_vec_);
        pDin_ = ::add_2d_byte(pDin_, incD2_, numD_, cntDin_, incD1_);
        result = ::mac_elem_32_2(slope, remainder, result);

        return result.template extract<Vec::size()>(0);
    }

private:
    int  step_bits_;
    int  shift_offset_;
    int* LUT_ab_;
    int* LUT_cd_;
    int  shift_addr_;

    bias_type  bias_vec_;
    vector<uint32, 16> idx_max_vec_;
    vector<uint8, 64>  rem_mask_vec_;

    //Variables for scratch pad
    constexpr static int scratch_size_ = 128; // 4 vectors * sizeof(vector)
    alignas(32) int8_t scratch_pad_[scratch_size_];
    v32int8 chess_storage(DM_bankD) * __restrict pDin_;
    v32int8 chess_storage(DM_bankD) * __restrict pDout_;
    addr_t cntDin_;
    addr_t cntDout_;
    int numD_;
    int incD1_;
    int incD2_;
};

template <typename OffsetType>
struct linear_approx<int16, lut<4, OffsetType, OffsetType>>
{
    using MyLUT = lut<4, OffsetType, OffsetType>;
    template <unsigned Lanes>
    using accum_type = accum<acc64, Lanes>;

public:
    linear_approx(const MyLUT &l, unsigned step_bits, int bias = 0, int shift_offset = 0):
        step_bits_(step_bits),
        shift_offset_(shift_offset),
        cntDin_(0),
        cntDout_(0)
    {
        //TODO: Pending CRVO-4122 change to use const pointers of the correct types
        LUT_ab_ = (int *) l.LUT_ab_;
        LUT_cd_ = (int *) l.LUT_cd_;

        //equal to int(np.log2(self.actv.bits/8))) = log2(sizeof(int16) which is 1 for int16, 2 for int32
        constexpr int LUT_type_shift = std::is_same_v<OffsetType, int32> ? 2 : 1;
        constexpr int LUT_duplication_shift = 2;
        constexpr int min_step_bits = LUT_type_shift + LUT_duplication_shift;

        int used_bias = bias << (LUT_duplication_shift + LUT_type_shift);

#if AIE_API_NATIVE
        REQUIRES_MSG(step_bits >= min_step_bits, "step_bits is below minimum threshold");
#endif

        shift_addr_ = step_bits_ - min_step_bits;

        int idx_max   = (l.LUT_elems_ << (LUT_duplication_shift + LUT_type_shift)) - 1;
        int rem_mask  = (1 << step_bits_) - 1;

        pDin_  = (v16int16 chess_storage(DM_bankD) * __restrict) scratch_pad_;
        pDout_ = (v16int16 chess_storage(DM_bankD) * __restrict) scratch_pad_;

        numD_  = scratch_size_ / 32 - 1;
        incD1_ = 32;
        incD2_ = -incD1_ * numD_;

        bias_vec_.from_vector(broadcast<int32,16>::run(used_bias), shift_addr_);
        idx_max_vec_  = broadcast<uint32,16>::run(idx_max);
        rem_mask_vec_ = select<uint16, 32>::run(broadcast<uint16, 32>::run(rem_mask),
                                                zeros<uint16, 32>::run(),
                                                mask<32>::from_uint32(0xFFFF0000));
        rem_vec_      = zeros<uint16, 32>::run();
    }

    //Rounding abstracted implementation, one cycle worse than the kernel variant which depends on a specific rounding mode
    //TODO: Potential for getting to 5 cycles from initializing the accumulation with a mul instead of a lups to save a move slot instruction, or R+W of the sat/rnd control registers
    //if these instructions are pipelineable in the future
    template <typename Vec>
    __aie_inline
    accum_type<Vec::size()> compute(const Vec &input)
    {
        accum_type<16> result;
        vector<int16, 16> input_ = input.template grow<16>();

        vector<uint32, 16> chess_storage(x4) index; //TODO: Remove this chess_storage when possible CRVO-4142
        vector<uint16, 32> remainder;

        remainder = ::band((v32uint16)::set_v32int16(0, input_), rem_mask_vec_); //Mask at the start in this implementation, before storing through the scratchpad
        result.from_vector(remainder.extract<16>(0), 0); *pDout_ = result.to_vector<int16>(0); pDout_ = ::add_2d_byte(pDout_, incD2_, numD_, cntDout_, incD1_);
        //input - remainder to zero out the remainder bits so they don't impact when rounding
        result  = ::mac_elem_16_2_conf(::set_v32int16(0, input_), ::sel(broadcast<int16,32>::run(1), zeros<int16,32>::run(), 0xFFFF0000), result, 0, 0, 0, 1);

        result = ::add(result, bias_vec_);

        index = result.to_vector<uint32>(shift_addr_);
        index = ::min(index, idx_max_vec_);

        if constexpr (std::is_same_v<OffsetType, int16>) {
            v32int16 coeff0, coeff1; //TODO: Change to API types once they can be passed directly to the load_lut intrinsic (CRVO-4143)
            vector<int16, 32> slope_offset;

            chess_separator_scheduler_local(); //Needed to enforce read/write order on pDin/pDout in all cases (CRVO-3520)
            ::load_lut_2x_int16(LUT_ab_, LUT_cd_, index, coeff0, coeff1);

            slope_offset = ::shuffle(coeff0, coeff1, T16_16x4_lo);

            //Insert delayed result into retained remainder vector with upper half zeroed
            rem_vec_.insert<16>(0, (v16uint16)*pDin_); pDin_ = ::add_2d_byte(pDin_, incD2_, numD_, cntDin_, incD1_);

            result.from_vector(slope_offset.extract<16>(1), shift_offset_);
            result = ::mac_elem_16_2(slope_offset, rem_vec_, result);
        }
        else if constexpr (std::is_same_v<OffsetType, int32>) {
            v16int32 coeff0, coeff1; //TODO: Change to API types once they can be passed directly to the load_lut intrinsic (CRVO-4143)
            vector<int32, 16> slope, offset;
            const vector<int32, 16> zero = zeros<int32,16>::run();

            chess_separator_scheduler_local(); //Needed to enforce read/write order on pDin/pDout in all cases (CRVO-3520)
            ::load_lut_2x_int32(LUT_ab_, LUT_cd_, index, coeff0, coeff1);

            slope  = ::shuffle(coeff0, coeff1, T32_16x2_lo);
            offset = ::shuffle(coeff0, coeff1, T32_16x2_hi);

            //Insert delayed result into retained remainder vector with upper half zeroed
            rem_vec_.insert<16>(0, (v16uint16)*pDin_); pDin_ = ::add_2d_byte(pDin_, incD2_, numD_, cntDin_, incD1_);

            result.from_vector(offset, shift_offset_);
            result = ::mac_elem_16_2(slope, zero, rem_vec_, result);
        }

        return result.template extract<Vec::size()>(0);
    }

private:
    int  step_bits_;
    int* LUT_ab_;
    int* LUT_cd_;
    int  shift_offset_;
    int  shift_addr_;

    //Vecs initialized from params
    accum_type<16>  bias_vec_;
    vector<uint32, 16> idx_max_vec_;
    vector<uint16, 32> rem_mask_vec_;
    vector<uint16, 32> rem_vec_;

    //Variables for scratch pad
    constexpr static int scratch_size_ = 128; // 4 vectors * sizeof(vector)
    alignas(32) int8_t scratch_pad_[scratch_size_];
    v16int16 chess_storage(DM_bankD) * __restrict pDin_;
    v16int16 chess_storage(DM_bankD) * __restrict pDout_;
    addr_t cntDin_;
    addr_t cntDout_;
    int numD_;
    int incD1_;
    int incD2_;
};

template <>
struct linear_approx<bfloat16, lut<4, float, bfloat16>>
{
    using OffsetType = float;
    using SlopeType = bfloat16;
    using MyLUT = lut<4, OffsetType, SlopeType>;
    template <unsigned Lanes>
    using accum_type = accum<accfloat, Lanes>;

public:
    linear_approx(const MyLUT &l, unsigned step_bits, int bias = 0, int shift_offset = 0):
        step_bits_(step_bits),
        cntDin_(0),
        cntDout_(0)
    {
        REQUIRES_MSG(shift_offset == 0, "shift_offset is not used with bfloat and should remain 0");
        int LUT_type_shift = 2; //log2(sizeof(float) is 2
        int LUT_duplication_shift = 2;
        int bias_used     = (bias << (LUT_duplication_shift + LUT_type_shift));

        //TODO: Pending CRVO-4122 change to use const pointers of the correct types
        LUT_ab_ = ::byte_incr((int *) l.LUT_ab_, bias_used);
        LUT_cd_ = ::byte_incr((int *) l.LUT_cd_, bias_used);

        int idx_max  = (l.LUT_elems_ << (LUT_duplication_shift + LUT_type_shift)) - 1;

        shift_addr_ = step_bits_ - LUT_duplication_shift - LUT_type_shift;

        pDin_  = (v16bfloat16 chess_storage(DM_bankD) * __restrict) scratch_pad_;
        pDout_ = (v16bfloat16 chess_storage(DM_bankD) * __restrict) scratch_pad_;

        numD_  = scratch_size_ / 32 - 1;
        incD1_ = 32;
        incD2_ = -incD1_ * numD_;

        idx_min_  = broadcast<int32, 16>::run(-bias_used);
        idx_max_  = broadcast<int32, 16>::run(idx_max - bias_used);
        delay_in_ = zeros<bfloat16, 32>::run();
    }

    template <typename Vec>
    __aie_inline
    accum_type<Vec::size()> compute(const Vec &input)
    {
        vector<bfloat16, 16> input_ = input.template grow<16>();

        *pDout_ = input_;        pDout_ = ::add_2d_byte(pDout_, incD2_, numD_, cntDout_, incD1_);

        vector<int32, 16> index;
        v32bfloat16 coeff0, coeff1; //TODO: Change to API types once they can be passed directly to the load_lut intrinsic (CRVO-4143)

        index = ::bfloat16_to_int(input_, -shift_addr_);
        index = ::max(index, idx_min_);
        index = ::min(index, idx_max_);

        chess_separator_scheduler_local(); //Needed to enforce read/write order on pDin/pDout in all cases (CRVO-3520)

        ::load_lut_2x_float(LUT_ab_, LUT_cd_, index, coeff0, coeff1);

        accum_type<16> offset      = (v16accfloat) ::shuffle(coeff0, coeff1, T32_16x2_hi);
        vector<bfloat16, 32> slope = ::shuffle(coeff0, coeff1, T16_16x4_lo);

        delay_in_.insert<16>(1, *pDin_);        pDin_ = ::add_2d_byte(pDin_, incD2_, numD_, cntDin_, incD1_);

        accum_type<16> result = ::mac_elem_16_2(slope, delay_in_, offset);

        return result.template extract<Vec::size()>(0);
    }

private:
    int  step_bits_;
    int* LUT_ab_;
    int* LUT_cd_;
    int  shift_addr_;

    vector<int32, 16>    idx_min_;
    vector<int32, 16>    idx_max_;
    vector<bfloat16, 32> delay_in_;

    //Variables for scratch pad
    constexpr static int scratch_size_ = 128; // 4 vectors * sizeof(vector)
    alignas(32) int8_t scratch_pad_[scratch_size_];
    v16bfloat16 chess_storage(DM_bankD) * __restrict pDin_;
    v16bfloat16 chess_storage(DM_bankD) * __restrict pDout_;
    addr_t cntDin_;
    addr_t cntDout_;
    int numD_;
    int incD1_;
    int incD2_;
};

}

#endif
