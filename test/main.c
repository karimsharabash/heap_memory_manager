
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Hmm.h"
#include "common.h"

#define MAX_ALLOCS 1000000

#define getInt(x, ...) atoi(x)

extern char end, edata, etext;

void main(int argc, char *argv[])
{
    char *ptr[MAX_ALLOCS];
    int freeStep, freeMin, freeMax, blockSize, numAllocs, j;
    printf("etext = %p, edata=%p, end=%p, initial program break=%p\n", &etext, &edata, &end, (char*)sbrk(0));

    if (argc < 3 || strcmp(argv[1], "--help") == 0) {
        printf("%s num-allocs block-size [step [min [max]]]\n", argv[0]);
        exit(1);
    }

    numAllocs = getInt(argv[1], GN_GT_0, "num-allocs");
    if (numAllocs > MAX_ALLOCS) {
        printf("num-allocs > %d\n", MAX_ALLOCS);
        exit(1);
    }

    blockSize = getInt(argv[2], GN_GT_0 | GN_ANY_BASE, "block-size");

    freeStep = (argc > 3) ? getInt(argv[3], GN_GT_0, "step") : 1;
    freeMin =  (argc > 4) ? getInt(argv[4], GN_GT_0, "min") : 1;
    freeMax =  (argc > 5) ? getInt(argv[5], GN_GT_0, "max") : numAllocs;

    if (freeMax > numAllocs) {
        printf("free-max > num-allocs\n");
        exit(1);
    }

    printf("Initial program break:          %10p\n", sbrk(0));

    printf("Allocating %d*%d bytes\n", numAllocs, blockSize);
    for (j = 0; j < numAllocs; j++) {
        ptr[j] = malloc(blockSize);
        if (ptr[j] == NULL) {
            printf("malloc returned null\n");
            exit(1);
        }
        *ptr[j] = j;
    }

    printf("Program break is now:           %10p\n", sbrk(0));

    printf("Freeing blocks from %d to %d in steps of %d\n",
                freeMin, freeMax, freeStep);
    for (j = freeMin - 1; j < freeMax; j += freeStep)
    {
        free(ptr[j]);
    }
        
    printf("After free(), program break is: %10p\n", sbrk(0));


    uint32_t* allocatedPtr = malloc(30);

    allocatedPtr[0] = 1000;
    allocatedPtr[1] = 2000;
    allocatedPtr[2] = 3000;
    allocatedPtr[3] = 4000;

    printf("allocatedPtr address = %p , size = %lu , 0 = %d, 1 = %d, 2 = %d, 3 = %d\n", allocatedPtr,
                                                                                        hmmSize(allocatedPtr),
                                                                                        allocatedPtr[0], allocatedPtr[1],
                                                                                        allocatedPtr[2], allocatedPtr[3]);

    allocatedPtr = realloc(allocatedPtr, 50);        

    printf("allocatedPtr address = %p , size = %lu , 0 = %d, 1 = %d, 2 = %d, 3 = %d\n", allocatedPtr,
                                                                                        hmmSize(allocatedPtr),
                                                                                        allocatedPtr[0], allocatedPtr[1],
                                                                                        allocatedPtr[2], allocatedPtr[3]);                                                                            


    free(allocatedPtr);

    uint32_t* HeapArr = calloc(20, 4);


    HeapArr[2] = 3000;
    HeapArr[3] = 4000;

    printf("calloc allocatedPtr address = %p , size = %lu , 0 = %d, 1 = %d, 2 = %d, 3 = %d\n", HeapArr,
                                                                                    *((size_t*)HeapArr -1 ),
                                                                                    HeapArr[0], HeapArr[1],
                                                                                    HeapArr[2], HeapArr[3]);


    exit(EXIT_SUCCESS);

}