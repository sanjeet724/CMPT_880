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
	numPathsInLoop = getAnalysis<PathEncodingPass>().numPathsInLoop;
	edgeVals = getAnalysis<PathEncodingPass>().valuesInLoop;
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
	globalCounter = ai;
}

void
PathProfilingPass::initializeCounter(Loop *l) {
	BasicBlock *BB = l->getHeader();
	BBnumPaths = numPathsInLoop.find(l)->second;
	auto num = BBnumPaths.find(BB)->second;
	ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(BB->getParent()->getContext()),0);
	outs() << "Inserting Instructions\n";
	auto *SI = new StoreInst(zero,globalCounter, BB->getFirstNonPHI());
	// global variable injection
	Type *I = IntegerType::getInt32Ty(BB->getParent()->getContext());
	// uint64_t num = nbb.find(BB); // # of numpaths for the BB
    Type *arrayType = ArrayType::get(I, num);
    // pathcount in runtime
	GlobalVariable *PaThPrOfIlInG_pathCounts = new GlobalVariable(arrayType,
	    	                                        false,
	    	                                        GlobalValue::ExternalLinkage,
	    	                                        nullptr,
	    	                                        "pathCounts",
	    	                                        GlobalValue::NotThreadLocal,
	    	                                        0,
	    	                                        false);
	// numpaths in runtime
	GlobalVariable *PaThPrOfIlInG_numPaths = new GlobalVariable(I,
	    	                                        false,
	    	                                        GlobalValue::ExternalLinkage,
	    	                                        nullptr,
	    	                                        "numOfPaths",
	    	                                        GlobalValue::NotThreadLocal,
	    	                                        0,
	    	                                        false);
}

void
PathProfilingPass::instrument_local() {
	uint64_t LoopId = 0;
	for (auto &kv:edgeVals) {
		Loop *l = kv.first;
		initializeCounter(l); 
		for (auto &somePair:kv.second) {
			auto bbPair = somePair.first;
			auto val = somePair.second;
			BasicBlock *split = SplitEdge(bbPair.first,bbPair.second);
			instrument(split,l,LoopId,globalCounter,val);
		}
		LoopId++;
	}
}

void
PathProfilingPass::instrument(BasicBlock *bb, Loop *loop, uint64_t loopID,
                              Value *c, uint64_t edgeVal) {
	auto *LI = new LoadInst(c,"loadCounter", bb->getTerminator());
	ConstantInt *eVal = ConstantInt::get(Type::getInt32Ty(bb->getParent()->getContext()),edgeVal);
	BinaryOperator *bo = BinaryOperator::Create(Instruction::Add,LI,eVal,"", bb->getTerminator());
	auto *SI = new StoreInst(bo, c, bb->getTerminator());
}


// to do
// At the loopExitBlock, insert a callsite which will
// call the runtime method to track the instrumentation

/*
void
PathProfilingPass::instrument(Module &module, Value *counter,
                              Loop *loop, uint64_t loopID) {

  //

}
*/

