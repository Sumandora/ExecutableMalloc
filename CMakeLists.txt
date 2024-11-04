cmake_minimum_required(VERSION 3.20)

include_guard()

project(ExecutableMalloc)
add_library(ExecutableMalloc INTERFACE)
target_include_directories(ExecutableMalloc INTERFACE "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMalloc INTERFACE cxx_std_20)

if(PROJECT_IS_TOP_LEVEL)
	enable_testing()
	add_subdirectory("Example")
endif()
