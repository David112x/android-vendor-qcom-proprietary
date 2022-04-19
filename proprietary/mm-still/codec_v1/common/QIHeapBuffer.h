/*****************************************************************************
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIHEAPBUFFER_H__
#define __QIHEAPBUFFER_H__

#include "QIBuffer.h"
#include <stdlib.h>

/*===========================================================================
 * Class: QIHeapBuffer
 *
 * Description: This class represents the buffer allocated in heap memory
 *
 *
 * Notes: none
 *==========================================================================*/
class QIHeapBuffer : public QIBuffer {

public:

  /** New:
   *
   *  2 phase constructor for heap buffer
   **/
  static QIHeapBuffer *New(uint32_t aLength);

  /** ~QIHeapBuffer:
   *
   *  destructor
   **/
  virtual ~QIHeapBuffer();

private:

  /** QIHeapBuffer:
   *
   *  private constructor
   **/
  QIHeapBuffer();

private:

  /**
   *  member variables
   **/
};

#endif //__QIHEAPBUFFER_H__
