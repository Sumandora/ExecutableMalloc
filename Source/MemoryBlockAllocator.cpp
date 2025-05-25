#include "ExecutableMalloc.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>

using namespace ExecutableMalloc;

std::pair<decltype(MemoryBlockAllocator::mappings)::iterator, std::uintptr_t> MemoryBlockAllocator::find_closest(
	std::uintptr_t location,
	std::size_t size,
	std::size_t tolerance)
{
	auto best = mappings.end();
	std::uintptr_t best_location = 0;
	std::uintptr_t best_distance = tolerance;

	for (auto it = mappings.begin(); it != mappings.end(); it++) {
		auto& mapping = *it;
		if (mapping->to - mapping->from < size)
			continue;
		auto region = mapping->find_region_in_tolerance(location, size, tolerance);
		if (!region.has_value())
			continue;
		auto [region_begin, distance] = region.value();
		if (distance < best_distance) {
			best = it;
			best_location = region_begin;
			best_distance = distance;
		}
	}
	return { best, best_location };
}

void MemoryBlockAllocator::gc(MemoryMapping* page)
{
	auto it = std::ranges::find_if(mappings,
		[&](const std::unique_ptr<MemoryMapping>& other) {
			return other.get() == page;
		});
	if (it != mappings.end()) {
		deallocate_memory(page->from, page->to - page->from);
		mappings.erase(it);
	}
}

std::unique_ptr<MemoryMapping>& MemoryBlockAllocator::allocate_new_map(
	std::uintptr_t preferred_location,
	std::size_t size,
	bool writable,
	std::size_t tolerance)
{
	// round up integer division
	const size_t num_pages = (size + granularity - 1) / granularity;

	const std::uintptr_t new_mem = find_unused_memory(preferred_location, tolerance, num_pages, writable);
	auto& new_region = mappings.emplace_back(
		new MemoryMapping{ this, new_mem, new_mem + num_pages * granularity, writable });
	return new_region;
}

MemoryBlockAllocator::MemoryBlockAllocator(std::size_t granularity)
	: granularity(granularity)
{
}

MemoryBlockAllocator::~MemoryBlockAllocator() = default;

[[nodiscard]] std::unique_ptr<MemoryRegion> MemoryBlockAllocator::get_region(
	std::uintptr_t preferred_location,
	std::size_t size,
	bool needs_writable,
	std::size_t tolerance)
{
	if (size == 0)
		throw std::bad_alloc{};

	auto [iter, region_begin] = find_closest(preferred_location, size, tolerance);
	if (iter != mappings.end()) {
		auto& mapping = *iter;
		if (needs_writable && !mapping->is_writable())
			mapping->set_writable(needs_writable);
		return mapping->acquire_region(region_begin, size);
	}

	// Mhm, no luck, new memory needs to be allocated
	std::unique_ptr<MemoryMapping>& new_region = allocate_new_map(preferred_location, size, needs_writable, tolerance);

	auto region_bounds = new_region->find_region_in_tolerance(preferred_location, size, tolerance);
	if (!region_bounds.has_value()) // If this optional happens to be empty, then find_unused_memory didn't give a page inside the tolerance
		throw std::bad_alloc{};

	return new_region->acquire_region(region_bounds->first, size);
}
