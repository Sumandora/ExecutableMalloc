#include "ExecutableMalloc.hpp"

#include <ranges>

using namespace ExecutableMalloc;

MemoryMapping::MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable)
	: parent(parent)
	, from(from)
	, to(to)
	, writable(writable)
{
}

const MemoryBlockAllocator* MemoryMapping::getParent() const
{
	return parent;
}

std::uintptr_t MemoryMapping::getFrom() const
{
	return from;
}

std::uintptr_t MemoryMapping::getTo() const
{
	return to;
}

const decltype(MemoryMapping::usedRegions)& MemoryMapping::getUsedRegions() const
{
	return usedRegions;
}

bool MemoryMapping::isWritable() const
{
	return writable;
}

std::strong_ordering MemoryMapping::operator<=>(const MemoryMapping& other) const
{
	return from <=> other.from;
}

template <bool Reverse>
std::optional<std::uintptr_t> MemoryMapping::findRegion(std::size_t size) const {
	std::uintptr_t p = Reverse ? to : from;
	auto begin = [this] {
		if constexpr(Reverse)
			return usedRegions.rbegin();
		else
			return usedRegions.begin();
	}();
	auto end = [this] {
		if constexpr(Reverse)
			return usedRegions.rend();
		else
			return usedRegions.end();
	}();
	for (auto it = begin; it != end; it++) {
		auto& region = *it;
		if ((Reverse ? p - region->getTo() : region->getFrom() - p) >= size)
			return Reverse ? p - size : p;
		p = Reverse ? region->getFrom() : region->getTo();
	}
	if ((Reverse ? p - from : to - p) >= size)
		return Reverse ? p - size : p;
	return std::nullopt;
}

template std::optional<std::uintptr_t> MemoryMapping::findRegion<false>(std::size_t size) const;
template std::optional<std::uintptr_t> MemoryMapping::findRegion<true>(std::size_t size) const;

constexpr inline std::size_t dist(std::size_t a, std::size_t b)
{
	return std::max(a, b) - std::min(a, b);
}

std::optional<std::pair<std::uintptr_t, std::size_t>> MemoryMapping::findRegionInTolerance(std::uintptr_t location, std::size_t size, std::size_t tolerance) const {
	if(dist(from, location) > tolerance || dist(to, location) > tolerance)
		return std::nullopt;
	for(bool reverse : { false, true }) {
		auto region = reverse ? findRegion<true>(size) : findRegion<false>(size); // This is so incredibly stupid
		if(!region.has_value())
			continue;
		std::size_t distance = dist(region.value(), location);
		if (distance > tolerance || dist(region.value() + size, location) > tolerance)
			continue;
		return { { region.value(), distance } };
	}
	return std::nullopt;
}

void MemoryMapping::setWritable(bool newWritable)
{
	if (this->writable == newWritable)
		return;

	parent->changePermissions(*this, newWritable);
	this->writable = newWritable;
}

std::unique_ptr<MemoryRegion> MemoryMapping::acquireRegion(std::uintptr_t location, std::size_t size)
{
	if(location < from || location+size > to)
		throw std::bad_alloc{};
	auto region = std::unique_ptr<MemoryRegion>{ new MemoryRegion{ location, location + size, this } };
	usedRegions.insert(region.get());
	return region;
}

void MemoryMapping::gc(MemoryRegion* region)
{
	usedRegions.erase(region);
	if (usedRegions.empty()) {
		// F, I'm unemployed ._.
		parent->gc(this);
	}
}
