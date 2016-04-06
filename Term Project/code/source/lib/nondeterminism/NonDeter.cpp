
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

// std::sets are of struct types in IR code
void
NonDeterPass::checkAllocation(Instruction *i) {
  AllocaInst *allocaInst = dyn_cast<AllocaInst>(i);
  if (!allocaInst){
    return;
  }
  auto *p = allocaInst->getType();
  if (p->getElementType()->isStructTy()) { 
    getFunctionParameters(p->getElementType());
  }
}

// check if the allocation is a set
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

// this function finds all the functions that have an "insert"
// the IR function doing the actual allocation of a element to a set, have an "insert"
void
NonDeterPass::findInserts(Function *f) {
  if (f->getName().find("insert") != StringRef::npos){
    insertFunctions.push_back(f);
  }
}

// we check for all the functions in the IR code that have an "insert"
// specifically we check for the arguments in that function
// if one of the arguments is of the container type (set/hash-table) detected earlier,
// then most probably the allocation is occuring in one of these functions
void
NonDeterPass::checkInserts() {
  for (auto &f:insertFunctions) {
    for (auto ab = f->arg_begin(), ae = f->arg_end();ab != ae; ab++) {
      if (ab->getType()->isPointerTy()) {
        auto *p = ab->getType()->getPointerElementType();
        StructType *argType = dyn_cast<StructType>(p);
        if (argType){
          if (argType == detectedContainer) { 
            outs() << "Argument Type is same as Container Type\n";
            performDataFlow(f);
          }
        }
      }
    }
  }
  outs() << "Size of Search Space: " << searchSpace.size() << "\n";
  searchFunctions();
}

// Go though this function and create a map of all the functions it calls
void
NonDeterPass::performDataFlow(Function *f) {
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
  if (called->getName().startswith("llvm") || called->getName().startswith("__")) {
    return;
  }
  auto someFunction =  searchSpace.find(called);
  if (searchSpace.end() == someFunction) {
    searchSpace.insert(std::make_pair(called,false));
  }
  performDataFlow(called); // recurse
}

// Go through the map of called functions from our main function(having the "insert")
// Specifically look for PtrToIntInst instructions in these functions
void
NonDeterPass::searchFunctions() {
  for (auto &kv:searchSpace){
    auto &f = kv.first;
    if (checkAlocatorFunction(f)) {
      outs() << f->getName() << "\n";
      kv.second = true;
    }
  }
}

// A PtrToIntInst instruction shows us that the main "insert" function
// is allocating an integer to store the element
// Since the container(set/hash-map) is storing addresses and addresses are stored as ints
// internally by the allocator
bool 
NonDeterPass::checkAlocatorFunction(Function *f){
  for (auto &bb:*f) {
    for (auto &i : bb) {
      PtrToIntInst *ptrtoInt = dyn_cast<PtrToIntInst>(&i);
      if (ptrtoInt){
        outs() << "ptrtoInt Instruction Found\n";
        return true;
      }
    }
  }
  return false;
}






