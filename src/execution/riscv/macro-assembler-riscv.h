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

#ifndef V8_RISCV_MACRO_ASSEMBLER_RISCV_H_
#define V8_RISCV_MACRO_ASSEMBLER_RISCV_H_

#include "src/riscv/assembler-riscv.h"

namespace v8 {
namespace internal {

constexpr Register kReturnRegister0 = a0;
constexpr Register kReturnRegister1 = a1;

class TurboAssembler : public Assembler {
 public:
  TurboAssembler(Isolate* isolate,
                 void* buffer,
                 int buffer_size,
                 CodeObjectRequired create_code_object);

  void set_has_frame(bool value) { has_frame_ = value; }
  bool has_frame() const { return has_frame_; }

  Isolate* isolate() const { return isolate_; }

  Handle<HeapObject> CodeObject() {
    DCHECK(!code_object_.is_null());
    return code_object_;
  }

  void EnterFrame(StackFrame::Type type);
  void EnterFrame(StackFrame::Type type, bool load_constant_pool_pointer_reg) {
    // FIXME #0016: Revisit when you know what you're doing.
    UNREACHABLE();
  }
  void LeaveFrame(StackFrame::Type type);

  // Use enough instructions to load any constant of the given type
  void li(Register rd, int64_t j);
  void li(Register rd, int64_t j, Register scratch);
  void li(Register rd, int32_t j);
  // Use as few instruction bytes as possible to load the constant
  void li_smallest(Register rd, int64_t j);
  void li_smallest(Register rd, int64_t j, Register scratch);
  void li_smallest(Register rd, int32_t j);

  void mov(Register rd, Register rs) { emit(ADDI, rd, rs, 0); }

 private:
  bool has_frame_ = false;
  Isolate* const isolate_;
  // This handle will be patched with the code object on installation.
  Handle<HeapObject> code_object_;
};

class MacroAssembler : public TurboAssembler {
 public:
  MacroAssembler(Isolate* isolate,
                 void* buffer,
                 int size,
                 CodeObjectRequired create_code_object);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_RISCV_MACRO_ASSEMBLER_RISCV_H_
