/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef KEYMASTER_4_0_HMAC_BUILDER_H_
#define KEYMASTER_4_0_HMAC_BUILDER_H_

#include <km_main.h>

namespace keymaster {

template <class T>
class Vector {

  public:
    Vector() : data_(0), capacity_(0), size_(0) {
      reserve(initialSize);
    }

    ~Vector() {
      clear();
    }

    void clear() {
      delete[] data_;
      data_ = 0;
    }

    void reserve( unsigned n) {
      if ( n > capacity_ ) {
        // reserve more memory
        T* p = new T[n];

        // copy old data
        for (unsigned i=0; i < size_; ++i) {
          new (p+i) T( *(data_ + i) );  // in-place copy-construct
        }
        delete[] data_;
        data_     = p;
        capacity_ = n;
      }
    }

    void push_back(const T& t) {
      assert(capacity_ != 0);

      if (size_ == capacity_ ) {
        reserve( capacity_ * 2 );
      }

      new(data_+size_) T(t); // in-place construct
      ++size_;
    }

    const T& operator[](unsigned i) const
    {
      if (i < size_)
        return *(data_+i);
      else
        return *(data_+size_);
    }

    unsigned size() const { return size_; }

  private:
    T* data_;            // array of pointers
    unsigned capacity_;  // allocated
    unsigned size_;      // used
    enum { initialSize = 2};

}; // class Vector

class HmacSharingParamsBuilder {
  public:
    HmacSharingParamsBuilder() : Vec() {}

    HmacSharingParamsBuilder& HmacParams(hmac_sharing_params_t* params) {
        Vec.push_back(*params);
        return *this;
    }

    std::vector<hmac_sharing_params_t> build() const {
        std::vector<hmac_sharing_params_t> vec;
        for(size_t i = 0; i < Vec.size(); i++)
          vec.push_back(Vec[i]);
        return vec;
    }

  private:
    keymaster::Vector<hmac_sharing_params_t> Vec;

};// class HmacSharingParamsBuilder

}// namespace keymaster

#endif /* KEYMASTER_4_0_HMAC_BUILDER_H_ */
