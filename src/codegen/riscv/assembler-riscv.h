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

#ifndef V8_RISCV_ASSEMBLER_RISCV_H_
#define V8_RISCV_ASSEMBLER_RISCV_H_

#include "src/assembler.h"
#include "src/riscv/instructions-riscv.h"

namespace v8 {
namespace internal {

// clang-format off
#define GENERAL_REGISTERS(V) \
  V(zero_reg) V(ra) V(sp) V(gp) V(tp) \
  V(t0) V(t1) V(t2) \
  V(s0) V(s1) \
  V(a0) V(a1) V(a2) V(a3) V(a4) V(a5) V(a6) V(a7) \
  V(s2) V(s3) V(s4) V(s5) V(s6) V(s7) V(s8) V(s9) V(s10) V(s11) \
  V(t3) V(t4) V(t5) V(t6)

#define DOUBLE_REGISTERS(V)                               \
   V(f0)  V(f1)  V(f2)  V(f3)  V(f4)  V(f5)  V(f6)  V(f7) \
   V(f8)  V(f9) V(f10) V(f11) V(f12) V(f13) V(f14) V(f15) \
  V(f16) V(f17) V(f18) V(f19) V(f20) V(f21) V(f22) V(f23) \
  V(f24) V(f25) V(f26) V(f27) V(f28) V(f29) V(f30) V(f31)
// clang-format on

#define FLOAT_REGISTERS DOUBLE_REGISTERS
#define SIMD128_REGISTERS DOUBLE_REGISTERS

enum RegisterCode {
#define REGISTER_CODE(R) kRegCode_##R,
  GENERAL_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
      kRegAfterLast
};

class Register : public RegisterBase<Register, kRegAfterLast> {
 private:
  friend class RegisterBase;
  explicit constexpr Register(int code) : RegisterBase(code) {}
};

#define DECLARE_REGISTER(R) \
  constexpr Register R = Register::from_code<kRegCode_##R>();
GENERAL_REGISTERS(DECLARE_REGISTER)
#undef DECLARE_REGISTER

constexpr Register no_reg = Register::no_reg();

// FIXME #0019: Revisit when you know what you're doing.
constexpr bool kPadArguments = false;
constexpr bool kSimpleFPAliasing = true;

enum DoubleRegisterCode {
#define REGISTER_CODE(R) kDoubleCode_##R,
  DOUBLE_REGISTERS(REGISTER_CODE)
#undef REGISTER_CODE
      kDoubleAfterLast
};

class DoubleRegister : public RegisterBase<DoubleRegister, kDoubleAfterLast> {
 private:
  friend class RegisterBase;
  explicit constexpr DoubleRegister(int code) : RegisterBase(code) {}
};

typedef DoubleRegister FloatRegister;
typedef DoubleRegister Simd128Register;

#define DECLARE_DOUBLE_REGISTER(R) \
  constexpr DoubleRegister R = DoubleRegister::from_code<kDoubleCode_##R>();
DOUBLE_REGISTERS(DECLARE_DOUBLE_REGISTER)
#undef DECLARE_DOUBLE_REGISTER

constexpr DoubleRegister no_freg = DoubleRegister::no_reg();

class Assembler : public AssemblerBase {
 public:
  Assembler(Isolate* isolate, void* buffer, int buffer_size)
      : Assembler(IsolateData(isolate), buffer, buffer_size) {}
  Assembler(IsolateData isolate_data, void* buffer, int buffer_size);

  virtual ~Assembler();

  void GetCode(Isolate* isolate, CodeDesc* desc);

  void GrowBuffer();

  inline bool buffer_overflow() const {
    return pc_ >= reloc_info_writer.pos() - kGap;
  }

  inline static Address target_pointer_address_at(Address pc);
  inline static Address target_address_from_return_address(Address pc);

  inline static void set_target_address_at(
      Address pc, Address constant_pool, Address target,
      ICacheFlushMode icache_flush_mode = FLUSH_ICACHE_IF_NEEDED);

  // FIXME #0018: Possibly incorrect
  static constexpr int kCallTargetAddressOffset = 5 * 4;

  RelocInfoWriter reloc_info_writer;

  // Record a comment relocation entry that can be used by a disassembler.
  // Use --code-comments to enable.
  void RecordComment(const char* msg);

  // Record a deoptimization reason that can be used by a log or cpu profiler.
  // Use --trace-deopt to enable.
  void RecordDeoptReason(DeoptimizeReason reason, SourcePosition position,
                         int id);

  static uint32_t assemble(OpI op, Register rd, Register rs, int32_t imm);
  static uint32_t assemble(OpIS op, Register rd, Register rs, uint32_t imm);
  static uint32_t assemble(OpR op, Register rd, Register rs1, Register rs2);
  static uint32_t assemble(OpS op, Register rs1, Register rs2, int32_t imm);
  static uint32_t assemble(OpU op, Register rd, int32_t imm);
  template <typename... T>
  void emit(T... a);

  void nop() { emit(ADDI, zero_reg, zero_reg, 0); }

 protected:
  static const int kGap = 128;

 private:
  void RecordRelocInfo(RelocInfo::Mode rmode, intptr_t data = 0);
};

class Operand {
 public:
  // Register.
  INLINE(explicit Operand(Register rm)) : rm_(rm) {}

  // Return true if this is a register operand.
  INLINE(bool is_reg() const);

  Register rm() const { return rm_; }

 private:
  Register rm_;

  friend class Assembler;
  friend class MacroAssembler;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_RISCV_ASSEMBLER_RISCV_H_
