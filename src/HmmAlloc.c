/* 
    File : HmmAlloc.c
    Desc : Heap Memory Manager allocation functions implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "common.h"

#include "HmmFree.h"
#include "Hmm.h"

static size_t* heapEnd;

/* 100KB  + 1k for the libc +  the length block for this block*/
#define SBRK_INC_SIZE_TESTING ( (100 * 1024) + 1024 + sizeof(freeBlockStruct))

#define SBRK_INC_SIZE       (400 * 1024)

/* This function overwrites malloc function,
    1) it searches for a free block from the freeBlocksList
    2) if there is available free blocks then reserve more memory from the kernel
    3) if the found block is bigger than the needed then take the needed part only add the rest of the block to freeBLocksList
    4) return the block address in success and NUll in failure*/
void* HmmAlloc(size_t reqSize)
{
    size_t* allocPtr;
    size_t* remBlockPtr;
    uint32_t sbrkSize = 0;
    size_t size = reqSize + sizeof(size_t);
    uint32_t remSizeToAlign;

    if (reqSize == 0)
    {
        return NULL;
    }

    /* the block should at least equal the size of free block struct so we can add it later to the free block list*/
    if (size < (sizeof(freeBlockStruct)))
    {
        size = sizeof(freeBlockStruct);
    }

    remSizeToAlign = size % sizeof(size_t);

    if (remSizeToAlign != 0)
    {
        size += (sizeof(size_t) - remSizeToAlign);
    }

    // search for the best fit block in the free blocks list if there is no available block, increase the heap size again
    allocPtr = getFromfreeList(size);
    
    if (allocPtr == NULL)
    {
        
        /* if there is no avialable blocks on the free list, get new memory from the kernel*/
        if (size < SBRK_INC_SIZE)
        {
            sbrkSize = SBRK_INC_SIZE;
        }
        else if (size < (SBRK_INC_SIZE * 2))
        {
            sbrkSize = SBRK_INC_SIZE * 2;
        }
        else if (size < (SBRK_INC_SIZE * 4))
        {
            sbrkSize = SBRK_INC_SIZE * 4;
        }
        else if (size < (SBRK_INC_SIZE * 8))
        {
            sbrkSize = SBRK_INC_SIZE * 8;
        }
        else
        {
            /*exceeded the MAX */
            return NULL;
        }
        
        allocPtr = sbrk(sbrkSize);
        
        if (allocPtr == NULL)
        {
            return NULL;
        }
        heapEnd = (size_t*) ((size_t)allocPtr + sbrkSize);
        *allocPtr = size;
        if (size < sbrkSize)
        {
            remBlockPtr = (size_t*) ((size_t)allocPtr + size);
            *remBlockPtr = sbrkSize - size;
            
            HmmFree((freeBlockStruct *)((size_t)remBlockPtr + sizeof(size_t)));
        }
    }
    allocPtr = (size_t*) ((size_t)allocPtr + sizeof(size_t));

    ASSERT(((size_t)allocPtr % sizeof(size_t)) == 0 , "Malloc reserved unaligned pointer\n");
    return allocPtr;
}

/* This function overwrites calloc function, 
    it takes the sizes as elements count and element size and allocates the total size using malloc
    then initialize the whole block with zeros */
void* hmmCalloc(size_t nmemb, size_t size)
{
    size_t totalSize = nmemb * size;
    if (totalSize == 0)
    {
        return NULL;
    }

    void* retPtr = HmmAlloc(totalSize);
    if (retPtr == NULL)
    {
        return NULL;
    }

    memset(retPtr, 0, totalSize);
    return retPtr;

}

/*  The realloc() function changes the size of the memory block pointed to by ptr to size bytes.
    The contents will be unchanged, If the new size is larger than the old size, the added memory will not be initialized.
    If ptr is NULL, then the call is equivalent to malloc(size), for all values of size; if size is equal to zero, 
    and ptr is not NULL, then the call is equivalent to free(ptr). 
    Unless ptr is NULL, it must have been returned by an earlier call to malloc(), calloc(), or re‐alloc().
    If the area pointed to was moved, a free(ptr) is done. */

void* hmmReAlloc(void *ptr, size_t reqSize)
{
    size_t newSize = reqSize + sizeof(size_t);
    size_t oldSize ;
    size_t neededSize;

    freeBlockStruct* nextBlock;
    freeBlockStruct* newBlock;
    if (ptr == NULL)
    {
        return HmmAlloc(reqSize);
    }

    oldSize = *((size_t*) ptr - 1);
    if (reqSize == 0)
    {
        HmmFree(ptr);
        return NULL;
    }
    
    if (newSize == oldSize)
    {
        return ptr;
    }
    else if (newSize > oldSize)
    {
        /* check if the following block is free and the total size of it and the current block size can fit the new block*/
        nextBlock = (freeBlockStruct*) ((size_t) ptr + oldSize);
        neededSize = newSize - oldSize;


        if ((nextBlock < (freeBlockStruct*)heapEnd) && (isfreeBlock(nextBlock) == TRUE) && (nextBlock->length > neededSize))
        {
            removeBlockFromFreeList(nextBlock);

            HmmFree((freeBlockStruct*) ((size_t)nextBlock + neededSize));

            *((size_t*) ptr - 1) = newSize;
        }
        else
        {
            /* copy the content of the old block to the new block then free the old one */
            newBlock = HmmAlloc(reqSize);
            memcpy(newBlock, ptr, oldSize - sizeof(size_t));

            HmmFree(ptr);

            return newBlock;
        }
    }
    else
    {
        /* if the required size is smaller than the old size, then resize the old block and free the unneeded space */
        if ( oldSize - newSize > sizeof(freeBlockStruct))
        {
            nextBlock = (freeBlockStruct*) ((size_t) ptr + (newSize));
            nextBlock->length = oldSize - newSize;
        
            HmmFree((void*)((size_t)nextBlock + sizeof(size_t)));

            *((size_t*) ptr - 1) = newSize;
        }
    }
    ASSERT(((size_t)ptr % sizeof(size_t)) == 0, "Realloc reserved unaligned pointer\n");
    return ptr;
}

/* return the size of the allocated block without the size of the length variable area*/
size_t hmmSize(void *ptr)
{
    return (size_t) (*(size_t*) ((size_t*) ptr - 1 ) - sizeof(size_t));
}
