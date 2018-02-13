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

#include "src/riscv/assembler-riscv.h"

#include "src/riscv/assembler-riscv-inl.h"

#include "src/v8memory.h"

namespace v8 {
namespace internal {

static uint32_t Assembler::assemble(OpI op,
                                    Register rd,
                                    Register rs,
                                    int32_t imm) {
  // FIXME #0008: Verify once everything starts compiling.
  DCHECK_LT(imm, 2048);
  DCHECK_GE(imm, -2048);
  return op | rd.code() << 7 | rs.code() << 15 | imm << 20;
}

static uint32_t Assembler::assemble(OpIS op,
                                    Register rd,
                                    Register rs,
                                    uint32_t imm) {
// FIXME #0010: Verify once everything starts compiling.
#if defined(V8_TARGET_ARCH_32_BIT)
  DCHECK_LT(imm, 32);
#elif defined(V8_TARGET_ARCH_64_BIT)
  DCHECK_LT(imm, 64);
#else
  DCHECK_LT(imm, 128);
#endif
  return op | rd.code() << 7 | rs.code() << 15 | imm << 20;
}

static uint32_t Assembler::assemble(OpR op,
                                    Register rd,
                                    Register rs1,
                                    Register rs2) {
  // FIXME #0007: Verify once everything starts compiling.
  return op | rd.code() << 7 | rs1.code() << 15 | rs2.code() << 20;
}

static uint32_t Assembler::assemble(OpS op,
                                    Register rs1,
                                    Register rs2,
                                    int32_t imm) {
  // FIXME #0013: Verify once everything starts compiling.
  DCHECK_LT(imm, 2048);
  DCHECK_GE(imm, -2048);
  uint32_t cimm = imm << 20;
  return op | cimm & SUpperImmMask | rs2.code() << 20 | rs1.code() << 15 |
         (cimm >> 13) & SLowerImmMask;
}

static uint32_t Assembler::assemble(OpU op, Register rd, int32_t imm) {
  // FIXME #0009: Verify once everything starts compiling.
  DCHECK_LT(imm, 524288);
  DCHECK_GE(imm, -524288);
  return op | rd.code() << 7 | imm << 12;
}

template <typename... T>
void Assembler::emit(T... a) {
  auto instruction = assemble(a...);
  *reinterpret_cast<decltype(instruction)*>(pc_) = instruction;
  pc_ += sizeof(instruction);
}

void Assembler::RecordRelocInfo(RelocInfo::Mode rmode, intptr_t data) {
  // FIXME #0006: Revisit when you know what you're doing
  DCHECK(!RelocInfo::IsNone(rmode));
  // Don't record external references unless the heap will be serialized.
  if (rmode == RelocInfo::EXTERNAL_REFERENCE && !serializer_enabled() &&
      !emit_debug_code()) {
    return;
  }
  RelocInfo rinfo(pc_, rmode, data, nullptr);
  reloc_info_writer.Write(&rinfo);
}

void Assembler::GetCode(Isolate* isolate, CodeDesc* desc) {
  // FIXME #0017: Probably not working
  EmitForbiddenSlotInstruction();
  DCHECK(pc_ <= reloc_info_writer.pos());  // No overlap.

  AllocateAndInstallRequestedHeapObjects(isolate);

  // Set up code descriptor.
  desc->buffer = buffer_;
  desc->buffer_size = buffer_size_;
  desc->instr_size = pc_offset();
  desc->reloc_size =
      static_cast<int>((buffer_ + buffer_size_) - reloc_info_writer.pos());
  desc->origin = this;
  desc->constant_pool_size = 0;
  desc->unwinding_info_size = 0;
  desc->unwinding_info = nullptr;
}

void Assembler::GrowBuffer() {
  // FIXME #0005: Revisit when you know what you're doing
  DCHECK(buffer_overflow());
  if (!own_buffer_)
    FATAL("external code buffer is too small");

  // Compute new buffer size.
  CodeDesc desc;  // the new buffer
  desc.buffer_size = 2 * buffer_size_;

  // Some internal data structures overflow for very large buffers,
  // they must ensure that kMaximalBufferSize is not too large.
  if (desc.buffer_size > kMaximalBufferSize) {
    V8::FatalProcessOutOfMemory("Assembler::GrowBuffer");
  }

  // Set up new buffer.
  desc.buffer = NewArray<byte>(desc.buffer_size);
  desc.origin = this;
  desc.instr_size = pc_offset();
  desc.reloc_size =
      static_cast<int>((buffer_ + buffer_size_) - (reloc_info_writer.pos()));

  // Copy the data.
  intptr_t pc_delta = desc.buffer - buffer_;
  intptr_t rc_delta =
      (desc.buffer + desc.buffer_size) - (buffer_ + buffer_size_);
  MemMove(desc.buffer, buffer_, desc.instr_size);
  MemMove(rc_delta + reloc_info_writer.pos(), reloc_info_writer.pos(),
          desc.reloc_size);

  // Switch buffers.
  DeleteArray(buffer_);
  buffer_ = desc.buffer;
  buffer_size_ = desc.buffer_size;
  pc_ += pc_delta;
  reloc_info_writer.Reposition(reloc_info_writer.pos() + rc_delta,
                               reloc_info_writer.last_pc() + pc_delta);

  DCHECK(!buffer_overflow());
}

void CpuFeatures::ProbeImpl(bool cross_compile) {
  // TODO(aaron):
  // - Figure out how to detect V and possibly other extensions.
}

// FIXME #0003: Set again after you understand exactly what this does.
const int RelocInfo::kApplyMask = 1 << RelocInfo::INTERNAL_REFERENCE |
                                  1 << RelocInfo::INTERNAL_REFERENCE_ENCODED;

void RelocInfo::set_embedded_address(Address address,
                                     ICacheFlushMode flush_mode) {
  Assembler::set_target_address_at(pc_, constant_pool_, address, flush_mode);
}

void RelocInfo::set_embedded_size(uint32_t size, ICacheFlushMode flush_mode) {
  // FIXME #0002: Verify
  Memory::uint32_at(Assembler::target_pointer_address_at(pc_)) = size;
}

Address RelocInfo::embedded_address() const {
  return Memory::Address_at(Assembler::target_pointer_address_at(pc_));
}

}  // namespace internal
}  // namespace v8

#endif  // V8_TARGET_ARCH_RISCV
