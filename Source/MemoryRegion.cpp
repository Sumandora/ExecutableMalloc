#include "ExecutableMalloc.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

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

bool MemoryRegion::is_writable() const
{
	return parent->is_writable();
}

void MemoryRegion::set_writable(bool writable)
{
	parent->set_writable(writable);
}

void MemoryRegion::resize(std::size_t size) // The new size needs to be smaller or equally big as the current region
{
	if (size > to - from)
		throw std::exception{};

	to = from + size;
}
