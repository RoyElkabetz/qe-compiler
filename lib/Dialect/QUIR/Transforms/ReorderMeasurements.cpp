//===- ReorderMeasurements.cpp - Move measurement ops later -----*- C++ -*-===//
//
// (C) Copyright IBM 2022.
//
// Any modifications or derivative works of this code must retain this
// copyright notice, and modified files need to carry a notice indicating
// that they have been altered from the originals.
//
//===----------------------------------------------------------------------===//
///
///  This file implements the pass for moving measurements as late as possible
///
//===----------------------------------------------------------------------===//

#include "Dialect/QUIR/Transforms/ReorderMeasurements.h"

#include "Dialect/QUIR/IR/QUIROps.h"
#include "Dialect/QUIR/Utils/Utils.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#include <llvm/Support/Debug.h>

#include <algorithm>

#define DEBUG_TYPE "QUIRReorderMeasurements"

using namespace mlir;
using namespace mlir::quir;

namespace {
// This pattern matches on a measure op and a non-measure op and moves the
// non-measure op to occur earlier lexicographically if that does not change
// the topological ordering
struct ReorderMeasureAndNonMeasurePat : public OpRewritePattern<MeasureOp> {
  explicit ReorderMeasureAndNonMeasurePat(MLIRContext *ctx)
      : OpRewritePattern<MeasureOp>(ctx) {}

  LogicalResult matchAndRewrite(MeasureOp measureOp,
                                PatternRewriter &rewriter) const override {
    bool anyMove = false;
    do {
      // Accumulate qubits in measurement set
      std::set<uint> currQubits = measureOp.getOperatedQubits();
      LLVM_DEBUG(llvm::dbgs() << "Matching on measurement for qubits:\t");
      LLVM_DEBUG(for (uint id : currQubits) llvm::dbgs() << id << " ");
      LLVM_DEBUG(llvm::dbgs() << "\n");

      auto nextOpt = nextQuantumOrControlFlowOrNull(measureOp);
      if (!nextOpt.hasValue())
        break;

      Operation *nextOp = nextOpt.getValue();
      // don't reorder past the next measurement or reset or control flow
      if (nextOp->hasTrait<mlir::quir::CPTPOp>() ||
          nextOp->hasTrait<::mlir::RegionBranchOpInterface::Trait>())
        break;

      // The measure operates on something that is operated on by nextOp
      if (QubitOpInterface::opsShareQubits(measureOp, nextOp))
        break;

      LLVM_DEBUG(llvm::dbgs() << "Succeeded match with operation:\n");
      LLVM_DEBUG(nextOp->dump());
      LLVM_DEBUG(llvm::dbgs() << "on qubits:\t");
      LLVM_DEBUG(for (uint id // this is ugly but clang-format insists
                      : QubitOpInterface::getOperatedQubits(nextOp)) {
        llvm::dbgs() << id << " ";
      });
      LLVM_DEBUG(llvm::dbgs() << "\n\n");

      // good to move the nextOp before the measureOp
      nextOp->moveBefore(measureOp);
      anyMove = true;
    } while (true);

    if (anyMove)
      return success();

    return failure();
  } // matchAndRewrite
};  // struct ReorderMeasureAndNonMeasurePat
} // anonymous namespace

void ReorderMeasurementsPass::runOnOperation() {
  Operation *moduleOperation = getOperation();

  RewritePatternSet patterns(&getContext());
  patterns.insert<ReorderMeasureAndNonMeasurePat>(&getContext());

  if (failed(
          applyPatternsAndFoldGreedily(moduleOperation, std::move(patterns))))
    signalPassFailure();
} // runOnOperation

llvm::StringRef ReorderMeasurementsPass::getArgument() const {
  return "reorder-measures";
}

llvm::StringRef ReorderMeasurementsPass::getDescription() const {
  return "Move qubits to be as lexicograpically as late as possible without "
         "affecting the topological ordering.";
}