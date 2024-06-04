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

bool MemoryRegion::isWritable() const
{
	return parent->isWritable();
}

void MemoryRegion::setWritable(bool writable)
{
	parent->setWritable(writable);
}

void MemoryRegion::resize(std::size_t size)
{
	if (size > to - from)
		throw std::exception{}; // It needs to be smaller

	to = from + size;
}