#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct MemorySSADemoPass : PassInfoMixin<MemorySSADemoPass> {
  PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
    auto& MSSAResult = AM.getResult<MemorySSAAnalysis>(F);
    auto& MSSA = MSSAResult.getMSSA();

    errs() << "digraph \"MemorySSA_" << F.getName() << "\" {\n";
    errs() << "  node [shape=box, style=filled, fillcolor=lightgray];\n";

    DenseMap<const MemoryAccess*, std::string> NodeNames;
    unsigned ID = 0;

    // Assign unique names
    for (auto& BB : F) {
      for (auto& I : BB) {
        if (auto* MA = MSSA.getMemoryAccess(&I))
          NodeNames[MA] = "n" + std::to_string(ID++);
      }
      if (auto* Phi = MSSA.getMemoryAccess(&BB))
        NodeNames[Phi] = "n" + std::to_string(ID++);
    }

    // Emit nodes
    for (auto& Pair : NodeNames) {
      const MemoryAccess* MA = Pair.first;
      std::string Label;
      raw_string_ostream LS(Label);
      MA->print(LS);
      LS.flush();

      std::string SafeLabel;
      for (char C : Label) SafeLabel += (C == '"') ? '\'' : C;

      errs() << "  " << Pair.second << " [label=\"" << SafeLabel << "\"];\n";
    }

    // Emit edges
    for (auto& Pair : NodeNames) {
      const MemoryAccess* MA = Pair.first;

      if (auto* Def = dyn_cast<MemoryDef>(MA)) {
        if (auto* Src = Def->getDefiningAccess())
          if (NodeNames.count(Src))
            errs() << "  " << NodeNames[Src] << " -> " << NodeNames[MA]
                   << ";\n";
      } else if (auto* Use = dyn_cast<MemoryUse>(MA)) {
        if (auto* Src = Use->getDefiningAccess())
          if (NodeNames.count(Src))
            errs() << "  " << NodeNames[Src] << " -> " << NodeNames[MA]
                   << ";\n";
      } else if (auto* Phi = dyn_cast<MemoryPhi>(MA)) {
        for (unsigned i = 0; i < Phi->getNumIncomingValues(); ++i) {
          auto* Incoming = Phi->getIncomingValue(i);
          if (NodeNames.count(Incoming))
            errs() << "  " << NodeNames[Incoming] << " -> " << NodeNames[MA]
                   << ";\n";
        }
      }
    }

    errs() << "}\n";
    return PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MemorySSADemoPass", "v1.1",
          [](PassBuilder& PB) {
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager& FAM) {
                  FAM.registerPass([] { return MemorySSAAnalysis(); });
                });

            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "memssa-demo") {
                    FPM.addPass(MemorySSADemoPass());
                    return true;
                  }
                  return false;
                });
          }};
}
