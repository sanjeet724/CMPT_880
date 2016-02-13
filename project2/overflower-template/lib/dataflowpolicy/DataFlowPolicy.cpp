
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>

#include "DataFlowPolicy.h"

using namespace std;
using namespace llvm;
using namespace dataflowpolicy;


namespace dataflowpolicy {

char DataFlowPass::ID = 0;

}
namespace {

RegisterPass<DataFlowPass> X("dataflowpolicy",
                               "Simple checks for variable overflows");

}


// Note, the values for UNDEFINED and UNKNOWN are determined by OPEN and CLOSED.
// OPEN ∐ CLOSED = UNKNOWN in the lattice, so OPEN | CLOSED = UNKNOWN
// in our abstract state implementation.
// This is a simple use of a *bitvector framework*.
enum class FileState : char {
  UNDEFINED = 0,
  OPEN      = 1,
  CLOSED    = 2,
  UNKNOWN   = 3,
};


using StateMap = std::unordered_map<const llvm::Value*,FileState>;
using InstRange = iterator_range<inst_iterator>;


inline InstRange
instructions(Function &F) {
  return InstRange{inst_begin(F), inst_end(F)};
}


static FileState
operator|(const FileState s1, const FileState s2) {
  using repTy = std::underlying_type<FileState>::type;
  return FileState(static_cast<repTy>(s1) | static_cast<repTy>(s2));
}


static FileState
operator&(const FileState s1, const FileState s2) {
  using repTy = std::underlying_type<FileState>::type;
  return FileState(static_cast<repTy>(s1) & static_cast<repTy>(s2));
}


static bool
isSet(const FileState state, const FileState pattern) {
  return (state & pattern) == pattern;
}


static FileState
meet(const FileState state1, const FileState state2) {
  return state1 | state2;
}


static void
meet(StateMap &dest, const StateMap &src) {
  for (auto &kvPair : src) {
    const auto found = dest.find(kvPair.first);
    if (found == dest.end()) {
      dest.insert(kvPair);
    } else {
      found->second = meet(found->second, kvPair.second);
    }
  }
}


static void
dumpState(const StateMap &state) {
  for (auto &statePair : state) {
    outs() << "STATE " << static_cast<uint32_t>(statePair.second) << " "
           << statePair.first << " " << *statePair.first << "\n";
  }
}


static const Function *
getCalledFunction(const CallSite cs) {
  if (!cs.getInstruction()) {
    return nullptr;
  }

  const Value *called = cs.getCalledValue()->stripPointerCasts();
  return dyn_cast<Function>(called);
}


static bool
mayBeClosed(StateMap &state, const Value *arg) {
  const auto found = state.find(arg);
  return state.end() != found
      && isSet(found->second, FileState::CLOSED);
}


static FileState
meetOverPHI(const StateMap &state, const PHINode &phi) {
  auto phiState = FileState::UNDEFINED;
  for (auto &value : phi.incoming_values()) {
    auto found = state.find(value.get());
    if (state.end() != found) {
      phiState = meet(phiState, found->second);
    }
  }
  return phiState;
}


static void
transfer(Instruction &i, StateMap &state) {
  // Conservatively model all loaded info as UNKNOWN
  if (auto *li = dyn_cast<LoadInst>(&i)) {
    state[li] = FileState::UNKNOWN;
    return;
  }

  // All phis are explicit meet operations
  if (auto *phi = dyn_cast<PHINode>(&i)) {
    const auto met = meetOverPHI(state, *phi);
    if (met != FileState::UNDEFINED) {
      state[phi] = met;
    }
  }

  const CallSite cs{&i};
  const auto *fun = getCalledFunction(cs);
  // Pretend that indirect calls & non calls don't exist for this analysis
  if (!fun) {
    return;
  }

  // Apply the transfer function to the absract state
  if (fun->getName() == "fopen") {
    state[&i] = FileState::OPEN;
  } else if (fun->getName() == "fclose") {
    const auto *closed = cs.getArgument(0);
    state[closed] = FileState::CLOSED;
  }
}


class WorkList {
  unordered_set<BasicBlock*> inList;
  deque<BasicBlock*> work;

public:

  template<typename IterTy>
  WorkList(IterTy i, IterTy e)
    : inList{i, e},
      work{i, e} {
  }

  bool empty() { return work.empty(); }

  void add(BasicBlock *bb) {
    if (!inList.count(bb)) {
      work.push_back(bb);
    }
  }

  BasicBlock * take() {
    auto *front = work.front();
    work.pop_front();
    inList.erase(front);
    return front;
  }
};


static unordered_map<Instruction*,std::pair<StateMap,StateMap>>
computeFilePolicyState(Function &f) {
  // Initialize the abstract state of all BasicBlocks
  unordered_map<Instruction*,std::pair<StateMap,StateMap>> abstractState;
  for (auto &bb : f) {
    abstractState[bb.getTerminator()] = std::make_pair(StateMap{},StateMap{});
  }

  // First add all blocks to the worklist in topological order for efficiency
  ReversePostOrderTraversal<Function*> rpot{&f};
  WorkList work{begin(rpot), end(rpot)};

  while (!work.empty()) {
    auto *bb = work.take();

    // Save a copy of the initial and final abstract state to check for changes.
    auto oldExitState   = abstractState[bb->getTerminator()];
    auto &oldEntryState = abstractState[&*bb->begin()];

    // Merge the state coming in from all predecessors
    auto state = StateMap();
    for (auto p : llvm::predecessors(bb)) {
      meet(state, abstractState[p->getTerminator()].second);
    }

    // If we have already processed the block and no changes have been made to
    // the abstract input, we can skip processing the block.
    if (state == oldEntryState.first && !state.empty()) {
      continue;
    }

    // Propagate through all instructions in the block
    for (auto &i : *bb) {
      abstractState[&i].first = state;
      transfer(i, state);
      abstractState[&i].second = state;
    }

    // If the abstract state for this block did not change, then we are done
    // with this block. Otherwise, we must update the abstract state and
    // consider changes to successors.
    if (state == oldExitState.second) {
      continue;
    }

    for (auto s : llvm::successors(bb)) {
      work.add(s);
    }
  }

  return abstractState;
}


void
DataFlowPass::addNotOpenError(Instruction *i, unsigned argNo) {
  errors[i] = argNo;
}


// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
DataFlowPass::runOnFunction(Function &f) {
  auto abstractState = computeFilePolicyState(f);

  // Identify bugs via the abstract state
  for (auto &i : instructions(f)) {
    CallSite cs(&i);
    auto *fun = getCalledFunction(cs);
    if (!fun) {
      continue;
    }

    // Check the incoming state for errors
    auto &state = abstractState[&i].first;
    if ((fun->getName() == "fread" || fun->getName() == "fwrite")
        && mayBeClosed(state, cs.getArgument(3))) {
      addNotOpenError(&i, 3);

    } else if ((fun->getName() == "fprintf"
             || fun->getName() == "fflush"
             || fun->getName() == "fclose")
          && mayBeClosed(state, cs.getArgument(0))) {
      addNotOpenError(&i, 0);
    }
  }

  return false;
}


static void
printLineNumber(raw_ostream &out, Instruction *inst) {
  if (const DILocation *loc = inst->getDebugLoc()) {
    out << "At "    << loc->getFilename()
        << " line " << loc->getLine()
        << ":\n";
  } else {
    out << "At an unknown location:\n";
  }  
}


// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
void
DataFlowPass::print(raw_ostream &out, const Module *m) const {
  for (auto &errorPair : errors) {
    Instruction *fileOperation;
    unsigned argNum;
    std::tie(fileOperation, argNum) = errorPair;

    out.changeColor(raw_ostream::Colors::RED);
    printLineNumber(out, fileOperation);

    auto *called = getCalledFunction(llvm::CallSite{fileOperation});
    out.changeColor(raw_ostream::Colors::YELLOW);
    out << "In call to \"" << called->getName()
        << "\" argument (" << argNum
        << ") may not be an open file.\n";
  }

  if (errors.empty()) {
    out.changeColor(raw_ostream::Colors::GREEN);
    out << "No errors detected\n";
  }
  out.resetColor();
}
