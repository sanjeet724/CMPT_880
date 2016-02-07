
#include "CallGraph.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"

#include <deque>
#include <iterator>

using namespace llvm;
using namespace callgraphs;


char CallGraphPass::ID = 0;

char WeightedCallGraphPass::ID = 0;

RegisterPass<WeightedCallGraphPass> X{"weightedcg",
                                "construct a weighted call graph of a module"};


bool
CallGraphPass::runOnModule(Module &m) {
  // A good design might be to use the CallGraphPass to compute the call graph
  // and then use that call graph for computing and printing the weights in
  // WeightedCallGraph.
    for (auto &f : m) {
      if (!f.getName().startswith("llvm")) {
        callCounts.insert(std::make_pair(&f, 0));
        for (auto &bb : f) {
          for (auto &i : bb) {
           handleInstruction(CallSite(&i));
          }
        }
        // functionMap.insert(std::make_pair(&f,calledFunctions));
      }
    }
  return false;
}

void
CallGraphPass::handleInstruction(CallSite cs) {
  // Check whether the instruction is actually a call
  if (!cs.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (!called || called->getName().startswith("llvm")) {
    return;
  }

  auto caller = cs.getCaller();
  // outs() << caller->getName() << " calls " << called->getName() << "\n";
  std::vector<llvm::Function*> calledFunctionVector;

  auto parentFunction = functionMap.find(caller);
  if (functionMap.end() == parentFunction) {
      calledFunctionVector.push_back(called);
      functionMap.insert(std::make_pair(caller,calledFunctionVector));
  }
  else {
    parentFunction->second.push_back(called);
  }
 
  // Update the count for the particular call
  auto count = callCounts.find(called);
  // if (callCounts.end() == count) {
  //   count = callCounts.insert(std::make_pair(called, 0)).first;
  // }
  ++count->second;
}


// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. Any actual output, however, is produced separately.
bool
WeightedCallGraphPass::runOnModule(Module &m) {
  // The results of the call graph pass can be extracted and used here.
  auto &cgPass = getAnalysis<CallGraphPass>();
  callCountsW = cgPass.callCounts;
  computeWeights();

  return false;
}


// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
void
WeightedCallGraphPass::print(raw_ostream &out, const Module *m) const {
  // Print out all functions
  /*
  for (auto &kvPair : callCountsW) {
    auto *function = kvPair.first;
    uint64_t count = kvPair.second;
    out << function->getName() << "," << count << "\n";
  }
  */
}

void 
WeightedCallGraphPass::computeWeights() {
  auto &cgPass = getAnalysis<CallGraphPass>();
  auto tempMap = cgPass.functionMap;
  std::vector<llvm::Function*> called;
  for (auto &kvPair: tempMap) {
    auto *function = kvPair.first;
    outs() << function->getName() << "-->";
    std::vector<llvm::Function*> calledFunctionVector = kvPair.second;
    for (auto &c : calledFunctionVector) {
      outs() << c->getName() << ",";
    }
    outs() << "\n";
  }
}
