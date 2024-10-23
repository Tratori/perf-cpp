#pragma once

#include <cstdint>
#include <linux/perf_event.h>
#include "feature.h"

namespace perf {
enum BranchType : std::uint64_t
{
  None = 0ULL,

  User = PERF_SAMPLE_BRANCH_USER,
  Kernel = PERF_SAMPLE_BRANCH_KERNEL,
  HyperVisor = PERF_SAMPLE_BRANCH_HV,

  Any = PERF_SAMPLE_BRANCH_ANY,
#ifndef PERFCPP_NO_SAMPLE_BRANCH_CALL
  Call = PERF_SAMPLE_BRANCH_ANY_CALL,
#else
  Call = 1U << 62,
#endif
  DirectCall = PERF_SAMPLE_BRANCH_CALL,
  IndirectCall = PERF_SAMPLE_BRANCH_IND_CALL,
  Return = PERF_SAMPLE_BRANCH_ANY_RETURN,
#ifndef PERFCPP_NO_SAMPLE_BRANCH_IND_JUMP
  IndirectJump = PERF_SAMPLE_BRANCH_IND_JUMP,
#else
  IndirectJump = 1U << 63,
#endif
  Conditional = PERF_SAMPLE_BRANCH_COND,
  TransactionalMemoryAbort = PERF_SAMPLE_BRANCH_ABORT_TX,
  InTransaction = PERF_SAMPLE_BRANCH_IN_TX,
  NotInTransaction = PERF_SAMPLE_BRANCH_NO_TX
};
}