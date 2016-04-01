
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
      if(!(f.getName().startswith("_") || f.getName().startswith("llvm"))) {
        // check if function is already in the map
        outs() << "Function Name: " << f.getName() << "\n";
        std::vector<llvm::CallSite> functionCSVector;
        auto findFunction =  functionCallSiteMap.find(&f); 
        if (functionCallSiteMap.end() == findFunction) {
            functionCallSiteMap.insert(std::make_pair(&f,functionCSVector));
        }
        for (auto &bb : f) {
          for (auto &i : bb) {
            handleInstruction(CallSite(&i));
          }
        }
      }
    }
  return false;
}

void
NonDeterPass::handleInstruction(CallSite cs) {
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
