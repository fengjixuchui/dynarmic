/* This file is part of the dynarmic project.
 * Copyright (c) 2021 MerryMage
 * SPDX-License-Identifier: 0BSD
 */

#include "frontend/A32/translate/impl/translate_thumb.h"

namespace Dynarmic::A32 {
static IR::U32 Pack2x16To1x32(A32::IREmitter& ir, IR::U32 lo, IR::U32 hi) {
    return ir.Or(ir.And(lo, ir.Imm32(0xFFFF)), ir.LogicalShiftLeft(hi, ir.Imm8(16), ir.Imm1(0)).result);
}

static IR::U16 MostSignificantHalf(A32::IREmitter& ir, IR::U32 value) {
    return ir.LeastSignificantHalf(ir.LogicalShiftRight(value, ir.Imm8(16), ir.Imm1(0)).result);
}

using SaturationFunction = IR::ResultAndOverflow<IR::U32> (IREmitter::*)(const IR::U32&, size_t);

static bool Saturation16(ThumbTranslatorVisitor& v, Reg n, Reg d, size_t saturate_to, SaturationFunction sat_fn) {
    if (d == Reg::PC || n == Reg::PC) {
        return v.UnpredictableInstruction();
    }

    const auto reg_n = v.ir.GetRegister(n);

    const auto lo_operand = v.ir.SignExtendHalfToWord(v.ir.LeastSignificantHalf(reg_n));
    const auto hi_operand = v.ir.SignExtendHalfToWord(MostSignificantHalf(v.ir, reg_n));
    const auto lo_result = (v.ir.*sat_fn)(lo_operand, saturate_to);
    const auto hi_result = (v.ir.*sat_fn)(hi_operand, saturate_to);

    v.ir.SetRegister(d, Pack2x16To1x32(v.ir, lo_result.result, hi_result.result));
    v.ir.OrQFlag(lo_result.overflow);
    v.ir.OrQFlag(hi_result.overflow);
    return true;
}

bool ThumbTranslatorVisitor::thumb32_MOVT(Imm<1> imm1, Imm<4> imm4, Imm<3> imm3, Reg d, Imm<8> imm8) {
    if (d == Reg::PC) {
        return UnpredictableInstruction();
    }

    const IR::U32 imm16 = ir.Imm32(concatenate(imm4, imm1, imm3, imm8).ZeroExtend() << 16);
    const IR::U32 operand = ir.GetRegister(d);
    const IR::U32 result = ir.Or(ir.And(operand, ir.Imm32(0x0000FFFFU)), imm16);

    ir.SetRegister(d, result);
    return true;
}

bool ThumbTranslatorVisitor::thumb32_MOVW_imm(Imm<1> imm1, Imm<4> imm4, Imm<3> imm3, Reg d, Imm<8> imm8) {
    if (d == Reg::PC) {
        return UnpredictableInstruction();
    }

    const IR::U32 imm = ir.Imm32(concatenate(imm4, imm1, imm3, imm8).ZeroExtend());

    ir.SetRegister(d, imm);
    return true;
}

bool ThumbTranslatorVisitor::thumb32_SSAT16(Reg n, Reg d, Imm<4> sat_imm) {
    return Saturation16(*this, n, d, sat_imm.ZeroExtend() + 1, &IREmitter::SignedSaturation);
}

bool ThumbTranslatorVisitor::thumb32_USAT16(Reg n, Reg d, Imm<4> sat_imm) {
    return Saturation16(*this, n, d, sat_imm.ZeroExtend(), &IREmitter::UnsignedSaturation);
}

} // namespace Dynarmic::A32
