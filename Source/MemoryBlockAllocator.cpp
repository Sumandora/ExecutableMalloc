#include "ExecutableMalloc.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>

using namespace ExecutableMalloc;

MemoryBlockAllocator::MemoryBlockAllocator(
	decltype(findUnusedMemory)&& findUnusedMemory,
	decltype(deallocateMemory)&& deallocateMemory,
	decltype(changePermissions)&& changePermissions,
	std::size_t granularity)
	: findUnusedMemory(std::move(findUnusedMemory))
	, deallocateMemory(std::move(deallocateMemory))
	, changePermissions(std::move(changePermissions))
	, granularity(granularity)
{
}

MemoryBlockAllocator::~MemoryBlockAllocator() = default;

std::pair<decltype(MemoryBlockAllocator::mappings)::iterator, std::uintptr_t> MemoryBlockAllocator::findClosest(std::uintptr_t location, std::size_t size, std::size_t tolerance)
{
	auto best = mappings.end();
	std::uintptr_t bestLocation;
	std::uintptr_t bestDistance = tolerance;

	for (auto it = mappings.begin(); it != mappings.end(); it++) {
		auto& mapping = *it;
		if (mapping->to - mapping->from < size)
			continue;
		auto region = mapping->findRegionInTolerance(location, size, tolerance);
		if (!region.has_value())
			continue;
		auto [regionBegin, distance] = region.value();
		if (distance < bestDistance) {
			best = it;
			bestLocation = regionBegin;
			bestDistance = distance;
		}
	}
	return { best, bestLocation };
}

std::unique_ptr<MemoryRegion> MemoryBlockAllocator::getRegion(std::uintptr_t preferredLocation, std::size_t size, bool writable, std::size_t tolerance)
{
	if (size == 0)
		throw std::bad_alloc{};

	auto [iter, regionBegin] = findClosest(preferredLocation, size, tolerance);
	if (iter != mappings.end()) {
		auto& mapping = *iter;
		if (writable && !mapping->isWritable())
			mapping->setWritable(writable);
		return mapping->acquireRegion(regionBegin, size);
	}

	// Mhm, I guess we are out of luck, we need to allocate new memory
	auto effectiveSize = static_cast<std::size_t>(ceilf(static_cast<float>(size) / static_cast<float>(granularity))) * granularity;
	std::uintptr_t newMem = findUnusedMemory(preferredLocation, tolerance, effectiveSize, writable);
	auto& newRegion = mappings.emplace_back(std::unique_ptr<MemoryMapping>{ new MemoryMapping{ this, newMem, newMem + effectiveSize, writable } });

	auto regionBounds = newRegion->findRegionInTolerance(preferredLocation, size, tolerance);
	// If this optional happens to be empty then findUnusedMemory didn't give a page inside the tolerance
	return newRegion->acquireRegion(regionBounds->first, size);
}

void MemoryBlockAllocator::gc(MemoryMapping* page)
{
	auto it = std::find_if(mappings.begin(), mappings.end(), [&](const std::unique_ptr<MemoryMapping>& other) { return other.get() == page; });
	if(it != mappings.end()) {
		deallocateMemory(page->from, page->to - page->from);
		mappings.erase(it);
	}
}