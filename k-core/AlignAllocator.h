#ifndef ALIGNED_ALLOCATOR
#define ALIGNED_ALLOCATOR

#include <EASTL/allocator.h>

#define UDIV_UP(a, b) (((a) + (b) - 1) / (b))
// aligning an integer to the next boundary
#define ALIGN_UP(a, b) (UDIV_UP(a, b) * (b))

// for GCC, make your struct "packed"... non padded.
#define packed_data __attribute__((__packed__))


inline void* align_malloc( size_t align, size_t size )
{
   // we don't know what malloc will return, so have to allocate
   // enough padding so that we can align the data in the worst case offset
   size_t align_minus_1 = align - 1;
   uintptr_t mask = ~(uintptr_t)align_minus_1;
   assert((align & (align_minus_1)) == 0);
   size_t bytes_to_allocate = size + align_minus_1 + sizeof(void*);
   void *mem = malloc( bytes_to_allocate );
   if (!mem)
      return mem;
#if _DEBUG
   // zero out the (potential) header so debugging is easier
   memset( (char*)mem, 0, align_minus_1 + sizeof(void*) );
#endif
   void *ptr = (void *)(((size_t)mem + sizeof(void*) + align_minus_1) &
                        (size_t)mask);
   assert( ALIGN_UP(((size_t)mem+sizeof(size_t)), 16) == (size_t)ptr );
   ((void**)ptr)[-1] = mem;
   /*printf("mask:%x mem:0x%lx nxt:0x%lx ptr:0x%lx (%db diff) %lub total %lub size %lub overhead\n",
          (uint16_t)mask, (size_t)mem, ALIGN_UP(((size_t)mem+sizeof(size_t)), 16), (size_t)ptr,
          (int)((size_t)ptr-(size_t)mem),
          bytes_to_allocate, size, bytes_to_allocate-size );
   */
   return ptr;
}

inline void align_free( void* ptr )
{
   //printf( "free: %lx\n", (size_t)ptr );
   if (ptr)
      free(((void**)ptr)[-1]);
}

// eastl::map<std::string, vmMatrix4, eastl::less<std::string>, align_allocator<16> >
// eastl::map<std::string, vmVec4, eastl::less<std::string>, align_allocator<16> >
// eastl::map<std::string, float, eastl::less<std::string>, align_allocator<16> >
template <unsigned A>
class align_allocator : public eastl::allocator
{
public:
   EASTL_ALLOCATOR_EXPLICIT align_allocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) : eastl::allocator( pName ) {}
   align_allocator(const allocator& x) : eastl::allocator( x ) {}
   align_allocator(const allocator& x, const char* pName) : eastl::allocator( x, pName ) {}

   align_allocator& operator=(const align_allocator& x)
   {
      eastl::allocator::operator=( x );
      return *this;
   }
   void* allocate(size_t n, int flags = 0)
   {
      return align_malloc( A, n );
   }
   void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
   {
      assert( offset == 0 );
      assert( A < alignment &&
              alignment == (sqrtf(alignment) * sqrtf(alignment)));
      return align_malloc( alignment, n );
   }
   void  deallocate(void* p, size_t n)
   {
      align_free( p );
   }

   static void test()
   {
      printf( "Running aligned malloc/free tests... " );
      fflush( stdout );
      for (int x = 0; x < 1; ++x)
      {
         void* ptr = align_malloc( A, 1 );
         void* ptr1 = align_malloc( A, 1 );
         void* ptr2 = align_malloc( A, 1 );
         void* ptr3 = align_malloc( A, 78 );
         void* ptr4 = align_malloc( A, 456 );
         align_free( ptr2 );
         align_free( ptr3 );
         align_free( ptr );
         align_free( ptr1 );
         align_free( ptr4 );
      }
      printf( "done.\n" );
   }
};

#endif

