/* 
    File : HmmFree.c
    Desc : Heap Memory Manager Free implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

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
            if (newBlock->length <= tmpBlock->length )
            {
                /* insert here */
                newBlock->prev = tmpBlock->prev;
                newBlock->next = tmpBlock;
                if (tmpBlock->prev != NULL)
                {
                    tmpBlock->prev->next = newBlock;
                }
                tmpBlock->prev = newBlock;

                if (tmpBlock == freeBlocksListHead)
                {
                    freeBlocksListHead = newBlock;
                }

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

freeBlockStruct* isPrevBlockFree(freeBlockStruct* block)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;

    if (block == NULL)
    {
        return NULL;
    }
    while(tmpBlock != NULL)
    {
        if (((size_t)tmpBlock + (tmpBlock->length)) == (size_t)block)
        {
            return tmpBlock;

        }
        else
        {
            tmpBlock = tmpBlock->next;
        }
    }

    return NULL;
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
        if ((block == tmpBlock) && (block->length == tmpBlock->length))
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
        return;
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
            
            remBlock = (freeBlockStruct*)((size_t)tmpBlock + length );

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

void HmmFree(void *ptr)
{
    freeBlockStruct* blockAddress;
    freeBlockStruct* nextBlock,* prevBlock;
    /* validate the pointer */
    if (ptr == NULL)
    {
        return;
    }

    /* catch double free */
    assert( isfreeBlock( (freeBlockStruct*)((size_t*) ptr - 1)) != 1);

    /* subtract the bytes of the length */
    blockAddress = (freeBlockStruct*)((ssize_t) ptr - sizeof(ssize_t));

    /* check if the next block is a free block then gather the two blocks into one large block */
    nextBlock = (freeBlockStruct*) ((ssize_t) blockAddress + blockAddress->length);

    
    /* check if the next block is free also, then concatenate both blocks */
    if ((nextBlock < (freeBlockStruct*)sbrk(0)) && (isfreeBlock(nextBlock) == 1))
    {
        
        removeBlockFromFreeList(nextBlock);

        blockAddress->length += nextBlock->length;
    }

    /* check if the prev block is free also, then concatenate both blocks */
    prevBlock = isPrevBlockFree(blockAddress);

    if (prevBlock != NULL)
    {
        removeBlockFromFreeList(prevBlock);
        prevBlock->length += blockAddress->length;
        blockAddress = prevBlock;
    }

    /* add the block to the free block */
    addTofreeList(blockAddress);

}

/* print all the elements of the free blocks list for debugging */
void print_free_list(void)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;
    int i = 0 ;
    printf("\n----------\nfree list content\n----------\n");
    while(tmpBlock != NULL)
    {
        printf("node %d, length = %lu, prev = %p, next = %p\n", i,tmpBlock->length, tmpBlock->prev, tmpBlock->next );
        i++;
        tmpBlock = tmpBlock->next;
    }
}