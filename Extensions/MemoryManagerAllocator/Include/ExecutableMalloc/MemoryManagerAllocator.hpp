#ifndef EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP
#define EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP

#include "ExecutableMalloc.hpp"
#include "MemoryManager/MemoryManager.hpp"

namespace ExecutableMalloc {

	namespace MemoryManager {
		// These are provided for pure transparency, you will likely never need to call those manually
		int getGranularity(const ::MemoryManager::MemoryManager& memoryManager);
		std::uintptr_t findUnusedMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable);
		void deallocateMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size);
		void changePermissions(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size, bool writable);
	}

	class MemoryManagerMemoryBlockAllocator : public MemoryBlockAllocator {
		const ::MemoryManager::MemoryManager& memoryManager;

	public:
		explicit MemoryManagerMemoryBlockAllocator(const ::MemoryManager::MemoryManager& memoryManager);

		[[nodiscard]] const ::MemoryManager::MemoryManager& getMemoryManager() const { return memoryManager; }
	};

}

#endif
