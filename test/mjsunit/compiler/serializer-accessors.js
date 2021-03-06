// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

class C {
  get prop() {
    return 42;
  }
  set prop(v) {
    %TurbofanStaticAssert(v === 43);
  }
}

const c = new C();

function foo() {
  %TurbofanStaticAssert(c.prop === 42);
  c.prop = 43;
}

%PrepareFunctionForOptimization(
    Object.getOwnPropertyDescriptor(C.prototype, 'prop').get);
%PrepareFunctionForOptimization(
    Object.getOwnPropertyDescriptor(C.prototype, 'prop').set);
%PrepareFunctionForOptimization(foo);
foo();
foo();
%OptimizeFunctionOnNextCall(foo);
foo();
