# Executable Malloc

*Are you making a hooking library? Do you need some executable memory region? Are you too lazy to write a memory
allocator? Then look no further!*

## Usage

### CMake

Add the following to your CMakeLists.txt.

```cmake
FetchContent_Declare(ExecutableMalloc
        GIT_REPOSITORY https://github.com/Sumandora/ExecutableMalloc
        GIT_PROGRESS TRUE
        GIT_TAG [INSERT_GIT_TAG])
FetchContent_MakeAvailable(ExecutableMalloc)
target_link_libraries(${CMAKE_PROJECT_NAME} ExecutableMalloc)
```

If you are on a system that is posix-compliant, then you can also use the PosixAllocator extension.

```cmake
add_subdirectory("${ExecutableMalloc_SOURCE_DIR}/Extensions/PosixAllocator")
target_link_libraries(${CMAKE_PROJECT_NAME} ExecutableMallocPosixAllocator)
```

If your system is not posix-compliant, then you may take a look at the PosixAllocator extension for some guidance to
implement your own allocator.

### C++

Create your own MemoryBlockAllocator:

```c++
#include "ExecutableMalloc.hpp"
MemoryBlockAllocator allocator{
	[](std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
		// TODO Allocate numPages memory pages that lies at preferredLocation +/- tolerance
	},
	[](std::uintptr_t location, std::size_t size) {
		// TODO Deallocate the memory pages
	},
	PAGE_SIZE
};
```

or use the PosixMemoryBlockAllocator if you are using the posix extension:

```c++
#include "ExecutableMalloc/PosixAllocator.hpp"
PosixMemoryBlockAllocator allocator;
```

You can now use the `getRegion` member function to receive your allocated memory block:

```c++
// 'tolerance' and 'writable' are optional parameters
auto region = allocator.getRegion(location, size, /*tolerance =*/ INT32_MAX, /*writable =*/ true);
```

The region will be deallocated when `region` goes out of scope.

#### For more examples you can check the `Examples`-subdirectory