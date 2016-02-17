
#include "DataFlow.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Casting.h"

#include <deque>
#include <iterator>
#include <set>

using namespace llvm;
using namespace dataflows;


char DataFlowPass::ID = 0;

RegisterPass<DataFlowPass> X{"weightedcg",
                                "construct a weighted call graph of a module"};


bool
DataFlowPass::runOnModule(Module &m) {
  for (auto &f : m) {
      if(f.getName().equals("main")) {
        handleFunction(&f);
  }
      //   // check if function is already in the map
      //   std::vector<llvm::CallSite> functionCSVector;
      //   auto findFunction =  functionCallSiteMap.find(&f); 
      //   if (functionCallSiteMap.end() == findFunction) {
      //       functionCallSiteMap.insert(std::make_pair(&f,functionCSVector));
      //   }
      //   for (auto &bb : f) {
      //     for (auto &i : bb) {
      //      handleInstruction(CallSite(&i));
      //     }
      //   }
      // }
    }
  return false;
}

void 
DataFlowPass::handleFunction(Function *f) {
  outs() << "In Main\n" ;

}

void
DataFlowPass::handleInstruction(CallSite cs) {
  /*
  // Check whether the instruction is actually a call
  if (!cs.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (!called || called->getName().startswith("llvm")) {
    if (!called) {
      handleFunctionPointer(cs); // function pointer
    }
    else {
      return;
    }
  return;
  }

  // Store each CallSite of a function in a map (function* : vector of callsites)
  std::vector<llvm::CallSite> functionCSVector;
  auto caller = cs.getCaller();
  auto parentFunction =  functionCallSiteMap.find(caller);
  if (functionCallSiteMap.end() == parentFunction) {
    functionCSVector.push_back(cs);
    functionCallSiteMap.insert(std::make_pair(caller,functionCSVector));
  }
  else {
    parentFunction->second.push_back(cs);
  }
  */
}




