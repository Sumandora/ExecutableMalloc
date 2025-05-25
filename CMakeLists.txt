cmake_minimum_required(VERSION 3.20)

include_guard()

project(ExecutableMalloc)
add_library(ExecutableMalloc STATIC
	"Source/MemoryRegion.cpp"
	"Source/MemoryMapping.cpp"
	"Source/MemoryBlockAllocator.cpp")
target_include_directories(ExecutableMalloc PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMalloc PUBLIC cxx_std_20)

if(PROJECT_IS_TOP_LEVEL)
	enable_testing()
	add_subdirectory("Example")
endif()
