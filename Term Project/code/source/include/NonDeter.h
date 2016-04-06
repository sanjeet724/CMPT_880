
#ifndef NONDETER_H
#define NONDETER_H

#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>

namespace nondeterminism {


struct NonDeterPass : public llvm::ModulePass {

  static char ID;
  std::vector<llvm::Function*> insertFunctions;
  llvm::Type* detectedContainer;
  llvm::DenseMap<llvm::Function*, bool> searchSpace; // search all these functions

public:
  NonDeterPass()
    : ModulePass(ID)
      { }

  void
  getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.setPreservesAll();
  }

  void findInserts(llvm::Function *f); 

  void checkInserts(); 

  void performDataFlow(llvm::Function *f);

  bool checkAlocatorFunction(llvm::Function *f);

  void searchFunctions();

  void handleCallSite(llvm::CallSite cs);

  void checkAllocation(llvm::Instruction *i);

  bool runOnModule(llvm::Module &m) override;

  void handleFunctionPointer(llvm::CallSite cs);

  void createFunctionPointerMap(llvm::CallSite cs);

  void getFunctionParameters(llvm::Type *t);
};

}

#endif
