include_guard()

project(ExecutableMallocPosix)

add_library(ExecutableMallocPosix STATIC "Source/PosixAllocator.cpp")
target_include_directories(ExecutableMallocPosix PUBLIC "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMallocPosix PUBLIC cxx_std_20)

target_link_libraries(ExecutableMallocPosix PUBLIC ExecutableMalloc)
