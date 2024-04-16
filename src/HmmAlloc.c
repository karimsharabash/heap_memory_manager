/* 
    File : HmmAlloc.c
    Desc : Heap Memory Manager allocation functions implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

#include "HmmFree.h"
#include "Hmm.h"

//static uint32_t* heapStart = sbrk(0);

#define SBRK_INC_SIZE (4096) /* 4KB */


void* HmmAlloc(size_t reqSize)
{
    size_t* allocPtr;
    size_t* remBlockPtr;
    uint32_t sbrkSize = 0;

    size_t size = reqSize + sizeof(size_t);

    /* the block should at least equal the size of free block struct so we can add it later to the free block list*/
    if (size < (sizeof(freeBlockStruct)))
    {
        size = sizeof(freeBlockStruct);
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
            sbrkSize = SBRK_INC_SIZE* 4;
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
        
        allocPtr = increaseHeapSize(sbrkSize);

        if (allocPtr ==NULL)
        {
            return NULL;
        }

        *allocPtr = size;
        if (size < sbrkSize)
        {
            remBlockPtr = (size_t*) ((uint8_t*)allocPtr + size);
            *remBlockPtr = sbrkSize - size;
            
            addTofreeList((freeBlockStruct *)remBlockPtr);
        }
        
    }
    allocPtr = allocPtr + 1;


    return allocPtr;
}


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
        NULL;
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
    size_t oldSize = *((size_t*) ptr - 1);
    size_t neededSize;

    freeBlockStruct* nextBlock;
    freeBlockStruct* newBlock;

    if (ptr == NULL)
    {
        HmmAlloc(reqSize);
    }

    if (reqSize == 0)
    {
        HmmFree(ptr);
    }
    
    if (newSize == oldSize)
    {
        return ptr;
    }
    else if (newSize > oldSize)
    {
        /* check if the following block is free and the total size of it and the current block size can fit the new block*/
        nextBlock = (freeBlockStruct*) ((uint8_t*) ptr + oldSize);
        neededSize = newSize - oldSize;

        if ((nextBlock < (freeBlockStruct*)sbrk(0)) && (isfreeBlock(nextBlock) == 1) && (nextBlock->length > neededSize))
        {
            removeBlockFromFreeList(nextBlock);

            HmmFree((freeBlockStruct*) ((uint8_t*)nextBlock + neededSize));

            *((size_t*) ptr - 1) = newSize;
        }
        else
        {
            /* copy the content of the old block to the new block then free the old one */
            newBlock = HmmAlloc(newSize - sizeof(size_t));

            memcpy(newBlock, ptr, oldSize - sizeof(size_t));

            HmmFree(ptr);

            return newBlock;
        }
    }
    else
    {
        /* if the required size is smaller than the old size, then resize the old block and free the unneeded space */
        nextBlock = (freeBlockStruct*) ((uint8_t*) ptr + (oldSize - newSize));
        nextBlock->length = oldSize - newSize;
    
        HmmFree(nextBlock);

        *((size_t*) ptr - 1) = newSize;
    }

    return ptr;
}


size_t hmmSize(void *ptr)
{
    return (size_t) (*(size_t*) ((size_t*) ptr - 1 ) - sizeof(size_t));
}