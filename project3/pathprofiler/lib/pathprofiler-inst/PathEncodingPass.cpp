

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
		// printNumPathsInLoop(l);
		// printValuesInLoop(l);
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
	    createNumPaths(*BBBegin, loop);
	}
	numPathsInLoop.insert(std::make_pair(loop, numPaths));
	valuesInLoop.insert(std::make_pair(loop,values));
	numPaths.clear();
	values.clear();
}

void
PathEncodingPass::createNumPaths(BasicBlock *bb, Loop *l){
	// outs() << "Block: " << bb->getName() << ", ";
	if (l->getLoopLatch() == bb) {
		numPaths.insert(std::make_pair(bb,1));
	}
	else {
		numPaths.insert(std::make_pair(bb,0));
		const TerminatorInst *TInst = bb->getTerminator();
		// outs() << "# of Successors: " << TInst->getNumSuccessors() << "\n";
		for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
			BasicBlock *Succ = TInst->getSuccessor(I);
			if (l->contains(Succ)) {
				 auto v1 = numPaths.find(bb);
				 auto v2 = numPaths.find(Succ);
				 if (v1->second > 0) {
				 	values.insert(std::make_pair(std::make_pair(bb,Succ),v1->second));
				 }
				 v1->second = v1->second + v2->second;
			}
		}
	}
}

void
PathEncodingPass::printValues(llvm::DenseMap<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>, unsigned> val) {
	outs() << "Edges and Vals: \n";
	for (auto &kv:val){
		auto e = kv.first;
		outs() << e.first->getName() << "--->" << e.second->getName() << " " << kv.second << "\n";
	}
}

void
PathEncodingPass::printNumPaths(llvm::DenseMap<llvm::BasicBlock*, unsigned> n) {
	outs() << "NumPaths in Reverse Topological Order: \n";
	for (auto &kv:n){
		outs() << "BasicBlock: " << kv.first->getName() << ", ";
		outs() << "NumPaths: " << kv.second << "\n";
	}
}

void
PathEncodingPass::printNumPathsInLoop(Loop *l) {
	auto nl = numPathsInLoop.find(l);
	llvm::DenseMap<llvm::BasicBlock*, unsigned> n = nl->second;
	printNumPaths(n);
}

void
PathEncodingPass::printValuesInLoop(Loop *l){
	auto vl = valuesInLoop.find(l);
	llvm::DenseMap<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>, unsigned> v = vl->second;
	printValues(v);
}