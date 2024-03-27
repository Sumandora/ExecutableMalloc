#include "ExecutableMalloc.h"
#include "ExecutableMalloc/PosixAllocator.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

void* allocator;

void printMemory()
{
	printf("- Memory dump start\n");
	for (size_t i = 0; i < emalloc_memoryblockallocator_mappings_count(allocator); i++) {
		const void* mapping = emalloc_memoryblockallocator_mapping_at(allocator, i);
		printf("------- Mapping #%zu\n", i);
		printf("From: %lx\n", emalloc_mapping_get_from(mapping));
		printf("To: %lx\n", emalloc_mapping_get_to(mapping));
		printf("Writable: %s\n", emalloc_mapping_is_writable(mapping) ? "true" : "false");
		printf("-- Regions dump start\n");
		for (size_t j = 0; j < emalloc_mapping_used_regions_count(mapping); j++) {
			const void* region = emalloc_mapping_used_region_at(mapping, j);
			printf("------- Region #%zu\n", j);
			printf("From: %lx\n", emalloc_region_get_from(region));
			printf("To: %lx\n", emalloc_region_get_to(region));
		}
		printf("-- Regions dump end\n");
	}
	printf("- Memory dump end\n");
}

void assertMemory_(size_t mappings, const size_t* regions)
{
	assert(emalloc_memoryblockallocator_mappings_count(allocator) == mappings);
	for (size_t i = 0; i < mappings; i++) {
		assert(emalloc_mapping_used_regions_count(emalloc_memoryblockallocator_mapping_at(allocator, i)) == regions[i]);
	}
}

#define assertMemory(m_count, ...)      \
	do {                                \
		size_t arr[] = { __VA_ARGS__ }; \
		assertMemory_(m_count, arr);    \
	} while (0)

int main();

void test()
{
	const double pageSize_d = (double)emalloc_posix_getGranularity();
	printMemory();
	void* reg1 = emalloc_memoryblockallocator_get_region(allocator, &main, (int)(pageSize_d * 1.5), INT32_MAX, true);
	printf("%p\n", reg1);
	assertMemory(1, 1);
	printMemory();
	void* reg2 = emalloc_memoryblockallocator_get_region(allocator, &main, (int)(pageSize_d * 0.33), INT32_MAX, true);
	printf("%p\n", reg2);
	assertMemory(1, 2);
	printMemory();
	void* reg3 = emalloc_memoryblockallocator_get_region(allocator, &main, (int)(pageSize_d * 1.5), INT32_MAX, true);
	printf("%p\n", reg3);
	assertMemory(2, 2, 1);
	printMemory();
	void* reg4 = emalloc_memoryblockallocator_get_region(allocator, &main, (int)(pageSize_d * 0.33), INT32_MAX, true);
	printf("%p\n", reg4);
	assertMemory(2, 2, 2);
	printMemory();
	void* reg5 = emalloc_memoryblockallocator_get_region(allocator, &main, (int)(pageSize_d * 1.5), INT32_MAX, true);
	printf("%p\n", reg5);
	assertMemory(3, 2, 2, 1);
	printMemory();

	emalloc_memoryblockallocator_delete_region(reg1);
	emalloc_memoryblockallocator_delete_region(reg2);
	emalloc_memoryblockallocator_delete_region(reg3);
	emalloc_memoryblockallocator_delete_region(reg4);
	emalloc_memoryblockallocator_delete_region(reg5);
}

int main()
{
	allocator = malloc(emalloc_sizeof_posix_memoryblockallocator);
	emalloc_construct_posix_memoryblockallocator(allocator);

	assertMemory(0);
	test();
	printMemory();
	assertMemory(0);

	emalloc_memoryblockallocator_cleanup(allocator);
	free(allocator);

	return 0;
}
