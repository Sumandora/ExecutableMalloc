# Executable Malloc

*Are you making a hooking library? Do you need some executable memory region? Are you too lazy to write a memory
allocator? Then look no further!*

## Usage

Add the following to your CMakeLists.txt:

```cmake
FetchContent_Declare(ExecutableMalloc
        GIT_REPOSITORY https://github.com/Sumandora/ExecutableMalloc
        GIT_PROGRESS TRUE
        GIT_TAG [INSERT_GIT_TAG])
FetchContent_MakeAvailable(ExecutableMalloc)
target_link_libraries(${CMAKE_PROJECT_NAME} ExecutableMalloc)
```

After that create your own MemoryBlockAllocator like that:

```c++
#include "ExecutableMalloc.hpp"

class MyAllocator : public MemoryBlockAllocator {
public:
	MyAllocator()
		: MemoryBlockAllocator(PAGE_SIZE)
	{
	}

	std::uintptr_t findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) override
	{
		// TODO Allocate numPages memory pages that lies at preferredLocation +/- tolerance
	}

	void deallocateMemory(std::uintptr_t location, std::size_t size) override
	{
		// TODO Deallocate the memory pages
	}

	void changeProtection(std::uintptr_t location, std::size_t size, bool newWritable) override
	{
		// TODO Change protection to writable
	}
};
```

You can take a look at the PosixAllocator extension for some guidance to implement your own allocator.

### Posix

If you are on a system that is posix-compliant, then you can also use the PosixAllocator extension.

Add the following to your CMakeLists.txt:

```cmake
add_subdirectory("${ExecutableMalloc_SOURCE_DIR}/Modules/PosixAllocator")
target_link_libraries(${CMAKE_PROJECT_NAME} ExecutableMallocPosix)
```

Then include & instantiate like this:

```c++
#include "ExecutableMalloc/PosixAllocator.hpp"
ExecutableMalloc::PosixAllocator allocator;
```

## Usage

You can now use the `getRegion` member function to receive your allocated memory block:

```c++
// 'tolerance' and 'writable' are optional parameters
auto region = allocator.getRegion(location, size, /*writable =*/ true, /*tolerance =*/ INT32_MAX);
```

The region will be deallocated when `region` goes out of scope.

**For more examples you can check the `Examples`-subdirectory**
