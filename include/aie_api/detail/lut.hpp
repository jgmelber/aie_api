// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_LUT_HPP__
#define __AIE_API_DETAIL_LUT_HPP__

namespace aie::detail {

enum class lut_oor_policy {
    saturate,
    truncate
};

template <unsigned ParallelAccesses, typename OffsetType, typename SlopeType=OffsetType>
struct lut;

template <typename OffsetType, typename SlopeType>
struct lut<4, OffsetType, SlopeType>
{
  public:
      lut(unsigned LUT_elems, const void* LUT_ab, const void* LUT_cd):
      LUT_elems_(LUT_elems),
      LUT_ab_(LUT_ab),
      LUT_cd_(LUT_cd)
      {}

  unsigned LUT_elems_;
  const void* LUT_ab_;
  const void* LUT_cd_;
};

template <typename OffsetType, typename SlopeType>
struct lut<2, OffsetType, SlopeType>
{
  public:
      lut(unsigned LUT_elems, const void* LUT_ab):
      LUT_elems_(LUT_elems),
      LUT_ab_(LUT_ab)
      {}

  unsigned LUT_elems_;
  const void* LUT_ab_;
};

template <typename OffsetType, typename SlopeType>
struct lut<1, OffsetType, SlopeType>
{
  public:
      lut(unsigned LUT_elems, const void* LUT_a):
      LUT_elems_(LUT_elems),
      LUT_a_(LUT_a)
      {}

  private:
      unsigned LUT_elems_;
      const void* LUT_a_;
};

}

#endif
