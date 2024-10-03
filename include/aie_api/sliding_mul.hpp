// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Xilinx, Inc.
// Copyright (C) 2022-2024 Advanced Micro Devices, Inc.

#pragma once

#ifndef __AIE_API_SLIDING_MUL__HPP__
#define __AIE_API_SLIDING_MUL__HPP__

#include "concepts.hpp"

namespace aie {

/**
 * @ingroup group_mul_special
 *
 * This type provides a parametrized multiplication that implements the following compute pattern:
 *
 * @code
 *
 * DSX = DataStepX
 * DSY = DataStepY
 * CS  = CoeffStep
 * P   = Points
 * L   = Lanes
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * data[d_s +            ] + coeff[c_s + CS] * data[d_s +               DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s +               (P-1) * DSX]
 * out[1]   = coeff[c_s] * data[d_s +         DSY] + coeff[c_s + CS] * data[d_s +         DSY + DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s +         DSY + (P-1) * DSX]
 * ...
 * out[L-1] = coeff[c_s] * data[d_s + (L-1) * DSY] + coeff[c_s + CS] * data[d_s + (L-1) * DSY + DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s + (L-1) * DSY + (P-1) * DSX]
 *
 * @endcode
 *
 * <table>
 * <caption>Supported AIE parameters for sliding_mul</caption>
 * <tr><th>Types (coeff x data)          <th colspan=2>Native accum.       <th>Native lanes      <th>Native points<th>CoeffStep<th>DataStepX<th>DataStepY                     <th>coeff_start             <th>data_start
 * <tr><td rowspan=1>  8b x   8b         <td>          AIE-ML   <td> acc32 <td>32                <td>8            <td>1,2      <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3> 16b x  16b         <td>          AIE      <td> acc48 <td>8<br/>16          <td>32/Lanes     <td>1,2,3,4  <td>1        <td>1                             <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td rowspan=2>AIE-ML   <td> acc32 <td>16                <td>4            <td>1,2      <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr>                                                         <td> acc64 <td>16                <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3> 16b x  32b         <td rowspan=2>AIE      <td> acc48 <td>8<br/>16          <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2<br/>1                     <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td> acc80 <td>8                 <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2                           <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td> acc64 <td>16                <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3> 32b x  16b         <td rowspan=2>AIE      <td> acc48 <td>8<br/>16          <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2<br/>1                     <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td> acc80 <td>8                 <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2                           <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td> acc64 <td>16                <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2> 32b x  32b         <td>          AIE      <td> acc80 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td> acc64 <td>16                <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2> 16b x c16b         <td>          AIE      <td>cacc48 <td>4<br/>8           <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3> 16b x c32b         <td rowspan=2>AIE      <td>cacc48 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1, 2</sup> <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3> 32b x c16b         <td rowspan=2>AIE      <td>cacc48 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2> 32b x c32b         <td>          AIE      <td>cacc80 <td>2<br/>4           <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1, 2</sup> <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2>c16b x  16b         <td>          AIE      <td>cacc48 <td>4<br/>8           <td>16/Lanes     <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3>c16b x  32b         <td rowspan=2>AIE      <td>cacc48 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2>c16b x c16b         <td>          AIE      <td>cacc48 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3>c16b x c32b         <td rowspan=2>AIE      <td>cacc48 <td>4                 <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td> 8<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3>c32b x  16b         <td rowspan=2>AIE      <td>cacc48 <td>4<br/>8           <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4<br/>1,2               <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>8/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2>c32b x  32b         <td>          AIE      <td>cacc80 <td>2<br/>4           <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>16<sup>1</sup>    <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=3>c32b x c16b         <td rowspan=2>AIE      <td>cacc48 <td>4                 <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                                         <td>cacc80 <td>4                 <td>4/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>8<sup>1</sup>     <td>4            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * <tr><td rowspan=2>c32b x c32b         <td>          AIE      <td>cacc80 <td>2                 <td>2/Lanes      <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3,4                       <td>Unsigned smaller than 16<td>Signed
 * <tr>                                  <td>          AIE-ML   <td>cacc64 <td>8<sup>1</sup>     <td>1            <td>1        <td>1,2      <td>1,2 (needs to match DataStepX)<td>Unsigned                <td>Signed
 * </table>
 *
 * \note
 * <sup>1</sup>. Complex multiplication is emulated by decomposing the product into real x real multiplications. Real
 * and complex results are shuffled into the result vector at the end.
 *
 * \note
 * <sup>2</sup>. These real-coeff by complex-data operations compute real and complex components separately.
 *
 * <table>
 * <caption>Supported AIE parameters for sliding_mul with floating point accumulation</caption>
 * <tr><th>Types (coeff x data)          <th colspan=2>Native accum.   <th>Native lanes<th>Native points<th>CoeffStep<th>DataStepX<th>DataStepY                      <th>coeff_start              <th>data_start
 * <tr><td rowspan=1>bfloat16 x bfloat16 <td>AIE-ML   <td> accfloat    <td>16          <td>1            <td>1,2      <td>1,2      <td>1,2 (needs to match DataStepX) <td>Unsigned                 <td>Signed
 * <tr><td rowspan=2>float x float       <td>AIE      <td> accfloat    <td>8           <td>1            <td>1,2,3,4  <td>1,2,3,4  <td>1,2                            <td>Unsigned smaller than 16 <td>Signed
 * <tr>                                  <td>AIE-ML   <td> accfloat    <td>32          <td>1            <td>1,2      <td>1,2      <td>1,2 (needs to match DataStepX) <td>Unsigned                 <td>Signed
 * <tr><td> float x cfloat               <td>AIE      <td>caccfloat    <td>4           <td>1            <td>1,2,3    <td>1,2,3,4  <td>1,2,3                          <td>Unsigned smaller than 16 <td>Signed
 * <tr><td>cfloat x  float               <td>AIE      <td>caccfloat    <td>4           <td>1            <td>1,2,3,4  <td>1,2,3,4  <td>1,2,3                          <td>Unsigned smaller than 16 <td>Signed
 * <tr><td>cfloat x cfloat               <td>AIE      <td>caccfloat    <td>4           <td>1            <td>1,2,3    <td>1,2,3,4  <td>1,2,3                          <td>Unsigned smaller than 16 <td>Signed
 * </table>
 *
 * \note
 * Native lanes denotes the number of outputs handled by a single intrinsic call. For `Lanes = N * Native lanes`, N
 * calls to the underlying intrinsic are made. For `Lanes < Native lanes`, a single call is made and the requested lanes
 * extracted.
 *
 * \note
 * Native points denotes the number of multiplications and additions handled by a single intrinsic call.  Multiples of
 * points are emulated by unrolling multiple intrinsic calls.  For 32b accumulation modes, arbitrary points can be used
 * and are emulated by zeroing coeff lanes.
 *
 * \note
 * coeff_start and data_start wrap around if greater than the number of values.
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection
 *                   accross lanes.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
struct sliding_mul_ops {
    static constexpr unsigned accum_bits = detail::to_native_accum_bits_for_mul_types_tag<CoeffType, DataType, AccumTag>();
#if __AIE_ARCH__ == 10
    static constexpr unsigned max_coeff_bits = 256;
#else
    static constexpr unsigned max_coeff_bits = 512;
#endif
    static constexpr unsigned max_data_bits = 1024;

    using impl_type = detail::sliding_mul<Lanes, Points, CoeffStep, DataStepX, DataStepY, accum_bits, CoeffType, DataType>;

    enum class MulType
    {
        Mul,
        Acc_Mul,
        NegMul
    };

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = accum<detail::accum_tag_or_default_t<AccumTag, CoeffType, DataType>, Lanes>;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = impl_type::lanes;
    static constexpr unsigned          points = impl_type::points;

    template <MulType Mul, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &data, unsigned data_start,
                                           const Acc &...acc)
    {
        static_assert((std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag> && ...));

        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_ops::mul_common<Mul>(coeff, coeff_start, data, data_start, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_ops::mul_common<Mul>(op_none(coeff), coeff_start, data, data_start, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_ops::mul_common<Mul>(coeff, coeff_start, op_none(data), data_start, acc...);
        }
        else {
#if __AIE_ARCH__ == 10
            using T_Data  = typename VecData::value_type;

            if      constexpr (VecData::is_operation_not(Operation::Conj) && detail::is_floating_point_v<T_Data>)
                return sliding_mul_ops::mul_common<Mul>(coeff, coeff_start, data(), data_start, acc...);
            else if constexpr (VecData::is_operation_not(Operation::Conj, Operation::Abs, Operation::Max, Operation::Min))
                return sliding_mul_ops::mul_common<Mul>(coeff, coeff_start, data(), data_start, acc...);
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj))
                return sliding_mul_ops::mul_common<Mul>(coeff(), coeff_start, data, data_start, acc...);
#else
            if      constexpr (VecData::is_operation_not(Operation::Conj, Operation::Sign))
                return sliding_mul_ops::mul_common<Mul>(coeff, coeff_start, data(), data_start, acc...);
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj, Operation::Sign))
                return sliding_mul_ops::mul_common<Mul>(coeff(), coeff_start, data, data_start, acc...);
#endif
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (Mul == MulType::Mul)
                    return impl_type::template run<detail::to_mul_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data));
                else if constexpr (Mul == MulType::Acc_Mul)
                    return impl_type::template run<detail::to_mul_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data), acc.parent1()...);
                else if constexpr (Mul == MulType::Negmul)
                    return impl_type::template run<detail::to_negmul_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data));
            }
        }
    }

    /**
     * Performs the multiplication pattern defined by the class parameters using the input coefficient and data
     * arguments.
     *
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type mul(const VecCoeff &coeff, unsigned coeff_start,
                                    const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ops::mul_common<MulType::Mul>(coeff, coeff_start, data, data_start);
    }

    /**
     * Performs a multiply-add with the pattern defined by the class parameters using the input coefficient and data
     * arguments.
     *
     * @param acc         Accumulator that is added to the result of the multiplication.
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type mac(const Acc &acc,
                                    const VecCoeff &coeff, unsigned coeff_start,
                                    const VecData &data, unsigned data_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ops::mul_common<MulType::Acc_Mul>(coeff, coeff_start, data, data_start, acc);
    }

    /**
     * Performs a negation of the multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments.
     *
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type negmul(const VecCoeff &coeff, unsigned coeff_start,
                                       const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ops::mul_common<MulType::NegMul>(coeff, coeff_start, data, data_start);
    }
};

/**
 * @ingroup group_mul_special
 *
 * Similar to @ref sliding_mul_ops, but DataStepY is always 1.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DSX = DataStepX
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * data[d_s +     0] + coeff[c_s + CS] * data[d_s +         DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s +         (P-1) * DSX]
 * out[1]   = coeff[c_s] * data[d_s +     1] + coeff[c_s + CS] * data[d_s +     1 + DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s +     1 + (P-1) * DSX]
 * ...
 * out[L-1] = coeff[c_s] * data[d_s + (L-1)] + coeff[c_s + CS] * data[d_s + (L-1) + DSX] + ... + coeff[c_s + (P-1) * CS] * data[d_s + (L-1) + (P-1) * DSX]
 *
 * @endcode
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection within
 *                   a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
using sliding_mul_x_ops = sliding_mul_ops<Lanes, Points, CoeffStep, DataStepX, 1, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * Similar to @ref sliding_mul_ops, but DataStepX is always 1.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DSY = DataStepY
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * data[d_s +            ] + coeff[c_s + CS] * data[d_s +               1] + ... + coeff[c_s + (P-1) * CS] * data[d_s +               (P-1)]
 * out[1]   = coeff[c_s] * data[d_s +         DSY] + coeff[c_s + CS] * data[d_s +         DSY + 1] + ... + coeff[c_s + (P-1) * CS] * data[d_s +         DSY + (P-1)]
 * ...
 * out[L-1] = coeff[c_s] * data[d_s + (L-1) * DSY] + coeff[c_s + CS] * data[d_s + (L-1) * DSY + 1] + ... + coeff[c_s + (P-1) * CS] * data[d_s + (L-1) * DSY + (P-1)]
 *
 * @endcode
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection
 *                   accross lanes.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
using sliding_mul_y_ops = sliding_mul_ops<Lanes, Points, CoeffStep, 1, DataStepY, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * Similar to @ref sliding_mul_ops, but DataStepX is equal to DataStepY.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DS  = DataStepXY
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * data[d_s +           ] + coeff[c_s + CS] * data[d_s +     DS] + ... + coeff[c_s + (P-1) * CS] * data[d_s +   (P-1) * DS]
 * out[1]   = coeff[c_s] * data[d_s +         DS] + coeff[c_s + CS] * data[d_s + 2 * DS] + ... + coeff[c_s + (P-1) * CS] * data[d_s +       P * DS]
 * ...
 * out[L-1] = coeff[c_s] * data[d_s + (L-1) * DS] + coeff[c_s + CS] * data[d_s + L * DS] + ... + coeff[c_s + (P-1) * CS] * data[d_s + (P+L-2) * DS]
 *
 * @endcode
 *
 * @tparam Lanes      Number of output elements.
 * @tparam Points     Number of data elements used to compute each lane.
 * @tparam CoeffStep  Step used to select elements from the coeff buffer. This step is applied to element selection
 *                    within a lane.
 * @tparam DataStepXY Step used to select elements from the data buffer. This step is applied to element selection
 *                    within a lane and across lanes.
 * @tparam CoeffType  Type of the coefficient elements.
 * @tparam DataType   Type of the data elements.
 * @tparam AccumTag   Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                    the result of the multiplication of the coefficient and data types (real/complex).
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepXY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
using sliding_mul_xy_ops = sliding_mul_ops<Lanes, Points, CoeffStep, DataStepXY, DataStepXY, CoeffType, DataType, AccumTag>;

//TODO: implement dynamic sign support
template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul(const VecCoeff &coeff,
                 unsigned coeff_start,
                 const VecData  &data,
                 unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert with supported parameters
    using mul_ops = sliding_mul_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul(coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mul(const VecCoeff &coeff,
                 const VecData  &data,
                 unsigned data_start)
{
    return sliding_mul<Lanes, Points, CoeffStep, DataStep, AccumTag>(coeff, CoeffStart, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac(const Acc &acc,
                 const VecCoeff &coeff,
                 unsigned coeff_start,
                 const VecData  &data,
                 unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert with supported parameters
    using mul_ops = sliding_mul_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac(acc, coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mac(const Acc &acc,
                 const VecCoeff &coeff,
                 const VecData  &data,
                 unsigned data_start)
{
    return sliding_mac<Lanes, Points, CoeffStep, DataStep>(acc, coeff, CoeffStart, data, data_start);
}

/**
 * @ingroup group_mul_special
 *
 * @note Only supported in AIE.
 *
 * This type provides a parametrized multiplication that implements the following compute pattern:
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DSX = DataStepX
 * DSY = DataStepY
 * c_s = coeff_start
 * d_s = data_start
 * OP  = Operation type: '+' for symmetric, '-' for anisymmetric
 *
 * out[0]   = coeff[c_s] * (data[d_s +            ] OP data[d_s +               (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s +                         DSX] OP data[d_s +               (P-2) * DSX]) + ...
 *                                                                                            + coeff[c_s + (P/2-1 * CS)] * (data[d_s +               (P/2-1) * DSX] OP data[d_s +                 P/2 * DSX]);
 * out[1]   = coeff[c_s] * (data[d_s +         DSY] OP data[d_s +         DSY + (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s +         DSY +           DSX] OP data[d_s +         DSY + (P-2) * DSX]) + ...
 *                                                                                            + coeff[c_s + (P/2-1 * CS)] * (data[d_s +         DSY + (P/2-1) * DSX] OP data[d_s +         DSY +   P/2 * DSX]);
 * ...
 * out[L-1] = coeff[c_s] * (data[d_s + (L-1) * DSY] OP data[d_s + (L-1) * DSY + (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s + (L-1) * DSY +           DSX] OP data[d_s + (L-1) * DSY + (P-2) * DSX]) + ...
 *                                                                                            + coeff[c_s + (P/2-1 * CS)] * (data[d_s + (L-1) * DSY + (P/2-1) * DSX] OP data[d_s + (L-1) * DSY +   P/2 * DSX]);
 *
 * @endcode
 *
 * <table>
 * <caption>Supported parameters for sliding_mul_sym with 48b accumulation</caption>
 * <tr><th>Types (coeff x data)<th>Native lanes<th>Native points<th>CoeffStep<th>DataStepX<th>DataStepY<th>coeff_start<th>data_start
 * <tr><td>16b x 16b  <td>8<br/>16<td>64/Lanes<td>1,2,3,4<td>1      <td>1              <td>Unsigned smaller than 16<td>Signed
 * <tr><td>16b x 32b  <td>8<br/>16<td>32/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2<br/>1      <td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x 16b  <td>8<br/>16<td>32/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2<br/>1      <td>Unsigned smaller than 16<td>Signed
 * <tr><td>16b x c16b <td>4<br/>8 <td>32/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x 16b <td>4<br/>8 <td>32/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x c16b<td>4<br/>8 <td>16/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x 32b <td>4<br/>8 <td>16/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x c16b <td>4<br/>8 <td>16/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x 16b <td>4<br/>8 <td>16/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>16b x c32b <td>4<br/>8 <td>16/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x c16b<td>4       <td>       2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x c32b<td>4       <td>       2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * </table>
 *
 * <table>
 * <caption>Supported parameters for sliding_mul_sym with 80b accumulation</caption>
 * <tr><th>Types (coeff x data)<th>Native lanes<th>Native points<th>CoeffStep<th>DataStepX<th>DataStepY<th>coeff_start<th>data_start
 * <tr><td>32b x 16b  <td>8      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2            <td>Unsigned smaller than 16<td>Signed
 * <tr><td>16b x 32b  <td>8      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2            <td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x 32b  <td>4      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<br/>1,2<td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x c16b <td>4      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x 32b <td>4      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x 16b <td>4      <td>      2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>16b x c32b <td>4      <td>      4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x c16b<td>4      <td>      2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c16b x c32b<td>4      <td>      2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x 32b <td>2<br/>4<td>8/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x c32b <td>2<br/>4<td>8/Lanes<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x c32b<td>2      <td>      2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4        <td>Unsigned smaller than 16<td>Signed
 * </table>
 *
 * \note
 * Native lanes denotes the number of outputs handled by a single intrinsic call. For `Lanes = N * Native lanes`, N
 * calls to the underlying intrinsic are made. For `Lanes < Native lanes`, a single call is made and the requested lanes
 * extracted.
 *
 * \note
 * Native points denotes the number of multiplications and additions handled by a single intrinsic call.  Multiples of
 * points are emulated by unrolling multiple intrinsic calls.  For 32b accumulation modes, arbitrary points can be used
 * and are emulated by zeroing coeff lanes.
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection
 *                   across lanes.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, int DataStepY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE))
struct sliding_mul_sym_ops {
    static constexpr unsigned accum_bits = detail::to_native_accum_bits_for_mul_types_tag<CoeffType, DataType, AccumTag>();

    using impl_type = detail::sliding_mul_sym<Lanes, Points, CoeffStep, DataStepX, DataStepY, accum_bits, CoeffType, DataType>;

    enum class SymMulType
    {
        Sym,
        Antisym,
        Acc_Sym,
        Acc_Antisym,
    };

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = accum<detail::accum_tag_or_default_t<AccumTag, CoeffType, DataType>, Lanes>;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = impl_type::lanes;
    static constexpr unsigned          points = impl_type::points;

    template <SymMulType MulType, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &data, unsigned data_start,
                                           const Acc &...acc)
    {
        static_assert((std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag> && ...));

        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, data, data_start, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_sym_ops::mul_common<MulType>(op_none(coeff), coeff_start, data, data_start, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, op_none(data), data_start, acc...);
        }
        else {
            if      constexpr (VecData::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, data(), data_start, acc...);
            }
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff(), coeff_start, data, data_start, acc...);
            }
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (MulType == SymMulType::Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start);
                else if constexpr (MulType == SymMulType::Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start);
                else if constexpr (MulType == SymMulType::Acc_Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, acc.parent1()...);
                else if constexpr (MulType == SymMulType::Acc_Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, acc.parent1()...);
            }
        }
    }

    template <SymMulType MulType, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &data, unsigned ldata_start, unsigned rdata_start,
                                           const Acc &...acc)
    {
        static_assert((std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag> && ...));

        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, data, ldata_start, rdata_start, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_sym_ops::mul_common<MulType>(op_none(coeff), coeff_start, data, ldata_start, rdata_start, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, op_none(data), ldata_start, rdata_start, acc...);
        }
        else {
            if      constexpr (VecData::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, data(), ldata_start, rdata_start, acc...);
            }
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff(), coeff_start, data, ldata_start, rdata_start, acc...);
            }
            else {
                constexpr Operation OpCoeff = VecCoeff::operation;
                constexpr Operation OpData = VecData::operation;

                if      constexpr (MulType == SymMulType::Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), ldata_start, rdata_start);
                else if constexpr (MulType == SymMulType::Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), ldata_start, rdata_start);
                else if constexpr (MulType == SymMulType::Acc_Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), ldata_start, rdata_start, acc.parent1()...);
                else if constexpr (MulType == SymMulType::Acc_Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), ldata_start, rdata_start, acc.parent1()...);
            }
        }
    }

    template <SymMulType MulType, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &ldata, unsigned ldata_start,
                                           const VecData &rdata, unsigned rdata_start,
                                           const Acc &...acc)
    {
        static_assert((std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag> && ...));

        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_sym_ops::mul_common<MulType>(op_none(coeff), coeff_start, ldata, ldata_start, rdata, rdata_start, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, op_none(ldata), ldata_start, op_none(rdata), rdata_start, acc...);
        }
        else {
            if      constexpr (VecData::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff, coeff_start, ldata(), ldata_start, rdata(), rdata_start, acc...);
            }
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj)) {
                return sliding_mul_sym_ops::mul_common<MulType>(coeff(), coeff_start, ldata, ldata_start, rdata, rdata_start, acc...);
            }
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (MulType == SymMulType::Sym)
                    return impl_type::template run_2buff<detail::to_mul_sym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start);
                else if constexpr (MulType == SymMulType::Antisym)
                    return impl_type::template run_2buff<detail::to_mul_antisym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start);
                else if constexpr (MulType == SymMulType::Acc_Sym)
                    return impl_type::template run_2buff<detail::to_mul_sym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, acc.parent1()...);
                else if constexpr (MulType == SymMulType::Acc_Antisym)
                    return impl_type::template run_2buff<detail::to_mul_antisym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, acc.parent1()...);
            }
        }
    }

    /**
     * Performs the symmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_sym(const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Sym>(coeff, coeff_start, data, data_start);
    }

    /**
     * Performs the symmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_sym(const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &data, unsigned ldata_start, unsigned rdata_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Sym>(coeff, coeff_start, data, ldata_start, rdata_start);
    }

    /**
     * Performs the symmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant uses two input buffers for left/right elements.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param ldata       Vector of left data samples. The size is limitted to vectors of up to 512 bits.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata       Vector of right data samples. The size is limitted to vectors of up to 512 bits.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_sym(const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &ldata, unsigned ldata_start,
                                        const VecData &rdata, unsigned rdata_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Sym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
    }

    /**
     * Performs the symmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_sym(const Acc &acc,
                                        const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &data, unsigned data_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Sym>(coeff, coeff_start, data, data_start, acc);
    }

    /**
     * Performs the symmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_sym(const Acc &acc,
                                        const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &data, unsigned ldata_start, unsigned rdata_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Sym>(coeff, coeff_start, data, ldata_start, rdata_start, acc);
    }

    /**
     * Performs the symmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant uses two input buffers for left/right elements.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param ldata       Vector of left data samples. The size is limitted to vectors of up to 512 bits.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata       Vector of right data samples. The size is limitted to vectors of up to 512 bits.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_sym(const Acc &acc,
                                        const VecCoeff &coeff, unsigned coeff_start,
                                        const VecData &ldata, unsigned ldata_start,
                                        const VecData &rdata, unsigned rdata_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Sym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, acc);
    }

    /**
     * Performs the antisymmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_antisym(const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Antisym>(coeff, coeff_start, data, data_start);
    }

    /**
     * Performs the antisymmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_antisym(const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned ldata_start, unsigned rdata_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Antisym>(coeff, coeff_start, data, ldata_start, rdata_start);
    }

    /**
     * Performs the antisymmetric multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant uses two input buffers for left/right elements.
     *
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param ldata       Vector of left data samples. The size is limitted to vectors of up to 512 bits.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata       Vector of right data samples. The size is limitted to vectors of up to 512 bits.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_antisym(const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &ldata, unsigned ldata_start,
                                            const VecData &rdata, unsigned rdata_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Antisym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
    }

    /**
     * Performs the antisymmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_antisym(const Acc &acc,
                                            const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned data_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Antisym>(coeff, coeff_start, data, data_start, acc);
    }

    /**
     * Performs the antisymmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant allows two separate start indices for left/right elements.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_antisym(const Acc &acc,
                                            const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned ldata_start, unsigned rdata_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Antisym>(coeff, coeff_start, data, ldata_start, rdata_start, acc);
    }

    /**
     * Performs the antisymmetric multiply-add pattern defined by the class parameters using the input coefficient and
     * data arguments. This variant uses two input buffers for left/right elements.
     *
     * @param acc         Accumulator to be added to the result of the multiplication.
     * @param coeff       Vector of coefficients. On AIE the size is limited to vectors of up to 256 bits.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param ldata       Vector of left data samples. The size is limitted to vectors of up to 512 bits.
     * @param ldata_start Index of the first left data element to be used in the multiplication.
     * @param rdata       Vector of right data samples. The size is limitted to vectors of up to 512 bits.
     * @param rdata_start Index of the first right data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_antisym(const Acc &acc,
                                            const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &ldata, unsigned ldata_start,
                                            const VecData &rdata, unsigned rdata_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_ops::mul_common<SymMulType::Acc_Antisym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, acc);
    }

    //TODO: Add entry point for negmul
};

/**
 * @ingroup group_mul_special
 *
 * @note Only supported in AIE.
 *
 * Similar to @ref sliding_mul_sym_ops, but DataStepY is always 1.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DSX = DataStepX
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * (data[d_s +      ] + data[d_s +         (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s +                   DSX] + data[d_s +         (P-2) * DSX]) + ...
 *                                                                               + coeff[c_s + (P/2-1 * CS)] * (data[d_s +         (P/2-1) * DSX] + data[d_s +           P/2 * DSX]);
 * out[1]   = coeff[c_s] * (data[d_s +     1] + data[d_s +     1 + (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s +     1 +           DSX] + data[d_s +     1 + (P-2) * DSX]) + ...
 *                                                                               + coeff[c_s + (P/2-1 * CS)] * (data[d_s +     1 + (P/2-1) * DSX] + data[d_s +     1 +   P/2 * DSX]);
 * ...
 * out[L-1] = coeff[c_s] * (data[d_s + (L-1)] + data[d_s + (L-1) + (P-1) * DSX]) + coeff[c_s +           CS] * (data[d_s + (L-1) +           DSX] + data[d_s + (L-1) + (P-2) * DSX]) + ...
 *                                                                               + coeff[c_s + (P/2-1 * CS)] * (data[d_s + (L-1) + (P/2-1) * DSX] + data[d_s + (L-1) +   P/2 * DSX]);
 *
 * @endcode
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_sym_ops
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepX, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE))
using sliding_mul_sym_x_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, 1, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * @note Only supported in AIE.
 *
 * Similar to @ref sliding_mul_sym_ops, but DataStepX is always 1.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DSY = DataStepY
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * (data[d_s +            ] + data[d_s +               (P-1)]) + coeff[c_s +           CS] * (data[d_s +                     1] + data[d_s +               (P-2)]) + ...
 *                                                                                     + coeff[c_s + (P/2-1 * CS)] * (data[d_s +               (P/2-1)] + data[d_s +                 P/2]);
 * out[1]   = coeff[c_s] * (data[d_s +         DSY] + data[d_s +         DSY + (P-1)]) + coeff[c_s +           CS] * (data[d_s +         DSY +       1] + data[d_s +         DSY + (P-2)]) + ...
 *                                                                                     + coeff[c_s + (P/2-1 * CS)] * (data[d_s +         DSY + (P/2-1)] + data[d_s +         DSY +   P/2]);
 * ...
 * out[L-1] = coeff[c_s] * (data[d_s + (L-1) * DSY] + data[d_s + (L-1) * DSY + (P-1)]) + coeff[c_s +           CS] * (data[d_s + (L-1) * DSY +       1] + data[d_s + (L-1) * DSY + (P-2)]) + ...
 *                                                                                     + coeff[c_s + (P/2-1 * CS)] * (data[d_s + (L-1) * DSY + (P/2-1)] + data[d_s + (L-1) * DSY +   P/2]);
 *
 * @endcode
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection
 *                   across lanes.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_sym_ops
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE))
using sliding_mul_sym_y_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, 1, DataStepY, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * @note Only supported in AIE.
 *
 * Similar to @ref sliding_mul_sym_ops, but DataStepX is equal to DataStepY.
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DS  = DataStepXY
 * c_s = coeff_start
 * d_s = data_start
 *
 * out[0]   = coeff[c_s] * (data[d_s +           ] + data[d_s +   (P-1) * DS]) + coeff[c_s +           CS] * (data[d_s +             DS] + data[d_s +     (P-2) * DS]) + ...
 *                                                                             + coeff[c_s + (P/2-1 * CS)] * (data[d_s +   (P/2-1) * DS] + data[d_s +       P/2 * DS]);
 * out[1]   = coeff[c_s] * (data[d_s +         DS] + data[d_s +       P * DS]) + coeff[c_s +           CS] * (data[d_s +         2 * DS] + data[d_s +     (P-1) * DS]) + ...
 *                                                                             + coeff[c_s + (P/2-1 * CS)] * (data[d_s +       P/2 * DS] + data[d_s +   (P/2+1) * DS]);
 * ...
 * out[L-1] = coeff[c_s] * (data[d_s + (L-1) * DS] + data[d_s + (P+L-2) * DS]) + coeff[c_s +           CS] * (data[d_s +         L * DS] + data[d_s +   (P+L-3) * DS]) + ...
 *                                                                             + coeff[c_s + (P/2-1 * CS)] * (data[d_s + (P/2+L-2) * DS] + data[d_s + (P/2+L-1) * DS]);
 *
 * @endcode
 *
 * @tparam Lanes      Number of output elements.
 * @tparam Points     Number of data elements used to compute each lane.
 * @tparam CoeffStep  Step used to select elements from the coeff buffer. This step is applied to element selection
 *                    within a lane.
 * @tparam DataStepXY Step used to select elements from the data buffer. This step is applied to element selection
 *                    within a lane and across lanes.
 * @tparam CoeffType  Type of the coefficient elements.
 * @tparam DataType   Type of the data elements.
 * @tparam AccumTag   Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                    the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_sym_ops
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStepXY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE))
using sliding_mul_sym_xy_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepXY, DataStepXY, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * @note Only supported in AIE.
 *
 * This type provides a parametrized multiplication across the lower half of its lanes (equivalent to
 * sliding_mul_sym_ops), and upshifts one selected set of data in the upper half of the lanes.
 *
 * It implements the following compute pattern:
 *
 * @code
 *
 * L   = Lanes
 * P   = Points
 * CS  = CoeffStep
 * DS  = DataStep
 * c_s = coeff_start
 * d_s = data_start
 * ups = uct_shift
 *
 * out[0]     = coeff[c_s] * (data[d_s +             ] + data[d_s +     (P-1) * DS]) + coeff[c_s +           CS] * (data[d_s +               DS] + data[d_s +       (P-2) * DS]) + ...
 *                                                                                   + coeff[c_s + (P/2-1 * CS)] * (data[d_s +     (P/2-1) * DS] + data[d_s +         P/2 * DS]);
 * out[1]     = coeff[c_s] * (data[d_s +           DS] + data[d_s +         P * DS]) + coeff[c_s +           CS] * (data[d_s +           2 * DS] + data[d_s +       (P-1) * DS]) + ...
 *                                                                                   + coeff[c_s + (P/2-1 * CS)] * (data[d_s +         P/2 * DS] + data[d_s +     (P/2+1) * DS]);
 * ...
 * out[L/2-1] = coeff[c_s] * (data[d_s + (L/2-1) * DS] + data[d_s + (P+L/2-2) * DS]) + coeff[c_s +           CS] * (data[d_s +         L/2 * DS] + data[d_s +   (P+L/2-3) * DS]) + ...
 *                                                                                   + coeff[c_s + (P/2-1 * CS)] * (data[d_s + (P/2+L/2-2) * DS] + data[d_s + (P/2+L/2-1) * DS]);
 * out[L/2]   = data[d_s +               P/2 * DS] << ups
 * out[L/2+1] = data[d_s +           (P/2+1) * DS] << ups
 * ...
 * out[L-1]   = data[d_s +       (P/2+L/2-1) * DS] << ups
 *
 * @endcode
 *
 * <table>
 * <caption>Supported parameters for sliding_mul_sym_uct with 48b accumulation</caption>
 * <tr><th>Types (coeff x data)<th>Native lanes<th>Native points<th>CoeffStep<th>DataStepX<th>DataStepY<th>coeff_start<th>data_start
 * <tr><td>c16b x c16b<td>4<td>4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<td>Unsigned smaller than 16<td>Signed
 * <tr><td>32b x c16b <td>4<td>4<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<td>Unsigned smaller than 16<td>Signed
 * <tr><td>c32b x c16b<td>4<td>2<td>1,2,3,4<td>1,2,3,4<td>1,2,3,4<td>Unsigned smaller than 16<td>Signed
 * </table>
 *
 * \note
 * Native lanes denotes the number of outputs handled by a single intrinsic call. For `Lanes = N * Native lanes`, N
 * calls to the underlying intrinsic are made. For `Lanes < Native lanes`, a single call is made and the requested lanes
 * extracted.
 *
 * \note
 * Native points denotes the number of multiplications and additions handled by a single intrinsic call.  Multiples of
 * points are emulated by unrolling multiple intrinsic calls.  For 32b accumulation modes, arbitrary points can be used
 * and are emulated by zeroing coeff lanes.
 *
 * @tparam Lanes     Number of output elements.
 * @tparam Points    Number of data elements used to compute each lane in the first half of the output Lanes.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStep  Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane and across lanes.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_sym_ops
 */
template <unsigned Lanes, unsigned Points, int CoeffStep, int DataStep, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE))
struct sliding_mul_sym_uct_ops {
    static constexpr unsigned accum_bits = detail::to_native_accum_bits_for_mul_types_tag<CoeffType, DataType, AccumTag>();

    static_assert(Points % 2 == 0);

    using impl_type = detail::sliding_mul_sym_uct<Lanes, Points, CoeffStep, DataStep, accum_bits, CoeffType, DataType>;

    enum class SymMulType
    {
        Sym,
        Antisym,
        Acc_Sym,
        Acc_Antisym,
    };

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = accum<detail::accum_tag_or_default_t<AccumTag, CoeffType, DataType>, Lanes>;


    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = impl_type::lanes;
    static constexpr unsigned          points = impl_type::points;

    template <SymMulType MulType, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &data, unsigned data_start,
                                           unsigned uct_shift,
                                           const Acc &...acc)
    {
        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, data, data_start, uct_shift, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(op_none(coeff), coeff_start, data, data_start, uct_shift,  acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, op_none(data), data_start, uct_shift,  acc...);
        }
        else {
            if      constexpr (VecCoeff::is_operation_not(Operation::Conj))
                return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff(), coeff_start, data, data_start, uct_shift, acc...);
            else if constexpr (VecData::is_operation_not(Operation::Conj))
                return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, data(), data_start, uct_shift, acc...);
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (MulType == SymMulType::Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, uct_shift);
                else if constexpr (MulType == SymMulType::Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, uct_shift);
                else if constexpr (MulType == SymMulType::Acc_Sym)
                    return impl_type::template run<detail::to_mul_sym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, uct_shift, acc.parent1()...);
                else if constexpr (MulType == SymMulType::Acc_Antisym)
                    return impl_type::template run<detail::to_mul_antisym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, data.parent1(), data_start, uct_shift, acc.parent1()...);
            }

        }
    }

    template <SymMulType MulType, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &ldata, unsigned ldata_start,
                                           const VecData &rdata, unsigned rdata_start,
                                           unsigned uct_shift,
                                           const Acc &...acc)
    {
        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(op_none(coeff), coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, op_none(ldata), ldata_start, op_none(rdata), rdata_start, uct_shift, acc...);
        }
        else {
            if      constexpr (VecData::is_operation_not(Operation::Conj))
                return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff, coeff_start, ldata(), ldata_start, rdata(), rdata_start, uct_shift, acc...);
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj))
                return sliding_mul_sym_uct_ops::mul_common<MulType>(coeff(), coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, acc...);
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (MulType == SymMulType::Sym)
                    return impl_type::template run_2buff<detail::to_mul_sym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, uct_shift);
                else if constexpr (MulType == SymMulType::Antisym)
                    return impl_type::template run_2buff<detail::to_mul_antisym_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, uct_shift);
                else if constexpr (MulType == SymMulType::Acc_Sym)
                    return impl_type::template run_2buff<detail::to_mul_sym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, uct_shift, acc.parent1()...);
                else if constexpr (MulType == SymMulType::Acc_Antisym)
                    return impl_type::template run_2buff<detail::to_mul_antisym_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, ldata.parent1(), ldata_start, rdata.parent1(), rdata_start, uct_shift, acc.parent1()...);
            }
        }
    }


    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_sym_uct(const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned data_start,
                                            unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Sym>(coeff, coeff_start, data, data_start, uct_shift);
    }

    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_sym_uct(const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &ldata, unsigned ldata_start,
                                            const VecData &rdata, unsigned rdata_start,
                                            unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Sym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
    }

    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_sym_uct(const Acc &acc,
                                            const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &data, unsigned data_start,
                                            unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Acc_Sym>(coeff, coeff_start, data, data_start, uct_shift, acc);
    }

    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_sym_uct(const Acc &acc,
                                            const VecCoeff &coeff, unsigned coeff_start,
                                            const VecData &ldata, unsigned ldata_start,
                                            const VecData &rdata, unsigned rdata_start,
                                            unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Acc_Sym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, acc);
    }

    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_antisym_uct(const VecCoeff &coeff, unsigned coeff_start,
                                                const VecData &data, unsigned data_start,
                                                unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Antisym>(coeff, coeff_start, data, data_start, uct_shift);
    }

    template <VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mul_antisym_uct(const VecCoeff &coeff, unsigned coeff_start,
                                                const VecData &ldata, unsigned ldata_start,
                                                const VecData &rdata, unsigned rdata_start,
                                                unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Antisym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
    }

    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_antisym_uct(const Acc &acc,
                                                const VecCoeff &coeff, unsigned coeff_start,
                                                const VecData &data, unsigned data_start,
                                                unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Acc_Antisym>(coeff, coeff_start, data, data_start, uct_shift, acc);
    }

    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData> requires(is_valid_mul_op_v<CoeffType, DataType>)
    __aie_inline
    static constexpr accum_type mac_antisym_uct(const Acc &acc,
                                                const VecCoeff &coeff, unsigned coeff_start,
                                                const VecData &ldata, unsigned ldata_start,
                                                const VecData &rdata, unsigned rdata_start,
                                                unsigned uct_shift)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_sym_uct_ops::mul_common<SymMulType::Acc_Antisym>(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift, acc);
    }

    //TODO: Add entry point for negmul
};


template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_sym(const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &data,
                     unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_sym(coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_sym(const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &data,
                     unsigned ldata_start,
                     unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_sym(coeff, coeff_start, data, ldata_start, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mul_sym(const VecCoeff &coeff,
                     const VecData  &data,
                     unsigned data_start)
{
    return sliding_mul_sym<Lanes, Points, CoeffStep, DataStep, AccumTag>(coeff, CoeffStart, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_sym(const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &ldata,
                     unsigned ldata_start,
                     const VecData  &rdata,
                     unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;//accum_bits>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_sym(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, typename VecCoeff, typename VecData>
    requires(arch::is(arch::AIE) &&
             VectorOrOp<VecCoeff> && VectorOrOp<VecData> && is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mul_sym(const VecCoeff &coeff,
                     const VecData  &ldata,
                     unsigned ldata_start,
                     const VecData  &rdata,
                     unsigned rdata_start)
{
    return sliding_mul_sym<Lanes, Points, CoeffStep, DataStep, AccumTag>(coeff, CoeffStart, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_sym(const Acc &acc,
                     const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &data,
                     unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_sym(acc, coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_sym(const Acc &acc,
                     const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &data,
                     unsigned ldata_start,
                     unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_sym(acc, coeff, coeff_start, data, ldata_start, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mac_sym(const Acc &acc,
                     const VecCoeff &coeff,
                     const VecData  &data,
                     unsigned data_start)
{
    return sliding_mac_sym<Lanes, Points, CoeffStep, DataStep>(acc, coeff, CoeffStart, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_sym(const Acc &acc,
                     const VecCoeff &coeff,
                     unsigned coeff_start,
                     const VecData  &ldata,
                     unsigned ldata_start,
                     const VecData  &rdata,
                     unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_sym(acc, coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mac_sym(const Acc &acc,
                     const VecCoeff &coeff,
                     const VecData  &ldata,
                     unsigned ldata_start,
                     const VecData  &rdata,
                     unsigned rdata_start)
{
    return sliding_mac_sym<Lanes, Points, CoeffStep, DataStep>(acc, coeff, CoeffStart, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_antisym(const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &data,
                         unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_antisym(coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_antisym(const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &data,
                         unsigned ldata_start,
                         unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_antisym(coeff, coeff_start, data, ldata_start, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mul_antisym(const VecCoeff &coeff,
                         const VecData  &data,
                         unsigned data_start)
{
    return sliding_mul_antisym<Lanes, Points, CoeffStep, DataStep, AccumTag>(coeff, CoeffStart, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_antisym(const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &ldata,
                         unsigned ldata_start,
                         const VecData  &rdata,
                         unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_antisym(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, typename VecCoeff, typename VecData>
    requires(arch::is(arch::AIE) &&
             VectorOrOp<VecCoeff> && VectorOrOp<VecData> && is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mul_antisym(const VecCoeff &coeff,
                         const VecData  &ldata,
                         unsigned ldata_start,
                         const VecData  &rdata,
                         unsigned rdata_start)
{
    return sliding_mul_antisym<Lanes, Points, CoeffStep, DataStep, AccumTag>(coeff, CoeffStart, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_antisym(const Acc &acc,
                         const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &data,
                         unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_antisym(acc, coeff, coeff_start, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_antisym(const Acc &acc,
                         const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &data,
                         unsigned ldata_start,
                         unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_antisym(acc, coeff, coeff_start, data, ldata_start, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mac_antisym(const Acc &acc,
                         const VecCoeff &coeff,
                         const VecData  &data,
                         unsigned data_start)
{
    return sliding_mac_antisym<Lanes, Points, CoeffStep, DataStep>(acc, coeff, CoeffStart, data, data_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_antisym(const Acc &acc,
                         const VecCoeff &coeff,
                         unsigned coeff_start,
                         const VecData  &ldata,
                         unsigned ldata_start,
                         const VecData  &rdata,
                         unsigned rdata_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert?
    using mul_ops = sliding_mul_sym_ops<Lanes, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_antisym(acc, coeff, coeff_start, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStart = 0, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
[[deprecated("Use the variant with coeff_start as an argument")]]
__aie_inline
auto sliding_mac_antisym(const Acc &acc,
                         const VecCoeff &coeff,
                         const VecData  &ldata,
                         unsigned ldata_start,
                         const VecData  &rdata,
                         unsigned rdata_start)
{
    return sliding_mac_antisym<Lanes, Points, CoeffStep, DataStep>(acc, coeff, CoeffStart, ldata, ldata_start, rdata, rdata_start);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_sym_uct(const VecCoeff &coeff, unsigned coeff_start,
                         const VecData &data, unsigned data_start,
                         unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_sym_uct(coeff, coeff_start, data, data_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_sym_uct(const VecCoeff &coeff, unsigned coeff_start,
                         const VecData &ldata, unsigned ldata_start,
                         const VecData &rdata, unsigned rdata_start,
                         unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_sym_uct(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_sym_uct(const Acc &acc,
                         const VecCoeff &coeff, unsigned coeff_start,
                         const VecData &data, unsigned data_start,
                         unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_sym_uct(acc, coeff, coeff_start, data, data_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_sym_uct(const Acc &acc,
                         const VecCoeff &coeff, unsigned coeff_start,
                         const VecData &ldata, unsigned ldata_start,
                         const VecData &rdata, unsigned rdata_start,
                         unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_sym_uct(acc, coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_antisym_uct(const VecCoeff &coeff, unsigned coeff_start,
                             const VecData &data, unsigned data_start,
                             unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_antisym_uct(coeff, coeff_start, data, data_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_antisym_uct(const VecCoeff &coeff, unsigned coeff_start,
                             const VecData &ldata, unsigned ldata_start,
                             const VecData &rdata, unsigned rdata_start,
                             unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul_antisym_uct(coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_antisym_uct(const Acc &acc,
                             const VecCoeff &coeff, unsigned coeff_start,
                             const VecData &data, unsigned data_start,
                             unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_antisym_uct(acc, coeff, coeff_start, data, data_start, uct_shift);
}

template <unsigned Lanes, unsigned Points, int CoeffStep = 1, int DataStep = 1,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE) &&
             is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Lanes))
__aie_inline
auto sliding_mac_antisym_uct(const Acc &acc,
                             const VecCoeff &coeff, unsigned coeff_start,
                             const VecData &ldata, unsigned ldata_start,
                             const VecData &rdata, unsigned rdata_start,
                             unsigned uct_shift)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    using mul_ops = sliding_mul_sym_uct_ops<Lanes, Points, CoeffStep, DataStep, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac_antisym_uct(acc, coeff, coeff_start, ldata, ldata_start, rdata, rdata_start, uct_shift);
}

/**
 * @ingroup group_mul_special
 *
 * @note Supported added in AIE-ML.
 *
 * This type provides a parametrized multiplication that implements the following compute pattern:
 *
 * @code
 *
 * DSX = DataStepX
 * DSY = DataStepY
 * CS  = CoeffStep
 * P   = Points
 * C   = Channels
 * L   = Lanes
 * c_s = coeff_start
 * d_s = data_start
 *
 * for l in 0:L
 *   for p in 0:P
 *     out[l] += coeff[(c_s + (p * CS)) * C + (l % C)] * data[(d_s + ((l / C + p) * DSX)) * C + (l % C) * DSY]
 *
 * More explicitly:
 *
 * out[0]   = coeff[c_s * C        ] * data[ d_s                  * C              ] + coeff[(c_s + CS) * C        ] * data[(d_s +                 DSX) * C              ] + ... + coeff[(c_s + (P-1) * CS) * C        ] * data[(d_s +            (P-1)  * DSX) * C              ]
 * out[1]   = coeff[c_s * C +   1  ] * data[ d_s                  * C +         DSY] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +                 DSX) * C +         DSY] + ... + coeff[(c_s + (P-1) * CS) * C +   1  ] * data[(d_s +            (P-1)  * DSX) * C +         DSY]
 * ...
 * out[C-1] = coeff[c_s * C + (C-1)] * data[ d_s                  * C + (C-1) * DSY] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s +                 DSX) * C + (C-1) * DSY] + ... + coeff[(c_s + (P-1) * CS) + C + (C-1)] * data[(d_s +            (P-1)  * DSX) * C + (C-1) * DSY]
 * out[C]   = coeff[c_s * C +      ] * data[(d_s +           DSX) * C              ] + coeff[(c_s + CS) * C        ] * data[(d_s +       (1 + 1) * DSX) * C              ] + ... + coeff[(c_s + (P-1) * CS) + C        ] * data[(d_s +      (1  + (P-1)) * DSX) * C              ]
 * out[C+1] = coeff[c_s * C +   1  ] * data[(d_s +           DSX) * C +         DSY] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +       (1 + 1) * DSX) * C +         DSY] + ... + coeff[(c_s + (P-1) * CS) + C +   1  ] * data[(d_s +      (1  + (P-1)) * DSX) * C +         DSY]
 * ...
 * out[L-1] = coeff[c_s * C + (C-1)] * data[(d_s + (L-1)/C * DSX) * C + (C-1) * DSY] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s + ((L-1)/C + 1) * DSX) * C + (C-1) * DSY] + ... + coeff[(c_s + (P-1) * CS) * C + (C-1)] * data[(d_s + ((L-1)/C + (P-1)) * DSX) * C + (C-1) * DSY]
 *
 * @endcode
 *
 * <table>
 * <caption>Supported parameters for sliding_mul_ch with 32b accumulation</caption>
 * <tr><th>Types (coeff x data) <th colspan=2>Native accum.           <th>Native lanes <th>Native points <th>Channels <th>CoeffStep <th>DataStep <th>coeff_start <th>data_start
 * <tr><td rowspan=2>8b x 8b    <td rowspan=2>AIE-ML   <td> acc32     <td>4            <td>4             <td>8        <td>1         <td>1        <td>Unsigned    <td>Signed
 * <tr>                                                <td> acc32     <td>8            <td>8             <td>4        <td>1         <td>1        <td>Unsigned    <td>Signed
 * </table>
 *
 * \note
 * Native lanes denotes the number of outputs handled by a single intrinsic call. For `Lanes = N * Native lanes`, N
 * calls to the underlying intrinsic are made. For `Lanes < Native lanes`, a single call is made and the requested lanes
 * extracted .
 *
 * \note
 * Native points denotes the number of multiplications and additions handled by a single intrinsic call.  Other values
 * are emulated internally by zeroing certain coeff lanes and/or unrolling multiple intrinsic calls.
 *
 * \note
 * coeff_start and data_start wrap around if greater than the number of values.
 *
 * @tparam Outputs   Number of output samples per channel.
 * @tparam Channels  Number of channels.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_ch_x_ops, sliding_mul_ch_y_ops, sliding_mul_ch_xy_ops
 */
template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStepX, int DataStepY,
          ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::sliding_mul_ch_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE_ML))
struct sliding_mul_ch_ops
{
    static constexpr unsigned accum_bits = detail::to_native_accum_bits_for_mul_types_tag<CoeffType, DataType, AccumTag>();
    static constexpr unsigned max_coeff_bits = 512;
    static constexpr unsigned max_data_bits = 1024;
    static constexpr unsigned Lanes = Outputs * Channels;

    using impl_type = detail::sliding_mul_ch<Outputs, Channels, Points, CoeffStep, DataStepX, DataStepY, accum_bits, CoeffType, DataType>;

    enum class MulType
    {
        Mul,
        Acc_Mul,
        NegMul
    };

    using  data_type = typename impl_type::data_type;
    using coeff_type = typename impl_type::coeff_type;
    using accum_type = accum<detail::accum_tag_or_default_t<AccumTag, CoeffType, DataType>, Lanes>;

    static constexpr unsigned columns_per_mul = impl_type::columns_per_mul;
    static constexpr unsigned   lanes_per_mul = impl_type::lanes_per_mul;
    static constexpr unsigned         num_mul = impl_type::num_mul;
    static constexpr unsigned           lanes = impl_type::lanes;
    static constexpr unsigned          points = impl_type::points;

    template <MulType Mul, VectorOrOp VecCoeff, VectorOrOp VecData, AccumOrOp... Acc>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
    __aie_inline
    static constexpr accum_type mul_common(const VecCoeff &coeff, unsigned coeff_start,
                                           const VecData &data, unsigned data_start,
                                           const Acc &...acc)
    {
        static_assert((std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag> && ...));

        if      constexpr (sizeof...(Acc) == 1 && (... && !is_op_v<Acc>)) {
            return sliding_mul_ch_ops::mul_common<Mul>(coeff, coeff_start, data, data_start, op_add(acc)...);
        }
        else if constexpr (!is_op_v<VecCoeff>) {
            return sliding_mul_ch_ops::mul_common<Mul>(op_none(coeff), coeff_start, data, data_start, acc...);
        }
        else if constexpr (!is_op_v<VecData>) {
            return sliding_mul_ch_ops::mul_common<Mul>(coeff, coeff_start, op_none(data), data_start, acc...);
        }
        else {
            if      constexpr (VecData::is_operation_not(Operation::Conj, Operation::Sign))
                return sliding_mul_ch_ops::mul_common<Mul>(coeff, coeff_start, data(), data_start, acc...);
            else if constexpr (VecCoeff::is_operation_not(Operation::Conj, Operation::Sign))
                return sliding_mul_ch_ops::mul_common<Mul>(coeff(), coeff_start, data, data_start, acc...);
            else {
                constexpr Operation OpCoeff = detail::evaluate_mul_operation<VecCoeff>();
                constexpr Operation OpData  = detail::evaluate_mul_operation<VecData>();

                if      constexpr (Mul == MulType::Mul)
                    return impl_type::template run<detail::to_mul_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data));
                else if constexpr (Mul == MulType::Acc_Mul)
                    return impl_type::template run<detail::to_mul_macro_op<Acc::operation..., OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data), acc.parent1()...);
                else if constexpr (Mul == MulType::Negmul)
                    return impl_type::template run<detail::to_negmul_macro_op<OpData, OpCoeff>()>(coeff.parent1(), coeff_start, detail::get_mul_sign(coeff), data.parent1(), data_start, detail::get_mul_sign(data));
            }
        }
    }

    /**
     * Performs the multiplication pattern defined by the class parameters using the input coefficient and data
     * arguments.
     *
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type mul(const VecCoeff &coeff, unsigned coeff_start,
                                    const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ch_ops::mul_common<MulType::Mul>(coeff, coeff_start, data, data_start);
    }

    /**
     * Performs a multiply-add with the pattern defined by the class parameters using the input coefficient and data
     * arguments.
     *
     * @param acc         Accumulator that is added to the result of the multiplication.
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type mac(const Acc &acc,
                                    const VecCoeff &coeff, unsigned coeff_start,
                                    const VecData &data, unsigned data_start)
    {
        static_assert(std::is_same_v<typename operand_base_type_t<Acc>::value_type, AccumTag>);

        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ch_ops::mul_common<MulType::Acc_Mul>(coeff, coeff_start, data, data_start, acc);
    }

    /**
     * Performs a negation of the multiplication pattern defined by the class parameters using the input coefficient and
     * data arguments.
     *
     * @param coeff       Vector of coefficients. Vectors limited to 256b and 512b on AIE and AIE-ML respectively.
     * @param coeff_start Index of the first coefficient element to be used in the multiplication.
     * @param data        Vector of data samples.
     * @param data_start  Index of the first data element to be used in the multiplication.
     */
    template <VectorOrOp VecCoeff, VectorOrOp VecData>
        requires(is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>
                 && (VecCoeff::bits() <= max_coeff_bits)
                 && (VecData::bits() <= max_data_bits))
    __aie_inline
    static constexpr accum_type negmul(const VecCoeff &coeff, unsigned coeff_start,
                                       const VecData &data, unsigned data_start)
    {
        REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

        return sliding_mul_ch_ops::mul_common<MulType::NegMul>(coeff, coeff_start, data, data_start);
    }
};

/**
 * @ingroup group_mul_special
 *
 * @note Only supported from AIE-ML.
 *
 * Similar to @ref sliding_mul_ch_ops, but DataStepY is always 1.
 *
 * @code
 *
 * DSX = DataStepX
 * CS  = CoeffStep
 * P   = Points
 * C   = Channels
 * L   = Lanes
 * c_s = coeff_start
 * d_s = data_start
 *
 * for l in 0:L
 *   for p in 0:P
 *     out[l] += coeff[(c_s + (p * CS)) * C + (l % C)] * data[(d_s + ((l / C + p) * DSX)) * C + (l % C)]
 *
 * More explicitly:
 *
 * out[0]   = coeff[c_s * C        ] * data[ d_s                  * C              ] + coeff[(c_s + CS) * C        ] * data[(d_s +                 DSX) * C              ] + ... + coeff[(c_s + (P-1) * CS) * C        ] * data[(d_s +            (P-1)  * DSX) * C              ]
 * out[1]   = coeff[c_s * C +   1  ] * data[ d_s                  * C +           1] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +                 DSX) * C +           1] + ... + coeff[(c_s + (P-1) * CS) * C +   1  ] * data[(d_s +            (P-1)  * DSX) * C +           1]
 * ...
 * out[C-1] = coeff[c_s * C + (C-1)] * data[ d_s                  * C +       (C-1)] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s +                 DSX) * C +       (C-1)] + ... + coeff[(c_s + (P-1) * CS) + C + (C-1)] * data[(d_s +            (P-1)  * DSX) * C +       (C-1)]
 * out[C]   = coeff[c_s * C +      ] * data[(d_s +           DSX) * C              ] + coeff[(c_s + CS) * C +      ] * data[(d_s +       (1 + 1) * DSX) * C              ] + ... + coeff[(c_s + (P-1) * CS) + C        ] * data[(d_s +      (1  + (P-1)) * DSX) * C              ]
 * out[C+1] = coeff[c_s * C +   1  ] * data[(d_s +           DSX) * C +           1] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +       (1 + 1) * DSX) * C +           1] + ... + coeff[(c_s + (P-1) * CS) + C +   1  ] * data[(d_s +      (1  + (P-1)) * DSX) * C +           1]
 * ...
 * out[L-1] = coeff[c_s * C + (C-1)] * data[(d_s + (L-1)/C * DSX) * C +       (C-1)] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s + ((L-1)/C + 1) * DSX) * C +       (C-1)] + ... + coeff[(c_s + (P-1) * CS) * C + (C-1)] * data[(d_s + ((L-1)/C + (P-1)) * DSX) * C +       (C-1)]
 *
 * @endcode
 *
 * @tparam Outputs   Number of output samples.
 * @tparam Channels  Number of channels.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepX Step used to select elements from the data buffer. This step is applied to element selection within
 *                   a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_ch_ops, sliding_mul_ch_y_ops, sliding_mul_ch_xy_ops
 */
template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStepX, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE_ML))
using sliding_mul_ch_x_ops = sliding_mul_ch_ops<Outputs, Channels, Points, CoeffStep, DataStepX, 1, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * @note Only supported from AIE-ML.
 *
 * Similar to @ref sliding_mul_ch_ops, but DataStepX is always 1.
 *
 * @code
 *
 * DSY = DataStepY
 * CS  = CoeffStep
 * P   = Points
 * C   = Channels
 * L   = Lanes
 * c_s = coeff_start
 * d_s = data_start
 *
 * for l in 0:L
 *   for p in 0:P
 *     out[l] += coeff[(c_s + (p * CS)) * C + (l % C)] * data[(d_s + ((l / C + p))) * C + (l % C) * DSY]
 *
 * More explicitly:
 *
 * out[0]   = coeff[c_s * C        ] * data[ d_s            * C              ] + coeff[(c_s + CS) * C        ] * data[(d_s +             1) * C              ] + ... + coeff[(c_s + (P-1) * CS) * C        ] * data[(d_s +             (P-1)) * C              ]
 * out[1]   = coeff[c_s * C +   1  ] * data[ d_s            * C +         DSY] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +             1) * C +         DSY] + ... + coeff[(c_s + (P-1) * CS) * C +   1  ] * data[(d_s +             (P-1)) * C +         DSY]
 * ...
 * out[C-1] = coeff[c_s * C + (C-1)] * data[ d_s            * C + (C-1) * DSY] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s +             1) * C + (C-1) * DSY] + ... + coeff[(c_s + (P-1) * CS) + C + (C-1)] * data[(d_s +            (P-1) ) * C + (C-1) * DSY]
 * out[C]   = coeff[c_s * C +      ] * data[(d_s +       1) * C              ] + coeff[(c_s + CS) * C        ] * data[(d_s +       (1 + 1)) * C              ] + ... + coeff[(c_s + (P-1) * CS) + C        ] * data[(d_s +      (1  + (P-1))) * C              ]
 * out[C+1] = coeff[c_s * C +   1  ] * data[(d_s +       1) * C +         DSY] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +       (1 + 1)) * C +         DSY] + ... + coeff[(c_s + (P-1) * CS) + C +   1  ] * data[(d_s +      (1  + (P-1))) * C +         DSY]
 * ...
 * out[L-1] = coeff[c_s * C + (C-1)] * data[(d_s + (L-1)/C) * C + (C-1) * DSY] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s + ((L-1)/C + 1)) * C + (C-1) * DSY] + ... + coeff[(c_s + (P-1) * CS) * C + (C-1)] * data[(d_s + ((L-1)/C + (P-1))) * C + (C-1) * DSY]
 *
 * @endcode
 *
 * @tparam Outputs   Number of output samples.
 * @tparam Channels  Number of channels.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStepY Step used to select elements from the data buffer. This step is applied to element selection within
 *                   a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_ch_ops, sliding_mul_ch_x_ops, sliding_mul_ch_xy_ops
 */
template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStepY, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE_ML))
using sliding_mul_ch_y_ops = sliding_mul_ch_ops<Outputs, Channels, Points, CoeffStep, 1, DataStepY, CoeffType, DataType, AccumTag>;

/**
 * @ingroup group_mul_special
 *
 * @note Only supported from AIE-ML.
 *
 * Similar to @ref sliding_mul_ch_ops, but DataStepX is equal to DataStepY.
 *
 * @code
 *
 * DS  = DataStep
 * CS  = CoeffStep
 * P   = Points
 * C   = Channels
 * L   = Lanes
 * c_s = coeff_start
 * d_s = data_start
 *
 * for l in 0:L
 *   for p in 0:P
 *     out[l] += coeff[(c_s + (p * CS)) * C + (l % C)] * data[(d_s + ((l / C + p) * DS)) * C + (l % C) * DS]
 *
 * More explicitly:
 *
 * out[0]   = coeff[c_s * C        ] * data[ d_s                 * C             ] + coeff[(c_s + CS) * C        ] * data[(d_s +                 DS) * C             ] + ... + coeff[(c_s + (P-1) * CS) * C        ] * data[(d_s +            (P-1)  * DS) * C             ]
 * out[1]   = coeff[c_s * C +   1  ] * data[ d_s                 * C +         DS] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +                 DS) * C +         DS] + ... + coeff[(c_s + (P-1) * CS) * C +   1  ] * data[(d_s +            (P-1)  * DS) * C +         DS]
 * ...
 * out[C-1] = coeff[c_s * C + (C-1)] * data[ d_s                 * C + (C-1) * DS] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s +                 DS) * C + (C-1) * DS] + ... + coeff[(c_s + (P-1) * CS) + C + (C-1)] * data[(d_s +            (P-1)  * DS) * C + (C-1) * DS]
 * out[C]   = coeff[c_s * C +      ] * data[(d_s +           DS) * C             ] + coeff[(c_s + CS) * C        ] * data[(d_s +       (1 + 1) * DS) * C             ] + ... + coeff[(c_s + (P-1) * CS) + C        ] * data[(d_s +      (1  + (P-1)) * DS) * C             ]
 * out[C+1] = coeff[c_s * C +   1  ] * data[(d_s +           DS) * C +         DS] + coeff[(c_s + CS) * C +   1  ] * data[(d_s +       (1 + 1) * DS) * C +         DS] + ... + coeff[(c_s + (P-1) * CS) + C +   1  ] * data[(d_s +      (1  + (P-1)) * DS) * C +         DS]
 * ...
 * out[L-1] = coeff[c_s * C + (C-1)] * data[(d_s + (L-1)/C * DS) * C + (C-1) * DS] + coeff[(c_s + CS) * C + (C-1)] * data[(d_s + ((L-1)/C + 1) * DS) * C + (C-1) * DS] + ... + coeff[(c_s + (P-1) * CS) * C + (C-1)] * data[(d_s + ((L-1)/C + (P-1)) * DS) * C + (C-1) * DS]
 *
 * @endcode
 *
 * @tparam Outputs   Number of output samples.
 * @tparam Channels  Number of channels.
 * @tparam Points    Number of data elements used to compute each lane.
 * @tparam CoeffStep Step used to select elements from the coeff buffer. This step is applied to element selection
 *                   within a lane.
 * @tparam DataStep  Step used to select elements from the data buffer. This step is applied to element selection within
 *                   a lane.
 * @tparam CoeffType Type of the coefficient elements.
 * @tparam DataType  Type of the data elements.
 * @tparam AccumTag  Accumulator tag that specifies the required accumulation bits. The class must be compatible with
 *                   the result of the multiplication of the coefficient and data types (real/complex).
 *
 * @sa sliding_mul_ch_ops, sliding_mul_ch_x_ops, sliding_mul_ch_y_ops
 */
template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep, int DataStep, ElemBaseType CoeffType, ElemBaseType DataType, AccumElemBaseType AccumTag = detail::default_accum_tag_t<CoeffType, DataType>>
    requires(arch::is(arch::AIE_ML))
using sliding_mul_ch_xy_ops = sliding_mul_ch_ops<Outputs, Channels, Points, CoeffStep, DataStep, DataStep, CoeffType, DataType, AccumTag>;

//TODO: implement dynamic sign support
template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumElemBaseType AccumTag = accauto, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE_ML) && is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type>)
__aie_inline
auto sliding_mul_ch(const VecCoeff &coeff,
                    unsigned coeff_start,
                    const VecData  &data,
                    unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert with supported parameters
    using mul_ops = sliding_mul_ch_ops<Outputs, Channels, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, AccumTag>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mul(coeff, coeff_start, data, data_start);
}

template <unsigned Outputs, unsigned Channels, unsigned Points, int CoeffStep = 1, int DataStepX = 1, int DataStepY = DataStepX,
          AccumOrOp Acc, VectorOrOp VecCoeff, VectorOrOp VecData>
    requires(arch::is(arch::AIE_ML) && is_valid_mul_op_v<typename VecCoeff::value_type, typename VecData::value_type> && (Acc::size() == Outputs * Channels))
__aie_inline
auto sliding_mac_ch(const Acc &acc,
                    const VecCoeff &coeff,
                    unsigned coeff_start,
                    const VecData  &data,
                    unsigned data_start)
{
    using CoeffType = typename VecCoeff::value_type;
    using DataType  = typename VecData::value_type;

    // TODO: static_assert with supported parameters
    using mul_ops = sliding_mul_ch_ops<Outputs, Channels, Points, CoeffStep, DataStepX, DataStepY, CoeffType, DataType, typename Acc::value_type>;

    REQUIRES_CONSTANT_MSG(coeff_start, "coeff_start must be a compile-time constant");

    return mul_ops::mac(acc, coeff, coeff_start, data, data_start);
}

} // namespace aie

#endif
