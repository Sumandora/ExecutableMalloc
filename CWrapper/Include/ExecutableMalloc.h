#ifndef EXECUTABLEMALLOC_H
#define EXECUTABLEMALLOC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const size_t emalloc_sizeof_memoryregion;
extern const size_t emalloc_sizeof_memorymapping;
extern const size_t emalloc_sizeof_memoryblockallocator;

uintptr_t emalloc_region_get_from(const void* memoryregion);
uintptr_t emalloc_region_get_to(const void* memoryregion);
const void* getParent(const void* memoryregion);
bool emalloc_region_is_writable(const void* memoryregion);
void emalloc_region_set_writable(void* memoryregion, bool writable);

const void* emalloc_mapping_get_parent(const void* memorymapping);
uintptr_t emalloc_mapping_get_from(const void* memorymapping);
uintptr_t emalloc_mapping_get_to(const void* memorymapping);
const void* emalloc_mapping_used_region_at(const void* memorymapping, size_t index);
size_t emalloc_mapping_used_regions_count(const void* memorymapping);
bool emalloc_mapping_is_writable(const void* memorymapping);

typedef uintptr_t(*findUnusedMemory)(uintptr_t preferredLocation, size_t tolerance, size_t numPages, bool writable, void* data);
typedef void(*deallocateMemory)(uintptr_t location, size_t size, void* data);

void emalloc_construct_memoryblockallocator(
	void* memoryblockallocator,
	findUnusedMemory findUnusedMemory,
	void* data1,
	deallocateMemory deallocateMemory,
	void* data2,
	size_t granularity);

const void* emalloc_memoryblockallocator_mapping_at(const void* memoryblockallocator, size_t index);
size_t emalloc_memoryblockallocator_mappings_count(const void* memoryblockallocator);

void* emalloc_memoryblockallocator_get_region(void* memoryblockallocator, uintptr_t preferredLocation, size_t size, size_t tolerance /*= INT32_MAX*/, bool writable /*= true*/);
void emalloc_memoryblockallocator_delete_region(void* pointer);

void emalloc_memoryblockallocator_cleanup(void* memoryblockallocator);

#ifdef __cplusplus
}
#endif

#endif
