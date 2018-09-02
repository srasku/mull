#pragma once

#include "Test.h"

#include <memory>

namespace mull {

class Context;
class Filter;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Context &context,
                                                       Filter &filter) = 0;
  virtual std::vector<std::unique_ptr<Test>> findTests2(Context &context,
                                                        Filter &filter,
                                                        std::vector<std::string> &testNames) {
    return std::vector<std::unique_ptr<Test>>();
  };

  virtual ~TestFinder() {}
};

}
