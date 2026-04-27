// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>
#include <aie_api/utils.hpp>

//! [Matrix multiplication]
void gemm_bf16xbf16(bfloat16 * matA, bfloat16 * matB, bfloat16 *__restrict matC,
                    int rowsA, int inner, int colsB)
{
    using MMUL = aie::mmul<4, 8, 4, bfloat16, bfloat16>;

    auto a_desc =  aie::make_tensor_descriptor<bfloat16, 32>(
                                               aie::tensor_dim(rowsA / 4 / 4, 4),
                                               aie::tensor_dim(colsB / 4 / 4, 0),
                                               aie::tensor_dim(inner / 8, rowsA / 4),
                                               aie::tensor_dim(4u, 1));

    auto b_desc = aie::make_tensor_descriptor<bfloat16, 32>(
                                               aie::tensor_dim(colsB / 4 / 4, 0),
                                               aie::tensor_dim(colsB / 4 / 4, inner / 8 * 4),
                                               aie::tensor_dim(inner / 8, 1),
                                               aie::tensor_dim(4u, inner / 8));

    auto c_desc = aie::make_tensor_descriptor<bfloat16, 16>(
                                               aie::tensor_dim(rowsA / 4 / 4, 4),
                                               aie::tensor_dim(colsB / 4, rowsA / 4),
                                               aie::tensor_dim(4u, 1));

    auto tsA = aie::make_tensor_buffer_stream(matA, a_desc);
    auto tsB = aie::make_tensor_buffer_stream(matB, b_desc);
    auto tsC = aie::make_restrict_tensor_buffer_stream(matC, c_desc);

    aie::pipelined_loop</*Minimum iterations =*/ 2>(rowsA * colsB / (16 * 16), [&](unsigned j)  __aie_inline
    {
        MMUL C00, C01, C02, C03;
        MMUL C10, C11, C12, C13;
        MMUL C20, C21, C22, C23;
        MMUL C30, C31, C32, C33;

        aie::pipelined_loop</*Minimum iterations =*/ 2>(inner / 8, [&](unsigned i)  __aie_inline
        {
            // The following pop calls are required to access the inner leaf stream.
            // As tsA and tsB are 4D streams, the returned inner stream will be 1D.
            //
            // Note that these calls advance the outer stream
            auto tsA_inner = tsA.pop();
            auto tsB_inner = tsB.pop();

            aie::vector<bfloat16,32> Xbuff0, Xbuff1, Xbuff2, Xbuff3;
            tsA_inner >> Xbuff0 >> Xbuff1 >> Xbuff2 >> Xbuff3;

            aie::vector<bfloat16,32> Ybuff0, Ybuff1;
            tsB_inner >> Ybuff0 >> Ybuff1;

            C00.mac(Xbuff0, Ybuff0); C01.mac(Xbuff0, Ybuff1);
            C10.mac(Xbuff1, Ybuff0); C11.mac(Xbuff1, Ybuff1);
            C20.mac(Xbuff2, Ybuff0); C21.mac(Xbuff2, Ybuff1);
            C30.mac(Xbuff3, Ybuff0); C31.mac(Xbuff3, Ybuff1);

            tsB_inner >> Ybuff0 >> Ybuff1;

            C02.mac(Xbuff0, Ybuff0); C03.mac(Xbuff0, Ybuff1);
            C12.mac(Xbuff1, Ybuff0); C13.mac(Xbuff1, Ybuff1);
            C22.mac(Xbuff2, Ybuff0); C23.mac(Xbuff2, Ybuff1);
            C32.mac(Xbuff3, Ybuff0); C33.mac(Xbuff3, Ybuff1);
        });

        tsC << C00.to_vector<bfloat16>() << C10.to_vector<bfloat16>() << C20.to_vector<bfloat16>() << C30.to_vector<bfloat16>()
            << C01.to_vector<bfloat16>() << C11.to_vector<bfloat16>() << C21.to_vector<bfloat16>() << C31.to_vector<bfloat16>()
            << C02.to_vector<bfloat16>() << C12.to_vector<bfloat16>() << C22.to_vector<bfloat16>() << C32.to_vector<bfloat16>()
            << C03.to_vector<bfloat16>() << C13.to_vector<bfloat16>() << C23.to_vector<bfloat16>() << C33.to_vector<bfloat16>();
    });
}
//! [Matrix multiplication]
