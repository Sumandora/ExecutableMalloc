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
				  search(memoryManager.getPageGranularity(),
					  [&memoryManager](std::uintptr_t address, std::size_t length, bool writable) -> std::optional<std::uintptr_t> {
						  return memoryManager.allocateAt(address, length, { true, writable, true });
					  }),
				  [&memoryManager](std::uintptr_t location, std::size_t size) {
					  memoryManager.deallocate(location, size);
				  },
				  [&memoryManager](std::uintptr_t location, std::size_t size, bool newWritable) {
					  memoryManager.protect(location, size, { true, newWritable, true });
				  },
				  memoryManager.getPageGranularity())
			, memoryManager(&memoryManager)
		{
		}

		[[nodiscard]] const MemMgr* getMemoryManager() const { return memoryManager; }
	};

}

#endif
