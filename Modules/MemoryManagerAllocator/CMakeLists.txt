include_guard()

project(ExecutableMallocMemoryManager)

add_library(ExecutableMallocMemoryManager INTERFACE)
target_include_directories(ExecutableMallocMemoryManager INTERFACE "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMallocMemoryManager INTERFACE cxx_std_20)

if(NOT TARGET MemoryManager)
	include("FetchContent")

	FetchContent_Declare(
		MemoryManager
		GIT_REPOSITORY https://github.com/Sumandora/MemoryManager
		GIT_PROGRESS TRUE
		GIT_TAG 2.2.2)
	FetchContent_MakeAvailable(MemoryManager)
endif()

target_link_libraries(ExecutableMallocMemoryManager INTERFACE ExecutableMalloc MemoryManager)
