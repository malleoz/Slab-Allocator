#include <stdio.h>
#include <stdbool.h>
#include <limits.h>    /* for CHAR_BIT */
#include <stdint.h>   /* for uint32_t */


/* Function defines */

extern int mm_init( void );
extern void *my_malloc( unsigned int size );
extern void my_free( void *buf );
extern int check_canary( void *addr );
extern int check_type( void *addr, char type );
extern int check_count( void *addr );
extern void get_stats();
extern void canary_init( void );

/* Structure definitions */
typedef uint8_t word_t;


// bitmap - track free/used entries 
typedef struct bitmap {
	word_t *map;                // map of free/used entries
	unsigned int free;          // next free index in bitmap	
	unsigned int size;          // size of the bitmap
} bitmap_t;

extern void set_bitmap_free( bitmap_t *bitmap ); // Iterate through a bitmap until the next free index

// individual slab for particular object of real_size
typedef struct slab {
	unsigned int state;             // empty, partial, full
    void *start;                    // start address - page aligned
	struct slab *next;              // next slab in cache
	struct slab *prev;              // prev slab in cache
	bitmap_t *bitmap;               // buf allocations in slab
	unsigned int ct;                // number of objs allocated in slab
	unsigned int num_objs;          // number of objs total in slab
	unsigned int obj_size;          // size of object allocs - obj, canary, padding for alignment
	unsigned int real_size;         // size of just object data
} slab_t;

extern struct slab_t *slab_page_init( size_t size );
extern int encode_ct( slab_t *slab, unsigned int index, uintptr_t *addr );
extern unsigned int get_map_count( void ); // Debug function to assert that there are no discrepencies between bitmap and slab->ct
extern unsigned int get_canary( slab_t *slab, unsigned int index );
extern void set_canary( slab_t *slab, unsigned int index );
extern void increment_free_count( slab_t *slab, unsigned int index );
extern unsigned int get_free_count( slab_t *slab, unsigned int index );

// slab cache for slab pages
typedef struct slab_cache {
	slab_t *current;                                 // reference to current free slab (empty or partial)
	unsigned int ct;                                 // number of slab (pages) in the cache
	unsigned int obj_size;                           // size of objects in cache - just object data
	void *(*malloc_fn)( slab_t *slab, void *addr );  // cache-specific malloc functionality (count, canary)
	void (*free_fn)( slab_t *slab, void *addr );     // cache-specific free functionality (count, canary)
	int(*canary_fn)( void *addr );                   // cache-specific canary check
} slab_cache_t;


// the heap - with specific slab caches
typedef struct heap {
	void *start;
	unsigned int size;
	bitmap_t *bitmap; 
	slab_cache_t *slabA;
	slab_cache_t *slabB;
	slab_cache_t *slabC;
} heap_t;


// slab object formats
typedef struct allocA {
	struct A obj;
	unsigned int canary;
	unsigned int ct;
} allocA_t;


typedef struct allocB {
	struct B obj;
	unsigned int canary;
	unsigned int ct;
} allocB_t;


typedef struct allocC {
	struct C obj;
	unsigned int canary;
	unsigned int ct;
} allocC_t;


/* Defines for bitmap ops */

enum { BITS_PER_WORD = sizeof(word_t) * CHAR_BIT };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)


extern void set_bit(word_t *words, int n);
extern void clear_bit(word_t *words, int n);
extern int get_bit(word_t *words, int n);

#define PAGE_MASK  0xFFFFFFFFFFFFF000
#define PAGE_SIZE  4096
#define HEAP_SIZE 0x100000
#define SLAB_ALLOC_ALIGN 16
#define ALIGN_MASK 0xFFFFFFFFFFFFFFF0

#define SLAB_UNASSIGNED -1
#define SLAB_EMPTY 0
#define SLAB_PARTIAL 1
#define SLAB_FULL 2

#define PAGE_TO_INDEX ( addr ) (( addr - mmheap->start ) / PAGE_SIZE )

// USE - produce 16-byte aligned pointer removing encoded count - for using pointers to access memory
#define USE( ptr ) ((void *)((unsigned long)ptr & ALIGN_MASK ))
#define USE_A( ptr ) ((struct A *) USE( ptr ))
#define USE_B( ptr ) ((struct B *) USE( ptr ))
#define USE_C( ptr ) ((struct C *) USE( ptr ))


