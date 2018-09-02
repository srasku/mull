#include "GoogleTest/GoogleTestFinder.h"

#include "GoogleTest/GoogleTest_Test.h"

#include "Context.h"
#include "Filter.h"
#include "Logger.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>

#include <vector>

using namespace mull;
using namespace std;
using namespace llvm;

struct swift_test_demangled {
  std::string testModule;
  std::string testSuite;
  std::string testName;

  std::string fullTestName() {
    return testSuite + "." + testName;
  }

  std::string toString() {
    return testModule + "/" + testSuite + "." + testName;
  }
};

bool parseSwiftMangledName(const std::string &testName,
                           swift_test_demangled *result) {
    //  errs() << "parseSwiftMangledName: " << testName << "\n";
  vector<string> strings;
  string digits;

  for (auto it = testName.begin() + 3; it != testName.end(); it++) {
    auto ch = *it;

    if (isdigit(ch)) {
      digits += ch;
    } else if (digits.size() == 0) {
      continue;
    } else {
      auto length = stoul(digits);
      string chunk(it, it + length);
      strings.push_back(chunk);
      it += length - 1;
      digits = "";

      if (strings.size() == 3) {
        break;
      }
    }
  }

  if (strings.size() == 3) {
    if (testName.back() != 'F') {
      return false;
    }

    swift_test_demangled t;
    t.testModule = strings[0];
    t.testSuite = strings[1];
    t.testName = strings[2];

    *result = t;

    return true;
  }

  return false;
}

GoogleTestFinder::GoogleTestFinder() {
  this->sandbox = ForkProcessSandbox();
}

std::vector<std::unique_ptr<Test>>
GoogleTestFinder::findTests(Context &context,
                            Filter &filter) {

  std::vector<std::unique_ptr<Test>> tests;

  return tests;
}

std::vector<std::unique_ptr<Test>>
GoogleTestFinder::findTests2(Context &context,
                             Filter &filter,
                             std::vector<std::string> &testNames) {

  for (std::string &testName: testNames) {
    errs() << "testname: " << testName << "(" << testName.size() << ")" << "\n";
  }
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &module: context.getModules()) {
    for (auto &function: module->getModule()->functions()) {
//      errs() << "function name: " << function.getName() << "\n";

      swift_test_demangled demangled;
      if (parseSwiftMangledName(function.getName().str(), &demangled) == false) {
//        errs() << "Could not parse mangled test name: " << function.getName() << "\n";
        continue;
      }
      errs() << "demangled: " << demangled.fullTestName() << "(" << demangled.fullTestName().size() << ")" << "\n";

      if (std::find(testNames.begin(), testNames.end(), demangled.fullTestName()) != testNames.end()) {
        tests.emplace_back(make_unique<GoogleTest_Test>(demangled.fullTestName(),
                                                        &function,
                                                        context.getStaticConstructors()));

      }
    }
  }

  for (auto &test: tests) {
    errs() << "testname: " << test->getTestName() << "\n";
  }

//  exit(123);
  return tests;
}
