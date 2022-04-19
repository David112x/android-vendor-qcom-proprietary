/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _CACERT_REQUEST_H_
#define _CACERT_REQUEST_H_

/**
 * @file ICertificateRequest.h
 *
 * @brief Certificate Request (Abstract) Class
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Certificate Request
 *
 * Only the interfaces declared shall be used by the user of Certificate
 * Request
 */

#include <memory>
#include <vector>
#include "ICertificateCallback.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace cacert {

typedef enum
{
    SUCCESS             =  0, /**< -- Success */
    FAILED              = -1, /**< -- Generic error */
    SERVICE_UNAVAILABLE = -2, /**< -- Service unavailable */
    INVALID_ARGS        = -5, /**< -- Invalid argument */
    PERM_DENIED         = -6, /**< -- Permission denied */
} RetType;

/**
 * @brief
 * Certificate Request (Abstract) Class.
 */
class ICertificateRequest
{
public:
    /**
     * @brief
     * Creates a Certificate Request instance.
     *
     * The user must call the query method on the instance to initiate the request
     *
     * @dependencies
     * None
     *
     * @return One of the following
     *         <ul>
     *           <li>An instance of ICertificateRequest
     *           <li>nullptr if failed
     *         </ul>
     */
    static std::unique_ptr<ICertificateRequest> CreateCertificateRequest();

    /**
     * @brief
     * Makes request for CA Certificates.
     *
     * This method initiates the request for CA certificates matching the DN.
     * The transactionId will be generated to identify this particular request.
     * The same transactionId will be passed in the callback which will be
     * invoked once the requested certificate is ready.
     *
     * @param[in] encodedDn A byte vector of the DN for the certificate to query
     * represented in its DER encoded form.
     * @param[in] callback A shared pointer of class \c ICertificateCallback
     * @param[out] transactionId Unique value generated for this query used to
     * identify the associated callback invocation
     *
     * @dependencies
     * \c ICertificateRequest must be created
     *
     * @return One of the following codes
     *         <ul>
     *         <li> \c SUCCESS - Success
     *         <li> \c SERVICE_UNAVAILABLE - service error
     *         <li> \c PERM_DENIED - client does not have sufficient permission
     *         <li> \c INVALID_ARGS - invalid params
     *         <li> \c FAIL - general failure
     *         </ul>
     */
    virtual RetType queryCACertificates(
        std::vector<uint8_t> encodedDn,
        std::shared_ptr<ICertificateCallback> callback,
        uint64_t& transactionId
    ) = 0;

    /**
     * @brief Destructor.
     */
    virtual ~ICertificateRequest() {};

    ICertificateRequest(const ICertificateRequest&) = delete;
    ICertificateRequest& operator=(const ICertificateRequest&) = delete;

protected:
    ICertificateRequest() {};
};

} // namespace cacert
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif
