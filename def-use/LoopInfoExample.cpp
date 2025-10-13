// LoopInfoExample.cpp
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
void printLoopInfo(Loop *L, unsigned int depth) {
  errs() << "Loop at depth: " << L->getLoopDepth() << "\n";

  errs() << "with preheader: ";
  L->getLoopPreheader()->printAsOperand(errs(), false);
  errs() << "\n";

  errs() << "with latch: ";
  L->getLoopLatch()->printAsOperand(errs(), false);
  errs() << "\n";

  errs() << "with header: ";
  L->getHeader()->printAsOperand(errs(), false);
  errs() << "\n";
  for(Loop *SubL : L->getSubLoops()){
    printLoopInfo(SubL, depth + 1);
  }
}

struct LoopInfoExample : public PassInfoMixin<LoopInfoExample> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    errs() << "Analyzing function: " << F.getName() << "\n";

    // Get LoopInfo for this function
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    for(Loop *L : LI){
      printLoopInfo(L, 0);
    }

    return PreservedAnalyses::all();
  }
};
} // namespace

// Register pass plugin
llvm::PassPluginLibraryInfo getLoopInfoExamplePluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION, "loop-info-example", LLVM_VERSION_STRING,
      [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              if (Name == "loop-info-example") {
                FPM.addPass(LoopInfoExample());
                return true;
              }
              return false;
            });
      }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopInfoExamplePluginInfo();
}

