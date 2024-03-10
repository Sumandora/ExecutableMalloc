#include "ExecutableMalloc.hpp"

#include <sys/mman.h>
#include <algorithm>
#include <ranges>
#include <cmath>

using namespace ExecutableMalloc;

static const auto pageSize = getpagesize();

constexpr static std::uintptr_t align(std::uintptr_t ptr, std::size_t alignment)
{
	return ptr - ptr % alignment;
}

void* MemoryBlockAllocator::findUnusedMemory(std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages)
{
	// TODO Trust kernel
	for (std::size_t offset = 0; offset < tolerance; offset += pageSize)
		for (int sign = -1; sign <= 2; sign += 2) {
			void* pointer = mmap(
				reinterpret_cast<char*>(align(preferredLocation, pageSize)) + offset * sign,
				pageSize * numPages,
				PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
				-1,
				0);
			if (pointer != MAP_FAILED)
				return pointer;
		}
	return nullptr;
}

const std::vector<std::unique_ptr<MemoryMapping>>& MemoryBlockAllocator::getMappings() const {
	return mappings;
}

decltype(MemoryBlockAllocator::mappings)::pointer MemoryBlockAllocator::findClosest(std::uintptr_t location, std::size_t size)
{
	auto available = mappings | std::ranges::views::filter([size](const std::unique_ptr<MemoryMapping>& p) { return p->hasRegion(size); });
	auto best = std::min_element(available.begin(), available.end(), [location, size](const std::unique_ptr<MemoryMapping>& a, const std::unique_ptr<MemoryMapping>& b) {
		return std::min(a->distanceTo(location, size), b->distanceTo(location, size));
	});
	if (best == available.end())
		return nullptr;
	return &*best;
}

std::unique_ptr<MemoryMapping>& MemoryBlockAllocator::getBlock(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance)
{
	auto* closest = findClosest(preferredLocation, size);

	std::unique_ptr<MemoryMapping>& page = [&]() -> std::unique_ptr<MemoryMapping>& {
		if (closest == nullptr) {
			auto numPages = static_cast<std::size_t>(ceilf(static_cast<float>(size) / static_cast<float>(pageSize)));
			auto effectiveSize = pageSize * numPages;
			void* newMem = findUnusedMemory(preferredLocation, tolerance, numPages);
			return mappings.emplace_back(std::make_unique<MemoryMapping>(this, reinterpret_cast<std::uintptr_t>(newMem), reinterpret_cast<std::uintptr_t>(reinterpret_cast<char*>(newMem) + effectiveSize), false));
		} else {
			return *closest;
		}
	}();

	return page;
}

std::shared_ptr<MemoryRegion> MemoryBlockAllocator::getRegion(std::uintptr_t preferredLocation, std::size_t size, std::size_t tolerance)
{
	auto& block = getBlock(preferredLocation, size, tolerance);
	return block->acquireRegion(size);
}

void MemoryBlockAllocator::gc(MemoryMapping* page)
{
	std::erase_if(mappings, [page](const std::unique_ptr<MemoryMapping>& other) {
		return other.get() == page;
	});
}