cmake_minimum_required(VERSION 3.20)

include_guard()

project(ExecutableMallocCWrapper)

add_library(ExecutableMallocCWrapper STATIC "${PROJECT_SOURCE_DIR}/Source/ExecutableMallocWrapper.cpp")
target_include_directories(ExecutableMallocCWrapper PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_compile_options(ExecutableMallocCWrapper PRIVATE "-Ofast" "-Wno-conversion-null")
target_compile_features(ExecutableMallocCWrapper PRIVATE cxx_std_20)

target_link_libraries(ExecutableMallocCWrapper PUBLIC ExecutableMalloc)
