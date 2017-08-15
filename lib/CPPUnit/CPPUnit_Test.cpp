#include "CPPUnit/CPPUnit_Test.h"

using namespace mull;
using namespace std;

CPPUnit_Test::CPPUnit_Test(std::string Name,
                                 llvm::Function *TestBody,
                                 std::vector<llvm::Function *> Ctors) :
  Test(TK_CPPUnit), TestName(Name), TestBodyFunction(TestBody), GlobalCtors(Ctors)
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
