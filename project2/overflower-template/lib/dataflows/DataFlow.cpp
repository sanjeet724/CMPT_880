
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
         checkAllocation(&i);
         checkLoad(&i);
         //checkStore(&i);
         checkAlias(&i);
        }
      }
    }
  }

  // check the data flow in function calls upto a depth of 2
  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      // outs() << "In : " << f.getName();
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
DataFlowPass::checkAllocation(Instruction *i) {
  AllocaInst *allocaInst = dyn_cast<AllocaInst>(i);
  if (!allocaInst){
    return;
  }
  outs() << "Allocation Found \n";
  auto *p = allocaInst->getType();
  ArrayType *a = cast<ArrayType>(p->getElementType()); 
  // outs() << "ALlocaInst Name: " << allocaInst->getName() << "\n";
  // outs() << a->getNumElements() << "\n";
  // outs() << "Array Size: " << *allocaInst->getAllocatedType();
  // put the buffer size in a map
  functionBufferMap.insert(std::make_pair(i->getParent()->getParent(),a->getNumElements()));
}

void
DataFlowPass::checkLoad(Instruction *i) {
  LoadInst *lInst = dyn_cast<LoadInst>(i);
  if(!lInst) {
    return;
  }
  outs() << "Load Found \n";
  outs() << "Load Pointer operand: " << *lInst->getPointerOperand() << "\n";
  loadMap.insert(std::make_pair(i->getParent()->getParent(),lInst));

}

void
DataFlowPass::checkAlias(Instruction *i) {
  GetElementPtrInst *gep =  dyn_cast<GetElementPtrInst>(i);
  if (!gep) {
    return;
  }
  outs() << "GEP Found\n";
  outs() << "GEP Pointer operand: " << *gep->getPointerOperand() << "\n";
  outs() << "GEP Pointer operand Type: " << *gep->getPointerOperandType() << "\n";
  outs() << "GEP #of Indices: " << gep->getNumIndices() << "\n";
}

/*
void
DataFlowPass::checkStore(Instruction *i){
  StoreInst *sInst = dyn_cast<StoreInst>(i);
  if(!sInst) {
    return;
  }
  storeMap.insert(std::make_pair(i->getParent()->getParent(),sInst));
  outs() << "Store Found \n";
}
*/





