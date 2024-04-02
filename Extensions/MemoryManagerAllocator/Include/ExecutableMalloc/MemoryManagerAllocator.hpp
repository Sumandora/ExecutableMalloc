#ifndef EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP
#define EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP

#include "MemoryManager/MemoryManager.hpp"
#include "ExecutableMalloc.hpp"

namespace ExecutableMalloc {

	namespace MemoryManager {
		// These are provided for pure transparency, you will likely never need to call those manually
		int getGranularity(const ::MemoryManager::MemoryManager& memoryManager);
		std::uintptr_t findUnusedMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable);
		void deallocateMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size);
		void changePermissions(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size, bool writable);
	}

	class MemoryManagerMemoryBlockAllocator : public MemoryBlockAllocator {
	public:
		MemoryManagerMemoryBlockAllocator(const ::MemoryManager::MemoryManager& memoryManager);
	};

}

#endif
