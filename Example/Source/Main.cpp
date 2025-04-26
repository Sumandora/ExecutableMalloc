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

static void print_memory()
{
	std::size_t i = 0;
	const auto& mappings = allocator->get_mappings();
	std::println("Mappings: {}", mappings.size());
	for (const std::unique_ptr<MemoryMapping>& block : mappings) {
		std::println("Mapping #{}", i);
		std::println("\tFrom: {:#x}", block->get_from());
		std::println("\tTo: {:#x}", block->get_to());
		std::println("\tWritable: {}", block->is_writable());
		const auto& used_regions = block->get_used_regions();
		std::println("\tRegions: {}", used_regions.size());
		std::size_t j = 0;
		for (const MemoryRegion* region : used_regions) {
			std::println("\t\tRegion #{}", j);
			std::println("\t\t\tFrom: {:#x}", region->get_from());
			std::println("\t\t\tTo: {:#x}", region->get_to());
			j++;
		}
		i++;
	}
}

static void assert_memory(std::initializer_list<std::size_t> regions)
{
	assert(allocator->get_mappings().size() == regions.size());
	const auto* regsize = regions.begin();
	for (std::size_t i = 0; i < regions.size(); i++) {
		assert(allocator->get_mappings()[i]->get_used_regions().size() == *regsize);
		regsize++;
	}
}

static void test()
{
	const auto page_size = static_cast<double>(allocator->get_granularity());
	print_memory();
	auto reg1 = allocator->get_region(reinterpret_cast<std::uintptr_t>(&test), static_cast<int>(page_size * 1.5));
	std::println("Allocated at: {:#x}", reg1->get_from());
	print_memory();
	assert_memory({ 1 });
	auto reg2 = allocator->get_region(reinterpret_cast<std::uintptr_t>(&test), static_cast<int>(page_size * 0.33));
	std::println("Allocated at: {:#x}", reg2->get_from());
	print_memory();
	assert_memory({ 2 });
	auto reg3 = allocator->get_region(reinterpret_cast<std::uintptr_t>(&test), static_cast<int>(page_size * 1.5));
	std::println("Allocated at: {:#x}", reg3->get_from());
	print_memory();
	assert_memory({ 2, 1 });
	auto reg4 = allocator->get_region(reinterpret_cast<std::uintptr_t>(&test), static_cast<int>(page_size * 0.33));
	std::println("Allocated at: {:#x}", reg4->get_from());
	print_memory();
	assert_memory({ 2, 2 });
	auto reg5 = allocator->get_region(reinterpret_cast<std::uintptr_t>(&test), static_cast<int>(page_size * 1.5));
	std::println("Allocated at: {:#x}", reg5->get_from());
	print_memory();
	assert_memory({ 2, 2, 1 });
}

int main()
{
	{
		// Instantiate the template to test if it compiles.
		const MemoryManager::LinuxMemoryManager<true, true, true> mem_mgr;
		const ExecutableMalloc::MemoryManagerAllocator<decltype(mem_mgr)> dummy{ mem_mgr };
	}

	allocator = std::make_unique<ExecutableMalloc::PosixAllocator>();
	assert_memory({});
	test();
	print_memory();
	assert_memory({});
	return 0;
}
