/* 
    File : common.c
    Desc : This file inlcudes the common definitions
*/
#include <stdint.h>
#include <assert.h>
#ifndef __COMMON_H_
#define __COMMON_H_



typedef enum bool
{
    FALSE = 0,
    TRUE = 1,
}BOOL;

#define ENABLE_ASSERT

#ifdef ENABLE_ASSERT
#define ASSERT(cond, ... /* print string and args*/ )  \
    if (!(cond)) { printf(__VA_ARGS__); assert(0);}
#else
#define ASSERT(aCond, ... /* print string and args*/ ) 

#endif
#endif /* __COMMON_H_ */