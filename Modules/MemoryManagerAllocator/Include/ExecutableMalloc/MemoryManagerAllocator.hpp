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
		const MemMgr* memory_manager;

	public:
		explicit MemoryManagerAllocator(const MemMgr& memory_manager)
			: MemoryBlockAllocator(memory_manager.getPageGranularity())
			, memory_manager(&memory_manager)
		{
		}

		[[nodiscard]] const MemMgr* get_memory_manager() const { return memory_manager; }

		std::uintptr_t find_unused_memory(std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable) override
		{
			return search(memory_manager->getPageGranularity(),
				[this](std::uintptr_t address, std::size_t length, bool writable) {
					return memory_manager->allocateAt(address, length, { true, writable, true });
				})(preferred_location, tolerance, num_pages, writable);
		}

		void deallocate_memory(std::uintptr_t location, std::size_t size) override
		{
			memory_manager->deallocate(location, size);
		}

		void change_protection(std::uintptr_t location, std::size_t size, bool new_writable) override
		{
			memory_manager->protect(location, size, { true, new_writable, true });
		}
	};

}

#endif
