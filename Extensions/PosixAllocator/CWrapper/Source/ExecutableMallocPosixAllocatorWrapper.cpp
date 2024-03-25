#include "ExecutableMalloc/PosixAllocator.h"
#include "ExecutableMalloc/PosixAllocator.hpp"

#include <new>

using namespace ExecutableMalloc;

extern "C" {

const size_t emalloc_sizeof_posix_memoryblockallocator = sizeof(PosixMemoryBlockAllocator);

int emalloc_posix_getGranularity() {
	return posixGetGranularity();
}
uintptr_t emalloc_posix_findUnusedMemory(uintptr_t preferredLocation, size_t tolerance, size_t numPages, bool writable) {
	return posixFindUnusedMemory(preferredLocation, tolerance, numPages, writable);
}
void emalloc_posix_deallocateMemory(uintptr_t location, size_t size) {
	return posixDeallocateMemory(location, size);
}

void emalloc_construct_posix_memoryblockallocator(void* posixmemoryblockallocator) {
	new (posixmemoryblockallocator) PosixMemoryBlockAllocator{};
}

}
