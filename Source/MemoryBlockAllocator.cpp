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

const std::vector<std::unique_ptr<MemoryMapping>>& MemoryBlockAllocator::getMappings() const
{
	return mappings;
}

std::optional<std::pair<std::reference_wrapper<std::unique_ptr<MemoryMapping>>, std::uintptr_t>> MemoryBlockAllocator::findClosest(std::uintptr_t location, std::size_t size, std::size_t tolerance)
{
	std::unique_ptr<MemoryMapping>* best = nullptr;
	std::uintptr_t bestLocation;
	std::uintptr_t bestDistance = tolerance;

	for(auto& mapping : mappings) {
		if(mapping->to - mapping->from < size)
			continue;
		auto region = mapping->findRegionInTolerance(location, size, tolerance);
		if(!region.has_value())
			continue;
		auto [regionBegin, distance] = region.value();
		if(distance < bestDistance) {
			best = &mapping;
			bestLocation = regionBegin;
			bestDistance = distance;
		}
	}
	if (best == nullptr)
		return std::nullopt;
	return { { *best, bestLocation } };
}

std::unique_ptr<MemoryRegion> MemoryBlockAllocator::getRegion(std::uintptr_t preferredLocation, std::size_t size, bool writable, std::size_t tolerance)
{
	if(size == 0)
		throw std::bad_alloc{};

	auto closest = findClosest(preferredLocation, size, tolerance);
	if (closest.has_value()) {
		auto& [mapping, regionBegin] = closest.value();
		mapping.get()->setWritable(writable);
		return mapping.get()->acquireRegion(regionBegin, size);
	}

	// Mhm, I guess we are out of luck, we need to allocate new memory
	auto numPages = static_cast<std::size_t>(ceilf(static_cast<float>(size) / static_cast<float>(granularity)));
	auto effectiveSize = granularity * numPages;
	std::uintptr_t newMem = findUnusedMemory(preferredLocation, tolerance, numPages, writable);
	auto& newRegion = mappings.emplace_back(std::unique_ptr<MemoryMapping>{ new MemoryMapping{ this, newMem, newMem + effectiveSize, writable } });

	auto regionBegin = newRegion->findRegionInTolerance(preferredLocation, size, tolerance);
	// If this optional happens to be empty then findUnusedMemory didn't give a page inside the tolerance
	return newRegion->acquireRegion(regionBegin->first, size);
}

void MemoryBlockAllocator::gc(MemoryMapping* page)
{
	std::erase_if(mappings, [&](const std::unique_ptr<MemoryMapping>& other) {
		bool found = other.get() == page;
		if(found)
			deallocateMemory(page->from, page->to - page->from);
		return found;
	});
}