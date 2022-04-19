/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _CACERT_CALLBACK_H_
#define _CACERT_CALLBACK_H_

/**
 * @file ICertificateCallback.h
 *
 * @brief Certificate Callback (Abstract) Class
 *
 * This file contains the definitions of constants, data structures and
 * interfaces for the Certificate Callback
 *
 * User must override callback methods to process Certificates retrieved by
 * CertificateRequest.
 */
#include <string>
#include <vector>

namespace vendor {
namespace qti {
namespace hardware {
namespace cacert {

/**
 * Certificate result type for CertificateRequest
 */
typedef struct
{
    std::vector<uint8_t> encodedCert;
    std::string alias;
} CertificateEntry;

/**
 * @brief
 * The Certificate Callback class. The user
 * must extend this class with implementation of callback handler.
 */
class ICertificateCallback
{
public:
    ICertificateCallback(){};
    virtual ~ICertificateCallback(){};

    /**
     * @brief
     * Handler to process Certificate Callback associated with
     * a Certificate Request query.
     *
     * This method will be called whenever the \c ICertificateRequest wants to
     * send the response for an earlier CA Certificate query. The client shall
     * override this virtual method to process the retrieved certificates. A DN may
     * match with multiple Certificates, so a vector of Certificates will be passed.
     * Certificates are encoded in ASN.1 DER and represented as a vector of bytes.
     * Each certificate will also come with its alias, which is a unique string
     * used by Android Keystore to identify the certificate.
     *
     * @param[in] transactionId Unique identifier mapping to a CA Certificate query
     * @param[in] certificates A vector of CertificateEntry, where each entry
     *            contains the encoded certificate and alias.
     *
     * @dependencies
     * \c ICertificateRequest must be created by CreateCertificateRequest
     * \c ICertificateRequest::queryCACertificates must be called
     *
     * @return
     * None
    */
    virtual void onCACertificatesAvailable(uint64_t transactionId,
                                           std::vector<CertificateEntry> certificates) = 0;

    ICertificateCallback(const ICertificateCallback&) = delete;
    ICertificateCallback& operator=(const ICertificateCallback&) = delete;
};

} // namespace cacert
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif
