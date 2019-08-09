#include "tracenew.h"

#ifdef USE_DEBUG_NEW

const char* __file__ = "unknown";
size_t __line__ = 0;

void* operator new(size_t size) {
    void *ptr = malloc(size);
    record_alloc(ptr,__file__,__line__);
    __file__ = "unknown";
    __line__ = 0;
    return ptr;
}

void delete(void *ptr)
{
   unrecord_alloc(ptr);
   free(ptr);
}

#endif

