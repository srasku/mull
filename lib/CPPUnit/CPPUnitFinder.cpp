#include "CPPUnit/CPPUnitFinder.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "CPPUnit/CPPUnit_Test.h"

#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"

#include <queue>
#include <set>
#include <vector>

#include <cxxabi.h>

using namespace mull;
using namespace llvm;

CPPUnitFinder::CPPUnitFinder(
  std::vector<std::unique_ptr<MutationOperator>> mutationOperators,
  std::vector<std::string> testsToFilter,
  std::vector<std::string> excludeLocations,
  std::vector<CustomTest> &customTests)
  : TestFinder(),
  mutationOperators(std::move(mutationOperators)),
  filter(CPPUnitMutationOperatorFilter(testsToFilter, excludeLocations)),
  customTests(customTests)
{

}

std::vector<std::unique_ptr<Test>> CPPUnitFinder::findTests(Context &Ctx) {
  std::vector<std::unique_ptr<Test>> tests;

  for (auto &currentModule : Ctx.getModules()) {
    for (auto &function : currentModule->getModule()->getFunctionList()) {
      if (function.isDeclaration()) {
        continue;
      }

      for (CustomTest &test: customTests) {
        if (function.getName() != test.method) {
          continue;
        }

        tests.emplace_back(make_unique<CPPUnit_Test>(test.name,
                                                     test.target,
                                                     &function,
                                                     Ctx.getStaticConstructors()));
      }
    }
  }

  return tests;
}

std::vector<std::unique_ptr<Testee>>
CPPUnitFinder::findTestees(Test *Test,
                              Context &Ctx,
                              int maxDistance) {
  CPPUnit_Test *googleTest = dyn_cast<CPPUnit_Test>(Test);

  Function *targetFunction = nullptr;
  for (auto &currentModule : Ctx.getModules()) {
    for (auto &function : currentModule->getModule()->getFunctionList()) {
      if (function.isDeclaration()) {
        continue;
      }

      for (CustomTest &test: customTests) {
        if (function.getName() != test.target) {
          continue;
        }

        targetFunction = &function;
      }
    }
  }
  assert(targetFunction);

  Function *testFunction = googleTest->GetTestBodyFunction();

  std::vector<std::unique_ptr<Testee>> testees;
  testees.push_back(make_unique<Testee>(testFunction, nullptr, nullptr, 0));

  std::queue<std::unique_ptr<Testee>> traversees;
  std::set<Function *> checkedFunctions;

  Module *testBodyModule = testFunction->getParent();

  traversees.push(make_unique<Testee>(targetFunction, nullptr, nullptr, 0));

  while (!traversees.empty()) {
    std::unique_ptr<Testee> traversee = std::move(traversees.front());
    Testee *traverseePointer = traversee.get();

    traversees.pop();

    Function *traverseeFunction = traversee->getTesteeFunction();
    const int mutationDistance = traversee->getDistance();

    errs() << "WTF: " << traverseeFunction->getName().str() << "\n";

    testees.push_back(std::move(traversee));

    /// The function reached the max allowed distance
    /// Hence we don't go deeper
    if (mutationDistance == maxDistance) {
      continue;
    }

    for (auto &BB : *traverseeFunction) {
      for (auto &I : BB) {
        auto *instruction = &I;

        CallSite callInstruction(instruction);
        if (callInstruction.isCall() == false &&
            callInstruction.isInvoke() == false) {
          continue;
        }

        Function *calledFunction = callInstruction.getCalledFunction();
        if (!calledFunction) {
          continue;
        }

        Function *definedFunction =
          testBodyModule->getFunction(calledFunction->getName());
        /* errs() << calledFunction->getName() << " : " << calledFunction->isDeclaration() << "\n"; */

        if (!definedFunction || definedFunction->isDeclaration()) {
          definedFunction =
            Ctx.lookupDefinedFunction(calledFunction->getName());
        } else {
          /* Logger::debug() << "CPPUnitFinder> did not find a function: " */
          /*                 << definedFunction->getName() << '\n'; */
        }

        if (definedFunction) {
          /* errs() << definedFunction->getName() << "\n"; */
          auto functionWasNotProcessed =
            checkedFunctions.find(definedFunction) == checkedFunctions.end();

          checkedFunctions.insert(definedFunction);

          if (functionWasNotProcessed) {
            /// Filtering
            if (filter.shouldSkipTesteeFunction(definedFunction)) {
              continue;
            }

            /// The code below is not actually correct
            /// For each C++ constructor compiler can generate up to three
            /// functions*. Which means that the distance might be incorrect
            /// We need to find a clever way to fix this problem
            ///
            /// * Here is a good overview of what's going on:
            /// http://stackoverflow.com/a/6921467/829116
            ///
            traversees.push(make_unique<Testee>(definedFunction,
                                                instruction,
                                                traverseePointer,
                                                mutationDistance + 1));
          }
        }
      }
    }
  }

  errs() << "WTF: " << testees.size() << "\n";

  return testees;
}

std::vector<MutationPoint *>
CPPUnitFinder::findMutationPoints(const Context &context,
                                     llvm::Function &testee) {

  if (MutationPointsRegistry.count(&testee) != 0) {
    return MutationPointsRegistry.at(&testee);
  }

  std::vector<MutationPoint *> points;

  for (auto &mutationOperator : mutationOperators) {
    for (auto point : mutationOperator->getMutationPoints(context, &testee, filter)) {
      points.push_back(point);
      MutationPoints.emplace_back(std::unique_ptr<MutationPoint>(point));
    }
  }

  MutationPointsRegistry.insert(std::make_pair(&testee, points));
  return points;
}
