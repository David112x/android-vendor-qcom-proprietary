/*******************************************************************************
* Copyright (c) 2012, 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*******************************************************************************/

#ifndef __QIMESSAGE_H__
#define __QIMESSAGE_H__

#include "QIBase.h"
#include "qomx_common.h"
#include "QIQueue.h"

/*===========================================================================
 * Class: QIMessage
 *
 * Description: This class represents the OMX message objects
 *
 * Notes: none
 *==========================================================================*/
class QIMessage: public QIQueueObject
{
public:

  /** QIMessage:
   *
   * constructor
   **/
  QIMessage();

  /** ~QIMessage:
   *
   * destructor
   **/
  virtual ~QIMessage();

  /** m_qMessage:
   *
   * message type
   **/
  qomx_message_t m_qMessage;

  /**
   *  @pData: pointer to the data
   *  @iData: integer data
   *
   *  message data
   **/
  union {
    void *pData;
    int iData;
  };
};

#endif


