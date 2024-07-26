#include "ExecutableMalloc/PosixAllocator.hpp"
#include "ExecutableMalloc/MemoryManagerAllocator.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

using namespace ExecutableMalloc;

PosixMemoryBlockAllocator allocator;

void printMemory()
{
	std::size_t i = 0;
	auto& mappings = allocator.getMappings();
	std::cout << "Mappings: " << mappings.size() << std::endl;
	for (const std::unique_ptr<MemoryMapping>& block : mappings) {
		std::cout << "Mapping #" << i << std::endl;
		std::cout << "\tFrom: " << std::hex << block->getFrom() << std::dec << std::endl;
		std::cout << "\tTo: " << std::hex << block->getTo() << std::dec << std::endl;
		std::cout << "\tWritable: " << std::boolalpha << block->isWritable() << std::noboolalpha << std::endl;
		auto& usedRegions = block->getUsedRegions();
		std::cout << "\tRegions: " << usedRegions.size() << std::endl;
		std::size_t j = 0;
		for (const MemoryRegion* region : usedRegions) {
			std::cout << "\t\tRegion #" << j << std::endl;
			std::cout << "\t\t\tFrom: " << std::hex << region->getFrom() << std::dec << std::endl;
			std::cout << "\t\t\tTo: " << std::hex << region->getTo() << std::dec << std::endl;
			j++;
		}
		i++;
	}
}

void assertMemory(std::initializer_list<std::size_t> regions)
{
	assert(allocator.getMappings().size() == regions.size());
	auto regsize = regions.begin();
	for (std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator.getMappings()[i]->getUsedRegions().size() == *regsize);
		regsize++;
	}
}

int main();

void test()
{
	const auto pageSize_d = static_cast<double>(allocator.getGranularity());
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
