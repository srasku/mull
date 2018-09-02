#include "GoogleTest/GoogleTestRunner.h"

#include "GoogleTest/GoogleTest_Test.h"
#include "Mangler.h"

#include "Toolchain/Resolvers/InstrumentationResolver.h"
#include "Toolchain/Resolvers/NativeResolver.h"

#include <llvm/IR/Function.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>



using namespace mull;
using namespace llvm;
using namespace llvm::orc;

namespace {
  class UnitTest;
}

GoogleTestRunner::GoogleTestRunner(llvm::TargetMachine &machine) :
  TestRunner(machine),
  mangler(Mangler(machine.createDataLayout())),
  overrides([this](const char *name) {
    return this->mangler.getNameWithPrefix(name);
  }),
  fGoogleTestInit(mangler.getNameWithPrefix("_ZN7testing14InitGoogleTestEPiPPc")),
  fGoogleTestInstance(mangler.getNameWithPrefix("_ZN7testing8UnitTest11GetInstanceEv")),
  fGoogleTestRun(mangler.getNameWithPrefix("_ZN7testing8UnitTest3RunEv")),
  trampoline(new InstrumentationInfo*)
{
}

GoogleTestRunner::~GoogleTestRunner() {
  delete trampoline;
}

void *GoogleTestRunner::GetCtorPointer(const llvm::Function &Function) {
  return
    getFunctionPointer(mangler.getNameWithPrefix(Function.getName().str()));
}

void *GoogleTestRunner::getFunctionPointer(const std::string &functionName) {
  JITSymbol &symbol = jit.getSymbol(functionName);

  void *fpointer =
    reinterpret_cast<void *>(static_cast<uintptr_t>(llvm_compat::JITSymbolAddress(symbol)));

  if (fpointer == nullptr) {
    errs() << "GoogleTestRunner> Can't find pointer to function: "
           << functionName << "\n";
    exit(1);
  }

  return fpointer;
}

void GoogleTestRunner::runStaticCtor(llvm::Function *Ctor) {
//  printf("Init: %s\n", Ctor->getName().str().c_str());
  return;

  void *CtorPointer = GetCtorPointer(*Ctor);

  auto ctor = ((int (*)())(intptr_t)CtorPointer);
  ctor();
}

void GoogleTestRunner::loadInstrumentedProgram(ObjectFiles &objectFiles,
                                               Instrumentation &instrumentation) {
//  return;
  InstrumentationResolver resolver(overrides, instrumentation, mangler, trampoline);

  void *objcEnabledMM = sys::DynamicLibrary::SearchForAddressOfSymbol("GetObjCEnabledMemoryManager");
  assert(objcEnabledMM);
  SectionMemoryManager *(*objcEnabledMMFP)(void) = ((SectionMemoryManager * (*)(void))objcEnabledMM);
//  if (runnerFPtr == nullptr) {
//    errs() << "Could not find SwiftRuntimeSetupLoadEverything function: SwiftRuntimeSetupLoadEverything()" << "\n";
//    exit(1);
//  }
//  int result = runnerFPtr();


  SectionMemoryManager *castedObjcEnabledMM = (SectionMemoryManager *)objcEnabledMMFP();

  printf("before addObjectFiles\n");
  jit.addObjectFiles(objectFiles, resolver, std::unique_ptr<SectionMemoryManager>(castedObjcEnabledMM));
  printf("after addObjectFiles\n");
//  jit.addObjectFiles(objectFiles, resolver, make_unique<ObjCEnabledMemoryManager>());
}

void GoogleTestRunner::loadProgram(ObjectFiles &objectFiles) {
//  return;
  NativeResolver resolver(overrides);
  void *objcEnabledMM = sys::DynamicLibrary::SearchForAddressOfSymbol("GetObjCEnabledMemoryManager");
  assert(objcEnabledMM);
  SectionMemoryManager *(*objcEnabledMMFP)(void) = ((SectionMemoryManager * (*)(void))objcEnabledMM);
    //  if (runnerFPtr == nullptr) {
    //    errs() << "Could not find SwiftRuntimeSetupLoadEverything function: SwiftRuntimeSetupLoadEverything()" << "\n";
    //    exit(1);
    //  }
    //  int result = runnerFPtr();


  SectionMemoryManager *castedObjcEnabledMM = (SectionMemoryManager *)objcEnabledMMFP();
  jit.addObjectFiles(objectFiles, resolver, std::unique_ptr<SectionMemoryManager>(castedObjcEnabledMM));
}

ExecutionStatus GoogleTestRunner::runTest(Test *test) {
  errs() << "GoogleTestRunner::runTest " << test->getTestName() << "\n";
  *trampoline = &test->getInstrumentationInfo();

  GoogleTest_Test *GTest = dyn_cast<GoogleTest_Test>(test);

  for (auto &Ctor: GTest->GetGlobalCtors()) {
    runStaticCtor(Ctor);
  }

  void *runnerPtr = sys::DynamicLibrary::SearchForAddressOfSymbol("CustomXCTestRunnerRunOne");
  auto runnerFPtr = ((int (*)(const char *const))runnerPtr);
  if (runnerFPtr == nullptr) {
    errs() << "Could not find CustomXCTestRunner function: CustomXCTestRunnerRunAll()" << "\n";
    exit(1);
  }
  int result = runnerFPtr(test->getTestName().c_str());
  overrides.runDestructors();

  if (result == 0) {
    return ExecutionStatus::Passed;
  }
  return ExecutionStatus::Failed;
}
