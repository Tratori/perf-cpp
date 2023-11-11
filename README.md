# *perf-cpp*: Performance Counter Wrapper for C++

*perf-cpp* is a lightweight wrapper for accessing CPU *performance counters* straight from C++.
The library defines a set of hard- and software counters and allows you to manually or via a list (in CSV format) add your own counters.

## How to use
(see more detailed examples below)
### 1) Create a `Perf` object

    #include <perfcpp/perf.h>
    auto counter_definitions = perf::CounterDefinition{};
    auto perf = perf::Perf{counter_definitions};

### 2) Define your counters

    perf.add({"instructions", "cycles", "branches", "branch-misses", "cache-misses", "cache-references"});

### 3) Wrap `start()` and `stop()` around your processing code

    perf.open();
    perf.start();

    /// ... do some computational work here...

    perf.stop();
    perf.close();

### 4) Get the counter values

    const auto cycles = perf.get("cycles");
    std::cout << "Took " << cycles.value() << " cycles" << std::endl;

## How to include *perf-cpp* in your project?
### Manual
* Download the source code
* call `cmake .` and `make` within the downloaded folder
* Copy the `include/` directory and the static library `libperf-cpp.a` to your project
* Include the `include/` folder and link the library: `-lperf-cpp`

### Using `CMake` and `ExternalProject`
* Add `include(ExternalProject)` to your `CMakeLists.txt`
* Define an external project:

```
  ExternalProject_Add(
      perf-cpp-external
      GIT_REPOSITORY "https://github.com/jmuehlig/perf-cpp"
      GIT_TAG "main"
      PREFIX "path/to/your/libs/perf-cpp"
      INSTALL_COMMAND cmake -E echo ""
  )
```
* Add `path/to/your/libs/perf-cpp/src/perf-cpp-external/include` to your `include_directories()`
* Add `perf-cpp` to your linked libraries

## TODOs
* Add sampling
* Automatically read performance counters provided by the machine