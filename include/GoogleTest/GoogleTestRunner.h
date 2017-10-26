#pragma once

#include "TestRunner.h"

#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Function;
class Module;

}

namespace mull {

class GoogleTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;

  std::map<std::string, std::string> mapping;

public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) override;

private:
  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}
