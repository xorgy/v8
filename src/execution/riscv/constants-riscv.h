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

#ifndef V8_RISCV_CONSTANTS_RISCV_H_
#define V8_RISCV_CONSTANTS_RISCV_H_

#include <inttypes.h>

namespace v8 {
namespace internal {

static constexpr int kMaximalBufferSize = 512 * MB;

// R-type ops
const uint32_t OpRMask = 0b11111110000000000111000001111111;
enum OpR : uint32_t {
  // RV32I
  OR    = 0b110000000110011,
  ADD   = 0b000000000110011,
};

// I-type ops
const uint32_t OpIMask = 0b111000001111111;
const uint32_t IImmMask = 0xFFF00000;
enum OpI : uint32_t {
  // RV32I
  ADDI  = 0b000000000010011,
  JALR  = 0b000000001100111,
  ORI   = 0b110000000010011,

  // RV64I
  LD    = 0b011000000000011,
};

// Special I-type ops for shift instructions with
// seven function bits at the top of the immediate.
enum OpIS : uint32_t {
  SLLI  = 0b00000000000000000001000000010011,
  SRLI  = 0b00000000000000000101000000010011,
  SRAI  = 0b01000000000000000101000000010011,
};

// U-type ops
const uint32_t OpUMask = 0b1111111;
const uint32_t UImmMask = 0xFFFFF000;
enum OpU : uint32_t {
  // RV32I
  AUIPC = 0b0010111,
  LUI   = 0b0110111,
};

// S-type ops
const uint32_t OpSMask = 0b111000001111111;
const uint32_t SUpperImmMask = 0b1111111 << 25;
const uint32_t SLowerImmMask = 0b11111 << 7;
enum OpS : uint32_t {
  // RV64I
  SD    = 0b011000000100011,
};

}  // namespace internal
}  // namespace v8

#endif  // V8_RISCV_CONSTANTS_RISCV_H_
