

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/LoopIterator.h"

#include "PathEncodingPass.h"
#include "InnermostLoops.h"

#include <algorithm>
#include <vector>


using namespace llvm;
using namespace pathprofiling;


namespace pathprofiling {
  char PathEncodingPass::ID = 0;
}


bool
PathEncodingPass::runOnModule(Module &module) {
  for (auto &f : module) {
    if (!f.getName().startswith("llvm") && !f.isDeclaration()) {
    	handleLoops(&f);
    }
  }
  return false;
}

void
PathEncodingPass::handleLoops(Function *f) {
	LI = &getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
	std::vector<llvm::Loop*> innerL = getInnermostLoops(*LI);
	if (innerL.size() == 0) {
		outs() << "No Inner Loops Found In Function\n";
		return;
	}
	// iterate over the loops and encode them
	for (auto &l : innerL) {
		encode(l);
	}
	// FunctionLoopMap.insert(std::make_pair(f,innerL));
}

void
PathEncodingPass::encode(Loop *loop) {
	// outs() << "In Encode\n";
	LoopBlocksDFS *loopDFS = new LoopBlocksDFS(loop);
	loopDFS->perform(LI);
	char name = 'a';
	char *str = &name;
	// reverse topological order is postorder
	for (auto BBBegin = loopDFS->beginPostorder(), 
		      BBend = loopDFS->endPostorder();
		      BBBegin!=BBend; BBBegin++){
		(*BBBegin)->setName(StringRef(str));
	    name++;
	    createValues(*BBBegin, loop);
	}
	printNumPaths();
}

void
PathEncodingPass::createValues(BasicBlock *bb, Loop *l){
	// outs() << "Block: " << bb->getName() << ", ";
	if (l->getLoopLatch() == bb) {
		NumPaths.insert(std::make_pair(bb,1));
	}
	else {
		NumPaths.insert(std::make_pair(bb,0));
		const TerminatorInst *TInst = bb->getTerminator();
		// outs() << "# of Successors: " << TInst->getNumSuccessors() << "\n";
		for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
			BasicBlock *Succ = TInst->getSuccessor(I);
			if (l->contains(Succ)) {
				 auto v1 = NumPaths.find(bb);
				 auto v2 = NumPaths.find(Succ);
				 v1->second = v1->second + v2->second;
			}
		}
	}
}

void
PathEncodingPass::printNumPaths() {
	outs() << "NumPaths in Reverse Topilogical Order: \n";
	for (auto &kv:NumPaths){
		outs() << "BasicBlock: " << kv.first->getName() << ", ";
		outs() << "NumPaths: " << kv.second << "\n";
	}
}