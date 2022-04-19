#ifndef _ADSP_FD_STUB_H
#define _ADSP_FD_STUB_H
/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "adsp_fd.h"
#ifndef _QAIC_ENV_H
#define _QAIC_ENV_H

#ifdef __GNUC__
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#else
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifndef _ATTRIBUTE_UNUSED

#ifdef _WIN32
#define _ATTRIBUTE_UNUSED
#else
#define _ATTRIBUTE_UNUSED __attribute__ ((unused))
#endif

#endif // _ATTRIBUTE_UNUSED

#ifndef __QAIC_REMOTE
#define __QAIC_REMOTE(ff) ff
#endif //__QAIC_REMOTE

#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE

#ifndef __QAIC_STUB
#define __QAIC_STUB(ff) ff
#endif //__QAIC_STUB

#ifndef __QAIC_STUB_EXPORT
#define __QAIC_STUB_EXPORT
#endif // __QAIC_STUB_EXPORT

#ifndef __QAIC_STUB_ATTRIBUTE
#define __QAIC_STUB_ATTRIBUTE
#endif // __QAIC_STUB_ATTRIBUTE

#ifndef __QAIC_SKEL
#define __QAIC_SKEL(ff) ff
#endif //__QAIC_SKEL__

#ifndef __QAIC_SKEL_EXPORT
#define __QAIC_SKEL_EXPORT
#endif // __QAIC_SKEL_EXPORT

#ifndef __QAIC_SKEL_ATTRIBUTE
#define __QAIC_SKEL_ATTRIBUTE
#endif // __QAIC_SKEL_ATTRIBUTE

#ifdef __QAIC_DEBUG__
   #ifndef __QAIC_DBG_PRINTF__
   #include <stdio.h>
   #define __QAIC_DBG_PRINTF__( ee ) do { printf ee ; } while(0)
   #endif
#else
   #define __QAIC_DBG_PRINTF__( ee ) (void)0
#endif


#define _OFFSET(src, sof)  ((void*)(((char*)(src)) + (sof)))

#define _COPY(dst, dof, src, sof, sz)  \
   do {\
         struct __copy { \
            char ar[sz]; \
         };\
         *(struct __copy*)_OFFSET(dst, dof) = *(struct __copy*)_OFFSET(src, sof);\
   } while (0)

#define _COPYIF(dst, dof, src, sof, sz)  \
   do {\
      if(_OFFSET(dst, dof) != _OFFSET(src, sof)) {\
         _COPY(dst, dof, src, sof, sz); \
      } \
   } while (0)

_ATTRIBUTE_UNUSED
static __inline void _qaic_memmove(void* dst, void* src, int size) {
   int i;
   for(i = 0; i < size; ++i) {
      ((char*)dst)[i] = ((char*)src)[i];
   }
}

#define _MEMMOVEIF(dst, src, sz)  \
   do {\
      if(dst != src) {\
         _qaic_memmove(dst, src, sz);\
      } \
   } while (0)


#define _ASSIGN(dst, src, sof)  \
   do {\
      dst = OFFSET(src, sof); \
   } while (0)

#define _STD_STRLEN_IF(str) (str == 0 ? 0 : strlen(str))

#include "AEEStdErr.h"

#define _TRY(ee, func) \
   do { \
      if (AEE_SUCCESS != ((ee) = func)) {\
         __QAIC_DBG_PRINTF__((__FILE__ ":%d:error:%d:%s\n", __LINE__, (int)(ee),#func));\
         goto ee##bail;\
      } \
   } while (0)

#define _CATCH(exception) exception##bail: if (exception != AEE_SUCCESS)

#define _ASSERT(nErr, ff) _TRY(nErr, 0 == (ff) ? AEE_EBADPARM : AEE_SUCCESS)

#ifdef __QAIC_DEBUG__
#define _ALLOCATE(nErr, pal, size, alignment, pv) _TRY(nErr, _allocator_alloc(pal, __FILE_LINE__, size, alignment, (void**)&pv))
#else
#define _ALLOCATE(nErr, pal, size, alignment, pv) _TRY(nErr, _allocator_alloc(pal, 0, size, alignment, (void**)&pv))
#endif


#endif // _QAIC_ENV_H

#include "remote.h"
#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stdlib.h>
#include <stdint.h>

typedef struct _heap _heap;
struct _heap {
   _heap* pPrev;
   const char* loc;
   uint64_t buf;
};

typedef struct _allocator {
   _heap* pheap;
   uint8_t* stack;
   uint8_t* stackEnd;
   int nSize;
} _allocator;

_ATTRIBUTE_UNUSED
static __inline int _heap_alloc(_heap** ppa, const char* loc, int size, void** ppbuf) {
   _heap* pn = 0;
   pn = malloc(size + sizeof(_heap) - sizeof(uint64_t));
   if(pn != 0) {
      pn->pPrev = *ppa;
      pn->loc = loc;
      *ppa = pn;
      *ppbuf = (void*)&(pn->buf);
      return 0;
   } else {
      return -1;
   }
}
#define _ALIGN_SIZE(x, y) (((x) + (y-1)) & ~(y-1))

_ATTRIBUTE_UNUSED
static __inline int _allocator_alloc(_allocator* me,
                                    const char* loc,
                                    int size,
                                    unsigned int al,
                                    void** ppbuf) {
   if(size < 0) {
      return -1;
   } else if (size == 0) {
      *ppbuf = 0;
      return 0;
   }
   if((_ALIGN_SIZE((uintptr_t)me->stackEnd, al) + size) < (uintptr_t)me->stack + me->nSize) {
      *ppbuf = (uint8_t*)_ALIGN_SIZE((uintptr_t)me->stackEnd, al);
      me->stackEnd = (uint8_t*)_ALIGN_SIZE((uintptr_t)me->stackEnd, al) + size;
      return 0;
   } else {
      return _heap_alloc(&me->pheap, loc, size, ppbuf);
   }
}

_ATTRIBUTE_UNUSED
static __inline void _allocator_deinit(_allocator* me) {
   _heap* pa = me->pheap;
   while(pa != 0) {
      _heap* pn = pa;
      const char* loc = pn->loc;
      (void)loc;
      pa = pn->pPrev;
      free(pn);
   }
}

_ATTRIBUTE_UNUSED
static __inline void _allocator_init(_allocator* me, uint8_t* stack, int stackSize) {
   me->stack =  stack;
   me->stackEnd =  stack + stackSize;
   me->nSize = stackSize;
   me->pheap = 0;
}


#endif // _ALLOCATOR_H

#ifndef SLIM_H
#define SLIM_H

#include <stdint.h>

//a C data structure for the idl types that can be used to implement
//static and dynamic language bindings fairly efficiently.
//
//the goal is to have a minimal ROM and RAM footprint and without
//doing too many allocations.  A good way to package these things seemed
//like the module boundary, so all the idls within  one module can share
//all the type references.


#define PARAMETER_IN       0x0
#define PARAMETER_OUT      0x1
#define PARAMETER_INOUT    0x2
#define PARAMETER_ROUT     0x3
#define PARAMETER_INROUT   0x4

//the types that we get from idl
#define TYPE_OBJECT             0x0
#define TYPE_INTERFACE          0x1
#define TYPE_PRIMITIVE          0x2
#define TYPE_ENUM               0x3
#define TYPE_STRING             0x4
#define TYPE_WSTRING            0x5
#define TYPE_STRUCTURE          0x6
#define TYPE_UNION              0x7
#define TYPE_ARRAY              0x8
#define TYPE_SEQUENCE           0x9

//these require the pack/unpack to recurse
//so it's a hint to those languages that can optimize in cases where
//recursion isn't necessary.
#define TYPE_COMPLEX_STRUCTURE  (0x10 | TYPE_STRUCTURE)
#define TYPE_COMPLEX_UNION      (0x10 | TYPE_UNION)
#define TYPE_COMPLEX_ARRAY      (0x10 | TYPE_ARRAY)
#define TYPE_COMPLEX_SEQUENCE   (0x10 | TYPE_SEQUENCE)


typedef struct Type Type;

#define INHERIT_TYPE\
   int32_t nativeSize;                /*in the simple case its the same as wire size and alignment*/\
   union {\
      struct {\
         const uintptr_t         p1;\
         const uintptr_t         p2;\
      } _cast;\
      struct {\
         uint32_t  iid;\
         uint32_t  bNotNil;\
      } object;\
      struct {\
         const Type  *arrayType;\
         int32_t      nItems;\
      } array;\
      struct {\
         const Type *seqType;\
         int32_t      nMaxLen;\
      } seqSimple; \
      struct {\
         uint32_t bFloating;\
         uint32_t bSigned;\
      } prim; \
      const SequenceType* seqComplex;\
      const UnionType  *unionType;\
      const StructType *structType;\
      int32_t         stringMaxLen;\
      uint8_t        bInterfaceNotNil;\
   } param;\
   uint8_t    type;\
   uint8_t    nativeAlignment\

typedef struct UnionType UnionType;
typedef struct StructType StructType;
typedef struct SequenceType SequenceType;
struct Type {
   INHERIT_TYPE;
};

struct SequenceType {
   const Type *         seqType;
   uint32_t               nMaxLen;
   uint32_t               inSize;
   uint32_t               routSizePrimIn;
   uint32_t               routSizePrimROut;
};

//byte offset from the start of the case values for
//this unions case value array.  it MUST be aligned
//at the alignment requrements for the descriptor
//
//if negative it means that the unions cases are
//simple enumerators, so the value read from the descriptor
//can be used directly to find the correct case
typedef union CaseValuePtr CaseValuePtr;
union CaseValuePtr {
   const uint8_t*   value8s;
   const uint16_t*  value16s;
   const uint32_t*  value32s;
   const uint64_t*  value64s;
};

//these are only used in complex cases
//so I pulled them out of the type definition as references to make
//the type smaller
struct UnionType {
   const Type           *descriptor;
   uint32_t               nCases;
   const CaseValuePtr   caseValues;
   const Type * const   *cases;
   int32_t               inSize;
   int32_t               routSizePrimIn;
   int32_t               routSizePrimROut;
   uint8_t                inAlignment;
   uint8_t                routAlignmentPrimIn;
   uint8_t                routAlignmentPrimROut;
   uint8_t                inCaseAlignment;
   uint8_t                routCaseAlignmentPrimIn;
   uint8_t                routCaseAlignmentPrimROut;
   uint8_t                nativeCaseAlignment;
   uint8_t              bDefaultCase;
};

struct StructType {
   uint32_t               nMembers;
   const Type * const   *members;
   int32_t               inSize;
   int32_t               routSizePrimIn;
   int32_t               routSizePrimROut;
   uint8_t                inAlignment;
   uint8_t                routAlignmentPrimIn;
   uint8_t                routAlignmentPrimROut;
};

typedef struct Parameter Parameter;
struct Parameter {
   INHERIT_TYPE;
   uint8_t    mode;
   uint8_t  bNotNil;
};

#define SLIM_IFPTR32(is32,is64) (sizeof(uintptr_t) == 4 ? (is32) : (is64))
#define SLIM_SCALARS_IS_DYNAMIC(u) (((u) & 0x00ffffff) == 0x00ffffff)

typedef struct Method Method;
struct Method {
   uint32_t                    uScalars;            //no method index
   int32_t                     primInSize;
   int32_t                     primROutSize;
   int                         maxArgs;
   int                         numParams;
   const Parameter * const     *params;
   uint8_t                       primInAlignment;
   uint8_t                       primROutAlignment;
};

typedef struct Interface Interface;

struct Interface {
   int                            nMethods;
   const Method  * const          *methodArray;
   int                            nIIds;
   const uint32_t                   *iids;
   const uint16_t*                  methodStringArray;
   const uint16_t*                  methodStrings;
   const char*                    strings;
};


#endif //SLIM_H


#ifndef _ADSP_FD_SLIM_H
#define _ADSP_FD_SLIM_H
#include "remote.h"
#include <stdint.h>

#ifndef __QAIC_SLIM
#define __QAIC_SLIM(ff) ff
#endif
#ifndef __QAIC_SLIM_EXPORT
#define __QAIC_SLIM_EXPORT
#endif

static const Type types[7];
static const Type* const typeArrays[36] = {&(types[1]),&(types[1]),&(types[0]),&(types[0]),&(types[4]),&(types[6]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[6]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[2]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0]),&(types[0])};
static const StructType structTypes[4] = {{0x2,&(typeArrays[2]),0x8,0x0,0x8,0x4,0x1,0x4},{0xd,&(typeArrays[23]),0x38,0x0,0x38,0x4,0x1,0x4},{0x4,&(typeArrays[6]),0x10,0x0,0x10,0x4,0x1,0x4},{0x17,&(typeArrays[0]),0x78,0x0,0x78,0x4,0x1,0x4}};
static const Type types[7] = {{0x4,{{(const uintptr_t)0,(const uintptr_t)1}}, 2,0x4},{0x1,{{(const uintptr_t)0,(const uintptr_t)0}}, 2,0x1},{0x8,{{(const uintptr_t)&(structTypes[0]),0}}, 6,0x4},{0x38,{{(const uintptr_t)&(structTypes[1]),0}}, 6,0x4},{0xc,{{(const uintptr_t)&(types[5]),(const uintptr_t)0x3}}, 8,0x4},{0x4,{{(const uintptr_t)0,(const uintptr_t)0}}, 2,0x4},{0x10,{{(const uintptr_t)&(structTypes[2]),0}}, 6,0x4}};
static const Parameter parameters[14] = {{0x1,{{(const uintptr_t)0,(const uintptr_t)0}}, 2,0x1,3,0},{0x4,{{(const uintptr_t)0,(const uintptr_t)1}}, 2,0x4,0,0},{0x4,{{(const uintptr_t)0,(const uintptr_t)1}}, 2,0x4,3,0},{0x4,{{(const uintptr_t)0,(const uintptr_t)0}}, 2,0x4,0,0},{0x8,{{(const uintptr_t)&(structTypes[0]),0}}, 6,0x4,0,0},{0x8,{{(const uintptr_t)&(structTypes[0]),0}}, 6,0x4,3,0},{SLIM_IFPTR32(0x8,0x10),{{(const uintptr_t)&(types[1]),(const uintptr_t)0x0}}, 9,SLIM_IFPTR32(0x4,0x8),0,0},{0x38,{{(const uintptr_t)&(structTypes[1]),0}}, 6,0x4,3,0},{SLIM_IFPTR32(0x8,0x10),{{(const uintptr_t)&(types[3]),(const uintptr_t)0x0}}, 9,SLIM_IFPTR32(0x4,0x8),3,0},{0x4,{{(const uintptr_t)0,(const uintptr_t)0}}, 2,0x4,3,0},{0x10,{{(const uintptr_t)&(structTypes[2]),0}}, 6,0x4,0,0},{0x10,{{(const uintptr_t)&(structTypes[2]),0}}, 6,0x4,3,0},{0x78,{{(const uintptr_t)&(structTypes[3]),0}}, 6,0x4,0,0},{0x78,{{(const uintptr_t)&(structTypes[3]),0}}, 6,0x4,3,0}};
static const Parameter* const parameterArrays[84] = {(&(parameters[3])),(&(parameters[6])),(&(parameters[6])),(&(parameters[6])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[3])),(&(parameters[2])),(&(parameters[3])),(&(parameters[6])),(&(parameters[6])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[3])),(&(parameters[2])),(&(parameters[3])),(&(parameters[6])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[3])),(&(parameters[2])),(&(parameters[4])),(&(parameters[1])),(&(parameters[1])),(&(parameters[5])),(&(parameters[5])),(&(parameters[5])),(&(parameters[5])),(&(parameters[4])),(&(parameters[4])),(&(parameters[4])),(&(parameters[4])),(&(parameters[5])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[3])),(&(parameters[2])),(&(parameters[2])),(&(parameters[2])),(&(parameters[3])),(&(parameters[1])),(&(parameters[1])),(&(parameters[1])),(&(parameters[3])),(&(parameters[3])),(&(parameters[3])),(&(parameters[3])),(&(parameters[3])),(&(parameters[1])),(&(parameters[13])),(&(parameters[3])),(&(parameters[1])),(&(parameters[12])),(&(parameters[3])),(&(parameters[3])),(&(parameters[9])),(&(parameters[3])),(&(parameters[1])),(&(parameters[7])),(&(parameters[3])),(&(parameters[11])),(&(parameters[3])),(&(parameters[10])),(&(parameters[3])),(&(parameters[8])),(&(parameters[0])),(&(parameters[0]))};
static const Method methods[25] = {{REMOTE_SCALARS_MAKEX(0,0,0x0,0x1,0x0,0x0),0x0,0x2,2,2,(&(parameterArrays[82])),0x1,0x1},{REMOTE_SCALARS_MAKEX(0,0,0x0,0x0,0x0,0x0),0x0,0x0,0,0,0,0x0,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x1c,0x0,7,7,(&(parameterArrays[45])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x0,0x1,0x0,0x0),0x0,0x1c,7,7,(&(parameterArrays[36])),0x1,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x10,0x0,4,4,(&(parameterArrays[60])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x4,0x0,1,1,(&(parameterArrays[0])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x20,0x10,7,7,(&(parameterArrays[31])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x10,0x20,7,7,(&(parameterArrays[24])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0xc,0x0,3,3,(&(parameterArrays[56])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x2,0x1,0x0,0x0),0x18,0x4,8,7,(&(parameterArrays[17])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x3,0x1,0x0,0x0),0x1c,0x4,10,8,(&(parameterArrays[9])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x4,0x1,0x0,0x0),0x20,0x4,12,9,(&(parameterArrays[0])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x4,0x4,2,2,(&(parameterArrays[7])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x8,0x38,3,3,(&(parameterArrays[73])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x8,0x0,4,2,(&(parameterArrays[80])),0x4,0x1},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x4,0x8,3,3,(&(parameterArrays[52])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0xc,0x0,3,3,(&(parameterArrays[60])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x8,0x4,3,3,(&(parameterArrays[70])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x14,0x0,2,2,(&(parameterArrays[78])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x4,0x10,2,2,(&(parameterArrays[76])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x8,0x0,2,2,(&(parameterArrays[56])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x10,0x0,4,4,(&(parameterArrays[56])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x4,0xc,4,4,(&(parameterArrays[52])),0x4,0x4},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x0,0x0,0x0),0x80,0x0,3,3,(&(parameterArrays[67])),0x4,0x0},{REMOTE_SCALARS_MAKEX(0,0,0x1,0x1,0x0,0x0),0x8,0x78,3,3,(&(parameterArrays[64])),0x4,0x4}};
static const Method* const methodArrays[59] = {&(methods[0]),&(methods[1]),&(methods[1]),&(methods[2]),&(methods[3]),&(methods[0]),&(methods[1]),&(methods[4]),&(methods[5]),&(methods[6]),&(methods[7]),&(methods[0]),&(methods[8]),&(methods[5]),&(methods[5]),&(methods[5]),&(methods[9]),&(methods[9]),&(methods[10]),&(methods[11]),&(methods[12]),&(methods[13]),&(methods[13]),&(methods[14]),&(methods[14]),&(methods[8]),&(methods[15]),&(methods[16]),&(methods[17]),&(methods[18]),&(methods[19]),&(methods[20]),&(methods[12]),&(methods[20]),&(methods[12]),&(methods[5]),&(methods[20]),&(methods[21]),&(methods[22]),&(methods[8]),&(methods[15]),&(methods[8]),&(methods[15]),&(methods[20]),&(methods[12]),&(methods[20]),&(methods[12]),&(methods[18]),&(methods[19]),&(methods[20]),&(methods[12]),&(methods[20]),&(methods[12]),&(methods[8]),&(methods[15]),&(methods[20]),&(methods[12]),&(methods[23]),&(methods[24])};
static const char strings[2581] = "FACEPROC_DT_MV_GetTrackingSwapParam\0FACEPROC_DT_MV_SetTrackingSwapParam\0FACEPROC_DT_MV_GetTrackingEdgeMask\0FACEPROC_DT_MV_SetTrackingEdgeMask\0FACEPROC_DT_MV_GetSteadinessParam\0FACEPROC_DT_MV_SetSteadinessParam\0FACEPROC_DT_MV_ToggleTrackingLock\0FACEPROC_CO_ConvertCenterToSquare\0FACEPROC_CO_ConvertSquareToCenter\0FACEPROC_DT_MV_GetAngleExtension\0FACEPROC_DT_MV_SetAngleExtension\0FACEPROC_DT_MV_GetDirectionMask\0FACEPROC_DT_MV_SetDirectionMask\0FACEPROC_DT_MV_GetPoseExtension\0FACEPROC_DT_MV_SetPoseExtension\0FACEPROC_DT_GetAllRawResultInfo\0FACEPROC_DT_DeleteResultHandle\0FACEPROC_DT_CreateResultHandle\0FACEPROC_CO_CreateHandleMemory\0FACEPROC_DT_MV_GetSearchCycle\0FACEPROC_DT_MV_SetSearchCycle\0FACEPROC_DT_MV_GetDelayCount\0FACEPROC_DT_MV_SetDelayCount\0FACEPROC_DT_MV_ResetTracking\0FACEPROC_DT_GetSearchDensity\0FACEPROC_DT_SetSearchDensity\0FACEPROC_DT_GetAllResultInfo\0FACEPROC_DT_GetRawResultInfo\0FACEPROC_DT_MV_GetLostParam\0FACEPROC_DT_MV_SetLostParam\0FACEPROC_DT_Detect_YUV420FP\0FACEPROC_DT_Detect_YUV420SP\0FACEPROC_DT_MV_GetAccuracy\0FACEPROC_DT_MV_SetAccuracy\0FACEPROC_DT_GetResultCount\0FACEPROC_DT_GetResultInfo\0FACEPROC_DT_Detect_YUV422\0pnInitialFaceSearchCycle\0FACEPROC_DT_GetThreshold\0FACEPROC_DT_SetThreshold\0FACEPROC_DT_GetSizeRange\0FACEPROC_DT_SetSizeRange\0FACEPROC_DT_DeleteHandle\0FACEPROC_DT_CreateHandle\0FACEPROC_CO_DeleteHandle\0FACEPROC_CO_CreateHandle\0FACEPROC_DT_GetDTConfig\0FACEPROC_DT_SetDTConfig\0pnNewFaceSearchInterval\0FACEPROC_DT_GetEdgeMask\0FACEPROC_DT_SetEdgeMask\0FACEPROC_DT_Detect_GRAY\0FACEPROC_DT_GetVersion\0FACEPROC_CO_GetVersion\0pnSizeSteadinessParam\0FDDSP_DeInitDetection\0pnPosSteadinessParam\0pnNewFaceSearchCycle\0FACEPROC_DT_GetAngle\0FACEPROC_DT_SetAngle\0FDDSP_GetClockConfig\0FDDSP_SetClockConfig\0pnTrackingSwapParam\0FDDSP_InitDetection\0rollAngleExtension\0faceSearchEdgeMask\0nMaxDetectionCount\0yawAngleExtension\0pbUseHeadTracking\0trackingEdgeMask\0nDetectionMethod\0FDDSP_GetVersion\0p_dt_new_params\0pnMaxRetryCount\0pnSearchDensity\0psDetectionInfo\0detection_mode\0pnMaxHoldCount\0nDetectionMode\0pptRightBottom\0dcvs_min_clock\0nDetectorType\0pptLeftBottom\0unWMemorySize\0unBMemorySize\0minorVersion\0majorVersion\0pnDelayCount\0pWMemoryAddr\0pBMemoryAddr\0p_dt_params\0pbExtension\0pnThreshold\0prcEdgeMask\0nConfidence\0nObjectType\0pptRightTop\0dcvs_enable\0pnAccuracy\0nReserved2\0nHoldCount\0ImageOrder\0pImageGRAY\0pptLeftTop\0p_b_absapi\0dcvs_clock\0pnMaxSize\0pnMinSize\0nReserved\0pImageYUV\0hDtResult\0pptCenter\0p_latency\0pImageCr\0pImageCb\0pImageCx\0pucMinor\0pucMajor\0pImageY\0pnCount\0nHeight\0pnAngle\0p_clock\0pbMask\0bottom\0nIndex\0nWidth\0pnSize\0right\0nPose\0p_bus\0left\0top\0nID\0hDT\0hCO\0";
static const uint16_t methodStrings[388] = {1364,2573,1972,1908,2116,2103,2363,2353,2492,1783,1941,2193,1140,1620,1413,1925,1988,1599,1555,1725,2130,1857,2265,1764,1821,1840,2508,2560,2565,2542,2514,2560,2565,2542,2514,1388,2573,1972,2168,2116,2103,2363,2353,2492,1783,1941,2193,1140,1620,1413,1925,1988,1599,1555,1725,2130,1857,2265,1764,1821,1840,2508,2560,2565,2542,2514,2560,2565,2542,2514,865,2392,2521,1956,2047,2228,2569,2216,2403,2528,2483,2492,2548,2372,1874,2286,2275,2447,629,1087,2392,2521,1956,2047,2228,2569,2216,2403,2528,2483,2492,2548,2372,1874,2286,2275,2447,629,506,2392,1956,2047,2228,2569,2216,2403,2528,2483,2492,2548,2372,1874,2286,2275,2447,629,836,2392,1956,2047,2228,2569,2216,2403,2528,2483,2492,2548,2372,1874,2286,2275,2447,629,244,2403,2447,629,2366,2492,2319,2447,629,2240,2447,629,2061,2447,629,2017,2447,629,278,2320,2447,629,2241,2447,629,2062,2447,629,2018,2447,629,2402,2447,629,2535,2491,950,2573,2467,2431,2422,2528,2483,2297,2392,2475,978,2573,2467,2440,2528,2483,2297,2392,2475,1113,2573,2382,2528,2483,2297,2392,2475,1484,2573,2308,2528,2483,2297,2392,2475,1682,2499,2554,2412,2330,2341,2032,2252,1703,2041,2556,2414,2332,2341,2032,2252,72,2573,2204,2560,2565,2542,2514,107,2573,2205,2560,2565,2542,2514,1436,2573,2204,2560,2565,2542,2514,1460,2573,2205,2560,2565,2542,2514,631,2573,1139,1619,1412,661,2573,1140,1620,1413,600,2155,2089,2142,2075,442,2573,2180,1839,474,2573,2181,1840,142,2573,1598,1554,176,2573,1599,1555,894,2573,1924,1987,922,2573,1925,1988,1640,2573,2548,2491,1661,2573,2548,2492,1214,2573,2362,2352,1239,2573,2363,2353,1289,2577,2002,1802,378,2573,2507,410,2573,2508,312,2573,2180,345,2573,2181,1006,2573,2264,1033,2573,2265,691,2573,2129,720,2573,2130,0,2573,1724,36,2573,1725,210,2573,2569,1164,2573,2192,1189,2573,2193,778,2573,1940,807,2573,1941,1060,2392,2475,1508,2458,2449,1531,2458,2449,1891,2458,2449,749,2573,538,2392,569,2577,1264,2573,1314,2577,1339,1576,1744};
static const uint16_t methodStringsArrays[59] = {372,387,386,223,215,369,385,269,383,162,144,366,314,381,379,377,207,199,190,180,363,89,70,126,108,310,306,302,298,252,245,360,357,354,351,375,348,264,259,294,290,286,282,345,342,339,336,238,231,333,330,327,324,278,274,321,318,35,0};
__QAIC_SLIM_EXPORT const Interface __QAIC_SLIM(adsp_fd_slim) = {59,&(methodArrays[0]),0,0,&(methodStringsArrays [0]),methodStrings,strings};
#endif //_ADSP_FD_SLIM_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _const_adsp_fd_handle
#define _const_adsp_fd_handle ((remote_handle)-1)
#endif //_const_adsp_fd_handle

static void _adsp_fd_pls_dtor(void* data) {
   remote_handle* ph = (remote_handle*)data;
   if(_const_adsp_fd_handle != *ph) {
      (void)__QAIC_REMOTE(remote_handle_close)(*ph);
      *ph = _const_adsp_fd_handle;
   }
}

static int _adsp_fd_pls_ctor(void* ctx, void* data) {
   remote_handle* ph = (remote_handle*)data;
   *ph = _const_adsp_fd_handle;
   if(*ph == (remote_handle)-1) {
      return __QAIC_REMOTE(remote_handle_open)((const char*)ctx, ph);
   }
   return 0;
}

#if (defined __qdsp6__) || (defined __hexagon__)
#pragma weak  adsp_pls_add_lookup
extern int adsp_pls_add_lookup(uint32_t type, uint32_t key, int size, int (*ctor)(void* ctx, void* data), void* ctx, void (*dtor)(void* ctx), void** ppo);
#pragma weak  HAP_pls_add_lookup
extern int HAP_pls_add_lookup(uint32_t type, uint32_t key, int size, int (*ctor)(void* ctx, void* data), void* ctx, void (*dtor)(void* ctx), void** ppo);

__QAIC_STUB_EXPORT remote_handle _adsp_fd_handle(void) {
   remote_handle* ph;
   if(adsp_pls_add_lookup) {
      if(0 == adsp_pls_add_lookup((uint32_t)_adsp_fd_handle, 0, sizeof(*ph),  _adsp_fd_pls_ctor, "adsp_fd",  _adsp_fd_pls_dtor, (void**)&ph))  {
         return *ph;
      }
      return (remote_handle)-1;
   } else if(HAP_pls_add_lookup) {
      if(0 == HAP_pls_add_lookup((uint32_t)_adsp_fd_handle, 0, sizeof(*ph),  _adsp_fd_pls_ctor, "adsp_fd",  _adsp_fd_pls_dtor, (void**)&ph))  {
         return *ph;
      }
      return (remote_handle)-1;
   }
   return(remote_handle)-1;
}

#else //__qdsp6__ || __hexagon__

uint32_t _adsp_fd_atomic_CompareAndExchange(uint32_t * volatile puDest, uint32_t uExchange, uint32_t uCompare);

#ifdef _WIN32
#include "Windows.h"
uint32_t _adsp_fd_atomic_CompareAndExchange(uint32_t * volatile puDest, uint32_t uExchange, uint32_t uCompare) {
   return (uint32_t)InterlockedCompareExchange((volatile LONG*)puDest, (LONG)uExchange, (LONG)uCompare);
}
#elif __GNUC__
uint32_t _adsp_fd_atomic_CompareAndExchange(uint32_t * volatile puDest, uint32_t uExchange, uint32_t uCompare) {
   return __sync_val_compare_and_swap(puDest, uCompare, uExchange);
}
#endif //_WIN32


__QAIC_STUB_EXPORT remote_handle _adsp_fd_handle(void) {
   static remote_handle handle = _const_adsp_fd_handle;
   if((remote_handle)-1 != handle) {
      return handle;
   } else {
      remote_handle tmp;
      int nErr = _adsp_fd_pls_ctor("adsp_fd", (void*)&tmp);
      if(nErr) {
         return (remote_handle)-1;
      }
      if(((remote_handle)-1 != handle) || ((remote_handle)-1 != (remote_handle)_adsp_fd_atomic_CompareAndExchange((uint32_t*)&handle, (uint32_t)tmp, (uint32_t)-1))) {
         _adsp_fd_pls_dtor(&tmp);
      }
      return handle;
   }
}

#endif //__qdsp6__

__QAIC_STUB_EXPORT int __QAIC_STUB(adsp_fd_skel_invoke)(uint32_t _sc, remote_arg* _pra) __QAIC_STUB_ATTRIBUTE {
   return __QAIC_REMOTE(remote_handle_invoke)(_adsp_fd_handle(), _sc, _pra);
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif
static __inline int _stub_method(remote_handle _handle, uint32_t _mid, uint8_t _rout0[1], uint8_t _rout1[1]) {
   int _numIn[1];
   remote_arg _pra[1];
   uint8_t _primROut[2];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[(_numIn[0] + 0)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 0)].buf.nLen = sizeof(_primROut);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 0, 1, 0, 0), _pra));
   _COPY(_rout0, 0, _primROut, 0, 1);
   _COPY(_rout1, 0, _primROut, 1, 1);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FDDSP_GetVersion)(UINT8_SKEL* pucMajor, UINT8_SKEL* pucMinor) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 0;
   return _stub_method(_adsp_fd_handle(), _mid, (uint8_t*)pucMajor, (uint8_t*)pucMinor);
}
static __inline int _stub_method_1(remote_handle _handle, uint32_t _mid) {
   remote_arg* _pra = 0;
   int _nErr = 0;
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 0, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FDDSP_InitDetection)(void) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 1;
   return _stub_method_1(_adsp_fd_handle(), _mid);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FDDSP_DeInitDetection)(void) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 2;
   return _stub_method_1(_adsp_fd_handle(), _mid);
}
static __inline int _stub_method_2(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1], uint32_t _in3[1], uint32_t _in4[1], uint32_t _in5[1], uint32_t _in6[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[7];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _COPY(_primIn, 12, _in3, 0, 4);
   _COPY(_primIn, 16, _in4, 0, 4);
   _COPY(_primIn, 20, _in5, 0, 4);
   _COPY(_primIn, 24, _in6, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FDDSP_SetClockConfig)(INT32_SKEL clock, INT32_SKEL bus, INT32_SKEL latency, INT32_SKEL b_absapi, INT32_SKEL dcvs_clock, INT32_SKEL dcvs_min_clock, INT32_SKEL dcvs_enable) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 3;
   return _stub_method_2(_adsp_fd_handle(), _mid, (uint32_t*)&clock, (uint32_t*)&bus, (uint32_t*)&latency, (uint32_t*)&b_absapi, (uint32_t*)&dcvs_clock, (uint32_t*)&dcvs_min_clock, (uint32_t*)&dcvs_enable);
}
static __inline int _stub_method_3(remote_handle _handle, uint32_t _mid, uint32_t _rout0[1], uint32_t _rout1[1], uint32_t _rout2[1], uint32_t _rout3[1], uint32_t _rout4[1], uint32_t _rout5[1], uint32_t _rout6[1]) {
   int _numIn[1];
   remote_arg _pra[1];
   uint32_t _primROut[7];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[(_numIn[0] + 0)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 0)].buf.nLen = sizeof(_primROut);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 0, 1, 0, 0), _pra));
   _COPY(_rout0, 0, _primROut, 0, 4);
   _COPY(_rout1, 0, _primROut, 4, 4);
   _COPY(_rout2, 0, _primROut, 8, 4);
   _COPY(_rout3, 0, _primROut, 12, 4);
   _COPY(_rout4, 0, _primROut, 16, 4);
   _COPY(_rout5, 0, _primROut, 20, 4);
   _COPY(_rout6, 0, _primROut, 24, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FDDSP_GetClockConfig)(INT32_SKEL* p_clock, INT32_SKEL* p_bus, INT32_SKEL* p_latency, INT32_SKEL* p_b_absapi, INT32_SKEL* dcvs_clock, INT32_SKEL* dcvs_min_clock, INT32_SKEL* dcvs_enable) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 4;
   return _stub_method_3(_adsp_fd_handle(), _mid, (uint32_t*)p_clock, (uint32_t*)p_bus, (uint32_t*)p_latency, (uint32_t*)p_b_absapi, (uint32_t*)dcvs_clock, (uint32_t*)dcvs_min_clock, (uint32_t*)dcvs_enable);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_GetVersion)(UINT8_SKEL* pucMajor, UINT8_SKEL* pucMinor) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 5;
   return _stub_method(_adsp_fd_handle(), _mid, (uint8_t*)pucMajor, (uint8_t*)pucMinor);
}
__QAIC_STUB_EXPORT HCOMMON_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_CreateHandle)(void) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 6;
   return _stub_method_1(_adsp_fd_handle(), _mid);
}
static __inline int _stub_method_4(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1], uint32_t _in3[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[4];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _COPY(_primIn, 12, _in3, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT HCOMMON_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_CreateHandleMemory)(UINT32_SKEL pBMemoryAddr, UINT32_SKEL unBMemorySize, UINT32_SKEL pWMemoryAddr, UINT32_SKEL unWMemorySize) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 7;
   return _stub_method_4(_adsp_fd_handle(), _mid, (uint32_t*)&pBMemoryAddr, (uint32_t*)&unBMemorySize, (uint32_t*)&pWMemoryAddr, (uint32_t*)&unWMemorySize);
}
static __inline int _stub_method_5(remote_handle _handle, uint32_t _mid, uint32_t _in0[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[1];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_DeleteHandle)(HCOMMON_SKEL hCO) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 8;
   return _stub_method_5(_adsp_fd_handle(), _mid, (uint32_t*)&hCO);
}
static __inline int _stub_method_6(remote_handle _handle, uint32_t _mid, uint32_t _in0[2], uint32_t _in1[2], uint32_t _in2[2], uint32_t _in3[2], uint32_t _rout4[2], uint32_t _rout5[1], uint32_t _rout6[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[8];
   uint32_t _primROut[4];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 8);
   _COPY(_primIn, 8, _in1, 0, 8);
   _COPY(_primIn, 16, _in2, 0, 8);
   _COPY(_primIn, 24, _in3, 0, 8);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout4, 0, _primROut, 0, 8);
   _COPY(_rout5, 0, _primROut, 8, 4);
   _COPY(_rout6, 0, _primROut, 12, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_ConvertSquareToCenter)(const POINT_SKEL* ptLeftTop, const POINT_SKEL* ptRightTop, const POINT_SKEL* ptLeftBottom, const POINT_SKEL* ptRightBottom, POINT_SKEL* pptCenter, INT32_SKEL* pnSize, INT32_SKEL* pnAngle) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 9;
   return _stub_method_6(_adsp_fd_handle(), _mid, (uint32_t*)ptLeftTop, (uint32_t*)ptRightTop, (uint32_t*)ptLeftBottom, (uint32_t*)ptRightBottom, (uint32_t*)pptCenter, (uint32_t*)pnSize, (uint32_t*)pnAngle);
}
static __inline int _stub_method_7(remote_handle _handle, uint32_t _mid, uint32_t _in0[2], uint32_t _in1[1], uint32_t _in2[1], uint32_t _rout3[2], uint32_t _rout4[2], uint32_t _rout5[2], uint32_t _rout6[2]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[4];
   uint32_t _primROut[8];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 8);
   _COPY(_primIn, 8, _in1, 0, 4);
   _COPY(_primIn, 12, _in2, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout3, 0, _primROut, 0, 8);
   _COPY(_rout4, 0, _primROut, 8, 8);
   _COPY(_rout5, 0, _primROut, 16, 8);
   _COPY(_rout6, 0, _primROut, 24, 8);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_CO_ConvertCenterToSquare)(const POINT_SKEL* ptCenter, INT32_SKEL nSize, INT32_SKEL nAngle, POINT_SKEL* pptLeftTop, POINT_SKEL* pptRightTop, POINT_SKEL* pptLeftBottom, POINT_SKEL* pptRightBottom) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 10;
   return _stub_method_7(_adsp_fd_handle(), _mid, (uint32_t*)ptCenter, (uint32_t*)&nSize, (uint32_t*)&nAngle, (uint32_t*)pptLeftTop, (uint32_t*)pptRightTop, (uint32_t*)pptLeftBottom, (uint32_t*)pptRightBottom);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetVersion)(UINT8_SKEL* pucMajor, UINT8_SKEL* pucMinor) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 11;
   return _stub_method(_adsp_fd_handle(), _mid, (uint8_t*)pucMajor, (uint8_t*)pucMinor);
}
static __inline int _stub_method_8(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[3];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT HDETECTION_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_CreateHandle)(HCOMMON_SKEL hCO, INT32_SKEL nDetectionMode, INT32_SKEL nMaxDetectionCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 12;
   return _stub_method_8(_adsp_fd_handle(), _mid, (uint32_t*)&hCO, (uint32_t*)&nDetectionMode, (uint32_t*)&nMaxDetectionCount);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_DeleteHandle)(HDETECTION_SKEL hDT) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 13;
   return _stub_method_5(_adsp_fd_handle(), _mid, (uint32_t*)&hDT);
}
__QAIC_STUB_EXPORT HDTRESULT_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_CreateResultHandle)(HCOMMON_SKEL hCO) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 14;
   return _stub_method_5(_adsp_fd_handle(), _mid, (uint32_t*)&hCO);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_DeleteResultHandle)(HDTRESULT_SKEL hDtResult) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 15;
   return _stub_method_5(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult);
}
static __inline int _stub_method_9(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], char* _in1[1], uint32_t _in1Len[1], uint32_t _in2[1], uint32_t _in3[1], uint32_t _in4[1], uint32_t _in5[1], uint32_t _rout6[1]) {
   int _numIn[1];
   remote_arg _pra[3];
   uint32_t _primIn[6];
   uint32_t _primROut[1];
   remote_arg* _praIn;
   int _nErr = 0;
   _numIn[0] = 1;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1Len, 0, 4);
   _praIn = (_pra + 1);
   _praIn[0].buf.pv = _in1[0];
   _praIn[0].buf.nLen = (1 * _in1Len[0]);
   _COPY(_primIn, 8, _in2, 0, 4);
   _COPY(_primIn, 12, _in3, 0, 4);
   _COPY(_primIn, 16, _in4, 0, 4);
   _COPY(_primIn, 20, _in5, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 2, 1, 0, 0), _pra));
   _COPY(_rout6, 0, _primROut, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_Detect_GRAY)(HDETECTION_SKEL hDT, const UINT8_SKEL* pImageGRAY, int pImageGRAYLen, INT32_SKEL nWidth, INT32_SKEL nHeight, GRAY_ORDER_SKEL ImageOrder, HDTRESULT_SKEL hDtResult, INT32_SKEL* pnCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 16;
   return _stub_method_9(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (char**)&pImageGRAY, (uint32_t*)&pImageGRAYLen, (uint32_t*)&nWidth, (uint32_t*)&nHeight, (uint32_t*)&ImageOrder, (uint32_t*)&hDtResult, (uint32_t*)pnCount);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_Detect_YUV422)(HDETECTION_SKEL hDT, const UINT8_SKEL* pImageYUV, int pImageYUVLen, INT32_SKEL nWidth, INT32_SKEL nHeight, YUV422_ORDER_SKEL ImageOrder, HDTRESULT_SKEL hDtResult, INT32_SKEL* pnCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 17;
   return _stub_method_9(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (char**)&pImageYUV, (uint32_t*)&pImageYUVLen, (uint32_t*)&nWidth, (uint32_t*)&nHeight, (uint32_t*)&ImageOrder, (uint32_t*)&hDtResult, (uint32_t*)pnCount);
}
static __inline int _stub_method_10(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], char* _in1[1], uint32_t _in1Len[1], char* _in2[1], uint32_t _in2Len[1], uint32_t _in3[1], uint32_t _in4[1], uint32_t _in5[1], uint32_t _in6[1], uint32_t _rout7[1]) {
   int _numIn[1];
   remote_arg _pra[4];
   uint32_t _primIn[7];
   uint32_t _primROut[1];
   remote_arg* _praIn;
   int _nErr = 0;
   _numIn[0] = 2;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1Len, 0, 4);
   _praIn = (_pra + 1);
   _praIn[0].buf.pv = _in1[0];
   _praIn[0].buf.nLen = (1 * _in1Len[0]);
   _COPY(_primIn, 8, _in2Len, 0, 4);
   _praIn[1].buf.pv = _in2[0];
   _praIn[1].buf.nLen = (1 * _in2Len[0]);
   _COPY(_primIn, 12, _in3, 0, 4);
   _COPY(_primIn, 16, _in4, 0, 4);
   _COPY(_primIn, 20, _in5, 0, 4);
   _COPY(_primIn, 24, _in6, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 3, 1, 0, 0), _pra));
   _COPY(_rout7, 0, _primROut, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_Detect_YUV420SP)(HDETECTION_SKEL hDT, const UINT8_SKEL* pImageY, int pImageYLen, const UINT8_SKEL* pImageCx, int pImageCxLen, INT32_SKEL nWidth, INT32_SKEL nHeight, YUV420SP_ORDER_SKEL ImageOrder, HDTRESULT_SKEL hDtResult, INT32_SKEL* pnCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 18;
   return _stub_method_10(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (char**)&pImageY, (uint32_t*)&pImageYLen, (char**)&pImageCx, (uint32_t*)&pImageCxLen, (uint32_t*)&nWidth, (uint32_t*)&nHeight, (uint32_t*)&ImageOrder, (uint32_t*)&hDtResult, (uint32_t*)pnCount);
}
static __inline int _stub_method_11(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], char* _in1[1], uint32_t _in1Len[1], char* _in2[1], uint32_t _in2Len[1], char* _in3[1], uint32_t _in3Len[1], uint32_t _in4[1], uint32_t _in5[1], uint32_t _in6[1], uint32_t _in7[1], uint32_t _rout8[1]) {
   int _numIn[1];
   remote_arg _pra[5];
   uint32_t _primIn[8];
   uint32_t _primROut[1];
   remote_arg* _praIn;
   int _nErr = 0;
   _numIn[0] = 3;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1Len, 0, 4);
   _praIn = (_pra + 1);
   _praIn[0].buf.pv = _in1[0];
   _praIn[0].buf.nLen = (1 * _in1Len[0]);
   _COPY(_primIn, 8, _in2Len, 0, 4);
   _praIn[1].buf.pv = _in2[0];
   _praIn[1].buf.nLen = (1 * _in2Len[0]);
   _COPY(_primIn, 12, _in3Len, 0, 4);
   _praIn[2].buf.pv = _in3[0];
   _praIn[2].buf.nLen = (1 * _in3Len[0]);
   _COPY(_primIn, 16, _in4, 0, 4);
   _COPY(_primIn, 20, _in5, 0, 4);
   _COPY(_primIn, 24, _in6, 0, 4);
   _COPY(_primIn, 28, _in7, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 4, 1, 0, 0), _pra));
   _COPY(_rout8, 0, _primROut, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_Detect_YUV420FP)(HDETECTION_SKEL hDT, const UINT8_SKEL* pImageY, int pImageYLen, const UINT8_SKEL* pImageCb, int pImageCbLen, const UINT8_SKEL* pImageCr, int pImageCrLen, INT32_SKEL nWidth, INT32_SKEL nHeight, YUV420FP_ORDER_SKEL ImageOrder, HDTRESULT_SKEL hDtResult, INT32_SKEL* pnCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 19;
   return _stub_method_11(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (char**)&pImageY, (uint32_t*)&pImageYLen, (char**)&pImageCb, (uint32_t*)&pImageCbLen, (char**)&pImageCr, (uint32_t*)&pImageCrLen, (uint32_t*)&nWidth, (uint32_t*)&nHeight, (uint32_t*)&ImageOrder, (uint32_t*)&hDtResult, (uint32_t*)pnCount);
}
static __inline int _stub_method_12(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _rout1[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[1];
   uint32_t _primROut[1];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout1, 0, _primROut, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetResultCount)(HDTRESULT_SKEL hDtResult, INT32_SKEL* pnCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 20;
   return _stub_method_12(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult, (uint32_t*)pnCount);
}
static __inline int _stub_method_13(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _rout2[14]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   uint32_t _primROut[14];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout2, 0, _primROut, 0, 56);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetResultInfo)(HDTRESULT_SKEL hDtResult, INT32_SKEL nIndex, DETECTION_INFO_SKEL* psDetectionInfo) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 21;
   return _stub_method_13(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult, (uint32_t*)&nIndex, (uint32_t*)psDetectionInfo);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetRawResultInfo)(HDTRESULT_SKEL hDtResult, INT32_SKEL nIndex, DETECTION_INFO_SKEL* psDetectionInfo) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 22;
   return _stub_method_13(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult, (uint32_t*)&nIndex, (uint32_t*)psDetectionInfo);
}
static __inline int _stub_method_14(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], char* _rout1[1], uint32_t _rout1Len[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   remote_arg* _praIn;
   remote_arg* _praROut;
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _rout1Len, 0, 4);
   _praIn = (_pra + 1);
   _praROut = (_praIn + _numIn[0] + 0);
   _praROut[0].buf.pv = _rout1[0];
   _praROut[0].buf.nLen = (56 * _rout1Len[0]);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetAllResultInfo)(HDTRESULT_SKEL hDtResult, DETECTION_INFO_SKEL* psDetectionInfo, int psDetectionInfoLen) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 23;
   return _stub_method_14(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult, (char**)&psDetectionInfo, (uint32_t*)&psDetectionInfoLen);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetAllRawResultInfo)(HDTRESULT_SKEL hDtResult, DETECTION_INFO_SKEL* psDetectionInfo, int psDetectionInfoLen) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 24;
   return _stub_method_14(_adsp_fd_handle(), _mid, (uint32_t*)&hDtResult, (char**)&psDetectionInfo, (uint32_t*)&psDetectionInfoLen);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetSizeRange)(HDETECTION_SKEL hDT, INT32_SKEL nMinSize, INT32_SKEL nMaxSize) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 25;
   return _stub_method_8(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)&nMinSize, (uint32_t*)&nMaxSize);
}
static __inline int _stub_method_15(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _rout1[1], uint32_t _rout2[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[1];
   uint32_t _primROut[2];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout1, 0, _primROut, 0, 4);
   _COPY(_rout2, 0, _primROut, 4, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetSizeRange)(HDETECTION_SKEL hDT, INT32_SKEL* pnMinSize, INT32_SKEL* pnMaxSize) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 26;
   return _stub_method_15(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)pnMinSize, (uint32_t*)pnMaxSize);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetAngle)(HDETECTION_SKEL hDT, UINT32_SKEL nPose, UINT32_SKEL nAngle) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 27;
   return _stub_method_8(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)&nPose, (uint32_t*)&nAngle);
}
static __inline int _stub_method_16(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _rout2[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   uint32_t _primROut[1];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout2, 0, _primROut, 0, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetAngle)(HDETECTION_SKEL hDT, UINT32_SKEL nPose, UINT32_SKEL* pnAngle) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 28;
   return _stub_method_16(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)&nPose, (uint32_t*)pnAngle);
}
static __inline int _stub_method_17(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[4]) {
   remote_arg _pra[1];
   uint32_t _primIn[5];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 16);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetEdgeMask)(HDETECTION_SKEL hDT, const RECT_SKEL* rcEdgeMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 29;
   return _stub_method_17(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)rcEdgeMask);
}
static __inline int _stub_method_18(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _rout1[4]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[1];
   uint32_t _primROut[4];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout1, 0, _primROut, 0, 16);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetEdgeMask)(HDETECTION_SKEL hDT, RECT_SKEL* prcEdgeMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 30;
   return _stub_method_18(_adsp_fd_handle(), _mid, (uint32_t*)&hDT, (uint32_t*)prcEdgeMask);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetSearchDensity)(HDETECTION_SKEL hDT, INT32_SKEL nSearchDensity) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 31;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nSearchDensity);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetSearchDensity)(HDETECTION_SKEL hDT, INT32_SKEL* pnSearchDensity) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 32;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnSearchDensity);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetThreshold)(HDETECTION_SKEL hDT, INT32_SKEL nThreshold) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 33;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nThreshold);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetThreshold)(HDETECTION_SKEL hDT, INT32_SKEL* pnThreshold) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 34;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnThreshold);
}
static __inline int _stub_method_19(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[2];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_ResetTracking)(HDETECTION_SKEL hDT) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 35;
   return _stub_method_19(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_ToggleTrackingLock)(HDETECTION_SKEL hDT, INT32_SKEL nID) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 36;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nID);
}
static __inline int _stub_method_20(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1], uint32_t _in3[1], uint32_t _in4[1]) {
   remote_arg _pra[1];
   uint32_t _primIn[5];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _COPY(_primIn, 12, _in3, 0, 4);
   _COPY(_primIn, 16, _in4, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetSearchCycle)(HDETECTION_SKEL hDT, INT32_SKEL nInitialFaceSearchCycle, INT32_SKEL nNewFaceSearchCycle, INT32_SKEL nNewFaceSearchInterval) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 37;
   return _stub_method_20(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nInitialFaceSearchCycle, (uint32_t*)&nNewFaceSearchCycle, (uint32_t*)&nNewFaceSearchInterval);
}
static __inline int _stub_method_21(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _rout2[1], uint32_t _rout3[1], uint32_t _rout4[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   uint32_t _primROut[3];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout2, 0, _primROut, 0, 4);
   _COPY(_rout3, 0, _primROut, 4, 4);
   _COPY(_rout4, 0, _primROut, 8, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetSearchCycle)(HDETECTION_SKEL hDT, INT32_SKEL* pnInitialFaceSearchCycle, INT32_SKEL* pnNewFaceSearchCycle, INT32_SKEL* pnNewFaceSearchInterval) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 38;
   return _stub_method_21(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnInitialFaceSearchCycle, (uint32_t*)pnNewFaceSearchCycle, (uint32_t*)pnNewFaceSearchInterval);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetLostParam)(HDETECTION_SKEL hDT, INT32_SKEL nMaxRetryCount, INT32_SKEL nMaxHoldCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 39;
   return _stub_method_4(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nMaxRetryCount, (uint32_t*)&nMaxHoldCount);
}
static __inline int _stub_method_22(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _rout2[1], uint32_t _rout3[1]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   uint32_t _primROut[2];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout2, 0, _primROut, 0, 4);
   _COPY(_rout3, 0, _primROut, 4, 4);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetLostParam)(HDETECTION_SKEL hDT, INT32_SKEL* pnMaxRetryCount, INT32_SKEL* pnMaxHoldCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 40;
   return _stub_method_22(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnMaxRetryCount, (uint32_t*)pnMaxHoldCount);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetSteadinessParam)(HDETECTION_SKEL hDT, INT32_SKEL nPosSteadinessParam, INT32_SKEL nSizeSteadinessParam) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 41;
   return _stub_method_4(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nPosSteadinessParam, (uint32_t*)&nSizeSteadinessParam);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetSteadinessParam)(HDETECTION_SKEL hDT, INT32_SKEL* pnPosSteadinessParam, INT32_SKEL* pnSizeSteadinessParam) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 42;
   return _stub_method_22(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnPosSteadinessParam, (uint32_t*)pnSizeSteadinessParam);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetTrackingSwapParam)(HDETECTION_SKEL hDT, INT32_SKEL nTrackingSwapParam) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 43;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nTrackingSwapParam);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetTrackingSwapParam)(HDETECTION_SKEL hDT, INT32_SKEL* pnTrackingSwapParam) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 44;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnTrackingSwapParam);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetDelayCount)(HDETECTION_SKEL hDT, INT32_SKEL nDelayCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 45;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nDelayCount);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetDelayCount)(HDETECTION_SKEL hDT, INT32_SKEL* pnDelayCount) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 46;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnDelayCount);
}
static __inline int _stub_method_23(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[4]) {
   remote_arg _pra[1];
   uint32_t _primIn[6];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 16);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetTrackingEdgeMask)(HDETECTION_SKEL hDT, const RECT_SKEL* rcEdgeMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 47;
   return _stub_method_23(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)rcEdgeMask);
}
static __inline int _stub_method_24(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _rout2[4]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[2];
   uint32_t _primROut[4];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout2, 0, _primROut, 0, 16);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetTrackingEdgeMask)(HDETECTION_SKEL hDT, RECT_SKEL* prcEdgeMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 48;
   return _stub_method_24(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)prcEdgeMask);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetAccuracy)(HDETECTION_SKEL hDT, INT32_SKEL nAccuracy) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 49;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&nAccuracy);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetAccuracy)(HDETECTION_SKEL hDT, INT32_SKEL* pnAccuracy) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 50;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pnAccuracy);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetAngleExtension)(HDETECTION_SKEL hDT, BOOL_SKEL bExtension) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 51;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&bExtension);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetAngleExtension)(HDETECTION_SKEL hDT, BOOL_SKEL* pbExtension) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 52;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pbExtension);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetPoseExtension)(HDETECTION_SKEL hDT, BOOL_SKEL bExtension, BOOL_SKEL bUseHeadTracking) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 53;
   return _stub_method_4(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&bExtension, (uint32_t*)&bUseHeadTracking);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetPoseExtension)(HDETECTION_SKEL hDT, BOOL_SKEL* pbExtension, BOOL_SKEL* pbUseHeadTracking) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 54;
   return _stub_method_22(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pbExtension, (uint32_t*)pbUseHeadTracking);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_SetDirectionMask)(HDETECTION_SKEL hDT, BOOL_SKEL bMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 55;
   return _stub_method_8(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&bMask);
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_MV_GetDirectionMask)(HDETECTION_SKEL hDT, BOOL_SKEL* pbMask) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 56;
   return _stub_method_16(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)pbMask);
}
static __inline int _stub_method_25(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1], uint32_t _in3[30]) {
   remote_arg _pra[1];
   uint32_t _primIn[33];
   int _nErr = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _COPY(_primIn, 12, _in3, 0, 120);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 0, 0, 0), _pra));
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_SetDTConfig)(HDETECTION_SKEL hDT, INT32_SKEL detection_mode, const fpsww_dt_params_t_skel* p_dt_params) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 57;
   return _stub_method_25(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&detection_mode, (uint32_t*)p_dt_params);
}
static __inline int _stub_method_26(remote_handle _handle, uint32_t _mid, uint32_t _in0[1], uint32_t _in1[1], uint32_t _in2[1], uint32_t _rout3[30]) {
   int _numIn[1];
   remote_arg _pra[2];
   uint32_t _primIn[3];
   uint32_t _primROut[30];
   int _nErr = 0;
   _numIn[0] = 0;
   _pra[0].buf.pv = (void*)_primIn;
   _pra[0].buf.nLen = sizeof(_primIn);
   _pra[(_numIn[0] + 1)].buf.pv = (void*)_primROut;
   _pra[(_numIn[0] + 1)].buf.nLen = sizeof(_primROut);
   _COPY(_primIn, 0, _in0, 0, 4);
   _COPY(_primIn, 4, _in1, 0, 4);
   _COPY(_primIn, 8, _in2, 0, 4);
   _TRY(_nErr, __QAIC_REMOTE(remote_handle_invoke)(_handle, REMOTE_SCALARS_MAKEX(0, _mid, 1, 1, 0, 0), _pra));
   _COPY(_rout3, 0, _primROut, 0, 120);
   _CATCH(_nErr) {}
   return _nErr;
}
__QAIC_STUB_EXPORT INT32_SKEL __QAIC_STUB(adsp_fd_FACEPROC_DT_GetDTConfig)(HDETECTION_SKEL hDT, INT32_SKEL detection_mode, fpsww_dt_params_t_skel* p_dt_new_params) __QAIC_STUB_ATTRIBUTE {
   uint32_t _mid = 58;
   return _stub_method_26(_adsp_fd_handle(), 31, &_mid, (uint32_t*)&hDT, (uint32_t*)&detection_mode, (uint32_t*)p_dt_new_params);
}
#ifdef __cplusplus
}
#endif
#endif //_ADSP_FD_STUB_H
