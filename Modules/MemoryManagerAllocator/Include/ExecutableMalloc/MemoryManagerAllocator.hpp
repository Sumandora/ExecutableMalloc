#ifndef EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP
#define EXECUTABLEMALLOC_MEMORYMANAGERALLOCATOR_HPP

#include "ExecutableMalloc.hpp"
#include "MemoryManager/MemoryManager.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ExecutableMalloc {

	template <typename MemMgr>
		requires MemoryManager::PositionedAllocator<MemMgr> && MemoryManager::Deallocator<MemMgr> && MemoryManager::Protector<MemMgr>
	class MemoryManagerAllocator : public MemoryBlockAllocator {
		const MemMgr* memoryManager;

	public:
		explicit MemoryManagerAllocator(const MemMgr& memoryManager)
			: MemoryBlockAllocator(
				  +[](void* self, std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
					  auto* memMgrAllocator = static_cast<MemoryManagerAllocator<MemMgr>*>(self);
					  const auto* memoryManager = memMgrAllocator->memoryManager;
					  return search(memoryManager->getPageGranularity(),
						  [memoryManager](std::uintptr_t address, std::size_t length, bool writable) -> std::optional<std::uintptr_t> {
							  return memoryManager->allocateAt(address, length, { true, writable, true });
						  })(preferredLocation, tolerance, numPages, writable);
				  },
				  +[](void* self, std::uintptr_t location, std::size_t size) {
					  auto* memMgrAllocator = static_cast<MemoryManagerAllocator<MemMgr>*>(self);
					  const auto* memoryManager = memMgrAllocator->memoryManager;
					  memoryManager->deallocate(location, size);
				  },
				  +[](void* self, std::uintptr_t location, std::size_t size, bool newWritable) {
					  auto* memMgrAllocator = static_cast<MemoryManagerAllocator<MemMgr>*>(self);
					  const auto* memoryManager = memMgrAllocator->memoryManager;
					  memoryManager->protect(location, size, { true, newWritable, true });
				  },
				  memoryManager.getPageGranularity())
			, memoryManager(&memoryManager)
		{
		}

		[[nodiscard]] const MemMgr* getMemoryManager() const { return memoryManager; }
	};

}

#endif
