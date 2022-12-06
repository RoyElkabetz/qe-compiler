//===- MergeDelays.h - Merge back to back pulse.delays  ---------*- C++ -*-===//
//
// (C) Copyright IBM 2022.
//
// Any modifications or derivative works of this code must retain this
// copyright notice, and modified files need to carry a notice indicating
// that they have been altered from the originals.
//
//===----------------------------------------------------------------------===//
///
///  This file declares the pass for merging back to back pulse.delays.
///  The current implementation defaults to ignoring the target, there
///  is an option to override this.
///
//===----------------------------------------------------------------------===//

#ifndef PULSE_MERGE_DELAYS_H
#define PULSE_MERGE_DELAYS_H

#include "mlir/Pass/Pass.h"

namespace mlir::pulse {

class MergeDelayPass
    : public PassWrapper<MergeDelayPass, OperationPass<ModuleOp>> {
public:
  void runOnOperation() override;

  llvm::StringRef getArgument() const override;
  llvm::StringRef getDescription() const override;
};
} // namespace mlir::pulse

#endif // PULSE_MERGE_DELAY_H