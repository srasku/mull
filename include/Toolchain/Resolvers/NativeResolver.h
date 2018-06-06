#pragma once

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>

namespace mull {
class NativeResolver : public llvm::JITSymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
public:
  NativeResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides);
  llvm::JITSymbol findSymbol(const std::string &name) override;
  llvm::JITSymbol findSymbolInLogicalDylib(const std::string &name) override;
};
}
