/***********************************************************
 Copyright (c) 2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
************************************************************/
/* c++ implementation skeleton base class */
#ifndef __IMPL_BASE_HPP
#define __IMPL_BASE_HPP
#include "object.h"
#ifdef _LIBCPP_HAS_NO_THREADS
#define ATOMIC(x) x
#else
#include <atomic>
#define ATOMIC(x) std::atomic<x>
#endif
#include <cassert>

class ImplBase {
   public:
    ImplBase() : refcount(1) {};
    virtual ~ImplBase() {};

    // Standard reference counting boilerplate.
    static int32_t invoke(ObjectCxt context,
                          ObjectOp op,
                          ObjectArg* args,
                          ObjectCounts arg_count) {
        ImplBase* me = (ImplBase*)context;
        switch (ObjectOp_methodID(op)) {
            case Object_OP_release: {
                if (me->release()) {
                    delete (me);
                }
                return Object_OK;
            }
            case Object_OP_retain: {
                me->retain();
                return Object_OK;
            }
            default:
                return me->invoke(op, args, arg_count);
        }
    }

   protected:
    // Auto-generated classes override this function
    virtual int32_t invoke(ObjectOp op, ObjectArg* args, ObjectCounts arg_count) = 0;

    // Impl classes may override retain and release, e.g. for singletons
    virtual void retain() {
        auto old_value = refcount++;
        assert(old_value > 0);
    }

    virtual bool release() {
        auto old_value = refcount--;
        assert(old_value > 0);
        return old_value == 1;
    }

   private:
    ATOMIC(uint32_t) refcount;
};

#endif // __IMPL_BASE_HPP
