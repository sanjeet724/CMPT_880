
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


// First detect if we have a set/hash-table type in the IR code
bool
NonDeterPass::runOnModule(Module &m) {
  for (auto &f : m) {
    totalIRFunctions++;
    if(!(f.getName().startswith("llvm") || f.getName().startswith("vsn")) && !f.isDeclaration()) {
      findInserts(&f);
      findIterators(&f);
      // handleLoops(&f); // experimental - this may not be a better way
      for (auto &bb : f) {
        for (auto &i : bb) {
          checkAllocation(&i);
        }
      }
    }
  }
  checkInserts();
  checkIterators();
  detectNonDeterminism();
  return false;
}

void
NonDeterPass::handleLoops(Function *f){
  LI = &getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
  if (LI) {
    for (auto &loop:*LI){
      analyzeLoop(loop);
    }
  }
}

// this might not be a better way to implement as there are many loops
// we are specifically looking for node iterators rather than loops
void
NonDeterPass::analyzeLoop(Loop *l) {
  // outs() << "------Loop Analysis for In Function: " << l->getHeader()->getParent()->getName() << "-----\n";
  for (auto bb = l->block_begin(), be = l->block_end(); bb!= be; bb++){
    // outs() << **bb << "\n";
    // for (auto &i:bb) {
    //   outs() << *i << "\n";
    // }
  }
}


// std::sets are of struct types in IR code
void
NonDeterPass::checkAllocation(Instruction *i) {
  AllocaInst *allocaInst = dyn_cast<AllocaInst>(i);
  if (!allocaInst){
    return;
  }
  auto *p = allocaInst->getType();
  if (p->getElementType()->isStructTy()) { 
    getFunctionParameters(p->getElementType(),i);
  }
}

// check if the allocation is a set
// check for iterators
void
NonDeterPass::getFunctionParameters(Type *t, Instruction *i) {
  StructType *st = dyn_cast<StructType>(t);
  if (st) {
    if (st->getName().endswith("unordered_set")) {
      detectedContainer = st;
      outs() << "Type of Container: Unordered Set \n";
      return;
    }
    else if (st->getName().endswith("unordered_map")) {
      detectedContainer = st;
      outs() << "Type of Container: Unordered Map \n";
      return;
    }
    else if (st->getName().endswith("_Node_iterator")) {
      detectedIterator = st;
      return;
    }
  }
}

void
NonDeterPass::findIterators(Function *f) {
  if((f->getName().find("unordered_set") != StringRef::npos ||
     f->getName().find("unordered_map") != StringRef::npos) && 
     (f->getName().find("beginEv") != StringRef::npos ||
     f->getName().find("endEv") != StringRef::npos)){
    iteratorFunctions.push_back(f);
  }
}

void
NonDeterPass::checkIterators() {
  for (auto &f:iteratorFunctions) {
    for (auto ab = f->arg_begin(), ae = f->arg_end(); ab!=ae; ab++){
      if (ab->getType()->isPointerTy()){
        auto *p = ab->getType()->getPointerElementType();
        StructType *argType = dyn_cast<StructType>(p);
        if(argType){
          if(argType == detectedContainer){
            outs() << "Function used for Iteration: " << f->getName() << "\n";
            outs() << "Argument Type(for Loop) is of same type as Container\n";
            loopIteratorType = true;
            return;
          }
        }
      }
    }
  }
}

// this function finds all the functions that have an "insert"
// the IR function doing the actual allocation of a element to a set, have an "insert"
void
NonDeterPass::findInserts(Function *f) {
  if ((f->getName().find("unordered_set") != StringRef::npos || 
       f->getName().find("unordered_map") != StringRef::npos) &&
       f->getName().find("insert") != StringRef::npos){
    insertFunctions.push_back(f);
  }
}

// we check for all the functions in the IR code that have an "insert"
// specifically we check for the arguments in that function
// if one of the arguments is of the container type (set/hash-table) detected earlier,
// then most probably the allocation is occuring in one of these functions
void
NonDeterPass::checkInserts() {
  // outs() << "Size of InsertFunctions: " << insertFunctions.size() << "\n";
  for (auto &f:insertFunctions) {
    for (auto ab = f->arg_begin(), ae = f->arg_end();ab != ae; ab++) {
      if (ab->getType()->isPointerTy()) {
        auto *p = ab->getType()->getPointerElementType();
        StructType *argType = dyn_cast<StructType>(p);
        if (argType){
          if (argType == detectedContainer) { 
            outs() << "Function used for Insert: " << f->getName() << "\n";
            outs() << "Argument Type(for Insert) is same as Container\n";
            createSearchSpace(f);
          }
        }
      }
    }
  }
  // searchFunctions(); // not needed anymore as we are doing callsite sensitivity
}

// Go though this function and create a map of all the functions it calls
void
NonDeterPass::createSearchSpace(Function *f) {
  for (auto &bb:*f) {
    for (auto &i : bb) {
      handleCallSite(CallSite(&i));
    }
  }
}

// Check if an instruction is a call-site and then recurse on this call-site to
// generate the searchSpace map (Recursion)
void 
NonDeterPass::handleCallSite(CallSite cs) {
  if (!cs.getInstruction()) {
    return;
  }
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (called && !pointersAsAddress) {
    callDepth++;
    if (called->getName().startswith("llvm") || called->getName().startswith("__")) {
     return;
    }
    auto someFunction =  searchSpace.find(called);
    if (searchSpace.end() == someFunction) {
      searchSpace.insert(std::make_pair(called,false));
    }
    if (analyzeCallSite(called)) {
      return;
    }
    createSearchSpace(called); // recurse
  }
}

bool 
NonDeterPass::analyzeCallSite(Function *f) {
  for (auto &bb:*f) {
    for (auto &i : bb) {
      PtrToIntInst *ptrtoInt = dyn_cast<PtrToIntInst>(&i);
      if (ptrtoInt){
        outs() << "ptrtoInt Instruction Found in:-->" << f->getName() <<"\n";
        outs() << "CallSite Sensitivity Depth: " << callDepth << "\n";
        pointersAsAddress = true;
        return true;
      }
    }
  }
  return false;
}

// This is not needed anymore as we are doing call-site sensitivity
// Data-Flow Analysis : - check if an address is getting coverted to an integer
// Go through the map of called functions from our main function(having the "insert")
// Specifically look for PtrToIntInst instructions in these functions
void
NonDeterPass::searchFunctions() {
  for (auto &kv:searchSpace){
    auto &f = kv.first;
    if (checkAllocatorFunction(f)) {
      outs() << "Function storing the pointers: " << f->getName() << "\n";
      kv.second = true;
    }
  }
}

// A PtrToIntInst instruction shows us that the main "insert" function
// is allocating an integer to store the element
// Since the container(set/hash-map) is storing addresses and addresses are stored as ints
// internally by the allocator
bool 
NonDeterPass::checkAllocatorFunction(Function *f){
  for (auto &bb:*f) {
    for (auto &i : bb) {
      PtrToIntInst *ptrtoInt = dyn_cast<PtrToIntInst>(&i);
      if (ptrtoInt){
        outs() << "ptrtoInt Instruction Found\n";
        pointersAsAddress = true;
        return true;
      }
    }
  }
  return false;
}

void 
NonDeterPass::detectNonDeterminism() {
  outs() << "Total # of IR Functions: " << totalIRFunctions << "\n";
  if (pointersAsAddress && loopIteratorType) {
    outs() << "Input file has non-deterministic behavior\n" ;
  }
}






