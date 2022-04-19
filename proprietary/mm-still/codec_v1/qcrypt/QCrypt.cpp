/*****************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QCrypt.h"


/*==============================================================================
* Function : New
* Parameters: void
* Return Value : QCrypt * - pointer to new instance, NULL if failure
* Description: QCrypt static factory method
==============================================================================*/
QCrypt * QCrypt::New()
{
  return NULL;
}

/*==============================================================================
* Function : setEncKey
* Parameters: aEncKey, aLen
* Return Value : int
* Description: set encryption key
==============================================================================*/
int QCrypt::setEncKey(char *aEncKey __unused, uint32_t aLen __unused)
{
  return QI_SUCCESS;
}

/*==============================================================================
* Function : setDecKey
* Parameters: aDecKey, aLen
* Return Value : int
* Description: set decryption key
==============================================================================*/
int QCrypt::setDecKey(char *aDecKey __unused, uint32_t aLen __unused)
{
  return QI_SUCCESS;
}

/*==============================================================================
* Function : setEncMsgLen
* Parameters: aEncLen
* Return Value : uint32_t
* Description: Set length of message to be encrypted. Returns max length
* of encrypted message.
==============================================================================*/
uint32_t QCrypt::setEncMsgLen(uint32_t aEncLen __unused)
{
    return 0;
  }

/*==============================================================================
* Function : encrypt
* Parameters: aSrc, aDst
* Return Value : uint32_t
* Description: Encrypt aSrc into aDst. Returns length of encrypted message.
==============================================================================*/
uint32_t QCrypt::encrypt(uint8_t *aSrc __unused, uint8_t *aDst __unused)
{
    return 0;
  }

/*==============================================================================
* Function : setDecMsgLen
* Parameters: dec_len
* Return Value : void
* Description: Set length of message to be decrypted
==============================================================================*/
void QCrypt::setDecMsgLen(uint32_t aDecLen __unused)
{
  return;
}

/*==============================================================================
* Function : decrypt
* Parameters: aSrc, aDst
* Return Value : uint32_t
* Description: Decrypt aSrc into aDst
==============================================================================*/
uint32_t QCrypt::decrypt(uint8_t *aSrc __unused, uint8_t *aDst __unused)
{
    return 0;
  }

/*==============================================================================
* Function : ~QCrypt
* Parameters: void
* Return Value : void
* Description: destructor
==============================================================================*/
QCrypt::~QCrypt()
{
}
