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

#ifndef V8_RISCV_ASSEMBLER_RISCV_INL_H_
#define V8_RISCV_ASSEMBLER_RISCV_INL_H_

#include <limits>

#include "src/globals.h"
#include "src/objects-inl.h"
#include "src/riscv/assembler-riscv.h"
#include "src/riscv/constants-riscv.h"
#include "src/riscv/instructions-riscv.h"

namespace v8 {
namespace internal {

Address Assembler::target_pointer_address_at(Address pc) {
  Instruction* ui = reinterpret_cast<Instruction*>(pc);
  DCHECK(ui->Is(AUIPC));
  Instruction* ii = reinterpret_cast<Instruction*>(pc + 4);
  DCHECK(ii->Is(LD));

  return reinterpret_cast<Address>(ui->ImmU() + pc + ii->ImmI());
}

Address Assembler::target_address_from_return_address(Address pc) {
  return pc - kCallTargetAddressOffset;
}

void Assembler::set_target_address_at(Address pc,
                                      Address constant_pool,
                                      Address target,
                                      ICacheFlushMode icache_flush_mode) {
  // TODO(aaron):
  // - Interacts with the C extension.
  // - Possibly necessary to use indirect branches, since between
  //   AUIPC and JALR we have "only" 2GiB of virtual range either way.
  //
  // FIXME #0001: Verify, probably utterly broken.
  uint32_t* ui = reinterpret_cast<uint32_t*>(pc);
  DCHECK(reinterpret_cast<Instruction*>(ui)->Is(AUIPC) ||
         reinterpret_cast<Instruction*>(ui)->Is(LUI));
  uint32_t* jalr = reinterpret_cast<uint32_t*>(pc + 4);
  DCHECK(reinterpret_cast<Instruction*>(jalr)->Is(JALR));

  int64_t pc_delta = target - pc;
  uint32_t rest;

  if (pc_delta > std::numeric_limits<int32_t>::max() ||
      pc_delta < std::numeric_limits<int32_t>::min()) {
    DCHECK(uintptr_t(target) < std::numeric_limits<uint32_t>::max());
    uint32_t target32 = reinterpret_cast<uintptr_t>(target);
    *ui = assemble(LUI, zero_reg, target32 >> 12) | (*ui & ~UImmMask);
    rest = target32 & 0xFFF;
  } else {
    *ui = assemble(AUIPC, zero_reg, pc_delta >> 12) | (*ui & ~UImmMask);
    rest = pc_delta & 0xFFF;
  }


  *jalr = assemble(JALR, zero_reg, zero_reg, rest) | (*jalr & ~IImmMask);

  if (icache_flush_mode != SKIP_ICACHE_FLUSH) {
    FlushICache(pc, sizeof(*ui) + sizeof(*jalr));
  }
}

}  // namespace internal
}  // namespace v8

#endif  // V8_RISCV_ASSEMBLER_RISCV_INL_H_
