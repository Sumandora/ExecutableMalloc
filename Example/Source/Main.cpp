#include "ExecutableMalloc.hpp"
#include "ExecutableMalloc/PosixAllocator.hpp"

// Include to verify, that this compiles
#include "ExecutableMalloc/MemoryManagerAllocator.hpp"
#include "MemoryManager/LinuxMemoryManager.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <print>

static std::unique_ptr<ExecutableMalloc::MemoryBlockAllocator> allocator;

using namespace ExecutableMalloc;

static void printMemory()
{
	std::size_t i = 0;
	const auto& mappings = allocator->getMappings();
	std::println("Mappings: {}", mappings.size());
	for (const std::unique_ptr<MemoryMapping>& block : mappings) {
		std::println("Mapping #{}", i);
		std::println("\tFrom: {:#x}", block->getFrom());
		std::println("\tTo: {:#x}", block->getTo());
		std::println("\tWritable: {}", block->isWritable());
		const auto& usedRegions = block->getUsedRegions();
		std::println("\tRegions: {}", usedRegions.size());
		std::size_t j = 0;
		for (const MemoryRegion* region : usedRegions) {
			std::println("\t\tRegion #{}", j);
			std::println("\t\t\tFrom: {:#x}", region->getFrom());
			std::println("\t\t\tTo: {:#x}", region->getTo());
			j++;
		}
		i++;
	}
}

static void assertMemory(std::initializer_list<std::size_t> regions)
{
	assert(allocator->getMappings().size() == regions.size());
	const auto* regsize = regions.begin();
	for (std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator->getMappings()[i]->getUsedRegions().size() == *regsize);
		regsize++;
	}
}

int main();

static void test()
{
	const auto pageSize_d = static_cast<double>(allocator->getGranularity());
	printMemory();
	auto reg1 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::println("Allocated at: {:#x}", reg1->getFrom());
	printMemory();
	assertMemory({ 1 });
	auto reg2 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::println("Allocated at: {:#x}", reg2->getFrom());
	printMemory();
	assertMemory({ 2 });
	auto reg3 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::println("Allocated at: {:#x}", reg3->getFrom());
	printMemory();
	assertMemory({ 2, 1 });
	auto reg4 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 0.33));
	std::println("Allocated at: {:#x}", reg4->getFrom());
	printMemory();
	assertMemory({ 2, 2 });
	auto reg5 = allocator->getRegion(reinterpret_cast<std::uintptr_t>(&main), static_cast<int>(pageSize_d * 1.5));
	std::println("Allocated at: {:#x}", reg5->getFrom());
	printMemory();
	assertMemory({ 2, 2, 1 });
}

int main()
{
	{
		// Instantiate the template to test if it compiles.
		const MemoryManager::LinuxMemoryManager<true, true, true> memMgr;
		const ExecutableMalloc::MemoryManagerAllocator<decltype(memMgr)> dummy{ memMgr };
	}

	allocator = std::make_unique<ExecutableMalloc::PosixAllocator>();
	assertMemory({});
	test();
	printMemory();
	assertMemory({});
	return 0;
}
