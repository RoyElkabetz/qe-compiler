// RUN: qss-compiler -X=mlir --quir-eliminate-loads %s | FileCheck %s --implicit-check-not 'oq3.variable_load @a'
// RUN: qss-compiler -X=mlir --quir-eliminate-loads --remove-unused-variables %s | FileCheck %s --check-prefix REMOVE-UNUSED
//
// This test case serves to validate the behavior of the load elimination pass.

//
// This code is part of Qiskit.
//
// (C) Copyright IBM 2023.
//
// This code is licensed under the Apache License, Version 2.0 with LLVM
// Exceptions. You may obtain a copy of this license in the LICENSE.txt
// file in the root directory of this source tree.
//
// Any modifications or derivative works of this code must retain this
// copyright notice, and modified files need to carry a notice indicating
// that they have been altered from the originals.

module {

  // CHECK: oq3.declare_variable {input} @a : !quir.angle<64>
  // CHECK: oq3.declare_variable @b : !quir.angle<64>
  oq3.declare_variable {input} @a : !quir.angle<64>
  oq3.declare_variable @b : !quir.angle<64>

  // REMOVE-UNUSED-NOT: oq3.declare_variable {input} @a

  func.func @main() -> !quir.angle<64> {

    // CHECK: [[CONST314_ANGLE:%.*]] = quir.constant #quir.angle<3.140000e+00> : !quir.angle<64>
    // REMOVE-UNUSED: [[CONST314_ANGLE:%.*]] = quir.constant #quir.angle<3.140000e+00> : !quir.angle<64>
    %angle = quir.constant #quir.angle<3.140000e+00> : !quir.angle<64>
    %angle2 = quir.constant #quir.angle<3.140000e+00> : !quir.angle<64>

    // REMOVE-UNUSED-NOT: oq3.variable_assign @a
    oq3.variable_assign @a : !quir.angle<64> = %angle

    %angle_0 = quir.constant #quir.angle<1.000000e+00> : !quir.angle<64>
    oq3.variable_assign @b : !quir.angle<64> = %angle_0

    // The load elimination pass should forward-propagate the initializer to the
    // assignment of b.
    // CHECK: oq3.variable_assign @b : !quir.angle<64> = [[CONST314_ANGLE]]
    // The variable a should never be read.
    // REMOVE-UNUSED-NOT: oq3.variable_load @a
    %1 = oq3.variable_load @a : !quir.angle<64>
    oq3.variable_assign @b : !quir.angle<64> = %1

    %2 = oq3.variable_load @b : !quir.angle<64>
    return %2 : !quir.angle<64>
  }
}
