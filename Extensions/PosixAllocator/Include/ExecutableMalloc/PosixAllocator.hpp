#ifndef EXECUTABLEMALLOC_POSIXALLOCATOR_HPP
#define EXECUTABLEMALLOC_POSIXALLOCATOR_HPP

#include "ExecutableMalloc.hpp"

namespace ExecutableMalloc {

	namespace Posix {
		// These are provided for pure transparency, you will likely never need to call those manually
		int getGranularity();
		std::uintptr_t findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable);
		void deallocateMemory(std::uintptr_t location, std::size_t size);
		void changePermissions(std::uintptr_t location, std::size_t size, bool writable);
	}

	class PosixMemoryBlockAllocator : public MemoryBlockAllocator {
	public:
		PosixMemoryBlockAllocator();
	};

}

#endif
