add_subdirectory("../Modules/PosixAllocator" "PosixAllocator")

# It's not used, but compile it anyways to check for errors
add_subdirectory("../Modules/MemoryManagerAllocator" "MemoryManagerAllocator")
add_subdirectory("${MemoryManager_SOURCE_DIR}/Modules/Linux" "LinuxMemoryManager")

add_executable(ExecutableMallocExample "Source/Main.cpp")
target_link_libraries(ExecutableMallocExample PRIVATE ExecutableMalloc ExecutableMallocPosix
													  ExecutableMallocMemoryManager LinuxMemoryManager)
target_compile_features(ExecutableMallocExample PRIVATE cxx_std_20)

add_test(NAME TestExecutableMallocExample COMMAND $<TARGET_FILE:ExecutableMallocExample>)
