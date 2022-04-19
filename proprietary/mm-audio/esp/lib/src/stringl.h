/*======================= COPYRIGHT NOTICE ==================================*]
[* Copyright (c) 2014 Qualcomm Technologies, Inc.                            *]
[* All Rights Reserved.                                                      *]
[* Confidential and Proprietary - Qualcomm Technologies, Inc.                *]
[*===========================================================================*/

// Please note that this will be removed in QDSP6 integration

#ifndef _STRINGL_H_
#define _STRINGL_H_


#include <string.h>



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


size_t memsmove(
          void        *dst,
          size_t      dst_size,
          const void  *src,
          size_t      src_size
          );


size_t  memscpy(
          void        *dst,
          size_t      dst_size,
          const void  *src,
          size_t      src_size
          );



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _STRINGL_H_ */

