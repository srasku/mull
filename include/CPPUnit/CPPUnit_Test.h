#pragma once

#include "Test.h"

#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class CPPUnit_Test : public Test {
  std::string TestName;
  llvm::Function *TestBodyFunction;
  std::vector<llvm::Function *> GlobalCtors;
public:
  CPPUnit_Test(std::string Name,
                  llvm::Function *TestBody,
                  std::vector<llvm::Function *> Ctors);

  std::string getTestName() override;
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  std::vector<llvm::Function *> &GetGlobalCtors();
  llvm::Function *GetTestBodyFunction();

  static bool classof(const Test *T) {
    return T->getKind() == TK_CPPUnit;
  }
};

}
