#ifndef _CNE_CERTIFICATE_REQUEST_H_
#define _CNE_CERTIFICATE_REQUEST_H_
/**
 * @file ICertificateRequest.h
 *
 * @brief Certificate Request Class
 *        (Abstract)
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Certificate Request
 *
 * Only the interfaces declared shall be used by the user of Certificate
 * Request
 *
 *                   Copyright (c) 2018 Qualcomm Technologies, Inc.
 *                   All Rights Reserved.
 *                   Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <memory>

/**
 * @brief
 * Certificate Request interface class (abstract).
 */
class ICertificateRequest
{
  public:

  /**
   * @addtogroup cr_api
   * @{
   */

  /**
  @brief
  makes request to CNE for CA Certificates.

  This method initiates the request for CA certificates matching the DN.
  The transactionId will be set by CNE to identify this particular request.
  The same transactionId will be passed in the callback which will be
  invoked once the requested certificate is ready.

  @param[in] encodedDn A byte vector of the DN for the certificate to query
    represented in its DER encoded form.
  @param[in] callback A shared pointer of class \c ICertificateCallback
  @param[out] transactionId Unique value generated for this query used to
    identify the associated callback invocation

  @dependencies
  \c ICertificateRequest must be created

  @return One of the following codes
          <ul>
            <li> \c CO_RET_SUCCESS - Success
            <li> \c CO_RET_SERVICE_UNAVAILABLE - CNR service error
            <li> \c CO_RET_PERM_DENIED - client does not have sufficient permission
            <li> \c CO_RET_INVALID_ARGS - invalid params
            <li> \c CO_RET_FAIL - general failure
          </ul>
  @sa
  \c CneFactory::CreateCertificateRequest
  */
  virtual CORetType queryCACertificates (std::vector<uint8_t> encodedDn,
        std::shared_ptr<ICertificateCallback> callback, uint64_t& transactionId) = 0;

  /**
    * @}
    */
   /**
   @brief
   Destructor.
   */
   virtual ~ICertificateRequest(){};

};

#endif /* _CERTIFICATE_REQUEST_H_ */
