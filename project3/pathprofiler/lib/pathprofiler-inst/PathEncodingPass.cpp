

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
	outs() << "In Encode\n";
	LoopBlocksDFS *loopDFS = new LoopBlocksDFS(loop);
	loopDFS->perform(LI);
	for (auto BBBegin = loopDFS->beginPostorder(), 
		 BBend = loopDFS->endPostorder(); BBBegin!=BBend; BBBegin++){
		// iterate over the BB's in post order
		outs() << "Block's PostOrderNumber: " << loopDFS->getPostorder(*BBBegin) << "\n";
	}
}
