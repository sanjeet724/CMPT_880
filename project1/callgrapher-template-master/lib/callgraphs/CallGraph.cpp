
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
      for (auto &bb : f) {
        for (auto &i : bb) {
         handleInstruction(CallSite(&i));
        }
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

  // Store each CallSite of a function in a map (function* : vector of callsites)
  std::vector<llvm::CallSite> functionCSVector;
  auto caller = cs.getCaller();
  auto parentFunction2 =  functionCallSiteMap.find(caller);
  if (functionCallSiteMap.end() == parentFunction2) {
    functionCSVector.push_back(cs);
    functionCallSiteMap.insert(std::make_pair(caller,functionCSVector));
  }
  else {
    parentFunction2->second.push_back(cs);
  }
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. Any actual output, however, is produced separately.
bool
WeightedCallGraphPass::runOnModule(Module &m) {
  // The results of the call graph pass can be extracted and used here.
  auto &cgPass = getAnalysis<CallGraphPass>();
  tempMap = cgPass.functionCallSiteMap;
  computeWeights();
  functionMetaData();
  outs() << "\n";
  functionEdges();
  return false;
}


// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
void
WeightedCallGraphPass::print(raw_ostream &out, const Module *m) const {
}

void 
WeightedCallGraphPass::computeWeights() {
  // initialize the weights to 0 for all functions
  for (auto &kvPair:tempMap) {
     auto *function = kvPair.first;
     functionWeights.insert(std::make_pair(function,0));
  }
  // iterate over the callsites of each function to calculate weights
  for (auto &kvPair:tempMap) {
    auto *function = kvPair.first;
    std::vector<llvm::CallSite> calls = kvPair.second;
    for (auto &c : calls) {
      auto calledF = dyn_cast<Function>(c.getCalledValue());
      auto functionW = functionWeights.find(calledF);
      if (functionWeights.end() == functionW) {
        functionWeights.insert(std::make_pair(calledF, 1));
      }
      else {
        ++functionW->second;
      }
    }
  }
}

void
WeightedCallGraphPass::functionMetaData() {
  for (auto &kvPair:functionWeights) {
    auto *function = kvPair.first;
    uint64_t weight = kvPair.second;
    outs() << function->getName() << "," << weight ;
    checkFunctionCalls(function);
    outs() << "\n";
  }
}

static llvm::StringRef getOnlyFileName(llvm::StringRef lf){
  llvm::SmallVector<llvm::StringRef,16> splits;
  lf.split(splits, "/",-1, false);
  return splits.back();
}

void 
WeightedCallGraphPass::checkFunctionCalls(Function *searchFunction) {
  for (auto &kvPair:tempMap) {
    auto *function = kvPair.first;
    if (function == searchFunction) {
      // This function has some callsites
      std::vector<llvm::CallSite> calls = kvPair.second;
      unsigned siteID = 0;
      for (auto &c : calls) {
        llvm::StringRef longFileName = c.getInstruction()->getDebugLoc()->getFilename();
        llvm::StringRef fileName = getOnlyFileName(longFileName);
        getOnlyFileName(longFileName);
        uint64_t lineNumber = c.getInstruction()->getDebugLoc()->getLine();
        outs() << "," << siteID << "," 
                      << fileName << ","
                      << lineNumber;
        ++siteID;
      }
    }
  }
}

void
WeightedCallGraphPass::functionEdges() {
  for (auto &kvPair:tempMap) {
    auto *function = kvPair.first;
    unsigned siteID = 0;
    std::vector<llvm::CallSite> calls = kvPair.second;
    for (auto &c : calls) {
      auto calledF = dyn_cast<Function>(c.getCalledValue());
      outs() << function->getName() << "," << siteID << ","
             << calledF->getName() << "\n" ;
      ++siteID;
    }
  }
}

/*This function is just for sanity check*/
void
WeightedCallGraphPass::printFunctionSiteMap() {
  outs() <<"\nPrinting the functions in FunctionSiteMap\n";
  auto &cgPass = getAnalysis<CallGraphPass>();
  auto tempMap = cgPass.functionCallSiteMap;
  for (auto &kvPair:tempMap){
    auto *function = kvPair.first;
    outs() << function->getName() << "\n";
  }
  outs() <<"\n";
}
