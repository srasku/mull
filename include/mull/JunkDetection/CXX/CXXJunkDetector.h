#pragma once

#include "ASTStorage.h"
#include "mull/MutationPoint.h"
#include "mull/JunkDetection/JunkDetector.h"

namespace mull {

class MutationPoint;
struct JunkDetectionConfig;

class CXXJunkDetector : public JunkDetector {
public:
  explicit CXXJunkDetector(JunkDetectionConfig &config);
  ~CXXJunkDetector() override = default;

  bool isJunk(MutationPoint *point) override;

  const std::map<MutationPoint *, BeginEndPair> &
  getMutationPointsRanges() const;

private:
  ASTStorage astStorage;
  std::map<MutationPoint *, BeginEndPair> mutationPointsRanges;
};

} // namespace mull
