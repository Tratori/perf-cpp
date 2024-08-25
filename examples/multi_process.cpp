#include "access_benchmark.h"
#include "perfcpp/event_counter.h"
#include <atomic>
#include <iostream>
#include <numeric>
#include <thread>
#include <unistd.h>
int
main()
{
  std::cout << "libperf-cpp example: Record performance counter for "
               "random access to an in-memory array per process."
            << std::endl;
  std::cout << "We will record the counters per process and merge the results "
               "afterwards."
            << std::endl;

  /// Initialize performance counters.
  /// Note that the perf::CounterDefinition holds all counter names and must be
  /// alive until the benchmark finishes.
  auto counter_definitions = perf::CounterDefinition{};
  auto event_counter = perf::EventCounter{ counter_definitions };

  /// Add all the performance counters we want to record.
  if (!event_counter.add({ "instructions",
                           "cycles",
                           "branches",
                           "cache-misses",
                           "dTLB-miss-ratio",
                           "L1-data-miss-ratio",
                           "cycles-per-instruction" })) {
    std::cerr << "Could not add performance counters." << std::endl;
  }

  /// Create random access benchmark.
  auto benchmark = perf::example::AccessBenchmark{ /*randomize the accesses*/ true,
                                                   /* create benchmark of 1024 MB */ 1024U };

  /// One event_counter instance for every thread.
  constexpr auto count_threads = 2U;
  const auto items_per_thread = benchmark.size() / count_threads;
  auto threads = std::vector<std::thread>{};
  auto thread_local_results =
    std::vector<std::uint64_t>(count_threads, 0U); /// Array to store the thread-local results.

  /// process ids to record performance counters.
  auto process_ids = std::vector<pid_t>{};
  process_ids.resize(count_threads);

  /// Barrier for the threads to wait.
  auto thread_barrier = std::atomic<bool>{ false };

  /// Barrier for main thread to wait until threads have written their process ids.
  auto written_pid_counter = std::atomic<std::uint16_t>{ 0U };

  for (auto thread_index = 0U; thread_index < count_threads; ++thread_index) {
    threads.emplace_back([thread_index,
                          items_per_thread,
                          &thread_local_results,
                          &benchmark,
                          &process_ids,
                          &thread_barrier,
                          &written_pid_counter]() {
      auto local_value = 0ULL;

      /// Store the process id for creating performance counters on that thread.
      /// To the best of our knowledge, there is no other way to get the (linux) tid/pid of an std::thread :-(.
      process_ids[thread_index] = gettid();

      /// Notify the main thread that the pid/tid was written by this thread.
      written_pid_counter.fetch_add(1U);

      /// Wait for the barrier to become "true".
      while (!thread_barrier)
        ;

      /// Process the data.
      for (auto index = 0U; index < items_per_thread; ++index) {
        local_value += benchmark[(thread_index * items_per_thread) + index].value;
      }

      thread_local_results[thread_index] = local_value;
    });
  }

  /// Wait for all threads to write their pid/tid.
  while (written_pid_counter < count_threads)
    ;

  /// Create process ids to watch.
  std::cout << "Creating counters for Processes: ";
  for (const auto pid : process_ids) {
    std::cout << pid << " ";
  }
  std::cout << std::endl;

  /// Turn the single EventCounter into a multi-core CPU counter for all cores on the machine.
  auto multi_cpu_event_counter = perf::EventCounterMP{ std::move(event_counter), std::move(process_ids) };

  /// Start recording performance counter.
  /// In contrast to the inherit-thread example (see inherit_thread.cpp), we
  /// will record the performance counters on each thread on every core.
  multi_cpu_event_counter.start();

  /// Let threads start.
  thread_barrier = true;

  /// Wait for all threads to finish.
  for (auto& thread : threads) {
    thread.join();
  }

  /// Stop performance counter recording.
  multi_cpu_event_counter.stop();

  /// Add up the results so that the compiler does not get the idea of
  /// optimizing away the accesses.
  auto value = std::accumulate(thread_local_results.begin(), thread_local_results.end(), 0UL);
  asm volatile("" : "+r,m"(value) : : "memory");

  /// Get the result (normalized per cache line) from the
  /// multithread_event_counter.
  auto result = multi_cpu_event_counter.result(benchmark.size());

  /// Print the performance counters.
  std::cout << "\nHere are the results for " << count_threads << " threads:\n" << std::endl;
  for (const auto& [counter_name, counter_value] : result) {
    std::cout << counter_value << " " << counter_name << " per cache line" << std::endl;
  }

  return 0;
}
