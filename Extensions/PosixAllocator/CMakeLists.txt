include_guard()

project(ExecutableMallocPosixAllocator)
add_library(ExecutableMallocPosixAllocator STATIC
        "${PROJECT_SOURCE_DIR}/Source/PosixAllocator.cpp"
)
target_include_directories(ExecutableMallocPosixAllocator PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMallocPosixAllocator PRIVATE cxx_std_20)

target_link_libraries(ExecutableMallocPosixAllocator ExecutableMalloc)
