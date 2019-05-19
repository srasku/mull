#include "mull/Reporters/MutationTestingElementsReporter.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Config/RawConfig.h"
#include "mull/Filter.h"
#include "mull/Metrics/Metrics.h"
#include "mull/ModuleLoader.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "mull/Testee.h"

#include "json11/json11.hpp"
#include "gtest/gtest.h"

#include <cstring>
#include <ostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace mull;
using namespace llvm;
using namespace json11;

TEST(MutationTestingElementsReporterTest, integrationTest) {

  /// STEP 1. Long setup of:
  /// - 1 test with 1 testee with 1 mutation point.
  /// - 1 test execution result which includes 1 normal test execution and 1
  /// mutated test execution.
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModuleWithTests = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto mullModuleWithTestees = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_count_letters_bc_path(), llvmContext);

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModuleWithTests));
  modules.push_back(std::move(mullModuleWithTestees));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  std::unique_ptr<MathAddMutator> addMutator = make_unique<MathAddMutator>();
  mutators.emplace_back(std::move(addMutator));
  MutationsFinder mutationsFinder(std::move(mutators), configuration);
  Filter filter;

  SimpleTestFinder testFinder;
  auto tests = testFinder.findTests(program, filter);

  auto &test = tests.front();

  Function *testeeFunction = program.lookupDefinedFunction("count_letters");
  ASSERT_FALSE(testeeFunction->empty());

  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(testeeFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(1U, mutationPoints.size());

  MutationPoint *mutationPoint = mutationPoints.front();

  std::vector<std::string> testIds(
      {test.getUniqueIdentifier(), test.getUniqueIdentifier()});
  std::vector<std::string> mutationPointIds(
      {"", mutationPoint->getUniqueIdentifier()});

  const long long RunningTime_1 = 1;
  const long long RunningTime_2 = 2;

  ExecutionResult testExecutionResult;
  testExecutionResult.status = Passed;
  testExecutionResult.runningTime = RunningTime_1;
  testExecutionResult.stdoutOutput = "testExecutionResult.STDOUT";
  testExecutionResult.stderrOutput = "testExecutionResult.STDERR";

  test.setExecutionResult(testExecutionResult);

  ExecutionResult mutatedTestExecutionResult;
  mutatedTestExecutionResult.status = Failed;
  mutatedTestExecutionResult.runningTime = RunningTime_2;
  mutatedTestExecutionResult.stdoutOutput = "mutatedTestExecutionResult.STDOUT";
  mutatedTestExecutionResult.stderrOutput = "mutatedTestExecutionResult.STDERR";

  auto mutationResult =
      make_unique<MutationResult>(mutatedTestExecutionResult, mutationPoint,
                                  testees.front()->getDistance(), &test);

  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  mutationResults.push_back(std::move(mutationResult));

  MetricsMeasure resultTime;

  Result result(std::move(tests), std::move(mutationResults), mutationPoints);

  /// STEP2. Reporting results to JSON
  MutationTestingElementsReporter reporter;
  Metrics metrics;
  metrics.setDriverRunTime(resultTime);
  reporter.reportResults(result, RawConfig(), metrics);

  /// STEP3. Making assertions.
  std::vector<ExecutionResult> executionResults{testExecutionResult,
                                                mutatedTestExecutionResult};

  std::ifstream t("/tmp/mull.mutation-testing-elements.json");
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  std::string err;
  Json object = Json::parse(str, err);

  std::cout << object.dump() << '\n';
  ASSERT_EQ(object.is_null(), false);
}
