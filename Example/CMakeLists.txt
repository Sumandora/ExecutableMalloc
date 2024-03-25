add_subdirectory("../Extensions/PosixAllocator" "PosixAllocator")

add_executable(ExampleCXX "CXX/Main.cpp")
target_link_libraries(ExampleCXX PRIVATE ExecutableMalloc ExecutableMallocPosixAllocator)
target_compile_features(ExampleCXX PRIVATE cxx_std_20)

# Include C Support
add_subdirectory("../CWrapper" "CWrapper")
add_subdirectory("../Extensions/PosixAllocator/CWrapper" "PosixAllocator/CWrapper")

add_executable(ExampleCHeap "C/Heap.c")
target_link_libraries(ExampleCHeap PRIVATE ExecutableMallocCWrapper ExecutableMallocPosixAllocatorCWrapper)
target_compile_options(ExampleCHeap PRIVATE "-Wno-int-conversion" "-Wno-pointer-integer-compare")

add_executable(ExampleCStack "C/Stack.c")
target_link_libraries(ExampleCStack PRIVATE ExecutableMallocCWrapper ExecutableMallocPosixAllocatorCWrapper)
target_compile_options(ExampleCStack PRIVATE "-Wno-int-conversion" "-Wno-pointer-integer-compare")

add_test(NAME TestExampleCXX COMMAND $<TARGET_FILE:ExampleCXX>)
add_test(NAME TestExampleCHeap COMMAND $<TARGET_FILE:ExampleCHeap>)
add_test(NAME TestExampleCStack COMMAND $<TARGET_FILE:ExampleCStack>)