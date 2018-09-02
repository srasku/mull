#pragma once

#include <functional>
#include "ExecutionResult.h"

namespace mull {

  typedef struct {
    mull::ExecutionStatus executionStatus;
    char payload[1024 * 1024];
  } SharedData;


class ProcessSandbox {
public:
  virtual ~ProcessSandbox() {}
  virtual ExecutionResult run(std::function<ExecutionStatus ()> function,
                              long long timeoutMilliseconds) = 0;
  virtual SharedData runShared(std::function<void (SharedData *)> function,
                               long long timeoutMilliseconds) = 0;

};

class ForkProcessSandbox : public ProcessSandbox {
public:
  const static int MullExitCode = 227;
  const static int MullTimeoutCode = 239;

  ExecutionResult run(std::function<ExecutionStatus ()> function,
                      long long timeoutMilliseconds);
  SharedData runShared(std::function<void (SharedData *)> function,
                       long long timeoutMilliseconds);

};

class NullProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<ExecutionStatus ()> function,
                      long long timeoutMilliseconds);
  SharedData runShared(std::function<void (SharedData *)> function,
                       long long timeoutMilliseconds);

};

}
