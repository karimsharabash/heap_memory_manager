# Heap Memory Manager
Custom Implementation of heap functions family (Malloc, Free, Calloc, Realloc) with best-fit block algorithm.
## HMM Concept

- To allocate memory using the HMM, first check if there is an available memory block in the Free Block List then use a block or part of a block from it, if not then reserve memory space from the kernel using "sbrk" and take the needed part from the new reserved block and put the rest as a block in the Free block List, but it adds an extra 8 bytes before the data block to hold the length of this block.
- To free a memory block, using the attached length for the block, the block should be put in order in the Free Block List to make the HMM work in the best-fit block algorithm, but first, it checks whether the following block and the preceding blocks are free then it's better to concatenate the blocks then add the new concatenated block to the Free Block List.
- Additional Featurs:
    - Detect the double free, by searching for the block in the freeBlockList.
    - New API to get the size of the allocated block.

## Implementation
These four functions are defined with weak definitions, to provide a custom definition for them, define the functions and call from them the alternative functions, this is implemented in heapWrappers.c.
- hmmAlloc (void* HmmAlloc(size_t reqSize)): the alternative function for malloc, it dynamically allocates memory space with the required size.
- hmmFree (void HmmFree(void *ptr)): the alternative function for free, it frees the dynamically allocated memory and adds it to the Free Block List.
- hmmCalloc (void* hmmCalloc(size_t nmemb, size_t size)): the alternative function for calloc, it dynamically allocates memory space as an array, it takes the elements count and size, allocates it, and initializes it with zeros.
- hmmReAlloc (void* HmmReAlloc(void *ptr, size_t reqSize)): the alternative function for realloc, it resizes dynamically allocated memory with the new requested size, if the requested size is zero then this means to free this memory block, if the passed pointer is NULL then it is a call to malloc.

 Additionally there is an extra function to get the size of the allocated block
- hmmSize (size_t hmmSize(void *ptr)) : returns the size of the allocated block, without the added section used to hold the block length

#### Tests
1) Test the utilization of the reserved memory from the kernel by doing a lot of allocs and frees for the whole reserved block and check the HMM didn't reserve more memory and utilize what it's already reserved
2) Test the writing and reading of contents in the allocated blocks by the HMM
3) Test allocating and freeing small blocks( lower than the size of 2 pointers)
4) Testing Realloc function 
5) Testing Calloc function

## how to use
- ```make test``` : to build the HMM and the tests included and will generate elf file (./build/hmm_test)
- ```make static_lib``` : to build the HMM and generate a static library under build/lib
- ```make clean``` : to clean all the generated files
