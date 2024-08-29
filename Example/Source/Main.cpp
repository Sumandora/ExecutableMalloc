#include "ExecutableMalloc.hpp"
#include "ExecutableMalloc/PosixAllocator.hpp"

// Include to verify, that this compiles
#include "ExecutableMalloc/MemoryManagerAllocator.hpp" // IWYU pragma: keep

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <memory>

ExecutableMalloc::MemoryBlockAllocator* allocator;

using namespace ExecutableMalloc;

void printMemory()
{
	std::size_t i = 0;
	auto& mappings = allocator->getMappings();
	std::cout << "Mappings: " << mappings.size() << '\n';
	for (const std::unique_ptr<MemoryMapping>& block : mappings) {
		std::cout << "Mapping #" << i << '\n';
		std::cout << "\tFrom: " << std::hex << block->getFrom() << std::dec << '\n';
		std::cout << "\tTo: " << std::hex << block->getTo() << std::dec << '\n';
		std::cout << "\tWritable: " << std::boolalpha << block->isWritable() << std::noboolalpha << '\n';
		auto& usedRegions = block->getUsedRegions();
		std::cout << "\tRegions: " << usedRegions.size() << '\n';
		std::size_t j = 0;
		for (const MemoryRegion* region : usedRegions) {
			std::cout << "\t\tRegion #" << j << '\n';
			std::cout << "\t\t\tFrom: " << std::hex << region->getFrom() << std::dec << '\n';
			std::cout << "\t\t\tTo: " << std::hex << region->getTo() << std::dec << '\n';
			j++;
		}
		i++;
	}
}

void assertMemory(std::initializer_list<std::size_t> regions)
{
	assert(allocator->getMappings().size() == regions.size());
	const auto* regsize = regions.begin();
	for (std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator->getMappings()[i]->getUsedRegions().size() == *regsize);
		regsize++;
	}
}

int main();

void test()
{
	const auto pageSize_d = static_cast<double>(allocator->getGranularity());
	printMemory();
	auto reg1 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg1->getFrom() << std::dec << '\n';
	printMemory();
	assertMemory({ 1 });
	auto reg2 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << std::hex << reg2->getFrom() << std::dec << '\n';
	printMemory();
	assertMemory({ 2 });
	auto reg3 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg3->getFrom() << std::dec << '\n';
	printMemory();
	assertMemory({ 2, 1 });
	auto reg4 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::cout << std::hex << reg4->getFrom() << std::dec << '\n';
	printMemory();
	assertMemory({ 2, 2 });
	auto reg5 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::cout << std::hex << reg5->getFrom() << std::dec << '\n';
	printMemory();
	assertMemory({ 2, 2, 1 });
}

int main()
{
	allocator = new ExecutableMalloc::PosixAllocator{};
	assertMemory({});
	test();
	printMemory();
	assertMemory({});
	return 0;
}
