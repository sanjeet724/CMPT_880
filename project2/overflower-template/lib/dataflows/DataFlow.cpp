
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

RegisterPass<DataFlowPass> X{"dataflows",
                                "detect data overflows"};


bool
DataFlowPass::runOnModule(Module &m) {
  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      for (auto &bb : f) {
        for (auto &i : bb) {
         handleInstruction(CallSite(&i));
        }
      }
    }
  }

  // check the data flow in function calls upto a depth of 2
  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      outs() << "In : " << f.getName();
      handleFunction(&f, callDepth);
      outs() << '\n';
    }
  }

  return false;
}

void 
DataFlowPass::handleFunction(Function *f, uint64_t cd) {
  for(auto &bb : *f) {
    for (auto &i : bb) {
        auto cs = CallSite(&i);
        if(cs.isCall()){
         if (!cs.getCalledFunction()->getName().startswith("llvm")){
             cd++;
             outs() << " calling : " << cs.getCalledFunction()->getName() << "\n";
             handleFunction(cs.getCalledFunction(), callDepth);
             cd--;
         }
        }
        else {
          // To do
        }
    }
  }
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




