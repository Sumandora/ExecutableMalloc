#ifndef EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP
#define EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP

#include "ExecutableMalloc.hpp"
#include "MemoryManager/MemoryManager.hpp"

#include <cstddef>
#include <cstdint>

namespace ExecutableMalloc {

	template <typename MemMgr>
		requires MemoryManager::PositionedAllocator<MemMgr> && MemoryManager::Deallocator<MemMgr> && MemoryManager::Protector<MemMgr>
	class MemoryManagerAllocator : public MemoryBlockAllocator {
		const MemMgr* memoryManager;

	public:
		explicit MemoryManagerAllocator(const MemMgr& memoryManager)
			: MemoryBlockAllocator(memoryManager.getPageGranularity())
			, memoryManager(&memoryManager)
		{
		}

		[[nodiscard]] const MemMgr* getMemoryManager() const { return memoryManager; }

		std::uintptr_t findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) override
		{
			return search(memoryManager->getPageGranularity(),
				[this](std::uintptr_t address, std::size_t length, bool writable) {
					return memoryManager->allocateAt(address, length, { true, writable, true });
				})(preferredLocation, tolerance, numPages, writable);
		}

		void deallocateMemory(std::uintptr_t location, std::size_t size) override
		{
			memoryManager->deallocate(location, size);
		}

		void changeProtection(std::uintptr_t location, std::size_t size, bool newWritable) override
		{
			memoryManager->protect(location, size, { true, newWritable, true });
		}
	};

}

#endif
