#include "CPPUnit/CPPUnit_Test.h"

using namespace mull;
using namespace std;

CPPUnit_Test::CPPUnit_Test(std::string Name,
                           std::string targetName,
                           llvm::Function *TestBody,
                           std::vector<llvm::Function *> Ctors) :
  Test(TK_CPPUnit),
  TestName(Name),
  targetName(targetName),
  TestBodyFunction(TestBody),
  GlobalCtors(Ctors)
{
}

std::string CPPUnit_Test::getTestName() {
  return TestName;
}

std::string CPPUnit_Test::getTestDisplayName() {
  return getTestName();
}

std::vector<llvm::Function *> &CPPUnit_Test::GetGlobalCtors() {
  return GlobalCtors;
}

llvm::Function *CPPUnit_Test::GetTestBodyFunction() {
  return TestBodyFunction;
}

std::string CPPUnit_Test::getTargetName() const {
  return targetName;
}
