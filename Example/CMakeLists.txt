add_subdirectory("../Extensions/PosixAllocator" "PosixAllocator")

add_executable(ExecutableMallocExample "Source/Main.cpp")
target_link_libraries(ExecutableMallocExample PRIVATE ExecutableMalloc ExecutableMallocPosixAllocator)
target_compile_features(ExecutableMallocExample PRIVATE cxx_std_20)

add_test(NAME TestExecutableMallocExample COMMAND $<TARGET_FILE:ExecutableMallocExample>)
