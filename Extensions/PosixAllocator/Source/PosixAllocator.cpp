#include "ExecutableMalloc/PosixAllocator.hpp"

#include <sys/mman.h>

using namespace ExecutableMalloc;

int Posix::getGranularity()
{
	static const int pagesize = getpagesize(); // Reduce the sys-calls
	return pagesize;
}

std::uintptr_t Posix::findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable)
{
	int pageSize = getGranularity();
	// "If another mapping already exists there, the kernel picks a new address that may or may not depend on the hint" - manpage
	// Some kernels may respect the hint but to be sure that this works, lets do it manually
	int prot = PROT_READ | PROT_EXEC;
	if (writable)
		prot |= PROT_WRITE;
	for (std::size_t offset = 0; offset < tolerance; offset += pageSize)
		for (bool positive : { false, true }) {
			std::uintptr_t address = preferredLocation - preferredLocation % pageSize;
			if (positive)
				address += offset;
			else
				address -= offset;
			void* pointer = mmap(
				reinterpret_cast<void*>(address),
				pageSize * numPages,
				prot,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
				-1,
				0);
			if (pointer != MAP_FAILED)
				return reinterpret_cast<std::uintptr_t>(pointer);
		}
	throw std::bad_alloc{};
}

void Posix::deallocateMemory(std::uintptr_t location, std::size_t size)
{
	munmap(reinterpret_cast<void*>(location), size);
}

void Posix::changePermissions(std::uintptr_t location, std::size_t size, bool writable)
{
	int prot = PROT_READ | PROT_EXEC;
	if (writable)
		prot |= PROT_WRITE;

	mprotect(reinterpret_cast<void*>(location), size, prot);
}

PosixMemoryBlockAllocator::PosixMemoryBlockAllocator()
	: MemoryBlockAllocator(
		  Posix::findUnusedMemory,
		  Posix::deallocateMemory,
		  [](MemoryMapping& mapping, bool newWritable) {
			  Posix::changePermissions(mapping.getFrom(), mapping.getTo() - mapping.getFrom(), newWritable);
		  },
		  Posix::getGranularity())
{
}
