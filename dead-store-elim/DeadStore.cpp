#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct DeadStorePass : PassInfoMixin<DeadStorePass> {
  bool hasInterveningLoad(StoreInst* EarlierStore, StoreInst* LaterStore,
                          AAResults& AA) {
    if (EarlierStore->getParent() != LaterStore->getParent()) return true;

    for (auto It = EarlierStore->getIterator(); It != LaterStore->getIterator();
         ++It) {
      Instruction& I = *It;

      if (&I == EarlierStore) continue;

      if (auto* LI = dyn_cast<LoadInst>(&I)) {
        if (AA.alias(MemoryLocation::get(EarlierStore),
                     MemoryLocation::get(LI)) != AliasResult::NoAlias) {
          return true;
        }
      }
    }
    return false;
  }

  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    auto& MSSA = AM.getResult<MemorySSAAnalysis>(F).getMSSA();

    auto& AA = AM.getResult<AAManager>(F);

    SmallVector<StoreInst*, 16> DeadStores;

    for (auto& BB : F) {
      for (auto& I : BB) {
        auto* LaterStore = dyn_cast<StoreInst>(&I);
        if (!LaterStore) continue;

        auto* LaterDef =
            dyn_cast_or_null<MemoryDef>(MSSA.getMemoryAccess(LaterStore));
        if (!LaterDef) continue;

        auto* Clobber = MSSA.getWalker()->getClobberingMemoryAccess(LaterDef);

        if (auto* EarlierDef = dyn_cast<MemoryDef>(Clobber)) {
          Instruction* EarlierInst = EarlierDef->getMemoryInst();

          if (auto* EarlierStore = dyn_cast_or_null<StoreInst>(EarlierInst)) {
            if (AA.alias(MemoryLocation::get(LaterStore),
                         MemoryLocation::get(EarlierStore)) ==
                AliasResult::MustAlias) {
              if (!hasInterveningLoad(EarlierStore, LaterStore, AA)) {
                DeadStores.push_back(EarlierStore);
              }
            }
          }
        }
      }
    }

    if (DeadStores.empty()) {
      return PreservedAnalyses::all();
    }

    MemorySSAUpdater Updater(&MSSA);
    for (StoreInst* SI : DeadStores) {
      errs() << "Removing dead store: " << *SI << "\n";
      Updater.removeMemoryAccess(SI);
      SI->eraseFromParent();
    }

    return PreservedAnalyses::none();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DeadStoreDemoPass", "v1.1",
          [](PassBuilder& PB) {
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