
#ifndef DATAFLOWPOLICY_H
#define DATAFLOWPOLICY_H

#include <unordered_map>


namespace dataflowpolicy {


struct DataFlowPass : public llvm::FunctionPass {

  static char ID;

  std::unordered_map<llvm::Instruction*,unsigned> errors;

  DataFlowPass()
    : FunctionPass(ID)
      { }

  virtual bool runOnFunction(llvm::Function &f) override;

  virtual void print(llvm::raw_ostream &out, const llvm::Module *m) const override;

  void addNotOpenError(llvm::Instruction *i, unsigned argNo);
};


}


#endif