

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"

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
    if (!f.getName().startswith("llvm")) {
    	handleLoops(&f);
    }
  }
  return false;
}

void
PathEncodingPass::handleLoops(Function *f) {
	LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>(*f).getLoopInfo();
	std::vector<llvm::Loop*> innerL = getInnermostLoops(*LI);
	if (innerL.size() == 0) {
		outs() << "No Inner Loops Found In Function\n";
	}
}

void
PathEncodingPass::encode(Loop *loop) {

  //

}
