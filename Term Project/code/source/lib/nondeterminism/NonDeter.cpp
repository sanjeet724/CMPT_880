
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
      if(!(f.getName().startswith("_") || f.getName().startswith("llvm") || f.getName().startswith("vsn"))) {
        outs() << "Function Name: " << f.getName() << "\n";
        for (auto &bb : f) {
          for (auto &i : bb) {
            checkAllocation(&i);
          }
        }
      }
    }
  return false;
}

void
NonDeterPass::checkAllocation(Instruction *i) {
  AllocaInst *allocaInst = dyn_cast<AllocaInst>(i);
  if (!allocaInst){
    return;
  }
  auto *p = allocaInst->getType();
  outs() << "AllocaInst Name: " << allocaInst->getName() << "\n";
  outs() << "Allocated Type: " << *allocaInst->getAllocatedType() << "\n";
  //outs() << "Allocation Type: " << *p->getElementType() << "\n";
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
