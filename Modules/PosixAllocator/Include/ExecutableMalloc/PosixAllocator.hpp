#ifndef EXECUTABLEMALLOC_POSIXALLOCATOR_HPP
#define EXECUTABLEMALLOC_POSIXALLOCATOR_HPP

#include "ExecutableMalloc.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sys/mman.h>
#include <unistd.h>

namespace ExecutableMalloc {

	class PosixAllocator : public MemoryBlockAllocator {
		static int getPageSize()
		{
			static const int pagesize = getpagesize(); // Reduce the sys-calls
			return pagesize;
		}

		static constexpr int getFlags(bool writable)
		{
			int prot = PROT_READ | PROT_EXEC;
			if (writable)
				prot |= PROT_WRITE;
			return prot;
		}

	public:
		PosixAllocator()
			: MemoryBlockAllocator(
				  search(getPageSize(), [](std::uintptr_t address, std::size_t length, bool writable) -> std::optional<std::uintptr_t> {
					  void* ptr = mmap(
						  reinterpret_cast<void*>(address),
						  length,
						  getFlags(writable),
						  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
						  -1,
						  0);
					  if (ptr != MAP_FAILED) {
						  return reinterpret_cast<std::uintptr_t>(ptr);
					  }
					  return std::nullopt;
				  }),
				  [](std::uintptr_t location, std::size_t size) {
					  munmap(reinterpret_cast<void*>(location), size);
				  },
				  [](std::uintptr_t location, std::size_t size, bool newWritable) {
					  mprotect(reinterpret_cast<void*>(location), size, getFlags(newWritable));
				  },
				  getPageSize())
		{
		}
	};

}

#endif
