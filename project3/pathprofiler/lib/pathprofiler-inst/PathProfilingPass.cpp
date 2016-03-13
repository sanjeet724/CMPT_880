#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
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
	instrument_local();
	return true;
}

void
PathProfilingPass::instrument_local() {
	uint64_t LoopId = 0;
	// function context
	// ConstantInt *zero = new ConstantInt(Type::getInt32Ty(getGlobalContext()), 0, false);
	// ConstantInt *zero = new ConstantInt(Type::getInt32Ty(getGlobalContext()));
	ConstantInt *zero;
	//llvm::ConstantInt::get(getGlobalContext, llvm::APInt(32, 0, false));
	for (auto &kv:vl) {
		Loop *l = kv.first;
		auto &entry = l->getHeader()->getParent()->getEntryBlock(); // get the entry block
		auto firstInst = entry.getFirstInsertionPt(); // get the first instruction of the entry block
        AllocaInst* ai = new AllocaInst(Type::getInt32Ty(getGlobalContext()),"counter", firstInst); //create a new instruction
        auto *SI = new StoreInst(ai,zero);
        SI->insertAfter(ai);
       //  entry.getInstList().insert(firstInst, ai);    // insert the instruction
		for ( auto &somePair:kv.second) {
			auto bbPair = somePair.first;
			BasicBlock *split = SplitEdge(bbPair.first,bbPair.second);
			instrument(split,l,LoopId,ai);
		}
		LoopId++;
	}
}

void
PathProfilingPass::instrument(BasicBlock *bb, Loop *loop, uint64_t loopID, Instruction *a) {
	// LoadInst *lInst = dyn_cast<LoadInst>(a);
	// <result> = add <ty> <op1>, <op2>          ; yields ty:result
}

// to Do
// 1 - Insert Instrcution at bbIteraor
// store instrcution with const integer value, location to store will be allocainst
// everytime we split the block, increment the pointer (load,)
// now when we exit the loop, save this in the runtime

/*
void
PathProfilingPass::instrument(Module &module, Value *counter,
                              Loop *loop, uint64_t loopID) {

  //

}
*/

