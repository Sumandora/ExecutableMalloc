#include "ExecutableMalloc.hpp"

#include <sys/mman.h>

using namespace ExecutableMalloc;

MemoryMapping::MemoryMapping(MemoryBlockAllocator* parent, std::uintptr_t from, std::uintptr_t to, bool writable)
	: parent(parent)
	, from(from)
	, to(to)
	, writable(writable)
{
}

const MemoryBlockAllocator* MemoryMapping::getParent() const {
	return parent;
}

std::uintptr_t MemoryMapping::getFrom() const {
	return from;
}

std::uintptr_t MemoryMapping::getTo() const {
	return to;
}

std::set<MemoryRegion*> MemoryMapping::getUsedRegions() const {
	return usedRegions;
}

bool MemoryMapping::isWritable() const {
	return writable;
}

constexpr static std::size_t safeSub(std::size_t a, std::size_t b)
{
	return std::max(a, b) - std::min(a, b);
}

[[nodiscard]] std::size_t MemoryMapping::distanceTo(std::uintptr_t address, std::size_t size) const
{
	if (size > to) {
		return safeSub(from, address);
	}
	return std::min(safeSub(from, address), safeSub(to - size, address));
}

std::strong_ordering MemoryMapping::operator<=>(const MemoryMapping& other) const
{
	return from <=> other.from;
}

[[nodiscard]] bool MemoryMapping::hasRegion(size_t size) const
{
	std::uintptr_t p = from;
	for (const MemoryRegion* region : usedRegions) {
		std::uintptr_t space = region->getFrom() - p;
		if (space >= size)
			return true;
		p = region->getTo();
	}
	return to - p >= size;
}

void MemoryMapping::setWritable(bool newWritable)
{
	if (this->writable == newWritable)
		return;

	int perms = PROT_READ | PROT_WRITE;

	if (newWritable) {
		perms |= PROT_EXEC;
	}

	mprotect(reinterpret_cast<void*>(this->from), this->to - this->from, perms);
	this->writable = newWritable;
}

std::unique_ptr<MemoryRegion> MemoryMapping::acquireRegion(size_t size)
{
	std::uintptr_t p = from;
	for (const MemoryRegion* region : usedRegions) {
		std::uintptr_t space = region->getFrom() - p;
		if (space >= size) {
			break;
		}
		p = region->getTo();
	}

	if(p + size > to)
		throw std::bad_alloc{};
	auto region = std::make_unique<MemoryRegion>(p, p + size, this);
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
