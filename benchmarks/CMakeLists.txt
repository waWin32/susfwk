# CMakeLists - kernel/benchmarks
#
message(STATUS "Building benchmarks")

set(CMAKE_EXECUTABLE_SUFFIX "_bm")

sus_define_modules(benchmarks)
sus_add_all_benchmarks_modules()
sus_add_all_framework_modules()
