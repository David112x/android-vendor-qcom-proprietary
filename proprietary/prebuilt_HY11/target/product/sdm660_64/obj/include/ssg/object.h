// Copyright (c) 2015 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.

//------------------------------------------------------------------------
// object.h : invocable objects
//------------------------------------------------------------------------

#ifndef __OBJECT_H
#define __OBJECT_H

#include <stddef.h>    // size_t
#include <stdint.h>    // int32_t


//------------------------------------------------------------------------
// ObjectOp
//------------------------------------------------------------------------

typedef uint32_t ObjectOp;

// Method bits are not modified by transport layers.  These describe the
// method (member function) being requested by the client.

#define ObjectOp_METHOD_MASK     ((ObjectOp) 0x0000FFFFu)
#define ObjectOp_methodID(op)    ((op) & ObjectOp_METHOD_MASK)

// end of user-defined range

#define ObjectOp_METHOD_USERMAX  ((ObjectOp) 0x00003FFF)

// Modifier bits are reserved for transport-layer semantics.

#define ObjectOp_MODIFIER_MASK   ((ObjectOp) 0xFFFF0000u)

// ObjectOp_REMOTE_BUFS is set by transports when buffers may reside in
//    untrusted memory and buffer alignment is untrusted. Implementations of
//    invoke may skip some validation and/or copying when this is clear.

#define ObjectOp_REMOTE_BUFS     ((ObjectOp) 0x00010000u)

// Potential extensions...
// #define ObjectOp_ONEWAY       ((ObjectOp) 0x01000000u)
// #define ObjectOp_COALESCE     ((ObjectOp) 0x02000000u)
// #define ObjectOp_REALTIME     ((ObjectOp) 0x04000000u)

// Local ops are not forwarded by transports.

#define ObjectOp_LOCAL           ((ObjectOp) 0x00008000U)

#define ObjectOp_isLocal(op)     (((op) & ObjectOp_LOCAL) != 0)


//------------------------------------------------------------------------
// ObjectCounts
//
// The number and kinds of arguments passed to invoke are encoded in a
// 32-bit quantity `ObjectCounts`. Currently only 16-bits are used; the
// remainder are reserved for future enhancements.
//------------------------------------------------------------------------

typedef uint32_t ObjectCounts;

#define ObjectCounts_pack(nBuffersIn, nBuffersOut, nObjectsIn, nObjectsOut) \
   ((ObjectCounts) ((nBuffersIn) |         \
                    ((nBuffersOut) << 4) | \
                    ((nObjectsIn) << 8)  | \
                    ((nObjectsOut) << 12)))

#define ObjectCounts_maxBI   0xF
#define ObjectCounts_maxBO   0xF
#define ObjectCounts_maxOI   0xF
#define ObjectCounts_maxOO   0xF

// unpack counts

#define ObjectCounts_numBI(k)   ( (size_t) ( ((k) >> 0) & ObjectCounts_maxBI) )
#define ObjectCounts_numBO(k)   ( (size_t) ( ((k) >> 4) & ObjectCounts_maxBO) )
#define ObjectCounts_numOI(k)   ( (size_t) ( ((k) >> 8) & ObjectCounts_maxOI) )
#define ObjectCounts_numOO(k)   ( (size_t) ( ((k) >> 12) & ObjectCounts_maxOO) )

// Indices into args[]

#define ObjectCounts_indexBI(k)   0
#define ObjectCounts_indexBO(k)   (ObjectCounts_indexBI(k) + ObjectCounts_numBI(k))
#define ObjectCounts_indexOI(k)   (ObjectCounts_indexBO(k) + ObjectCounts_numBO(k))
#define ObjectCounts_indexOO(k)   (ObjectCounts_indexOI(k) + ObjectCounts_numOI(k))
#define ObjectCounts_total(k)     (ObjectCounts_indexOO(k) + ObjectCounts_numOO(k))


//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

typedef struct Object Object;
typedef struct ObjectBuf ObjectBuf;
typedef struct ObjectBufIn ObjectBufIn;
typedef union ObjectArg ObjectArg;
typedef void *ObjectCxt;

typedef int32_t (*ObjectInvoke)(ObjectCxt h,
                                ObjectOp op,
                                ObjectArg *args,
                                ObjectCounts counts);

struct Object {
   ObjectInvoke invoke;
   ObjectCxt context;    // context data to pass to the invoke function
};

struct ObjectBuf {
   void *ptr;
   size_t size;
};

struct ObjectBufIn {
   const void *ptr;
   size_t size;
};

union ObjectArg {
   ObjectBuf b;
   ObjectBufIn bi;
   Object o;
};


static inline int32_t Object_invoke(Object o, ObjectOp op, ObjectArg *args, ObjectCounts k)
{
   return o.invoke(o.context, op, args, k);
}


#define Object_NULL                ( (Object) { NULL, NULL } )


//------------------------------------------------------------------------
// Object_invoke return codes
//------------------------------------------------------------------------
//
// A value of zero (Object_OK) indicates that the invocation has succeeded.
// Negative values are reserved for use by transports -- implementations of
// invoke() that forward requests to another protection domain -- and
// indicate problems communicating between domains.  Positive error codes
// are sub-divided into generic and user-defined errors.  Generic errors are
// can be used by IDL-generated code and object implementations.
// User-case-specific error codes should be allocated from the user defined
// range.
//

#define Object_isOK(err)        ((err) == 0)
#define Object_isERROR(err)     ((err) != 0)

/*
 * Object_OK indicates that the invocation has succeeded.
 */
#define Object_OK                  0

/*
 * Object_ERROR is non-specific, and can be used whenever the error
 * condition does not need to be distinguished from others in the interface.
 */
#define Object_ERROR               1

/*
 * Object_ERROR_INVALID indicates that the request was not understood by the
 * object.  This can result when `op` is unrecognized, or when the number
 * and/or sizes of arguments does not match what is expected for `op`.
 */
#define Object_ERROR_INVALID       2

/*
 * Object_ERROR_SIZE_IN that an input buffer was too large to be marshaled.
 */
#define Object_ERROR_SIZE_IN       3

/*
 * Object_ERROR_SIZE_OUT that an output buffer was too large to be
 * marshaled.
 */
#define Object_ERROR_SIZE_OUT      4

/*
 * Object_ERROR_USERBASE is the beginning of the user-defined range.  Error
 * codes in this rangecan be defined on an object-by-object or
 * interface-by-interface basis.  IDL-specified error codes are allocated
 * from this range.
 */
#define Object_ERROR_USERBASE     10

/*
 * DEFUNCT indicates that the object reference will no longer work.  This is
 * returned when the process hosting the object has terminated, or when the
 * communication link to the object is unrecoverably lost.
 */
#define Object_ERROR_DEFUNCT     -90

/*
 * Object_ERROR_ABORT indicates that the caller should return to the point
 * at which it was invoked from a remote domain.  Unlike other error codes,
 * this pertains to the state of the calling thread, not the state of the
 * target object or transport.
 *
 * For example, when a process is terminated while a kernel thread is
 * executing on its behalf, that kernel thread should return back to its
 * entry point so that it can be reaped safely.  (Synchronously killing the
 * thread could leave kernel data structures in a corrupt state.)  If it
 * attempts to invoke an object that would result in it blocking on another
 * thread (or if it is already blocking in an invokcation) the invocation
 * will immediately return this error code.
 */
#define Object_ERROR_ABORT       -91

/*
 * Object_ERROR_BADOBJ indicates that the caller provided a mal-formed
 * Object structure as a target object or an input parameter.  In general,
 * mal-formed Object structures cannot be reliably distinguished from valid
 * ones, since Object contains a function pointer and a context pointer.  In
 * the case of some transports, however, such as the Mink user-to-kernel
 * transport, an invalid context value will be detected and will result in
 * this error code.
 */
#define Object_ERROR_BADOBJ      -92

/*
 * Object_ERROR_NOSLOTS indicates that an object could not be returned
 * because the calling domain has reached the maximum number of remote
 * object references on this transport.
 */
#define Object_ERROR_NOSLOTS     -93

/*
 * Object_ERROR_MAXARGS indicates that the `args` array length exceeds the
 * maximum supported by the object or by some transport between the caller
 * and the object.
 */
#define Object_ERROR_MAXARGS     -94

/*
 * Object_ERROR_MAXDATA indicates the the complete payload (input buffers
 * and/or output buffers) exceed
 */
#define Object_ERROR_MAXDATA     -95

/*
 * Object_ERROR_UNAVAIL indicates that the destination process cannot
 * fulfill the request at the current time, but that retrying the operation
 * in the future might result in success.
 *
 * This may be a result of resource constraints, such as exhaustion of the
 * object table in the destination process.
 */
#define Object_ERROR_UNAVAIL     -96

/*
 * Object_ERROR_KMEM indicates a failure of memory allocation outside of the
 * caller's domain and outside of the destination domain.
 *
 * This may occur when marshaling objects.  It may also occur when passing
 * strings or other buffers that must be copied for security reasons in the
 * destination domain.
 */
#define Object_ERROR_KMEM        -97

/*
 * Object_ERROR_REMOTE indicates that a *local* operation has been requested
 * when the target object is remote.  Transports do not forward local
 * operations.
 */
#define Object_ERROR_REMOTE      -98


//------------------------------------------------------------------------
// Base interface: `Object` also serves as the name of the interface
//     that all interfaces implicitly inherit.
//------------------------------------------------------------------------

#define Object_OP_release       (ObjectOp_METHOD_MASK - 0)
#define Object_OP_retain        (ObjectOp_METHOD_MASK - 1)
#define Object_OP_unwrapFd      (ObjectOp_METHOD_MASK - 2)


//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------

// OBJECT_NOT_RETAINED and OBJECT_CONSUMED can be used to annotate
// parameters or return values that deviate from ordinary reference counting
// conventions.  Note that all

// This is used when a returned value or output parameter conveys an object
// reference but the function will not increment the reference count.  In
// these cases, the caller is not responsible for releasing it.  The object
// reference count is guaranteed to be non-zero because the caller knows of
// some other reference to the object (usually an input parameter to the
// same function).
#define OBJECT_NOT_RETAINED

// This is used when an input parameter is an object reference that will be
// released by the function.
#define OBJECT_CONSUMED

static inline int32_t Object_release(OBJECT_CONSUMED Object o) {
  return Object_invoke((o), Object_OP_release, 0, 0);
}
static inline int32_t Object_retain(Object o) {
  return Object_invoke((o), Object_OP_retain, 0, 0);
}
static inline int32_t Object_unwrapFd(Object o, int* pFd) {
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { pFd, sizeof(int) * 1 };
  return Object_invoke((o), Object_OP_unwrapFd, a, ObjectCounts_pack(0, 1, 0, 0));
}

#define Object_isNull(o)           ( (o).invoke == NULL )


#define Object_RELEASE_IF(o)                                            \
   do { Object o_ = (o); if (!Object_isNull(o_)) (void) Object_release(o_); } while (0)

static inline void Object_replace(Object *loc, Object objNew)
{
  if (!Object_isNull(*loc)) {
    Object_release(*loc);
  }
  if (!Object_isNull(objNew)) {
    Object_retain(objNew);
 }
  *loc = objNew;
}

#define Object_ASSIGN(loc, obj)  Object_replace(&(loc), (obj))
#define Object_ASSIGN_NULL(loc)  Object_replace(&(loc), Object_NULL)

#define Object_INIT(loc, obj)                   \
  do {                                          \
    Object o_ = (obj);                          \
    (loc) = o_;                                 \
    if (!Object_isNull(o_)) {                   \
      Object_retain(o_);                        \
    }                                           \
  } while(0)

#endif /* __OBJECT_H */
