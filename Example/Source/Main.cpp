#include "ExecutableMalloc/PosixAllocator.hpp"

#include <cstdint>
#include <iostream>
#include <cassert>

using namespace ExecutableMalloc;

PosixMemoryBlockAllocator allocator;

void printMemory()
{
	std::size_t i = 0;
	std::cout << "- Memory dump start" << std::endl;
	for (const std::unique_ptr<MemoryMapping>& block : allocator.getMappings()) {
		std::cout << "------- Mapping #" << i << std::endl;
		std::cout << "From: " << std::hex << block->getFrom() << std::dec << std::endl;
		std::cout << "To: " << std::hex << block->getTo() << std::dec << std::endl;
		std::cout << "Writable: " << std::boolalpha << block->isWritable() << std::noboolalpha << std::endl;
		std::cout << "-- Regions dump start" << std::endl;
		std::size_t j = 0;
		for (const MemoryRegion* region : block->getUsedRegions()) {
			std::cout << "------- Region #" << j << std::endl;
			std::cout << "From: " << std::hex << region->getFrom() << std::dec << std::endl;
			std::cout << "To: " << std::hex << region->getTo() << std::dec << std::endl;
			j++;
		}
		std::cout << "-- Regions dump end" << std::endl;
		i++;
	}
	std::cout << "- Memory dump end" << std::endl;
}

void assertMemory(std::initializer_list<std::size_t> regions) {
	assert(allocator.getMappings().size() == regions.size());
	auto regsize = regions.begin();
	for(std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator.getMappings()[i]->getUsedRegions().size() == *regsize);
		regsize++;
	}
}

int main();

void test()
{
	const auto pageSize_d = static_cast<double>(posixGetGranularity());
	printMemory();
	auto reg1 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg1->getFrom() << std::dec << std::endl;
	printMemory();
	assertMemory({ 1 });
	auto reg2 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << std::hex << reg2->getFrom() << std::dec << std::endl;
	printMemory();
	assertMemory({ 2 });
	auto reg3 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg3->getFrom() << std::dec << std::endl;
	printMemory();
	assertMemory({ 2, 1 });
	auto reg4 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << std::hex << reg4->getFrom() << std::dec << std::endl;
	printMemory();
	assertMemory({ 2, 2 });
	auto reg5 = allocator.getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg5->getFrom() << std::dec << std::endl;
	printMemory();
	assertMemory({ 2, 2, 1 });
}

int main()
{
	assertMemory({});
	test();
	printMemory();
	assertMemory({});
	return 0;
}
