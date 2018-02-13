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

#ifndef V8_RISCV_FRAME_CONSTANTS_RISCV_H_
#define V8_RISCV_FRAME_CONSTANTS_RISCV_H_

namespace v8 {
namespace internal {

class EntryFrameConstants : public AllStatic {
 public:
  static const int kCallerFPOffset =
      -(StandardFrameConstants::kFixedFrameSizeFromFp + kPointerSize);
};

class ExitFrameConstants : public TypedFrameConstants {
 public:
  static const int kSPOffset = TYPED_FRAME_PUSHED_VALUE_OFFSET(0);
  static const int kCodeOffset = TYPED_FRAME_PUSHED_VALUE_OFFSET(1);
  DEFINE_TYPED_FRAME_SIZES(2);

  // The caller fields are below the frame pointer on the stack.
  static const int kCallerFPOffset = +0 * kPointerSize;
  // The calling JS function is between FP and PC.
  static const int kCallerPCOffset = +1 * kPointerSize;

  // MIPS-specific: a pointer to the old sp to avoid unnecessary calculations.
  static const int kCallerSPOffset = +2 * kPointerSize;

  // FP-relative displacement of the caller's SP.
  static const int kCallerSPDisplacement = +2 * kPointerSize;

  static const int kConstantPoolOffset = 0;  // Not used.
};

class JavaScriptFrameConstants : public AllStatic {
 public:
  // FP-relative.
  static const int kLocal0Offset = StandardFrameConstants::kExpressionsOffset;
  static const int kLastParameterOffset = +2 * kPointerSize;
  static const int kFunctionOffset = StandardFrameConstants::kFunctionOffset;

  // Caller SP-relative.
  static const int kParam0Offset = -2 * kPointerSize;
  static const int kReceiverOffset = -1 * kPointerSize;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_RISCV_FRAME_CONSTANTS_RISCV_H_
