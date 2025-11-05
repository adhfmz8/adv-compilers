/* DerivedInductionVar.cpp
 *
 * This pass detects derived induction variables using ScalarEvolution.
 *
 * Compatible with New Pass Manager
 */

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

using namespace llvm;

namespace {

class DerivedInductionVar : public PassInfoMixin<DerivedInductionVar> {
 public:
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    auto& LI = AM.getResult<LoopAnalysis>(F);
    auto& SE = AM.getResult<ScalarEvolutionAnalysis>(F);

    bool changed = false;
    for (Loop* L : LI) {
      if (analyzeLoopRecursively(L, SE)) changed = true;
    }

    if (changed) {
      return PreservedAnalyses::none();
    } else {
      return PreservedAnalyses::all();
    }
  }

  bool analyzeLoopRecursively(Loop* L, ScalarEvolution& SE) {
    errs().indent(L->getLoopDepth() * 2);
    errs() << "Analyzing Loop: " << L->getHeader()->getName() << "\n";
    bool loop_flag = false;

    SmallVector<PHINode*, 8> DeadPHIs;

    BasicBlock* Header = L->getHeader();
    if (!Header) return false;

    for (PHINode& PN : Header->phis()) {
      if (!PN.getType()->isIntegerTy()) continue;

      const SCEV* S = SE.getSCEV(&PN);

      if (auto* AR = dyn_cast<SCEVAddRecExpr>(S)) {
        // Check this add recurrence belongs to the current loop and is affine
        if (AR->getLoop() == L && AR->isAffine()) {
          const SCEV* Step = AR->getStepRecurrence(SE);
          const SCEV* Start = AR->getStart();

          SCEVExpander Expander(
              SE, L->getHeader()->getModule()->getDataLayout(), "iv.expanded");

          PHINode* BasicIV = L->getCanonicalInductionVariable();
          if (!BasicIV) continue;

          const SCEV* BasicIV_SCEV = SE.getSCEV(BasicIV);
          const SCEV* DerivedIV_SCEV = SE.getSCEV(&PN);
          Instruction* InsertPoint = BasicIV->getParent()->getFirstNonPHI();

          // Ensure it's safe to expand this SCEV into IR AND NOT basic
          // induction var
          if (Expander.isSafeToExpand(DerivedIV_SCEV) &&
              DerivedIV_SCEV != BasicIV_SCEV) {
            Value* NewVal = Expander.expandCodeFor(
                DerivedIV_SCEV, DerivedIV_SCEV->getType(), InsertPoint);

            // Replace all uses of the original PHI with the new computed value
            PN.replaceAllUsesWith(NewVal);
            DeadPHIs.push_back(&PN);
            loop_flag = true;

            errs().indent(L->getLoopDepth() * 2 + 2);
            errs() << "Found Derived IV: " << PN.getName() << " = {" << *Start
                   << ",+, " << *Step << "}\n";
          } else {
            errs().indent(L->getLoopDepth() * 2 + 2);
            errs() << "Skipping unsafe-to-expand IV: " << PN.getName() << "\n";
          }
        }
      }
    }

    for (auto Dead : DeadPHIs) {
      Dead->eraseFromParent();
    }

    // Recursively analyze nested loops
    for (auto* SubLoop : L->getSubLoops()) analyzeLoopRecursively(SubLoop, SE);

    return loop_flag;
  }
};

}  // namespace

// Register the pass
llvm::PassPluginLibraryInfo getDerivedInductionVarPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DerivedInductionVar", LLVM_VERSION_STRING,
          [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "derived-iv") {
                    FPM.addPass(DerivedInductionVar());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDerivedInductionVarPluginInfo();
}
