/* 
    File : HmmFree.c
    Desc : Heap Memory Manager Free implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "HmmFree.h"

static freeBlockStruct* freeBlocksListHead = NULL;

void addTofreeList(freeBlockStruct* newBlock)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;
    if (tmpBlock == NULL)
    {
        freeBlocksListHead = newBlock;
        newBlock->next = NULL;
        newBlock->prev = NULL;
    }
    else
    {
        while(tmpBlock != NULL)
        {
            /* add it on order to make the HMM works in best fit algorithm */
            if (newBlock->length < tmpBlock->length )
            {
                /* insert here */
                newBlock->prev = tmpBlock->prev;
                newBlock->next = tmpBlock;
                tmpBlock->prev = newBlock;

                break;
            }
            else
            {
                if (tmpBlock->next == NULL)
                {
                    /* insert at the end */
                    tmpBlock->next = newBlock;
                    newBlock->next = NULL;
                    newBlock->prev = tmpBlock;
                    break;
                }
                else
                {
                    tmpBlock = tmpBlock->next;
                }
            }
        }
    }
    
}

uint32_t isfreeBlock(freeBlockStruct* block)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;

    if (block == NULL)
    {
        return 0;
    }

    while(tmpBlock != NULL)
    {
        if ((block->prev == tmpBlock->prev) && (block->length == tmpBlock->length))
        {
            return 1;

        }
        else
        {
            tmpBlock = tmpBlock->next;
        }
    }

    return 0;
}

void removeBlockFromFreeList(freeBlockStruct* block)
{
    if (block == freeBlocksListHead)
    {
        freeBlocksListHead = block->next;
         if (block->next != NULL)
         {
             block->next->prev = NULL;
         }
        
    }

    /* remove the block from the free blocks list and make a new block from the remaining size and add it to the list */
    if (block->next != NULL)
    {
        block->next->prev = block->prev;
    }

    if (block->prev != NULL)
    {
        block->prev->next = block->next;
    }

}

void* getFromfreeList(size_t length)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;

    freeBlockStruct* remBlock = NULL;

    while(tmpBlock != NULL)
    {
        if (length == (tmpBlock->length))
        {
            removeBlockFromFreeList(tmpBlock);
            
            return tmpBlock;
        }
        if (length < (tmpBlock->length))
        {
            removeBlockFromFreeList(tmpBlock);
            
            remBlock = (freeBlockStruct*)((uint8_t*)tmpBlock + length );

            remBlock->length = tmpBlock->length - length;
            addTofreeList(remBlock);

            tmpBlock->length = length;
            return tmpBlock;
        }
        else
        {
            tmpBlock = tmpBlock->next;
        }
    }

    return NULL;
}

void* increaseHeapSize(size_t size)
{
    uint32_t* newItemPtr;
    freeBlockStruct* newBlock;
    /* use sbrk to allocate the new required size */
    newItemPtr = sbrk(size); /* should we increment 1 */

    if (newItemPtr == NULL)
    {
        return NULL;
    }

    newBlock = (freeBlockStruct*) newItemPtr;
    newBlock->length = size;
    newBlock->next = NULL;
    newBlock->prev = NULL;

    return newBlock;
}


void HmmFree(void *ptr)
{
    freeBlockStruct* blockAddress;
    freeBlockStruct* nextBlock;
    /* validate the pointer */
    if (ptr == NULL)
    {
        return;
    }
    if (isfreeBlock( (freeBlockStruct*)((size_t*) ptr - 1)) == 1)
    {
        return;
    }
    /* subtract the bytes of the length */
    blockAddress = (freeBlockStruct*)((size_t*) ptr - 1);

    /* check if the next block is a free block then gather the two blocks into one large block */
    nextBlock = (freeBlockStruct*) ((uint8_t*) blockAddress + blockAddress->length);
    
    if ((nextBlock < (freeBlockStruct*)sbrk(0)) && (isfreeBlock(nextBlock) == 1))
    {
        removeBlockFromFreeList(nextBlock);

        blockAddress->length = blockAddress->length + nextBlock->length;
    }
    /* add the block to the free block */
    addTofreeList(blockAddress);
}