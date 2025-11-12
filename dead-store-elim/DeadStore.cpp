#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct DeadStorePass : PassInfoMixin<DeadStorePass> {
  bool hasInterveningLoad(MemoryDef* MD, MemoryDef* NextDef, MemorySSA& MSSA,
                          AAManager::Result& AA) {
    Instruction* Inst = MD->getMemoryInst();
    Instruction* NextInst = NextDef->getMemoryInst();
    if (!Inst || !NextInst) return true;

    MemoryLocation Loc = MemoryLocation::get(Inst);

    SmallVector<MemoryAccess*, 8> WorkList;
    SmallPtrSet<MemoryAccess*, 16> Visited;

    WorkList.push_back(MD);
    Visited.insert(MD);

    while (!WorkList.empty()) {
      MemoryAccess* MA = WorkList.pop_back_val();

      for (Use& U : MA->uses()) {
        MemoryAccess* UserMA = cast<MemoryAccess>(U.getUser());
        if (!Visited.insert(UserMA).second) continue;

        if (UserMA == NextDef) continue;

        if (auto* MU = dyn_cast<MemoryUse>(UserMA)) {
          Instruction* LoadInst = MU->getMemoryInst();
          if (!LoadInst) continue;

          MemoryLocation UseLoc = MemoryLocation::get(LoadInst);
          // If the use aliases the same location as the original store
          if (AA.alias(Loc, UseLoc) != AliasResult::NoAlias) {
            return true;
          }
        }
        WorkList.push_back(UserMA);
      }
    }
    return true;
  }
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    auto& MSSAResult = AM.getResult<MemorySSAAnalysis>(F);
    auto& MSSA = MSSAResult.getMSSA();
    auto& AA = AM.getResult<AAManager>(F);
    MemorySSAUpdater Updater(&MSSA);
    bool changed = false;

    for (auto& BB : F) {
      for (auto& I : BB) {
        auto* SI = dyn_cast<StoreInst>(&I);
        if (!SI) continue;

        auto* MA = MSSA.getMemoryAccess(SI);
        if (!MA) continue;
        auto* MD = dyn_cast<MemoryDef>(MA);
        if (!MD) continue;

        auto* Clobber = MSSA.getWalker()->getClobberingMemoryAccess(MD);
        if (auto* NextDef = dyn_cast<MemoryDef>(Clobber)) {
          auto* NextInst = dyn_cast<StoreInst>(NextDef->getMemoryInst());
          if (!NextInst) continue;

          if (AA.alias(MemoryLocation::get(SI),
                       MemoryLocation::get(NextInst)) ==
              AliasResult::MustAlias) {
            if (!hasInterveningLoad(MD, NextDef, MSSA, AA)) {
              errs() << "Removing dead store: " << *SI << "\n";
              Updater.removeMemoryAccess(SI);
              SI->eraseFromParent();
              changed = true;
            }
          }
        }
      }
    }

    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DeadStoreDemoPass", "v1.1",
          [](PassBuilder& PB) {
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager& FAM) {
                  FAM.registerPass([] { return MemorySSAAnalysis(); });
                });

            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "dead-store-demo") {
                    FPM.addPass(DeadStorePass());
                    return true;
                  }
                  return false;
                });
          }};
}
