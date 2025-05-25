#include "ExecutableMalloc/PosixAllocator.hpp"

#include "ExecutableMalloc.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sys/mman.h>
#include <unistd.h>

using namespace ExecutableMalloc;

static int get_page_size()
{
	static const int CACHED_PAGE_SIZE = getpagesize(); // Reduce the system calls
	return CACHED_PAGE_SIZE;
}

static constexpr int get_flags(bool writable)
{
	int prot = PROT_READ | PROT_EXEC;
	if (writable)
		prot |= PROT_WRITE;
	return prot;
}

PosixAllocator::PosixAllocator()
	: MemoryBlockAllocator(get_page_size())
{
}

std::uintptr_t PosixAllocator::find_unused_memory(std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable)
{
	return search(get_page_size(),
		[](std::uintptr_t address, std::size_t length, bool writable) -> std::optional<std::uintptr_t> {
			void* ptr = mmap(
				reinterpret_cast<void*>(address),
				length,
				get_flags(writable),
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
				-1,
				0);
			if (ptr != MAP_FAILED) {
				return reinterpret_cast<std::uintptr_t>(ptr);
			}
			return std::nullopt;
		})(preferred_location, tolerance, num_pages, writable);
}

void PosixAllocator::deallocate_memory(std::uintptr_t location, std::size_t size)
{
	munmap(reinterpret_cast<void*>(location), size);
}

void PosixAllocator::change_protection(std::uintptr_t location, std::size_t size, bool new_writable)
{
	mprotect(reinterpret_cast<void*>(location), size, get_flags(new_writable));
}
