add_subdirectory("../Extensions/PosixAllocator" "PosixAllocator")

add_executable(ExampleCXX "Source/Main.cpp")
target_link_libraries(ExampleCXX PRIVATE ExecutableMalloc ExecutableMallocPosixAllocator)
target_compile_features(ExampleCXX PRIVATE cxx_std_20)

# Include C Support
add_subdirectory("../CWrapper" "CWrapper")
add_subdirectory("../Extensions/PosixAllocator/CWrapper" "PosixAllocator/CWrapper")

add_executable(ExampleC "Source/Main.c")
target_link_libraries(ExampleC PRIVATE ExecutableMallocCWrapper ExecutableMallocPosixAllocatorCWrapper)
target_compile_options(ExampleC PRIVATE "-Wno-int-conversion")

add_test(NAME TestExampleCXX COMMAND $<TARGET_FILE:ExampleCXX>)
add_test(NAME TestExampleC COMMAND $<TARGET_FILE:ExampleC>)
