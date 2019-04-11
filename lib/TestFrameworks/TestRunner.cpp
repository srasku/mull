#include "mull/TestFrameworks/TestRunner.h"

#include <llvm/Support/DynamicLibrary.h>

mull::TestRunner::TestRunner() : shouldSkipCtors(false) {
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}
