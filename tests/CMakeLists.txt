# CMakeLists - kernel/tests
#
message(STATUS "Building tests")

set(CMAKE_EXECUTABLE_SUFFIX "_test")

sus_define_modules(tests)
sus_add_all_tests_modules()
sus_add_all_framework_modules()
