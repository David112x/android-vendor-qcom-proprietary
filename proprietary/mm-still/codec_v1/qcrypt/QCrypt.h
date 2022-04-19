/*****************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QCRYPT_H__
#define __QCRYPT_H__

#include "QICommon.h"

extern "C" {
#include <stdlib.h>
}

/*===========================================================================
 * Class: QCrypt
 *
 * Description: This class represents an asymmetric key encryption engine
 *
 * Notes: none
 *==========================================================================*/
class QCrypt {

public:

  /** New:
   * static factory method
   */
  static QCrypt *New();

  /** ~QCrypt:
   *
   *  QCrypt class destructor
   **/
  virtual ~QCrypt();

  /** setEncKey:
   * @encKey - encryption key.
   *
   * Set the encryption key
   */
  virtual int setEncKey(char *aEncKey, uint32_t aLen);

  /** setDecKey:
   * @decKey - decryption key.
   *
   * Set the decryption key
   */
  virtual int setDecKey(char *aDecKey, uint32_t aLen);

  /** setEncMsgLen:
   * @enc_len - message len
   *
   * Set length of message to be encrypted.
   *
   */
  virtual uint32_t setEncMsgLen(uint32_t aEncLen);

  /** setDecMsgLen:
   * @dec_len - message len
   *
   * Set length of message to be encrypted.
   *
   */
  virtual void setDecMsgLen(uint32_t aDecLen);

  /** encrypt:
   * @src - Source message buffer
   * @dst - Destination buffer
   *
   * Encrypts message. Returns total encrypted message length.
   *
   */
  virtual uint32_t encrypt(uint8_t *aSrc, uint8_t *aDst);

  /** decrypt:
   * @src - Source message buffer
   * @dst - Destination buffer
   *
   * Decrypts message. Returns total decrypted message length.
   *
   */
  virtual uint32_t decrypt(uint8_t *aSrc, uint8_t *aDst);

};

#endif
