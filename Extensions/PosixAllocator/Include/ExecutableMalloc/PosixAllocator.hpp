#ifndef EXECUTABLEMALLOC_POSIXALLOCATOR_HPP
#define EXECUTABLEMALLOC_POSIXALLOCATOR_HPP

#include "ExecutableMalloc.hpp"

namespace ExecutableMalloc {

	// These are provided for pure transparency, you will likely never need to call those manually
	int posixGetGranularity();
	std::uintptr_t posixFindUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable);
	void posixDeallocateMemory(std::uintptr_t location, std::size_t size);

	class PosixMemoryBlockAllocator : public MemoryBlockAllocator {
	public:
		PosixMemoryBlockAllocator();
	};

}

#endif
