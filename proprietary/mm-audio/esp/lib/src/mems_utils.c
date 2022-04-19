/*======================= COPYRIGHT NOTICE ==================================*]
[* Copyright (c) 2014 Qualcomm Technologies, Inc.                            *]
[* All Rights Reserved.                                                      *]
[* Confidential and Proprietary - Qualcomm Technologies, Inc.                *]
[*===========================================================================*/

/*
 * DESCRIPTION: Implementation of secure functions for memory related operations.
 */


#include "stringl.h"

size_t memsmove(
          void        *dst,
          size_t      dst_size,
          const void  *src,
          size_t      copy_size
          )
{
    if(dst_size >= copy_size) {
        memmove(dst, src, copy_size);
        return copy_size;
    }

    memmove(dst, src, dst_size);
    return dst_size;
}


size_t memscpy(void *dst, size_t dst_size, const void *src, size_t copy_size)
{
    if(dst_size >= copy_size) {
#ifdef _WIN32
        memcpy_s(dst, dst_size, src, copy_size);
#else
        memcpy(dst, src, copy_size);
#endif
        return copy_size;
    }

#ifdef _WIN32
    memcpy_s(dst, dst_size, src, dst_size);
#else
    memcpy(dst, src, dst_size);
#endif

    return dst_size;
}

