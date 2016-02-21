
#include "DataFlow.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"

#include <deque>
#include <iterator>
#include <set>

using namespace llvm;
using namespace dataflows;


char DataFlowPass::ID = 0;

RegisterPass<DataFlowPass> X{"dataflows",
                              "detect data overflows"};


bool 
DataFlowPass::doInitialization(Module &m) {
  const DataLayout &dl = m.getDataLayout();
  dataL = &dl;
  return false;
}


bool
DataFlowPass::runOnModule(Module &m) {
  // check for memory allocations of an array

  for (auto &f : m) {
    if (!f.getName().startswith("llvm")) {
      for (auto &bb : f) {
        for (auto &i : bb) {
         checkAllocation(&i);
         checkLoad(&i);
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
 //  printAllocaInfo(allocaInst);
  allocated = i;
  // outs() << "Array Type is: " << *a->getElementType() << "\n";
  allocatedTypeSize = dataL->getTypeStoreSize(a->getElementType());
  // outs() << "allocatedTypeSize is: " << allocatedTypeSize << "\n";
  bufferSize = a->getNumElements();
  bufferSizeByte = bufferSize * allocatedTypeSize;
  outs() << "Buffer size in bytes: " << bufferSizeByte << "\n";
  functionBufferMap.insert(std::make_pair(allocated,bufferSize));
}

// helper functions just for diagnostic purposes
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
  // LoadInst *lInst = dyn_cast<LoadInst>(i);
  // if(!lInst) {
  //   return;
  // }
  if(!i) {
    return;
  }
  // loadMap.insert(std::make_pair(i->getParent()->getParent(),lInst));
  // get the GEP from the load instruction
  GetElementPtrInst *gep =  dyn_cast<GetElementPtrInst>(i);
  if (!gep) {
    return;
  }
  // printGEPInfo(gep);
  AliasAnalysis &AA = getAnalysis<AliasAnalysis>();
  AliasResult ar = AA.alias(allocated,gep->getPointerOperand());
  if ( ar == 3 ){
    // must alias
    // gep->getOperand(2) gives the index 
    ConstantInt *indexGEP = dyn_cast<ConstantInt>(gep->getOperand(2));
    if (!indexGEP) {
      auto *v = gep->getOperand(2);
      recurseOnValue(v);
      return;
    }
    signed accessedIndex = indexGEP->getLimitedValue();
    signed accesedSize = (accessedIndex)*allocatedTypeSize;
    if (accessedIndex < 0 || accessedIndex > bufferSize-1) {
      outs() << "Invalid Memory Access: " << accesedSize << "\n";
      return;
    }
    outs() << "Memory Access Offset: " << accesedSize << "\n" ;
    return;
  }
  outs() << "Invalid Alias Analysis: " << ar ;
  return;
}

void
DataFlowPass::recurseOnValue(Value *v){
  Instruction *i = dyn_cast<Instruction>(v);
  if (!i){
    ConstantInt *index = dyn_cast<ConstantInt>(v);
    if (!index) {
      outs() << "Unknown Memory Address in Recurse \n";
      return;
    }
    // base case
    signed accessedIndex = index->getLimitedValue();
    signed accesedSize = (accessedIndex)*allocatedTypeSize;
    if (accessedIndex < 0 || accessedIndex > bufferSize-1) {
      outs() << "Invalid Memory Access: " << accesedSize << "\n";
      return;
    }
    outs() << "Memory Access Offset: " << accesedSize << "\n" ;
    return;
  }
  // outs() << "i is : " << *i << "\n";
  signed numOfOperands = i->getNumOperands();
  for (signed op = 0; op < numOfOperands; op++) {
    recurseOnValue(i->getOperand(op));
  }
  return;
}





