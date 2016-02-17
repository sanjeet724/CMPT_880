
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
  // check for memory allocations of an array
  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      for (auto &bb : f) {
        for (auto &i : bb) {
         //handleInstruction(CallSite(&i));
         handleInstruction(&i);
        }
      }
    }
  }

  // check the data flow in function calls upto a depth of 2
  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      outs() << "In : " << f.getName();
      // handleFunction(&f, callDepth);
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
DataFlowPass::handleInstruction(Instruction *i) {
  // if (!cs.getInstruction()) {
  //   return;
  // }
  // auto i = cs.getInstruction();
  // auto i = dyn_cast<llvm::AllocaInst>(cs.getInstruction()->stripPointerCasts());
  // if (i) {
  //   outs() << "Alloca Found \n" ;
  // }
  // //auto i = cs.getInstruction();
  // // if (i->isArrayAllocation()) {
  // //    outs() << "Alloca Found \n" ;
  // // }
  
  if (AllocaInst *allocInst = dyn_cast<AllocaInst>(i)) {
     outs() << "Alloca Found \n" ;
  }
}




