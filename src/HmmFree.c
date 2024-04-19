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

/* Add this block to the freeBlockList in ascending order to fulfil the Best-Fit algorthim */
void addTofreeList(freeBlockStruct* newBlock)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;
    if (newBlock == NULL)
    {
        ASSERT(0, "Invalid pointer\n");
        return;
    }

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

/* Check if the prev block to this block is in the freeBlockList */
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

/* Check if this block is in the freeBlockList */
BOOL isfreeBlock(freeBlockStruct* block)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;

    if (block == NULL)
    {
        return FALSE;
    }
    while(tmpBlock != NULL)
    {
        if ((block == tmpBlock) && (block->length == tmpBlock->length))
        {
            return TRUE;

        }
        else
        {
            tmpBlock = tmpBlock->next;
        }
    }

    return FALSE;
}

/* Check if this address is located withon on of the blocks of the freeBlocksList */
BOOL isBlookinFreedBlock(freeBlockStruct* block)
{
    freeBlockStruct* tmpBlock = freeBlocksListHead;

    if (block == NULL)
    {
        return FALSE;
    }
    while(tmpBlock != NULL)
    {

        if ((block >= tmpBlock) && ((size_t)block < ((size_t)tmpBlock + tmpBlock->length)))
        {
            return TRUE;

        }
        else
        {
            tmpBlock = tmpBlock->next;
        }
    }

    return FALSE;
}

/* Remove this block from the freeBlocksList */
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

/* search for a block with the best-fit for this length and remove this block from the freeBlocksList */
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

/* The function that overwirtes free function
    1) Check that this pointer is not in the already freed area
    2) Check if the prev block and the next block are free blocks or not, to concatenate the blocks to produce a larger blocks
    3) Add this block to the freeBlocksList */
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
    ASSERT( isBlookinFreedBlock( (freeBlockStruct*)((size_t*) ptr - 1)) != 1, "This block is already free\n");

    /* subtract the bytes of the length */
    blockAddress = (freeBlockStruct*)((ssize_t) ptr - sizeof(ssize_t));

    /* check if the next block is a free block then gather the two blocks into one large block */
    nextBlock = (freeBlockStruct*) ((ssize_t) blockAddress + blockAddress->length);

    
    /* check if the next block is free also, then concatenate both blocks */
    if ((nextBlock < (freeBlockStruct*)sbrk(0)) && (isfreeBlock(nextBlock) == TRUE))
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