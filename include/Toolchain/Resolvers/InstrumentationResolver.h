#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <string>

namespace llvm {
namespace orc {
  class LocalCXXRuntimeOverrides;
}
}

namespace mull {

class Mangler;
class Instrumentation;
struct InstrumentationInfo;

class InstrumentationResolver : public llvm::JITSymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
  Instrumentation &instrumentation;
  std::string instrumentationInfoName;
  std::string functionOffsetPrefix;
  InstrumentationInfo **trampoline;
public:
  InstrumentationResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides,
                          Instrumentation &instrumentation,
                          mull::Mangler &mangler,
                          InstrumentationInfo **trampoline);

  llvm::JITSymbol findSymbol(const std::string &name) override;
  llvm::JITSymbol findSymbolInLogicalDylib(const std::string &name) override;
};
}
