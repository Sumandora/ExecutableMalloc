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
	public:
		PosixAllocator();

		std::uintptr_t find_unused_memory(std::uintptr_t preferred_location, std::size_t tolerance, std::size_t num_pages, bool writable) override;
		void deallocate_memory(std::uintptr_t location, std::size_t size) override;
		void change_protection(std::uintptr_t location, std::size_t size, bool new_writable) override;
	};

}

#endif
