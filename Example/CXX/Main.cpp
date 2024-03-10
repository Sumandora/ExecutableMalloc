#include "ExecutableMalloc.hpp"

#include <iostream>
#include <cstdint>

using namespace ExecutableMalloc;

MemoryBlockAllocator allocator;

void printMemory()
{
	std::size_t i = 0;
	std::cout << "- Memory dump start" << std::endl;
	for (const std::unique_ptr<MemoryMapping>& block : allocator.getMappings()) {
		std::cout << "------- Block #" << i << std::endl;
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

int main();

void test()
{
	const auto pageSize = getpagesize();
	printMemory();
	auto reg1 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize * 1.5));
	std::cout << reg1 << std::endl;
	printMemory();
	auto reg2 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize * 0.33));
	std::cout << reg2 << std::endl;
	printMemory();
	auto reg3 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize * 1.5));
	std::cout << reg3 << std::endl;
	printMemory();
	auto reg4 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize * 0.33));
	std::cout << reg4 << std::endl;
	printMemory();
	auto reg5 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize * 1.5));
	std::cout << reg5 << std::endl;
	printMemory();
}

int main()
{
	test();
	printMemory();
	return 0;
}
