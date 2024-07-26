include_guard()

project(ExecutableMallocPosix)

add_library(ExecutableMallocPosix INTERFACE)
target_include_directories(ExecutableMallocPosix INTERFACE "${PROJECT_SOURCE_DIR}/Include")
target_compile_features(ExecutableMallocPosix INTERFACE cxx_std_20)

target_link_libraries(ExecutableMallocPosix INTERFACE ExecutableMalloc)
