#include "mull/JunkDetection/CXX/CXXJunkDetector.h"

#include "mull/Config/RawConfig.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/Mutator.h"

#include "mull/JunkDetection/CXX/Visitors/AndOrReplacementVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathAddVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathDivVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathMulVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/MathSubVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/NegateConditionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"
#include "mull/JunkDetection/CXX/Visitors/ReplaceCallVisitor.h"

using namespace mull;

template <typename Visitor>
static bool
isJunkMutation(ASTStorage &storage, MutationPoint *point,
               std::map<MutationPoint *, BeginEndPair> &mutationPointsRanges) {
  auto ast = storage.findAST(point);
  auto location = ast->getLocation(point);
  auto &sourceManager = ast->getSourceManager();

  if (ast->isInSystemHeader(location)) {
    return true;
  }

  VisitorParameters parameters = {.sourceManager = sourceManager,
    .sourceLocation = location,
    .astContext = ast->getASTContext()};
  Visitor visitor(parameters);
  visitor.TraverseDecl(ast->getASTContext().getTranslationUnitDecl());

  if (!visitor.foundMutant()) {
    return true;
  } else {

    clang::SourceRange sourceRange = visitor.getSourceRange();
    clang::SourceLocation sourceLocationBegin = sourceRange.getBegin();
    clang::SourceLocation sourceLocationEnd = sourceRange.getEnd();

    unsigned int beginCol =
      sourceManager.getExpansionColumnNumber(sourceLocationBegin);
    unsigned int endCol =
      sourceManager.getExpansionColumnNumber(sourceLocationEnd);

    mutationPointsRanges[point] = std::make_pair(beginCol, endCol);

    return false;
  }
}

CXXJunkDetector::CXXJunkDetector(JunkDetectionConfig &config)
  : astStorage(config.cxxCompilationDatabasePath,
               config.cxxCompilationFlags) {}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  if (point->getSourceLocation().isNull()) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
  case MutatorKind::ConditionalsBoundaryMutator:
    return isJunkMutation<ConditionalsBoundaryVisitor>(astStorage, point,
                                                       mutationPointsRanges);
  case MutatorKind::MathAddMutator:
    return isJunkMutation<MathAddVisitor>(astStorage, point,
                                          mutationPointsRanges);
  case MutatorKind::MathSubMutator:
    return isJunkMutation<MathSubVisitor>(astStorage, point,
                                          mutationPointsRanges);
  case MutatorKind::MathMulMutator:
    return isJunkMutation<MathMulVisitor>(astStorage, point,
                                          mutationPointsRanges);
  case MutatorKind::MathDivMutator:
    return isJunkMutation<MathDivVisitor>(astStorage, point,
                                          mutationPointsRanges);
  case MutatorKind::RemoveVoidFunctionMutator:
    return isJunkMutation<RemoveVoidFunctionVisitor>(astStorage, point,
                                                     mutationPointsRanges);
  case MutatorKind::ReplaceCallMutator:
    return isJunkMutation<ReplaceCallVisitor>(astStorage, point,
                                              mutationPointsRanges);
  case MutatorKind::NegateMutator:
    return isJunkMutation<NegateConditionVisitor>(astStorage, point,
                                                  mutationPointsRanges);
  case MutatorKind::AndOrReplacementMutator:
    return isJunkMutation<AndOrReplacementVisitor>(astStorage, point,
                                                   mutationPointsRanges);
  default:
    return false;
  }
}

const std::map<MutationPoint *, BeginEndPair> &
CXXJunkDetector::getMutationPointsRanges() const {
  return mutationPointsRanges;
}
