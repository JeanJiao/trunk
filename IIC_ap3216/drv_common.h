#ifndef __DRV_COMMON_H
#define __DRV_COMMON_H

#define DRV_SUCCESS 0
#define DRV_FAILED  1

#define VOID    void
#define IN      VOID
#define OUT     VOID
#define INOUT   VOID

/* 次设备号的起始值，约定次设备号从0开始 */
#define MY_MINOR_BASE   0
#define KER_PRINT  printk

typedef char INT8S;
typedef unsigned char INT8U;

typedef short INT16S;
typedef unsigned short INT16U;

typedef int INT32S;
typedef unsigned int INT32U;

typedef long LONG32S;
typedef unsigned long LONG32U;

#define CHECK_VALID(p) \
do  \
{   \
    if((p) == NULL)   \
    {   \
        printk("null pointer\n");   \
        return DRV_FAILED;  \
    }   \
} while (0);


#endif