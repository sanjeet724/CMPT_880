

#ifndef PATHENCODINGPASS_H
#define PATHENCODINGPASS_H


#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"


namespace pathprofiling {


struct PathEncodingPass : public llvm::ModulePass {

  static char ID;
  // a map of the functions and its innermost loops
  llvm::DenseMap<llvm::Function*,std::vector<llvm::Loop*>> FunctionLoopMap;
  llvm::LoopInfo *LI;
  llvm::DenseMap<llvm::BasicBlock*, unsigned> NumPaths;

  PathEncodingPass()
    : llvm::ModulePass(ID)
      { }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.addRequired<llvm::LoopInfoWrapperPass>();
    au.setPreservesAll();
  }

  virtual bool runOnModule(llvm::Module &m) override;

  void handleLoops(llvm::Function *f);

  void createValues(llvm::BasicBlock *bb, llvm::Loop *l);

  void printNumPaths();

  void encode(llvm::Loop *loop);
};


}


#endif

