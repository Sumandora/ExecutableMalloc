#include "ExecutableMalloc.hpp"

#include <cstdint>
#include <iostream>
#include <sys/mman.h>
#include <cassert>

using namespace ExecutableMalloc;

static const std::size_t pageSize = getpagesize();

constexpr static std::uintptr_t align(std::uintptr_t ptr, std::size_t alignment)
{
	return ptr - ptr % alignment;
}

MemoryBlockAllocator allocator{
	[](std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
		// "If another mapping already exists there, the kernel picks a new address that may or may not depend on the hint" - mmap manual
		// Some kernels may respect the hint but to be sure that this works, lets do it manually
		for (std::size_t offset = 0; offset < tolerance; offset += pageSize)
			for (int sign = -1; sign <= 2; sign += 2) {
				void* pointer = mmap(
					reinterpret_cast<char*>(align(preferredLocation, pageSize)) + offset * sign,
					pageSize * numPages,
					PROT_READ | PROT_WRITE | PROT_EXEC,
					MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
					-1,
					0);
				if (pointer != MAP_FAILED)
					return reinterpret_cast<std::uintptr_t>(pointer);
			}
		throw std::bad_alloc{};
	},
	[](std::uintptr_t location, std::size_t size) {
		munmap(reinterpret_cast<void*>(location), size);
	},
	pageSize
};

void printMemory()
{
	std::size_t i = 0;
	std::cout << "- Memory dump start" << std::endl;
	for (const std::unique_ptr<MemoryMapping>& block : allocator.getMappings()) {
		std::cout << "------- Mapping #" << i << std::endl;
		std::cout << "From: " << block->getFrom() << std::endl;
		std::cout << "To: " << block->getTo() << std::endl;
		std::cout << "Writable: " << block->isWritable() << std::endl;
		std::cout << "-- Regions dump start" << std::endl;
		std::size_t j = 0;
		for (const MemoryRegion* region : block->getUsedRegions()) {
			std::cout << "------- Region #" << j << std::endl;
			std::cout << "From: " << region->getFrom() << std::endl;
			std::cout << "To: " << region->getTo() << std::endl;
			j++;
		}
		std::cout << "-- Regions dump end" << std::endl;
		i++;
	}
	std::cout << "- Memory dump end" << std::endl;
}

void assertMemory(std::size_t mappings, std::initializer_list<std::size_t> regions) {
	assert(allocator.getMappings().size() == mappings);
	auto regsize = regions.begin();
	for(std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator.getMappings()[i]->getUsedRegions().size() == *regsize);
		regsize++;
	}
}

int main();

void test()
{
	const auto pageSize_d = static_cast<double>(pageSize);
	printMemory();
	auto reg1 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << reg1 << std::endl;
	assertMemory(1, { 1 });
	printMemory();
	auto reg2 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << reg2 << std::endl;
	assertMemory(1, { 2 });
	printMemory();
	auto reg3 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << reg3 << std::endl;
	assertMemory(2, { 2, 1 });
	printMemory();
	auto reg4 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << reg4 << std::endl;
	assertMemory(2, { 2, 2 });
	printMemory();
	auto reg5 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << reg5 << std::endl;
	assertMemory(3, { 2, 2, 1 });
	printMemory();
}

int main()
{
	assertMemory(0, {});
	test();
	printMemory();
	assertMemory(0, {});
	return 0;
}
