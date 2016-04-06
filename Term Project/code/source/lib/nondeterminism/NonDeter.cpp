
#include "NonDeter.h"
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
using namespace nondeterminism;


char NonDeterPass::ID = 0;

RegisterPass<NonDeterPass> X{"nondeterminism",
                             "detect non-determinism in a module"};


bool
NonDeterPass::runOnModule(Module &m) {
    for (auto &f : m) {
      if(!(f.getName().startswith("llvm") || f.getName().startswith("vsn"))) {
        findInserts(&f);
        for (auto &bb : f) {
          for (auto &i : bb) {
            checkAllocation(&i);
          }
        }
      }
    }
    checkInserts();
  return false;
}

// this function finds all the functions that have a "insert"
void
NonDeterPass::findInserts(Function *f) {
  if (f->getName().find("insert") != StringRef::npos){
    insertFunctions.push_back(f);
  }
}

void
NonDeterPass::checkInserts() {
  for (auto &f:insertFunctions) {
    for (auto ab = f->arg_begin(), ae = f->arg_end();ab != ae; ab++) {
      if (ab->getType()->isPointerTy()) {
        auto *p = ab->getType()->getPointerElementType();
        StructType *argType = dyn_cast<StructType>(p);
        if (argType){
          if (argType == detectedContainer) { 
            // data-flow analysis to be done on this function
            outs() << "Same Type Detected in Insert Function Call\n";
            performDataFlow(f);
          }
        }
      }
    }
  }
  if (allocatorFunction) {
     checkAlocatorFunction(allocatorFunction);
  }
}

void
NonDeterPass::performDataFlow(Function *f) {
  //outs() << "CallDepth: " << callDepthCounter << "-----------\n";
  //outs() << "Callsites for: " << f->getName() << "\n";
  //outs() << f->getName() << "----------\n";
  callDepthCounter++;
  for (auto &bb:*f) {
    for (auto &i : bb) {
      handleCallSite(CallSite(&i));
    }
  }
}

void 
NonDeterPass::checkAlocatorFunction(Function *f){
  outs() << "Call depth: " << callDepthCounter << "\n";
  outs() << f->getName() << "\n";
  for (auto &bb:*f) {
    for (auto &i : bb) {
      PtrToIntInst *ptrtoInt = dyn_cast<PtrToIntInst>(&i);
      if (ptrtoInt){
        outs() << "ptrtoInt Instruction Found\n";
      }
    }
  }
}

void 
NonDeterPass::handleCallSite(CallSite cs) {
  if (!cs.getInstruction()) {
    return;
  }
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (called->getName().startswith("llvm") || called->getName().startswith("__")) {
    return;
  }
  // searchPtrtoIntInstr(cs.getInstruction());
  if (called->getName().startswith("_ZNKSt4hashIPSsEclES0_")) {
    allocatorFunction = called;
    return;
  }
  else {
    performDataFlow(called);
  }
}

void
NonDeterPass::searchPtrtoIntInstr(Instruction *i){
  outs() << *i << "\n";
  PtrToIntInst *ptrtoInt = dyn_cast<PtrToIntInst>(i);
  if (ptrtoInt) {
    outs() << "ptrtoInt Instruction Found\n";
  }
}

void
NonDeterPass::getFunctionParameters(Type *t) {
  StructType *st = dyn_cast<StructType>(t); 
  if (st) {
    if (st->getName().endswith("unordered_set")) {
      detectedContainer = st;
      outs() << "Unordered Set Detected\n";
    }
  }
}

void
NonDeterPass::checkAllocation(Instruction *i) {
  AllocaInst *allocaInst = dyn_cast<AllocaInst>(i);
  if (!allocaInst){
    return;
  }
  auto *p = allocaInst->getType();
  //outs() << "AllocaInst Name: " << allocaInst->getName() << "\n";
  // outs() << "Allocated Type: " << *allocaInst->getAllocatedType() << "\n";
  // outs() << "Allocation Type: " << *p->getElementType() << "\n";
  if (p->getElementType()->isStructTy()) {
    getFunctionParameters(p->getElementType());
    // outs() << *p->getElementType() << ", ";
    // outs() << "ID: " << p->getElementType()->getTypeID() << "\n";
  }
  // if (p->getElementType()->getTypeID() == 12) {
  //   // outs() << "Type Name: " << *p->getElementType() << "\n";
  //  // getFunctionParameters(p->getElementType());
  //   if (isa<FunctionType>(*p->getElementType())) {
  //    //  outs() << "FunctionType Found\n"; 
  //   }
  // }
  // FunctionType *fType = dyn_cast<FunctionType>(p->getElementType());
  // if (fType) {
  //   outs() << "FunctionType Found\n"; 
  // }
  // else {
  //   outs() << "No FunctionType Found\n"; 
  // }
  // outs() << "Allocation Type: " << *p->getElementType() << ", ID: " <<  p->getElementType()->getTypeID() << "\n";


  /*
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
  // outs() << "Buffer size in bytes: " << bufferSizeByte << "\n";
  functionBufferMap.insert(std::make_pair(allocated,bufferSize));
  */
}


void
NonDeterPass::createFunctionPointerMap(CallSite cs) {
  // Store each CallSite of a function in a map (function* : vector of callsites)
  std::vector<llvm::CallSite> functionCSVector;
  auto caller = cs.getInstruction()->getParent()->getParent();
  auto parentFunction =  functionCallSiteMap.find(caller);
  if (functionCallSiteMap.end() == parentFunction) {
    functionCSVector.push_back(cs);
    functionCallSiteMap.insert(std::make_pair(caller,functionCSVector));
  }
  else {
    parentFunction->second.push_back(cs);
  }
}

void
NonDeterPass::handleFunctionPointer(CallSite cs) {
  for (auto &f:candidates) {
    if (cs.getNumArgOperands() == f->arg_size()){ 
      unsigned argCounter = 0;
      for(auto ab = f->arg_begin(), ae = f->arg_end();ab != ae; ab++) {
        if (ab->getType() != cs.getArgument(argCounter)->getType()) {
          return;
        }
        else {
          ++argCounter;
        }
      }
      if (argCounter == cs.getNumArgOperands()) {
         matchedVF.push_back(f);
      }
    }
  }
  if (matchedVF.size() > 0) {
    createFunctionPointerMap(cs);
  }
}
