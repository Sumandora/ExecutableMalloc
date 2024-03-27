#include "ExecutableMalloc.h"
#include "ExecutableMalloc.hpp"

#include <new>

using namespace ExecutableMalloc;

extern "C" {

extern const size_t emalloc_sizeof_memoryregion = sizeof(MemoryRegion);
extern const size_t emalloc_sizeof_memorymapping = sizeof(MemoryMapping);
extern const size_t emalloc_sizeof_memoryblockallocator = sizeof(MemoryBlockAllocator);

uintptr_t emalloc_region_get_from(const void* memoryregion)
{
	return static_cast<const MemoryRegion*>(memoryregion)->getFrom();
}
uintptr_t emalloc_region_get_to(const void* memoryregion)
{
	return static_cast<const MemoryRegion*>(memoryregion)->getTo();
}
const void* getParent(const void* memoryregion)
{
	return static_cast<const MemoryRegion*>(memoryregion)->getParent();
}
bool emalloc_region_is_writable(const void* memoryregion)
{
	return static_cast<const MemoryRegion*>(memoryregion)->isWritable();
}
void emalloc_region_set_writable(void* memoryregion, bool writable)
{
	static_cast<MemoryRegion*>(memoryregion)->setWritable(writable);
}

const void* emalloc_mapping_get_parent(const void* memorymapping)
{
	return static_cast<const MemoryMapping*>(memorymapping)->getParent();
}
uintptr_t emalloc_mapping_get_from(const void* memorymapping)
{
	return static_cast<const MemoryMapping*>(memorymapping)->getFrom();
}
uintptr_t emalloc_mapping_get_to(const void* memorymapping)
{
	return static_cast<const MemoryMapping*>(memorymapping)->getTo();
}
const void* emalloc_mapping_used_region_at(const void* memorymapping, size_t index) {
	return static_cast<const MemoryMapping*>(memorymapping)->getUsedRegions().at(index);
}
size_t emalloc_mapping_used_regions_count(const void* memorymapping) {
	return static_cast<const MemoryMapping*>(memorymapping)->getUsedRegions().size();
}
bool emalloc_mapping_is_writable(const void* memorymapping)
{
	return static_cast<const MemoryMapping*>(memorymapping)->isWritable();
}

void emalloc_construct_memoryblockallocator(
	void* memoryblockallocator,
	emalloc_findUnusedMemory findUnusedMemory,
	void* data1,
	emalloc_deallocateMemory deallocateMemory,
	void* data2,
	size_t granularity)
{
	new (memoryblockallocator) MemoryBlockAllocator{
		[findUnusedMemory, data1](std::uintptr_t preferredLocation, std::size_t tolerance, std::size_t numPages, bool writable) {
			return findUnusedMemory(preferredLocation, tolerance, numPages, writable, data1);
		},
		[deallocateMemory, data2](std::uintptr_t location, std::size_t size) {
			return deallocateMemory(location, size, data2);
		},
		granularity
	};
}

const void* emalloc_memoryblockallocator_mapping_at(const void* memoryblockallocator, size_t index) {
	return static_cast<const MemoryBlockAllocator*>(memoryblockallocator)->getMappings().at(index).get();
}
size_t emalloc_memoryblockallocator_mappings_count(const void* memoryblockallocator) {
	return static_cast<const MemoryBlockAllocator*>(memoryblockallocator)->getMappings().size();
}

static std::vector<std::unique_ptr<MemoryRegion>> regions;

uintptr_t emalloc_memoryblockallocator_get_region(void* memoryblockallocator, uintptr_t preferredLocation, size_t size, size_t tolerance, bool writable)
{
	std::unique_ptr<MemoryRegion> region = static_cast<MemoryBlockAllocator*>(memoryblockallocator)->getRegion(preferredLocation, size, tolerance, writable);
	auto& reg = regions.emplace_back(std::move(region));
	return reg.get()->getFrom();
}
void emalloc_memoryblockallocator_delete_region(void* pointer)
{
	auto it = regions.begin();
	while (it != regions.end()) {
		if (reinterpret_cast<void*>(it->get()->getFrom()) == pointer) {
			it = regions.erase(it);
			return;
		} else
			it++;
	}
	throw std::exception{};
}

void emalloc_memoryblockallocator_cleanup(void* memoryblockallocator) {
	static_cast<MemoryBlockAllocator*>(memoryblockallocator)->MemoryBlockAllocator::~MemoryBlockAllocator();
}

}