add_subdirectory("../Modules/PosixAllocator" "PosixAllocator")

# It's not used, but we want to compile it to check for errors
add_subdirectory("../Modules/MemoryManagerAllocator" "MemoryManagerAllocator")

add_executable(ExecutableMallocExample "Source/Main.cpp")
target_link_libraries(ExecutableMallocExample PRIVATE ExecutableMalloc ExecutableMallocPosix
													  ExecutableMallocMemoryManager)
target_compile_features(ExecutableMallocExample PRIVATE cxx_std_20)

add_test(NAME TestExecutableMallocExample COMMAND $<TARGET_FILE:ExecutableMallocExample>)
