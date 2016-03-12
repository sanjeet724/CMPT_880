

#ifndef PATHPROFILINGPASS_H
#define PATHPROFILINGPASS_H


#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "PathEncodingPass.h"

namespace pathprofiling {


struct PathProfilingPass : public llvm::ModulePass {

  static char ID;
  llvm::DenseMap<llvm::Loop*,llvm::DenseMap<llvm::BasicBlock*, unsigned>> nl;
  llvm::DenseMap<llvm::Loop*, 
                 llvm::DenseMap<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>, unsigned>> vl;

  PathProfilingPass()
    : llvm::ModulePass(ID)
      { }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.addRequired<llvm::LoopInfoWrapperPass>();
    au.addRequired<PathEncodingPass>();
  }

  virtual bool runOnModule(llvm::Module &m) override;

  void instrument_local();

  void instrument(llvm::BasicBlock* bb, llvm::Loop *loop, uint64_t loopID, uint64_t counter);

  // void instrument(llvm::Module &m, llvm::Value *counter,llvm::Loop *loop, uint64_t loopID);
};


}


#endif

