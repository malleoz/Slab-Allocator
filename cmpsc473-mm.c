
/**********************************************************************

   File          : cmpsc473-mm.c

   Description   : Slab allocation and defenses

***********************************************************************/
/**********************************************************************
Copyright (c) 2019 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <time.h> 
#include "cmpsc473-format-134.h"
#include "cmpsc473-mm.h"
#include <malloc.h>

/* Globals */
heap_t *mmheap;
unsigned int canary;

/* Defines */
#define FREE_ADDR( slab ) ( (unsigned long)slab->start + ( slab->obj_size * slab->bitmap->free ))
#define OBJ_PTR( slab, index ) (slab->start + (index * slab->obj_size))
#define SLAB_PTR( index ) (mmheap->start + ((index + 1) * PAGE_SIZE) - sizeof(slab_t))

/**********************************************************************

    Function    : set_canary/get_canary
    Description : Given a slab, create an allocX struct pointer and set the canary for the object at index
    Inputs      : slab, index

***********************************************************************/

void set_canary( slab_t *slab, unsigned int index )
{
    switch (slab->real_size) {
        case sizeof(struct A):
            ;
            allocA_t *objA = (allocA_t *) OBJ_PTR( slab, index );
            objA->canary = canary;
            break;
        case sizeof(struct B):
            ;
            allocB_t *objB = (allocB_t *) OBJ_PTR( slab, index );
            objB->canary = canary;
            break;
        case sizeof(struct C):
            ;
            allocC_t *objC = (allocC_t *) OBJ_PTR( slab, index );
            objC->canary = canary;
            break;
    }

    return;
}

unsigned int get_canary( slab_t *slab, unsigned int index )
{
    switch(slab->real_size) {
        case sizeof(struct A):
            ;
            allocA_t *objA = (allocA_t *) OBJ_PTR( slab, index );
            return (*objA).canary;
        case sizeof(struct B):
            ;
            allocB_t *objB = (allocB_t *) OBJ_PTR( slab, index );
            return (*objB).canary;
        case sizeof(struct C):
            ;
            allocC_t *objC = (allocC_t *) OBJ_PTR( slab, index );
            return (*objC).canary;
    }

    return 0;
}

/**********************************************************************

    Function    : increment_free_count / get_free_count
    Description : given a slab and an index, increment/retrieve the object's free count
    Inputs      : slab, index
    Outputs     : void / free_count

***********************************************************************/

void increment_free_count( slab_t *slab, unsigned int index )
{
    switch(slab->real_size) {
        case sizeof(struct A):
            ;
            allocA_t *objA = (allocA_t *) OBJ_PTR( slab, index );
            objA->ct++;
            break;
        case sizeof(struct B):
            ;
            allocB_t *objB = (allocB_t *) OBJ_PTR( slab, index );
            objB->ct++;
            break;
        case sizeof(struct C):
            ;
            allocC_t *objC = (allocC_t *) OBJ_PTR( slab, index );
            objC->ct++;
            break;
    }

    return;
}

unsigned int get_free_count( slab_t *slab, unsigned int index )
{
    switch (slab->real_size) {
        case sizeof(struct A):
            ;
            allocA_t *objA = (allocA_t *) OBJ_PTR( slab, index );
            return (*objA).ct;
        case sizeof(struct B):
            ;
            allocB_t *objB = (allocB_t *) OBJ_PTR( slab, index );
            return (*objB).ct;
        case sizeof(struct C):
            ;
            allocC_t *objC = (allocC_t *) OBJ_PTR( slab, index );
            return (*objC).ct;
    }

    return 0;
}

/**********************************************************************

    Function    : set_bitmap_free
    Description : Given a bitmap, iterate through the map and find and
                  set the first free index
    Inputs      : bitmap

***********************************************************************/

void set_bitmap_free (bitmap_t *bitmap)
{
    int i;
    for (i = 0; i < bitmap->size; i++) {
        if (!get_bit(bitmap->map, i)) {
            bitmap->free = i;
            break;
        }
    }

    return;
}

/**********************************************************************

    Function    : slab_page_init
    Description : Initialize and return an address to a new slab page
    Inputs      : size - size of structX
    Outputs     : addr - address of the new slab

***********************************************************************/

struct slab_t *slab_page_init( size_t size)
{
    // First retrieve the index in the heap for the new page
    unsigned int heap_index = (*mmheap->bitmap).free;

    // Update the heap bitmap
    set_bit(mmheap->bitmap->map, heap_index);

    // Now that this index will be used, we need to increment free to the next index
    set_bitmap_free( mmheap->bitmap );

    // Create a new slab at this index in the heap
    // The location is as follows:
    // Take the start of the heap, go to the heap_index+1th location, and subtract the sizeof the slab size.
    // This will therefore be the location of the slab metadata for the heap_indexth slab page.
    slab_t *new_slab = (slab_t *) SLAB_PTR( heap_index );

    // Link the list based off what size is passed through
    // Also set the obj_size... Must round up to multiple of SLAB_ALLOC_ALIGN(16)
    slab_cache_t *slabX;
    switch (size) {
        case sizeof(struct A):
            slabX = mmheap->slabA;
            new_slab->obj_size = sizeof(allocA_t);
            if (new_slab->obj_size % SLAB_ALLOC_ALIGN == 0) {
                break;
            }
            new_slab->obj_size += SLAB_ALLOC_ALIGN - (sizeof(allocA_t) % SLAB_ALLOC_ALIGN);
            break;
        case sizeof(struct B):
            slabX = mmheap->slabB;
            new_slab->obj_size = sizeof(allocB_t);
            if (new_slab->obj_size % SLAB_ALLOC_ALIGN == 0) {
                break;
            }
            new_slab->obj_size += SLAB_ALLOC_ALIGN - (sizeof(allocB_t) % SLAB_ALLOC_ALIGN);
            break;
        case sizeof(struct C):
            slabX = mmheap->slabC;
            new_slab->obj_size = sizeof(allocC_t);
            if (new_slab->obj_size % SLAB_ALLOC_ALIGN == 0) {
                break;
            }
            new_slab->obj_size += SLAB_ALLOC_ALIGN - (sizeof(allocC_t) % SLAB_ALLOC_ALIGN);
            break;
    }

    // First, take the current->prev = new_slab
    // Second, take the new_slab->next = slabA->current
    if (slabX->current != NULL) {
        if (slabX->current->prev != NULL) {
            slabX->current->prev->next = new_slab;
        }
        slabX->current->prev = new_slab;
        new_slab->next = slabX->current;
        // Also handle the case where current->next would be new_slab (if slabX->ct == 1)
        if ((*slabX).ct == 1) {
            slabX->current->next = new_slab;
            new_slab->prev = slabX->current;
        }
        else {
            // We still need to update the previous_slab->new_slab, so loop through all slabs until previous_slab->next == new_slab
            slab_t *previous_slab = slabX->current;
            while (previous_slab->next != new_slab) {
                previous_slab = previous_slab->next;
            }

            new_slab->prev = previous_slab;
        }

    }
    else {
        slabX->current = new_slab;
        new_slab->prev = NULL;
        new_slab->next = NULL;
    }

    // Increment the number of slabs in the cache
    slabX->ct++;

    // Set some more initial values
    new_slab->ct = 0; // 0 objects allocated
    new_slab->real_size = (*slabX).obj_size; // Size of struct A/B/C

    // num_objs has to factor in 16 byte alignment... So use obj_size to allocate enough memory for each object
    new_slab->num_objs = (PAGE_SIZE - sizeof(slab_t)) / (*new_slab).obj_size;

    new_slab->bitmap = (bitmap_t *) malloc(sizeof(bitmap_t));
    new_slab->bitmap->map = (word_t *) calloc(((*new_slab).num_objs/8)+1, sizeof(word_t)); // Initialize a bitmap with all 0's... 1 byte can represent 8 indeces, so allocate the nearest multiple of 8, rounded up
    new_slab->bitmap->free = 0; // Second index of bitmap is free
    new_slab->bitmap->size = (*new_slab).num_objs; // Simply the number of indeces we want to represent
    new_slab->state = SLAB_PARTIAL; // Specify that the slab is empty
    new_slab->start = (*mmheap).start + (PAGE_SIZE * heap_index); // Finds the starting address of this slab's data
    
    return (void *) new_slab;
}

/**********************************************************************

    Function    : encode_ct
    Description : Stores the lower 4 bits of the free count in the return address
    Inputs      : Takes in the entire slab, the obj index, and the address of the object in question

***********************************************************************/

int encode_ct (slab_t *slab, unsigned int index, uintptr_t *addr)
{
    unsigned int ct;
    ct = get_free_count(slab, index);

    ct = ct & 0xF; // Keep only lower 4 bits
    uintptr_t addrintptr = *addr;

    // Now that I have the pointer value, do some operations to insert ct into the pointer value
    // Remove the lowest 4 bits
    addrintptr = addrintptr >> 4;
    addrintptr = addrintptr << 4;
    addrintptr = addrintptr | ct; // Set bits from ct
    *addr = addrintptr;

    return 0;
}

/**********************************************************************

    Function    : mm_init
    Description : Initialize slab allocation
    Inputs      : void
    Outputs     : 0 if success, -1 on error

***********************************************************************/

int mm_init( void )
{
	srand(time(0));
    
    mmheap = (heap_t *)malloc( sizeof(heap_t) );
	if ( !mmheap ) return -1;

	// TASK 2: Initialize heap memory (using regular 'malloc') and 
	//   heap data structures in prep for malloc/free
    mmheap->start = (void *) calloc(1, HEAP_SIZE);
    // Find the nearest multiple of 4096 so that heap starts page-aligned
    uintptr_t heap_start = (uintptr_t) mmheap->start;
    heap_start += PAGE_SIZE - (heap_start % PAGE_SIZE);
    mmheap->start = (void *) heap_start;
    mmheap->size = HEAP_SIZE;
    // Each slab page is 4KB, so the number of pages can be found by taking HEAP_SIZE / PAGE_SIZE to get 0x100
    mmheap->bitmap = (bitmap_t *) malloc(sizeof(bitmap_t));
    mmheap->bitmap->map = (word_t *) calloc((HEAP_SIZE / PAGE_SIZE) / 8, sizeof(word_t));
    mmheap->bitmap->free = 0;
    mmheap->bitmap->size = ((HEAP_SIZE / PAGE_SIZE) / sizeof(word_t))-1; // -1 because we had to page align the start of the heap... So we miss a page
    mmheap->slabA = (slab_cache_t *) calloc(1, sizeof(slab_cache_t));
    mmheap->slabA->obj_size = sizeof(struct A);
    mmheap->slabB = (slab_cache_t *) calloc(1, sizeof(slab_cache_t));
    mmheap->slabB->obj_size = sizeof(struct B);
    mmheap->slabC = (slab_cache_t *) calloc(1, sizeof(slab_cache_t));
    mmheap->slabC->obj_size = sizeof(struct C);
    // heap_t data structure is fully initialized

    // initialize canary
	canary_init();

	return 0;
}


/**********************************************************************

    Function    : my_malloc
    Description : Allocate from slabs
    Inputs      : size: amount of memory to allocate
    Outputs     : address if success, NULL on error

***********************************************************************/

void *my_malloc( unsigned int size )
{
	// TASK 2: implement malloc function for slab allocator

    // We will need to reference the slab caches to determine where we can allocate an object.
    slab_cache_t *slabX;
    // We first have to determine which structure we need to allocate.
    // To do so, use a switch case
    switch( size )
    {
        case sizeof(struct A): // We need to allocate a struct A
            slabX = mmheap->slabA;
            break;
        case sizeof(struct B):
            slabX = mmheap->slabB;
            break;
        case sizeof(struct C):
            slabX = mmheap->slabC;
            break;
    }

    // I don't necessarily know which cache we're now looking at, but either way I can just determine if I can allocate the object through *structX

    // Go through the cache to try and find an open cache
    slab_t *free_slab = slabX->current;

    while (free_slab != NULL && free_slab->state == SLAB_FULL) {
        free_slab = free_slab->next;

        if (free_slab == slabX->current) {
            // We've gone full circle!... There are no partial slabs so exit out of the while statement
            free_slab = NULL;
            break;
        }
    }
    
    // From here, there are two cases: 1) There is room in the last slab. 2) The last slab is full and I need to create and link a new slab.
    // Case 1: We need to create a new slab
    if (free_slab == NULL) {
        free_slab = (slab_t *) slab_page_init( (*slabX).obj_size );

        // Update bitmap
        set_bit(free_slab->bitmap->map, 0);
        free_slab->bitmap->free = 1;
        free_slab->ct++;
        // Encode ct
        uintptr_t addrintptr = (uintptr_t) free_slab->start;
        encode_ct( free_slab, 0, &addrintptr);
        set_canary(free_slab, 0);
        return (void *) addrintptr;
    }

    // Case 2: There is room in free_slab
    // Update bitmap
    set_bit(free_slab->bitmap->map, (*free_slab->bitmap).free);

    // Check to see if slab is now full
    // num_objs-1 because I haven't incremented ct yet
    if ((*free_slab).ct == (*free_slab).num_objs-1) {
        free_slab->state = SLAB_FULL;
    }

    uintptr_t addrintptr = (uintptr_t) FREE_ADDR( free_slab );
    encode_ct(free_slab, (*free_slab->bitmap).free, &addrintptr);
    set_canary(free_slab, (*free_slab->bitmap).free);

    free_slab->ct++;

    // Update the bitmap's free index
    set_bitmap_free( free_slab->bitmap );

    return (void *) addrintptr;
} 

/**********************************************************************

    Function    : my_free
    Description : deallocate from slabs
    Inputs      : buf: full pointer (with counter) to deallocate
    Outputs     : address if success, NULL on error

***********************************************************************/

void my_free( void *buf )
{
	// TASK 2: Implement free function for slab allocator
    // First, let's verify that this pointer exists within the heap
    if (buf < (*mmheap).start || buf > (*mmheap).start + HEAP_SIZE) {
        buf = NULL;
        return;
    }
    
    // Now, we need to figure out what structX is stored here.
    // First, given an address, we can determine what index in the heap it exists at.
    unsigned int heap_index = (buf - (*mmheap).start) / PAGE_SIZE; // Find the offset from the start of the heap and find the nearest multiple of PAGE_SIZE
    
    // Now that we know the index, we can go to the end of that page and pull the slab_t metadata
    slab_t *slab = (slab_t *) SLAB_PTR( heap_index );

    // Verify that the object that the buffer points to is used
    unsigned int obj_index = (buf - (*slab).start) / (*slab).obj_size;

    if (!get_bit(slab->bitmap->map, obj_index)) {
        // The object is not used!
        buf = NULL;
        return;
    }
    
    // Okay, now that we verified that this address can be freed, free it!
    // We first want to lower the number of objects in the slab
    slab->ct--;
    // Also update the bitmap
    clear_bit(slab->bitmap->map, obj_index);

    // Update the slab->bitmap->free
    set_bitmap_free( slab->bitmap );
    
    // Okay, now update the slab_t state to reflect this change
    if (slab->state == SLAB_FULL) {
        slab->state = SLAB_PARTIAL;
    }

    // Increment the free count at the object's location
    increment_free_count( slab, obj_index );

	return;
}


/**********************************************************************

    Function    : canary_init
    Description : Generate random number for canary - fresh each time 
    Inputs      : 
    Outputs     : void

***********************************************************************/

void canary_init( void )
{ 
	// This program will create different sequence of  
	// random numbers on every program run  
  
	canary = rand(); 
	printf("canary is %d\n", canary );
    
    // I also need to set the canary value for each allocated object
    // Loop through all pages
    // I first need to find out how many pages exist
    unsigned int pages = get_map_count();
    int i,j;
    for (i = 0; i < pages; i++) {
        slab_t *slab = mmheap->start + ((i+1) * PAGE_SIZE) - sizeof(slab_t);

        // Loop through all objects in the slab
        // Because the objects may not be in contiguous locations, reference each bit of the bitmap
        for (j = 0; j < slab->bitmap->size; j++) {
            // If the bit at index j is 1, then this object has been allocated.
            if (get_bit(slab->bitmap->map, j)) {
                set_canary(slab, j);
            }
        }
    }

    // Canaries are now set!
    return;
} 


/**********************************************************************

    Function    : check_canary
    Description : Find canary for obj and check against program canary
    Inputs      : addr: address of object
    Outputs     : 0 for success, -1 for failure

***********************************************************************/

int check_canary( void *addr )
{
	// TASK 3: Implement canary defense
    // Find the heap index based off of the address
    unsigned int heap_index = (addr - mmheap->start) / PAGE_SIZE;
    // Next, look at slab_t metadata for that heap index
    slab_t *slab = (slab_t *) SLAB_PTR( heap_index );
    // Determine object index based off address
    unsigned int obj_index = (addr - slab->start) / slab->obj_size;

    if (canary != get_canary(slab, obj_index) ) {
        return -1;
    }

	return 0;
}


/**********************************************************************

    Function    : check_type
    Description : Verify type requested complies with object 
    Inputs      : addr: address of object
                  type: type requested
    Outputs     : 0 on success, -1 on failure

***********************************************************************/

int check_type( void *addr, char type ) 
{
	// TASK 3: Implement type confusion defense
    // Find the heap index
    unsigned int heap_index = (addr - mmheap->start) / PAGE_SIZE;
    // Next, look at slab_t metadata for that heap index
    slab_t *slab = (slab_t *) SLAB_PTR( heap_index );

    switch (type) {
        case 'A':
            if (slab->real_size == sizeof(struct A)) {
                return 0;
            }
            break;
        case 'B':
            if (slab->real_size == sizeof(struct B)) {
                return 0;
            }
            break;
        case 'C':
            if (slab->real_size == sizeof(struct C)) {
                return 0;
            }
            break;
    }

    return -1;
}


/**********************************************************************

    Function    : check_count
    Description : Verify that pointer count equals object count
    Inputs      : addr: address of pointer (must include metadata in pointer)
    Outputs     : 0 on success, or -1 on failure

***********************************************************************/

int check_count( void *addr ) 
{
	// TASK 3: Implement free count defense

    // First determine the object type
    uintptr_t addrintptr = (uintptr_t) addr;
    unsigned int heap_index = (addr - mmheap->start) / PAGE_SIZE;
    slab_t *slab = (slab_t *) SLAB_PTR( heap_index );

    unsigned int ct_mask = 0xF;
    unsigned int ct = addrintptr & ct_mask;

    // Verify that the ct matches the lower 4 bits of the free count in memory
    unsigned int obj_index = (addr - slab->start) / slab->obj_size;
    unsigned int mem_ct = get_free_count( slab, obj_index );
    
    if (ct != mem_ct) {
        return -1;
    }

	return 0;
}



/**********************************************************************

    Function    : set/clear/get_bit
    Description : Bit manipulation functions
    Inputs      : words: bitmap 
                  n: index in bitmap
    Outputs     : cache if success, or NULL on failure

***********************************************************************/

void set_bit(word_t *words, int n) {
	words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void clear_bit(word_t *words, int n) {
	words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

int get_bit(word_t *words, int n) {
	word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
	return bit != 0;
}


/**********************************************************************

    Function    : print_cache_slabs
    Description : Print current slab list of cache
    Inputs      : cache: slab cache
    Outputs     : void

***********************************************************************/

int print_cache_slabs( slab_cache_t *cache )
{
	slab_t *slab = cache->current;
	int count=0;
	printf("Cache %p has %d slabs\n", cache, cache->ct );
	do {
		printf("slab: %p; prev: %p; next: %p\n", slab, slab->prev, slab->next );
		count+=1;
		slab = slab->next;
	} while ( slab != cache->current );
	return count;
}


/**********************************************************************

    Function    : get_stats/slab_counts
    Description : Print stats on slab page and object allocations 
    Outputs     : void

***********************************************************************/

void slab_counts( slab_cache_t *cache, unsigned int *slab_count, unsigned int *object_count ){
	slab_t *slab = cache->current;
	int i;
	unsigned int orig_count;
	
	*slab_count = 0;
	*object_count = 0;
    
    if (slab == NULL) {
        return;
    }

	do {
		(*slab_count)++;

		// set orig to test objects per slab
		orig_count = *object_count;

		// count objects in slab
		for ( i = 0; i < slab->bitmap->size ; i++ ) {
			if ( get_bit( slab->bitmap->map, i )) {
				(*object_count)++;
			}
		}

		if (( *object_count - orig_count ) != slab->ct ) {
			printf("*** Discrepancy in object count in slab %p: %d:%d\n", 
			       slab, *object_count - orig_count, slab->ct);
		}
			

		slab = slab->next;
	} while ( slab != cache->current );

	if ( *slab_count != cache->ct ) {
		printf("*** Discrepancy in slab page count in cache %p: %d:%d\n", cache, *slab_count, cache->ct);
	}
}

void get_stats(){
	unsigned int slab_count, object_count;

	printf("--- Cache A ---\n");
	slab_counts( mmheap->slabA, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache A: %d:%d\n", slab_count, object_count );
	printf("--- Cache B ---\n");
	slab_counts( mmheap->slabB, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache B: %d:%d\n", slab_count, object_count );
	printf("--- Cache C ---\n");
	slab_counts( mmheap->slabC, &slab_count, &object_count );
	printf("Number of slab pages:objects in Cache C: %d:%d\n", slab_count, object_count );
}

/**********************************************************************

    Function    : get_heap_map_count
    Description : Determines how many pages have been allocated in the heap
    Outputs     : Number of pages allocated

**********************************************************************/

unsigned int get_map_count( void )
{
    unsigned int map_count = 0;
    int i;
    for (i=0; i < mmheap->bitmap->size; i++) {
        if (get_bit(mmheap->bitmap->map, i)) {
            map_count++;
        }
    }
    
    return map_count;
}
