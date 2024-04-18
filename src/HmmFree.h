/* 
    File : HmmFree.h
    Desc : Heap Memory Manager Free header file
*/

#ifndef __HMM_FREE_H_
#define __HMM_FREE_H_


typedef struct freeBlock
{
    size_t length;
    struct freeBlock* next;
    struct freeBlock* prev;
}freeBlockStruct;

void* getFromfreeList(size_t length);

uint32_t isfreeBlock(freeBlockStruct* block);

void removeBlockFromFreeList(freeBlockStruct* block);

void addTofreeList(freeBlockStruct* newBlock);

#endif /* __HMM_FREE_H_ */