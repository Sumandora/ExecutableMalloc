#include "ExecutableMalloc/MemoryManagerAllocator.hpp"

#include <sys/mman.h>

using namespace ExecutableMalloc;

std::uintptr_t ExecutableMalloc::MemoryManager::findUnusedMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable)
{
	std::size_t pageSize = memoryManager.getPageGranularity();
	// "If another mapping already exists there, the kernel picks a new address that may or may not depend on the hint" - manpage
	// Some kernels may respect the hint but to be sure that this works, lets do it manually
	for (std::size_t offset = 0; offset < tolerance; offset += pageSize)
		for (bool positive : {false, true}) {
			std::uintptr_t address = preferredLocation - preferredLocation % pageSize;
			if(positive)
				address += offset;
			else
				address -= offset;
			try {
				std::uintptr_t pointer = memoryManager.allocate(address, pageSize * numPages, { true, writable, true });
				return pointer;
			} catch (std::runtime_error& err) {
				// Allocation failed;
				continue;
			}
		}
	throw std::bad_alloc{};
}

void ExecutableMalloc::MemoryManager::deallocateMemory(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size)
{
	memoryManager.deallocate(location, size);
}

void ExecutableMalloc::MemoryManager::changePermissions(const ::MemoryManager::MemoryManager& memoryManager, std::uintptr_t location, std::size_t size, bool writable) {
	memoryManager.protect(location, size, { true, writable, true });
}

MemoryManagerMemoryBlockAllocator::MemoryManagerMemoryBlockAllocator(const ::MemoryManager::MemoryManager& memoryManager)
	: MemoryBlockAllocator(
		  [&memoryManager](std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
			  return MemoryManager::findUnusedMemory(memoryManager, preferredLocation, tolerance, numPages, writable);
		  },
		  [&memoryManager](std::uintptr_t location, std::size_t size) {
			  MemoryManager::deallocateMemory(memoryManager, location, size);
		  },
		  [&memoryManager](MemoryMapping& mapping, bool newWritable) {
			  MemoryManager::changePermissions(memoryManager, mapping.getFrom(), mapping.getTo() - mapping.getFrom(), newWritable);
		  },
		  memoryManager.getPageGranularity())
{
}
