# Slab Allocator
A C language dynamic storage allocator.

This program implements a slab allocator, while also preventing buffer overflow, use-after-free, and typse confusion attacks. This allocator is designed to allocate objects of predetermined sizes for commonly used data structures. At its root, the slab allocator has a single heap which consists of a contiguous sequence of pages that may be used for allocating data for each structure. This allocator manages heap memory in slabs (i.e. pages).

In order to prevent attacks, a canary is implemented to detect overflows, a check_type function is implemented to assure data of the correct type is being allocated, and a free count is implemented to avoid use-after-free attacks.
