/* 
    File : Hmm.h
    Desc : Heap Memory Manager include file
*/

#ifndef __HMM_H_
#define __HMM_H_

void HmmFree(void *ptr);

void* HmmAlloc(size_t size);

void* hmmCalloc(size_t nmemb, size_t size);

void* hmmReAlloc(void *ptr, size_t reqSize);

size_t hmmSize(void *ptr);

#endif /* __HMM_H_ */