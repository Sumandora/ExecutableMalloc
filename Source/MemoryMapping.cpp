#include "ExecutableMalloc.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <optional>
#include <ranges>
#include <utility>

using namespace ExecutableMalloc;

std::optional<std::uintptr_t> MemoryMapping::find_region(std::size_t size) const
{
	std::uintptr_t p = from;
	for (const auto& region : used_regions) {
		if (region->get_from() - p >= size)
			return p;
		p = region->get_to();
	}
	if (to - p >= size)
		return p;
	return std::nullopt;
}

std::optional<std::uintptr_t> MemoryMapping::find_region_reverse(std::size_t size) const
{
	std::uintptr_t p = to;
	for (const auto& region : used_regions | std::ranges::views::reverse) {
		if (p - region->get_to() >= size)
			return p - size;
		p = region->get_from();
	}
	if (p - from >= size)
		return p - size;
	return std::nullopt;
}

MemoryMapping::MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable)
	: parent(parent)
	, from(from)
	, to(to)
	, writable(writable)
{
}

void MemoryMapping::set_writable(bool new_writable)
{
	if (this->writable == new_writable)
		return;

	parent->change_protection(from, to - from, new_writable);
	this->writable = new_writable;
}

void MemoryMapping::gc(MemoryRegion* region)
{
	used_regions.erase(region);
	if (used_regions.empty()) {
		// This mapping is no longer needed.
		parent->gc(this);
	}
}

static constexpr size_t dist(std::size_t a, std::size_t b)
{
	return std::max(a, b) - std::min(a, b);
}

std::optional<std::pair<std::uintptr_t, std::size_t>> MemoryMapping::find_region_in_tolerance(
	std::uintptr_t location,
	std::size_t size,
	std::size_t tolerance) const
{
	if (dist(from, location) > tolerance || dist(to, location) > tolerance)
		return std::nullopt;
	for (const bool reverse : { false, true }) {
		auto region = reverse ? find_region(size) : find_region_reverse(size);
		if (!region.has_value())
			continue;
		const std::size_t distance = dist(region.value(), location);
		if (distance > tolerance || dist(region.value() + size, location) > tolerance)
			continue;
		return { { region.value(), distance } };
	}
	return std::nullopt;
}

std::unique_ptr<MemoryRegion> MemoryMapping::acquire_region(std::uintptr_t location, std::size_t size)
{
	if (location < from || location + size > to)
		throw std::bad_alloc{};
	auto* region_ptr = new MemoryRegion{ location, location + size, this };
	used_regions.insert(region_ptr);
	return std::unique_ptr<MemoryRegion>{ region_ptr };
}
