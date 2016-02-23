
#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>

namespace dataflows {


struct DataFlowPass : public llvm::ModulePass {

  static char ID;
  const llvm::DataLayout *dataL;
  llvm::Instruction * allocated;
  uint64_t allocatedTypeSize;
  signed bufferSize;
  signed bufferSizeByte;
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
    au.addRequired<llvm::LoopInfoWrapperPass>();
    au.setPreservesAll();
  }

  bool doInitialization(llvm::Module &m) override;

  bool runOnModule(llvm::Module &m) override;

  void handleInstruction(llvm::Instruction *i);

  void checkAllocation(llvm::Instruction *i);

  void checkLoad(llvm::Instruction *i);

  void recurseOnValue(llvm::Value *v);

  bool checkLoop(llvm::Instruction *i);

  void printGEPInfo(llvm::GetElementPtrInst *gep);

  void printAllocaInfo(llvm::AllocaInst *alloca);

  void handleFunction(llvm::Function *f, uint64_t cd);
};

}

#endif
