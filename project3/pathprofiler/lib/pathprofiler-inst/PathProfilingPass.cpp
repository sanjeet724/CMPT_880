#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <unordered_map>

#include "PathProfilingPass.h"
#include "InnermostLoops.h"


using namespace llvm;
using namespace pathprofiling;


namespace pathprofiling {
char PathProfilingPass::ID = 0;
}


bool
PathProfilingPass::runOnModule(Module &module) {
	nl = getAnalysis<PathEncodingPass>().numPathsInLoop;
	vl = getAnalysis<PathEncodingPass>().valuesInLoop;
	return true;
}

void
PathProfilingPass::instrument_local() {
	uint64_t counter = 0;
	for (auto &kv:vl) {
		Loop *l = kv.first;
		for ( auto &somePair:kv.second) {
			auto bbPair = somePair.first;
			BasicBlock *split = SplitEdge(bbPair.first,bbPair.second);
		}
	}
}

void
PathProfilingPass::instrument(BasicBlock *bb, Loop *loop, uint64_t loopID, uint64_t counter) {
	// AllocaInst* ai = new AllocaInst();
}

/*
void
PathProfilingPass::instrument(Module &module, Value *counter,
                              Loop *loop, uint64_t loopID) {

  //

}
*/

