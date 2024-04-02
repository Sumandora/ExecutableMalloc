include_guard()

project(ExecutableMallocMemoryManagerAllocator)
add_library(ExecutableMallocMemoryManagerAllocator STATIC
        "${PROJECT_SOURCE_DIR}/Source/MemoryManagerAllocator.cpp"
)
target_include_directories(ExecutableMallocMemoryManagerAllocator PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMallocMemoryManagerAllocator PRIVATE cxx_std_20)

include("FetchContent")

FetchContent_Declare(MemoryManager
        GIT_REPOSITORY https://github.com/Sumandora/MemoryManager
        GIT_PROGRESS TRUE
        GIT_TAG 3afceba)
FetchContent_MakeAvailable(MemoryManager)

target_link_libraries(ExecutableMallocMemoryManagerAllocator ExecutableMalloc MemoryManager)
