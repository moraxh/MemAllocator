# MemAllocator
A memory allocator written in C, all memory is requested using sbrk(). \
You can run it with:

~~~bash
make ; bin/main
~~~
## Implementation
The **<em>program break</em>** defines the end of the process's data segment, increasing it
has the effect of allocating memory to the process; decreasing it deallocates memory. But
this has a problem:

~~~

    ┌───────────────┐
    │               │
    │    Block A    │   0x0000
    │               │
    ├───────────────┤
    │               │
    │    Block B    │   0x0010
    │               │
    ├───────────────┤
    │               │
    │      BRK      │   0x0020
    │               │
    ├───────────────┤
    │               │
    │               │   ...
    │               │
    └───────────────┘

~~~

If we want to allocate 16 bytes, it's easy: just increment the brk by 16 bytes.

~~~

    ┌───────────────┐
    │               │
    │    Block A    │   0x0000
    │               │
    ├───────────────┤
    │               │
    │    Block B    │   0x0010
    │               │
    ├───────────────┤
    │               │
    │    Block C    │   0x0020
    │               │
    ├───────────────┤
    │               │
    │      BRK      │   0x0030
    │               │
    └───────────────┘

~~~

And if we want to deallocate "Block C" just decrease the brk.

~~~

    ┌───────────────┐
    │               │
    │    Block A    │   0x0000
    │               │
    ├───────────────┤
    │               │
    │    Block B    │   0x0010
    │               │
    ├───────────────┤
    │               │
    │      BRK      │   0x0020
    │               │
    ├───────────────┤
    │               │
    │               │   ...
    │               │
    └───────────────┘

~~~

But if we want to deallocate "Block B"? We just can't decrease the brk. The "Block C" will be deallocated. \
To solve this, we will create a "FreeList" for future reallocations.

### Blocks of Memory
For reallocation purposes, we need to know the size of the block we want to save in the "Free List" Otherwise, how the \
The program will know if the block in the "FreeList" can hold the data it wants to reallocate. For this, we need a "Header" \
to save this information, so the block of memory looks like this.

~~~
    ┌──────────────────┬────────┐
    │  Header(size_t)  │  Data  │
    └──────────────────┴────────┘
~~~

### FreeList
Ok. How we create the famous "Free List" and, more importantly, where it'll be.\
For performance reasons, the size of an allocated block needs to be alligned to 32 or 64 bits. So the minimum size of a block
will be 16 bytes ( size_t(8 bytes) ). We are going to use the space where the data was to store a pointer to the next "Free Block".

~~~
    head(Header *)
            │
            v
    ┌──────────────────┬──────────────────────┐
    │  Header(size_t)  │  FreeList(Header *)  │
    └──────────────────┴──────────────────────┘
            ┌──────────────────────┘
            v
    ┌──────────────────┬──────────────────────┐
    │  Header(size_t)  │  FreeList(Header *)  │
    └──────────────────┴──────────────────────┘
            ┌──────────────────────┘
            v
    ┌──────────────────┬──────────────────────┐
    │  Header(size_t)  │  FreeList(Header *)  │
    └──────────────────┴──────────────────────┘
            ┌──────────────────────┘
            v
          NULL
~~~

## TODO
- [ ] Implement it using mmap
