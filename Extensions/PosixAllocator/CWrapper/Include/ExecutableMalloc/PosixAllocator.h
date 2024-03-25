#ifndef EXECUTABLEMALLOC_POSIXALLOCATOR_H
#define EXECUTABLEMALLOC_POSIXALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const size_t emalloc_sizeof_posix_memoryblockallocator;

int emalloc_posix_getGranularity();
uintptr_t emalloc_posix_findUnusedMemory(uintptr_t preferredLocation, size_t tolerance, size_t numPages, bool writable);
void emalloc_posix_deallocateMemory(uintptr_t location, size_t size);

void emalloc_construct_posix_memoryblockallocator(void* posixmemoryblockallocator);

#ifdef __cplusplus
}
#endif

#endif
