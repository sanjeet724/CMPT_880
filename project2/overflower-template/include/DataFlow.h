
#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>

namespace dataflows {


struct DataFlowPass : public llvm::ModulePass {

  static char ID;
  llvm::DenseMap<llvm::Function*, std::vector<std::unordered_map<llvm::Instruction *, signed>>> functionAllocationMap;
  uint64_t callDepth = 0;

public:
  DataFlowPass()
    : ModulePass(ID)
      { }

  // void
  // getAnalysisUsage(llvm::AnalysisUsage &au) const override {
  //   au.setPreservesAll();
  // }

  bool runOnModule(llvm::Module &m) override;

  void handleInstruction(llvm::Instruction *i);

  void handleFunction(llvm::Function *f, uint64_t cd);
};

}

#endif
