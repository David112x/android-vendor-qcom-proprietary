/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <string>
#include <vector>
#include "JsonHandlerSDK.h"

class SensingReadDataRequest : public JsonDeviceRequest
{
public:
    explicit SensingReadDataRequest(const Json::Value& jsonRequestValue) :
        JsonDeviceRequest(jsonRequestValue)
    {
    }

    JsonValueBoxed<uint32_t> GetMaxSizeDwords() const
    {
        return JsonValueParser::ParseUnsignedValue(m_jsonRequestValue, "MaxSizeDwords");
    }
};

class SensingReadDataResponse final : public JsonResponse
{
public:
    SensingReadDataResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
        JsonResponse("SensingReadDataResponse", jsonRequestValue, jsonResponseValue)
    {}

    // Passed data string is expected to be encoded Base64
    void SetDataBase64(const std::string& dataBase64)
    {
        m_jsonResponseValue["DataBase64"] = dataBase64;
    }

    void SetDriTsfs(const std::vector<uint64_t>& driTsfVec);
};

class SensingReadStatsResponse final : public JsonResponse
{
public:
    SensingReadStatsResponse(const Json::Value& jsonRequestValue, Json::Value& jsonResponseValue) :
        JsonResponse("SensingReadStatsResponse", jsonRequestValue, jsonResponseValue)
    {}

    void SetDropCntFromLastRead(uint32_t dropCnt)
    {
        m_jsonResponseValue["DropCntFromLastRead"] = dropCnt;
    }

    void SetNumRemainingBursts(uint32_t numRemainingBursts)
    {
        m_jsonResponseValue["NumRemainingBursts"] = numRemainingBursts;
    }
};

class SensingSetReadTimeoutRequest : public JsonDeviceRequest
{
public:
    explicit SensingSetReadTimeoutRequest(const Json::Value& jsonRequestValue) :
        JsonDeviceRequest(jsonRequestValue)
    {
    }

    JsonValueBoxed<uint32_t> GetReadTimeoutMsec() const
    {
        return JsonValueParser::ParseUnsignedValue(m_jsonRequestValue, "ReadTimeoutMsec");
    }
};

class SensingReadDataHandler : public JsonOpCodeHandlerBase<SensingReadDataRequest, SensingReadDataResponse>
{
private:
    void HandleRequest(const SensingReadDataRequest& jsonRequest, SensingReadDataResponse& jsonResponse) override;
};

class SensingReadStatsHandler : public JsonOpCodeHandlerBase<Json::Value, SensingReadStatsResponse>
{
private:
    void HandleRequest(const Json::Value& jsonRequest, SensingReadStatsResponse& jsonResponse) override;
};

class SensingClearDataHandler : public JsonOpCodeHandlerBase<Json::Value, JsonBasicResponse>
{
private:
    void HandleRequest(const Json::Value& jsonRequest, JsonBasicResponse& jsonResponse) override;
};

class SensingSetReadTimeoutHandler : public JsonOpCodeHandlerBase<SensingSetReadTimeoutRequest, JsonBasicResponse>
{
private:
    void HandleRequest(const SensingSetReadTimeoutRequest& jsonRequest, JsonBasicResponse& jsonResponse) override;
};

class SensingCancelReadHandler : public JsonOpCodeHandlerBase<Json::Value, JsonBasicResponse>
{
private:
    void HandleRequest(const Json::Value& jsonRequest, JsonBasicResponse& jsonResponse) override;
};
