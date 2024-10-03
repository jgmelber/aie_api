// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_DETAIL_AIE1_CFR_DIT_ACC48_HPP__
#define __AIE_API_DETAIL_AIE1_CFR_DIT_ACC48_HPP__

#include "../array_helpers.hpp"

namespace aie::detail {

template <>
struct cfr<cint16>
{
    using accum_tag = accum_tag_for_type<cint16, 48>;
    using acc_type = accum<accum_tag, 8>;

    struct input_data
    {
        vector<cint16, 8> inA;
        vector<cint16, 8> inB;
        int cid;
    };

    template <typename Func>
    class stage_iterator
    {
    public:
        using        value_type = input_data;
        using         reference = value_type;
        using iterator_category = std::input_iterator_tag;
        using   difference_type = ptrdiff_t;

        stage_iterator(cint16_t * inpA, cint16_t * inpB, Func&& f, unsigned ctrl_upshift) :
            ptrA_((v8cint16 *)(inpA)),
            ptrB_((v8cint16 *)(inpB)),
            cid_(0),
            get_ctrl_(f),
            ctrl_upshift_(ctrl_upshift)
        {
        }

        stage_iterator &operator++()
        {
           unsigned ci;
           int idx;
           split(get_ctrl_(), 5, ctrl_upshift_, idx, ci);
           cid_ = delay1(ci);
           ptrA_ret_ = byte_incr(ptrA_, idx);
           ptrB_ret_ = byte_incr(ptrB_, idx);
           return *this;
        }

        stage_iterator  operator++(int)
        {
            const stage_iterator it = *this;
            ++(*this);
            return it;
        }

        reference operator*() const
        {
            return { ptrA_ret_[0], ptrB_ret_[0], cid_ };
        }

    private:
        v8cint16 * ptrA_;
        v8cint16 * ptrB_;
        v8cint16 * ptrA_ret_;
        v8cint16 * ptrB_ret_;
        int cid_;
        Func get_ctrl_;
        unsigned ctrl_upshift_;
    };

    template <typename Func>
    auto begin(cint16 * inA, cint16 * inB, Func&& get_ctrl_function, unsigned ctrl_upshift = 0)
    {
        return stage_iterator<Func>(inA, inB, get_ctrl_function, ctrl_upshift);
    }

    template <unsigned Elems>
    acc_type mul(const input_data &data, vector_elem_ref<cint16, Elems> elem) 
    {
        v16cint16 __aie_register(xa) inpA = (data.inA.template grow<16>());
        v16cint16 __aie_register(xb) inpB = (data.inB.template grow<16>());

        acc_type acc = ::mul8_cfr(     inpA, inpB, data.cid, 0, elem.parent.template grow_extract<8>(elem.offset / 8), elem.offset % 8);
        return acc;
    }

   template <unsigned Elems>
   acc_type mac(acc_type acc, const input_data &data, vector_elem_ref<cint16, Elems> elem) 
   {
        v16cint16 __aie_register(xa) inpA = (data.inA.template grow<16>());
        v16cint16 __aie_register(xb) inpB = (data.inB.template grow<16>());

        acc = ::mac8_cfr(acc, inpA, inpB, data.cid, 0, elem.parent.template grow_extract<8>(elem.offset / 8), elem.offset % 8);
        return acc;
    }
};

}

#endif
