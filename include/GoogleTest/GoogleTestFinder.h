#pragma once

#include "TestFinder.h"

#include "ForkProcessSandbox.h"

#include <vector>

namespace mull {

class Context;
class Filter;

class GoogleTestFinder : public TestFinder {
  ForkProcessSandbox sandbox;

public:
  GoogleTestFinder();

  std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                               Filter &filter) override;
  std::vector<std::unique_ptr<Test>> findTests2(Context &context,
                                                Filter &filter,
                                                std::vector<std::string> &testNames) override;

};

}
