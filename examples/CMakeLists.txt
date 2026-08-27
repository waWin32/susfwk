# CMakeLists - kernel/examples
#
message(STATUS "Building examples")

sus_define_modules(examples)
sus_add_all_examples_modules()
sus_add_all_framework_modules()
