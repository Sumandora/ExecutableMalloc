#include "ExecutableMalloc.hpp"

using namespace ExecutableMalloc;

MemoryRegion::MemoryRegion(std::uintptr_t from, std::uintptr_t to, MemoryMapping* parent)
	: from(from)
	, to(to)
	, parent(parent)
{
}

MemoryRegion::~MemoryRegion()
{
	parent->gc(this);
}

std::uintptr_t MemoryRegion::getFrom() const {
	return from;
}

std::uintptr_t MemoryRegion::getTo() const {
	return to;
}

const MemoryMapping* MemoryRegion::getParent() const {
	return parent;
}

std::strong_ordering MemoryRegion::operator<=>(const MemoryRegion& other) const
{
	return from <=> other.from;
}

bool MemoryRegion::isWritable()
{
	return parent->isWritable();
}

void MemoryRegion::setWritable(bool writable)
{
	parent->setWritable(writable);
}