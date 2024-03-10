add_executable(ExampleCXX "CXX/Main.cpp")
target_link_libraries(ExampleCXX PRIVATE ExecutableMalloc)
target_compile_features(ExampleCXX PRIVATE cxx_std_20)
