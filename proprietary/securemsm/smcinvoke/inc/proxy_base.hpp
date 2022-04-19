/***********************************************************
 Copyright (c) 2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
************************************************************/
/* c++ proxy base class */
#ifndef __PROXY_BASE_HPP
#define __PROXY_BASE_HPP
#include "object.h"

class ProxyBase {
   public:
    ProxyBase() : me_(Object_NULL) {}

    explicit ProxyBase(OBJECT_CONSUMED Object o) : me_(o) {}

    // Copy constructor
    ProxyBase(const ProxyBase& rhs) : me_(rhs.me_) {
        if (!Object_isNull(me_)) {
            Object_retain(me_);
        }
    }

    // Assignment operator (proxy-managed object)
    ProxyBase& operator = (const ProxyBase& rhs) {
        // Check for assignment to self
        if (this == &rhs) {
            return *this;
        }
        // Another sort of assignment to self.  We could do retain/release in
        // this case, but we'd just end up back where we started.
        if (me_.invoke != rhs.me_.invoke || me_.context != rhs.me_.context) {
            Object_replace(&me_, rhs.me_);
        }
        return *this;
    }

    // Behaves similarly to shared_ptr::get().
    // This method can be used to call methods that require a raw object.
    OBJECT_NOT_RETAINED Object get() const {
        return me_;
    }

    // Behaves similarly to shared_ptr::reset().
    // After this call, the caller must not directly use the argument.
    void consume(OBJECT_CONSUMED Object& rhs) {
        // Check for sort of assignment to self.  We could do retain/release in
        // this case, but we'd just end up back where we started.
        if (me_.invoke != rhs.invoke || me_.context != rhs.context) {
            if (!Object_isNull(me_)) {
                Object_release(me_);
            }
            me_ = rhs;
        }
    }

    // Force the proxy to stop managing its object.
    // There is no shared_ptr method that corresponds to this method.
    // This should typically only be used by auto-generated marshalling code.
    inline Object extract() {
        Object retval = me_;
        me_ = Object_NULL;
        return retval;
    }

    // Destructor
    virtual ~ProxyBase() {
        if (!Object_isNull(me_)) {
            Object_release(me_);
        }
    }

    bool isNull() {
        return Object_isNull(me_);
    }

   protected:
    inline int32_t invoke(ObjectOp op, ObjectArg *args, ObjectCounts k) {
        return Object_invoke(me_, op, args, k);
    }

   private:
    Object me_;
};

#endif // __PROXY_BASE_HPP
