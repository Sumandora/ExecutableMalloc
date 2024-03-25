#include "ExecutableMalloc.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>

using namespace ExecutableMalloc;

MemoryBlockAllocator::MemoryBlockAllocator(
	decltype(findUnusedMemory)&& findUnusedMemory,
	decltype(deallocateMemory)&& deallocateMemory,
	std::size_t granularity)
	: findUnusedMemory(std::move(findUnusedMemory))
	, deallocateMemory(std::move(deallocateMemory))
	, granularity(granularity)
{
}

MemoryBlockAllocator::~MemoryBlockAllocator() = default;

const std::vector<std::unique_ptr<MemoryMapping>>& MemoryBlockAllocator::getMappings() const
{
	return mappings;
}

std::optional<std::reference_wrapper<std::unique_ptr<MemoryMapping>>> MemoryBlockAllocator::findClosest(std::uintptr_t location, std::size_t size, bool writable)
{
	auto available = mappings | std::ranges::views::filter([size](const std::unique_ptr<MemoryMapping>& p) { return p->hasRegion(size); });
	auto best = std::min_element(available.begin(), available.end(), [location, size](const std::unique_ptr<MemoryMapping>& a, const std::unique_ptr<MemoryMapping>& b) {
		return std::min(a->distanceTo(location, size), b->distanceTo(location, size));
	});
	if (best == available.end())
		return std::nullopt;
	return *best;
}

std::unique_ptr<MemoryMapping>& MemoryBlockAllocator::getBlock(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance, bool writable)
{
	auto closest = findClosest(preferredLocation, size, writable);
	if (closest.has_value())
		return *closest;
	// Ok, so we don't have the same writable state, maybe we have one that we can change the writable state to the desired one?
	closest = findClosest(preferredLocation, size, !writable);
	if (closest.has_value()) {
		closest.value().get()->setWritable(writable);
		return *closest;
	}

	// Mhm, I guess we are out of luck, we need to allocate new memory
	auto numPages = static_cast<std::size_t>(ceilf(static_cast<float>(size) / static_cast<float>(granularity)));
	auto effectiveSize = granularity * numPages;
	std::uintptr_t newMem = findUnusedMemory(preferredLocation, tolerance, numPages, writable);
	return mappings.emplace_back(std::unique_ptr<MemoryMapping>{ new MemoryMapping{ this, newMem, newMem + effectiveSize, writable } });
}

std::shared_ptr<MemoryRegion> MemoryBlockAllocator::getRegion(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance, bool writable)
{
	if(size == 0)
		throw std::bad_alloc{};
	auto& block = getBlock(preferredLocation, size, tolerance, writable);
	return block->acquireRegion(size);
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