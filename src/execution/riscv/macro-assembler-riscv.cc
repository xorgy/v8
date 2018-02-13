// Copyright 2018 Aaron Muir Hamilton
//
// Permission to use, copy, modify, and/or distribute this software for
// any purpose with or without fee is hereby granted, provided that the
// above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
// WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
// AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
// PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
// TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#if V8_TARGET_ARCH_RISCV

#include "src/riscv/macro-assembler-riscv.h"
#include "src/base/bits.h"

namespace v8 {
namespace internal {

void TurboAssembler::EnterFrame(StackFrame::Type type) {
  // FIXME #0012:
  // - probably incorrect
  // - xlen-dependent
  int stack_offset, fp_offset;
  if (type == StackFrame::INTERNAL) {
    stack_offset = -4 * kPointerSize;
    fp_offset = 2 * kPointerSize;
  } else {
    stack_offset = -3 * kPointerSize;
    fp_offset = 1 * kPointerSize;
  }
  emit(ADDI, sp, sp, stack_offset);
  stack_offset = -stack_offset - kPointerSize;
  emit(SD, ra, sp, stack_offset);
  stack_offset -= kPointerSize;
  emit(SD, fp, sp, stack_offset);
  stack_offset -= kPointerSize;
  li(t6, Operand(StackFrame::TypeToMarker(type)));
  emit(SD, t6, sp, stack_offset);
  if (type == StackFrame::INTERNAL) {
    DCHECK_EQ(stack_offset, kPointerSize);
    li(t6, Operand(CodeObject()));
    emit(SD, t6, sp, 0);
  } else {
    DCHECK_EQ(stack_offset, 0);
  }
}

void TurboAssembler::LeaveFrame(StackFrame::Type type) {
  // FIXME #0015: definitely wrong
  emit(ADDI, sp, fp, 2 * kPointerSize);
  emit(LD, ra, fp, 1 * kPointerSize);
  emit(LD, fp, fp, 0 * kPointerSize);
}

void TurboAssembler::li(Register rd, int64_t j) {
  // FIXME #0021: verify
  BlockTrampolinePoolScope block_trampoline_pool(this);
  emit(LUI, rd, j >> 44 & 0xFFFFF);
  emit(ORI, rd, rd, j >> 32 & 0xFFF);
  emit(SLLI, rd, rd, 12);
  emit(ORI, rd, rd, j >> 20 & 0xFFF);
  emit(SLLI, rd, rd, 12);
  emit(ORI, rd, rd, j >> 8 & 0xFFF);
  emit(SLLI, rd, rd, 8);
  emit(ORI, rd, rd, j & 0xFF);
}

void TurboAssembler::li(Register rd, int32_t j) {
  // FIXME #0022: verify
  BlockTrampolinePoolScope block_trampoline_pool(this);
  emit(LUI, rd, (j & 0xFFFFF000) >> 12);
  emit(ORI, rd, rd, j & 0x00000FFF);
}

void TurboAssembler::li(Register rd, int64_t j, Register scratch) {
  // FIXME #0014: verify
  BlockTrampolinePoolScope block_trampoline_pool(this);
  DCHECK_NE(rd, scratch);
  emit(LUI, rd, (j & 0xFFFFF000) >> 12);
  emit(ORI, rd, rd, j & 0xFFF);
  emit(SLLI, rd, rd, 32);
  int32_t high_imm = j >> 32;
  emit(LUI, scratch, high_imm >> 12);
  emit(ORI, scratch, scratch, high_imm & 0xFFF);
  emit(OR, rd, rd, scratch);
}

void TurboAssembler::li_smallest(Register rd, int64_t j) {
  // FIXME #0023: fix and finish
  BlockTrampolinePoolScope block_trampoline_pool(this);
  unsigned trailing = CountTrailingZeros(j);
  unsigned leading = CountLeadingZeros(j);
  unsigned solid = 64 - trailing - leading;
  if (solid <= 12) {
    if (leading >= 44) {
      emit(ORI, rd, zero_reg, j);
    } else if (leading >= 32 && trailing >= 12) {
      emit(LUI, rd, j >> 44);
    } else {
      emit(ORI, rd, zero_reg, j >> trailing);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 20) {
    if (leading >= 32) {
      li_smallest(rd, reinterpret_cast<int32_t>(j));
    } else {
      emit(LUI, rd, j >> trailing);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 32) {
    if (leading >= 32) {
      li_smallest(rd, reinterpret_cast<int32_t>(j));
    } else {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> trailing));
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 44) {
    if (leading >= 20) {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> 12 & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j & 0xFFF);
    } else {
      li_smallest(rd,
                  reinterpret_cast<int32_t>(j >> (12 + trailing) & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> trailing & 0xFFF);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 56) {
    if (leading >= 8) {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> 24 & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> 12 & 0xFFF);
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j & 0xFFF);
    } else {
      li_smallest(rd,
                  reinterpret_cast<int32_t>(j >> (24 + trailing) & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> (12 + trailing) & 0xFFF);
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> trailing & 0xFFF);
      emit(SLLI, rd, rd, trailing);
    }
  } else {
    li(rd, j);
  }
}

void TurboAssembler::li_smallest(Register rd, int32_t j) {
  // FIXME #0024: verify
  BlockTrampolinePoolScope block_trampoline_pool(this);
  bool initialized = false;
  if (j & 0xFFFFF000) {
    emit(LUI, rd, (j & 0xFFFFF000) >> 12);
    initialized = true;
  }
  if (j & 0xFFF) {
    emit(ORI, rd, initialized ? rd : zero_reg, j & 0x00000FFF);
  }
}

void TurboAssembler::li_smallest(Register rd, int64_t j, Register scratch) {
  // FIXME #0026: fix and finish
  BlockTrampolinePoolScope block_trampoline_pool(this);
  unsigned trailing = CountTrailingZeros(j);
  unsigned leading = CountLeadingZeros(j);
  unsigned solid = 64 - trailing - leading;
  if (solid <= 12) {
    if (leading >= 44) {
      emit(ORI, rd, zero_reg, j);
    } else if (leading >= 32 && trailing >= 12) {
      emit(LUI, rd, j >> 44);
    } else {
      emit(ORI, rd, zero_reg, j >> trailing);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 20) {
    if (leading >= 32) {
      li_smallest(rd, reinterpret_cast<int32_t>(j));
    } else {
      emit(LUI, rd, j >> trailing);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 32) {
    if (leading >= 32) {
      li_smallest(rd, reinterpret_cast<int32_t>(j));
    } else {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> trailing));
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 44) {
    if (leading >= 20) {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> 12 & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j & 0xFFF);
    } else {
      li_smallest(rd,
                  reinterpret_cast<int32_t>(j >> (12 + trailing) & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> trailing & 0xFFF);
      emit(SLLI, rd, rd, trailing);
    }
  } else if (solid <= 56) {
    if (leading >= 8) {
      li_smallest(rd, reinterpret_cast<int32_t>(j >> 24 & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> 12 & 0xFFF);
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j & 0xFFF);
    } else {
      li_smallest(rd,
                  reinterpret_cast<int32_t>(j >> (24 + trailing) & 0xFFFFFFFF));
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> (12 + trailing) & 0xFFF);
      emit(SLLI, rd, rd, 12);
      emit(ORI, rd, rd, j >> trailing & 0xFFF);
      emit(SLLI, rd, rd, trailing);
    }
  } else {
    li_smallest(rd, j, scratch);
  }
}

}  // namespace internal
}  // namespace v8

#endif  // V8_TARGET_ARCH_RISCV
