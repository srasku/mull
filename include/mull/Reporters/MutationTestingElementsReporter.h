#pragma once

#include "Reporter.h"

#include "mull/MutationPoint.h"

#include <clang/Basic/SourceLocation.h>

#include <map>

namespace mull {
class MutationPoint;
class MutationTestingElementsReporter : public Reporter {
  const std::map<MutationPoint *, BeginEndPair> &mutationPointsRanges;
public:
  MutationTestingElementsReporter(
    const std::map<MutationPoint *, BeginEndPair> &mutationPointsRanges);
  void reportResults(const Result &result, const RawConfig &config,
                     const Metrics &metrics) override;
};

} // namespace mull
