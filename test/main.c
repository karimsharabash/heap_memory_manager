
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>

#define SMALL_BLOCKS_ALLOCS_COUNT (1024)
#define SMALL_BLOCKS_SIZE         (100 - sizeof(size_t))
#define LARGE_BLOCKS_ALLOCS_COUNT (100)
#define LARGE_BLOCKS_SIZE         (1024 - sizeof(size_t))
#define HUGE_BLOCKS_ALLOCS_COUNT  (2)
#define HUGE_BLOCKS_SIZE          ((50 * 1024) - sizeof(size_t))

#define VALIDATION_WORD           (0xFF55FF55)
#define ARRAY_SIZE                (100)
#define ARRAY_ELEMENTS_COUNT      (ARRAY_SIZE / sizeof(uint32_t))

/* remove those macros if realloc and calloc is not defined*/
#define REALLOC_IS_DEFINED
#define CALLOC_IS_DEFINED

extern char end, edata, etext;

void print_free_list(void);
void memset_word(void *ptr, unsigned int value, size_t num_words);

void main(int argc, char *argv[])
{
    char* heapEnd;
    int *ptr[SMALL_BLOCKS_ALLOCS_COUNT];
    printf("etext = %p, edata=%p, end=%p, initial program break=%p\n", &etext, &edata, &end, (char*)sbrk(0));

    /*  First test, check the allocation and the free process and the utilization of the reserved memory using sbrk
        There is an assumption that the first sbrk called with 100K (generic use)+ 1k + 8 bytes (for printf)*/
    heapEnd = (char*)sbrk(0);

    for (int i = 0; i < SMALL_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(SMALL_BLOCKS_SIZE);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }
    }
    for (int i = 0; i < SMALL_BLOCKS_ALLOCS_COUNT; i++)
    {
        free(ptr[i]);
    }
    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(LARGE_BLOCKS_SIZE);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }
    }

    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       free(ptr[i]);
    }

    for (int i = 0; i < HUGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(HUGE_BLOCKS_SIZE);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }
    }

    for (int i = 0; i < HUGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       free(ptr[i]);
    }
    if (heapEnd != (char*)sbrk(0))
    {
        printf("First test: failed, the old heap end = %p and the new end = %p \n", heapEnd, (char*)sbrk(0));
    }
    else
    {
        printf("First test: succeeded\n");
    }
    /* End of First Test*/
    /* Second Test, test the allocated buffers by writing and reading the content of it*/

    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(ARRAY_SIZE);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }

       memset_word(ptr[i],VALIDATION_WORD, ARRAY_ELEMENTS_COUNT);
    }

    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {   
        /* check the contect of the block by verifying the first, last and the middle elements */
        assert(ptr[i][0] == VALIDATION_WORD && ptr[i][ARRAY_ELEMENTS_COUNT - 1] == VALIDATION_WORD && ptr[i][ARRAY_ELEMENTS_COUNT / 2] == VALIDATION_WORD );
        free(ptr[i]);
    }

    printf("Second test: succeeded\n");
    /* End of Second Test*/
    /* Third Test, test small size blocks (lower than the size of two pointers)*/
    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(5);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }

       memset(ptr[i],0xff, 5);
    }

    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {   

        free(ptr[i]);
    }

    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {
       ptr[i] = malloc(5);
       if (ptr[i] == NULL)
       {
           printf("failed to allocate ptr[%d]\n", i);
           exit(1);
       }

       memset(ptr[i],0xff, 5);
    }
    for (int i = 0; i < LARGE_BLOCKS_ALLOCS_COUNT; i++)
    {   

        free(ptr[i]);
    }
    
    printf("Third test: succeeded\n");

    /* End of Third Test*/

#ifdef REALLOC_IS_DEFINED
    /* REALLOC test start, check that Realloc is working and it doesn't change the content of the old block*/
    int* allocatedPtr = malloc(4 * sizeof(uint32_t));

    allocatedPtr[0] = VALIDATION_WORD;
    allocatedPtr[1] = VALIDATION_WORD;
    allocatedPtr[2] = VALIDATION_WORD;
    allocatedPtr[3] = VALIDATION_WORD;


    allocatedPtr = realloc(allocatedPtr, 200);

    assert(allocatedPtr[0] == VALIDATION_WORD);
    assert(allocatedPtr[1] == VALIDATION_WORD);
    assert(allocatedPtr[2] == VALIDATION_WORD);
    assert(allocatedPtr[3] == VALIDATION_WORD);

    free(allocatedPtr);

    printf("REALLOC test: succeeded\n"); 
    /* REALLOC test end*/
#endif /* REALLOC_IS_DEFINED */
#ifdef CALLOC_IS_DEFINED
    /* CALLOC test start, check that Calloc intialize the whole block with zeros */
    uint32_t* HeapArr = calloc(20, 4);


    HeapArr[2] = VALIDATION_WORD;
    HeapArr[3] = VALIDATION_WORD;

    assert(HeapArr[0] == 0);
    assert(HeapArr[1] == 0);
    assert(HeapArr[2] == VALIDATION_WORD);
    assert(HeapArr[3] == VALIDATION_WORD);

    free(HeapArr);

    printf("CALLOC test: succeeded\n"); 
#endif /* CALLOC_IS_DEFINED */
    print_free_list();
    exit(EXIT_SUCCESS);

}


void memset_word(void *ptr, uint32_t value, size_t num_words) {
    uint32_t *p = (uint32_t* )ptr;
    while (num_words--) {
        *p++ = value;
    }
}