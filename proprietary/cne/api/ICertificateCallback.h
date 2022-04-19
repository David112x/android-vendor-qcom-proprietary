#ifndef _CNE_CERTIFICATE_CALLBACK_H_
#define _CNE_CERTIFICATE_CALLBACK_H_
/**
 * @file ICertificateCallback.h
 *
 * @brief
 * Connectivity Engine Certificate Callback Class Header file
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Connectivity Engine Certificate Callback
 *
 * User must override callback methods to process Certificates retrieved by
 * Connectivity Engine Certificate Request.
 *
 *                   Copyright (c) 2018 Qualcomm Technologies, Inc.
 *                   All Rights Reserved.
 *                   Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <ICneObserverDefs.h>
#include <memory>

/**
 * @brief
 * The connectivity Engine Certificate Callback class. The user
 * must extend this class with implementation of callback handler.
 */
class ICertificateCallback
{
  public:
  ICertificateCallback(){};
  virtual ~ICertificateCallback(){};

  /**
   * @addtogroup cr_api
   * @{
   */
  /**
  @brief
  Handler to process Connectivity Engine Certificate Callback associated with
  a Certificate Request query

  This method will be called whenever the \c ICertificateRequest wants to
  send the response for an earlier CA Certificate query. The client shall
  override this virtual method to process the retrieved certificates. A DN may
  match with multiple Certificates, so a vector of Certificates will be passed.
  Certificates are encoded in ASN.1 DER and represented as a vector of bytes.
  Each certificate will also come with its alias, which is a unique string
  used by Android Keystore to identify the certificate.

  @param[in] transactionId Unique identifier mapping to a CA Certificate query
  @param[in] certificates A vector of CertificateEntry, where each entry
    contains the encoded certificate and alias.

  @dependencies
  \c ICertificateRequest must be created by CneFactory
  \c ICertificateRequest::queryCACertificates must be called

  @return
  None

  @sa
  \c CneFactory::CreateCertificateRequest,
  \c ICertificateRequest::queryCACertificates
  */
  virtual void onCACertificatesAvailable(uint64_t transactionId,
        std::vector<CertificateEntry> certificates) = 0;

  /**
   * @}
   */
};

#endif /* _CERTIFICATE_CALLBACK_H_ */
