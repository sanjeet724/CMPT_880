
#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>

namespace dataflows {


struct DataFlowPass : public llvm::ModulePass {

  static char ID;
  //llvm::Instruction* allocated;
  llvm::Instruction * allocated;
  signed bufferSize;
  llvm::DenseMap<llvm::Instruction*, signed> functionBufferMap;
  llvm::DenseMap<llvm::Function*, llvm::Instruction*> loadMap;
  llvm::DenseMap<llvm::Function*, llvm::Instruction*> storeMap;
  uint64_t callDepth = 0;

public:
  DataFlowPass()
    : ModulePass(ID)
      { }

  void
  getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.addRequired<llvm::AliasAnalysis>();
    au.setPreservesAll();
  }

  bool runOnModule(llvm::Module &m) override;

  void handleInstruction(llvm::Instruction *i);

  void checkAllocation(llvm::Instruction *i);

  void checkLoad(llvm::Instruction *i);

  void recurseOnValue(llvm::Value *v);

  void printGEPInfo(llvm::GetElementPtrInst *gep);

  void printAllocaInfo(llvm::AllocaInst *alloca);

  // void checkStore(llvm::Instruction *i);

  void handleFunction(llvm::Function *f, uint64_t cd);
};

}

#endif
