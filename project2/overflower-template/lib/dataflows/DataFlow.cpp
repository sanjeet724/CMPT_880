
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
         //getGEP(&i);
         checkLoad(&i);
         //checkStore(&i);
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
  auto *p = allocaInst->getType();
  // we need it only if the allocation is an array
  ArrayType *a = dyn_cast<ArrayType>(p->getElementType()); 
  if (!a) {
    return;
  }
  // printAllocaInfo(allocaInst);
  // put the buffer size in a map
  allocated = i;
  functionBufferMap.insert(std::make_pair(i->getParent()->getParent(),a->getNumElements()));
}

void 
DataFlowPass::printAllocaInfo(AllocaInst *alloca) {
  outs() << "---Printing AllocaInst Info---\n";
  outs() << "AllocaInst Name: " << alloca->getName() << "\n";
  outs() << "Allocated Type: " << *alloca->getAllocatedType() << "\n";
  auto *p = alloca->getType();
  outs() << "allocaInst->getType(): " << *p << "\n";
  outs() << "Element Type of allocInst: " << *p->getElementType() << "\n";
  ArrayType *a = dyn_cast<ArrayType>(p->getElementType()); 
  outs() << "Allocated Array is:  " << *a << "\n";
  outs() << "# Elements in a: " << a->getNumElements() << "\n";
}

void
DataFlowPass::printGEPInfo(GetElementPtrInst *gep) {
  outs() << "---Printing GEP Info---\n";
  outs() << "GEP: " << *gep << "\n" ;
  outs() << "GEP's pointer operand: " << *gep->getPointerOperand() << "\n" ;
  outs() << "GEP #of Operands: " << gep->getNumOperands() << "\n";
  outs() << "GEP[2]: " << *gep->getOperand(2) << "\n";
}


void
DataFlowPass::checkLoad(Instruction *i) {
  LoadInst *lInst = dyn_cast<LoadInst>(i);
  if(!lInst) {
    return;
  }
  // outs() << "Load Instruction Found: " << lInst << "\n";
  // outs() << "Load Pointer operand: " << *lInst->getPointerOperand() << "\n";
  // outs() << "Load address space: " << lInst->getPointerAddressSpace() << "\n";
  loadMap.insert(std::make_pair(i->getParent()->getParent(),lInst));
  // get the GEP from the load instruction
  GetElementPtrInst *gep =  dyn_cast<GetElementPtrInst>(lInst->getPointerOperand());
  if (!gep) {
    return;
  }
  printGEPInfo(gep);
  outs() << "Alloca: " << *allocated << "\n";
  AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
  AliasResult ar = AA.alias(allocated,gep->getPointerOperand());
  if ( ar==3 ){
    outs() << "Must Alias\n" ;
  }

  for(auto ab=gep->idx_begin(),ae=gep->idx_end();ab != ae; ab++) {
    outs() << "ab is: " << ab << "\n";
  }
  
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





