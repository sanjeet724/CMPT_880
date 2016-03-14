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
	for (auto &f : module) {
	    if (!f.getName().startswith("llvm") && !f.isDeclaration()) {
	    	allocateCounter(&f);
	    }
    }
    instrument_local();
	return true;
}

void
PathProfilingPass::allocateCounter(Function *f) {
	// get the entry block
	auto &entry = f->getEntryBlock(); 
	// get the first instruction of the entry block
	auto firstInst = entry.getFirstInsertionPt(); 
	//create a new allocation
	AllocaInst* ai = new AllocaInst(Type::getInt32Ty(f->getContext()), "counter", firstInst); 
	globalCounter = dyn_cast<AllocaInst>(ai);
}

void
PathProfilingPass::initializeCounter(Function *f) {
	ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(f->getContext()),0);
	outs() << "Inserting Instructions\n";
	auto *SI = new StoreInst(zero,globalCounter);
	SI->insertAfter(globalCounter);
}

void
PathProfilingPass::instrument_local() {
	uint64_t LoopId = 0;
	for (auto &kv:vl) {
		Loop *l = kv.first;
		// counters are getting inserted multiple times, need to clear them
		initializeCounter(l->getHeader()->getParent()); 
		for ( auto &somePair:kv.second) {
			auto bbPair = somePair.first;
			BasicBlock *split = SplitEdge(bbPair.first,bbPair.second);
			instrument(split,l,LoopId);
		}
		LoopId++;
	}
}

void
PathProfilingPass::instrument(BasicBlock *bb, Loop *loop, uint64_t loopID) {
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

