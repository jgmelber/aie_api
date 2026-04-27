// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2026 Advanced Micro Devices, Inc.

#include <aie_api/aie.hpp>
#include <aie_api/utils.hpp>

//! [Sparse matrix multiplication]
void gemm_int8xint8_sparse(int8 * matA, int8 * matB, int8 *__restrict matC,
                           int rowsA, int inner, int colsB)
{
    using MMUL = aie::mmul<4, 16, 8, int8, int8>;

    auto a_desc = aie::make_tensor_descriptor<int8, 64>(aie::tensor_dim(rowsA / 4 / 4, 2),
                                                        aie::tensor_dim(colsB / 4 / 4, 0),
                                                        aie::tensor_dim(inner / 8, rowsA / 8),
                                                        aie::tensor_dim(2u, 1));

    auto c_desc = aie::make_tensor_descriptor<int8, 32>(aie::tensor_dim(rowsA / 4 / 4, 4),
                                                        aie::tensor_dim(colsB / 8, rowsA / 4),
                                                        aie::tensor_dim(4u, 1));

    auto tsA = aie::make_tensor_buffer_stream<aie_dm_resource::a>(matA, a_desc);
    auto tsC = aie::make_restrict_tensor_buffer_stream(matC, c_desc);

    aie::pipelined_loop</*Minimum iterations =*/ 2>(rowsA / 16, [&](unsigned j)  __aie_inline
    {
        auto tsB = aie::sparse_vector_input_buffer_stream<int8, 128, aie_dm_resource::a>(matB);

        aie::pipelined_loop</*Minimum iterations =*/ 2>(colsB / 16, [&](unsigned b)  __aie_inline
        {
            MMUL C00, C01;
            MMUL C10, C11;
            MMUL C20, C21;
            MMUL C30, C31;

            aie::pipelined_loop</*Minimum iterations =*/ 4>(inner / 16, [&](unsigned i)  __aie_inline
            {
                aie::vector<int8,64> Sbuff0, Sbuff1, Sbuff2, Sbuff3;
                tsA.pop() >> Sbuff0 >> Sbuff1;
                tsA.pop() >> Sbuff2 >> Sbuff3;

                auto [Xbuff0, Xbuff1] = aie::interleave_zip(Sbuff0, Sbuff2, 8);
                auto [Xbuff2, Xbuff3] = aie::interleave_zip(Sbuff1, Sbuff3, 8);

                aie::sparse_vector<int8,128> Ybuff0, Ybuff1;
                tsB >> Ybuff0 >> Ybuff1;

                C00.mac(Xbuff0, Ybuff0); C01.mac(Xbuff0, Ybuff1);
                C10.mac(Xbuff1, Ybuff0); C11.mac(Xbuff1, Ybuff1);
                C20.mac(Xbuff2, Ybuff0); C21.mac(Xbuff2, Ybuff1);
                C30.mac(Xbuff3, Ybuff0); C31.mac(Xbuff3, Ybuff1);
            });

            tsC << C00.to_vector<int8>() << C10.to_vector<int8>() << C20.to_vector<int8>() << C30.to_vector<int8>()
                << C01.to_vector<int8>() << C11.to_vector<int8>() << C21.to_vector<int8>() << C31.to_vector<int8>();
        });
    });
}
//! [Sparse matrix multiplication]
