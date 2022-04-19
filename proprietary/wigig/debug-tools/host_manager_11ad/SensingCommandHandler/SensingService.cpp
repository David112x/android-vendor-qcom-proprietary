/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "SensingService.h"
#include "DebugLogger.h"
#ifdef _WIGIG_SLPI_SENSING_
    #include <map>
    #include <sstream>
    #include <functional>
    #include "Utils.h"
    #include "DebugLogger.h"
    #include "wigig_sensing_lib.h"
#endif // _WIGIG_SLPI_SENSING_

namespace // unnamed
{
#ifndef _WIGIG_SLPI_SENSING_
     const char* sensingNotSupportedStr = "Sensing is not supported on the current platform";
#endif
}

SensingService& SensingService::GetInstance()
{
    static SensingService instance;
    return instance;
}

SensingService::SensingService() : m_sensingHandle(nullptr)
{}

SensingService::~SensingService()
{
    // make sure to close the sensing upon exit, no harm if already closed
    CloseSensing();
}

bool SensingService::IsRegistered() const
{
    return m_sensingHandle != nullptr;
}

OperationStatus SensingService::ValidateAvailability(const std::string& requestStr) const
{
    if (IsRegistered())
    {
        return OperationStatus(true);
    }

    const char* errStr = "Sensing is not opened";
    LOG_ERROR << requestStr << " request failed: " << errStr << std::endl;
    return OperationStatus(false, errStr);
}

OperationStatus SensingService::OpenSensing(bool autoRecovery)
{
#ifdef _WIGIG_SLPI_SENSING_
    // Cannot register twice, check registration state first
    // Enable and Connect can be called multiple times for successfully registered client
    if (!IsRegistered())
    {
        LOG_DEBUG << __FUNCTION__ << ": open sensing request arguments for registration: 'autoRecovery'=" << BoolStr(autoRecovery) << std::endl;

        auto res = wigig_sensing_lib_register(&m_sensingHandle, autoRecovery);
        if ( !(res == WIGIG_SENSING_LIB_NO_ERR && IsRegistered()) )
        {
            const char* errStr = wigig_sensing_lib_translate_error(res);
            LOG_ERROR << "Failed to Open Sensing. Registration request failed: " << errStr << " [" << res << "]" << std::endl;
            return OperationStatus(false, errStr);
        }
        LOG_DEBUG << __FUNCTION__ << ": received sensing handle is " << m_sensingHandle << std::endl;
    }

    return OperationStatus(true);
#else
    (void)autoRecovery;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::GetParameters(std::vector<unsigned char>& parameters) const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("get parameters");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    parameters.clear();
    parameters.resize(WIGIG_SENSING_PARAMETERS_MAX_SIZE);

    uint32_t receivedBytes = 0U;
    auto res = wigig_sensing_lib_get_parameters(m_sensingHandle, parameters.data(), parameters.size(), &receivedBytes);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to get Sensing parameters: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    LOG_ASSERT(receivedBytes <= parameters.size());

    // it is important to shrink the vector to the actual size in use
    parameters.resize(receivedBytes);
    LOG_DEBUG << __FUNCTION__ << ": number of Bytes in parameters received from WiGig Sensing Lib is " << receivedBytes << std::endl;

    return OperationStatus(true);
#else
    (void)parameters;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::ChangeMode(const std::string& modeStr, uint32_t preferredChannel, uint32_t& burstSizeBytes) const
{
#ifdef _WIGIG_SLPI_SENSING_
    static const std::map<std::string, wigig_sensing_lib_mode_type> sc_modeTranslationMap =
    {
        { "Stop",    WIGIG_SENSING_LIB_STOP_MODE },
        { "Search",  WIGIG_SENSING_LIB_SEARCH_MODE },
        { "Facial",  WIGIG_SENSING_LIB_FACIAL_MODE },
        { "Gesture", WIGIG_SENSING_LIB_GESTURE_MODE },
        { "Custom",  WIGIG_SENSING_LIB_CUSTOME_MODE },
    };

    auto valdityRes = ValidateAvailability("change mode");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    const auto it = sc_modeTranslationMap.find(modeStr);
    if (it == sc_modeTranslationMap.cend())
    {
        std::vector<std::string> modes;
        modes.reserve(sc_modeTranslationMap.size());
        for (const auto& modePair : sc_modeTranslationMap)
        {
            modes.push_back(modePair.first);
        }

        const std::string errStr = "Unknown mode " + modeStr + ". Supported modes: " + Utils::Concatenate(modes, ", ");
        LOG_ERROR << "Failed to change Sensing mode: " << errStr << std::endl;
        return OperationStatus(false, errStr);
    }

    // note: no test for channel validity by design, validated by sensing library

    LOG_DEBUG << __FUNCTION__ << ": change mode request arguments are 'mode'=" << modeStr
        << ", 'preferredChannel'=" << preferredChannel << std::endl;

    auto res = wigig_sensing_lib_change_mode(m_sensingHandle, it->second, preferredChannel, &burstSizeBytes);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to change Sensing mode: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    LOG_DEBUG << __FUNCTION__ << ": received burst size is " << burstSizeBytes << " [Bytes]" << std::endl;
    return OperationStatus(true);
#else
    (void)modeStr;
    (void)preferredChannel;
    (void)burstSizeBytes;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::GetMode(std::string& modeStr) const
{
#ifdef _WIGIG_SLPI_SENSING_
    static const std::map<wigig_sensing_lib_mode_type, std::string> sc_modeTranslationMap =
    {
        { WIGIG_SENSING_LIB_STOP_MODE,    "Stop" },
        { WIGIG_SENSING_LIB_SEARCH_MODE,  "Search" },
        { WIGIG_SENSING_LIB_FACIAL_MODE,  "Facial" },
        { WIGIG_SENSING_LIB_GESTURE_MODE, "Gesture" },
        { WIGIG_SENSING_LIB_CUSTOME_MODE, "Custom" },
    };

    auto valdityRes = ValidateAvailability("get mode");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    wigig_sensing_lib_mode_type sensingMode;
    auto res = wigig_sensing_lib_get_mode(m_sensingHandle, &sensingMode);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to get Sensing mode: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    const auto it = sc_modeTranslationMap.find(sensingMode);
    if (it == sc_modeTranslationMap.cend())
    {
        std::ostringstream oss; // to_string is not supported by GCC 4.8.3
        oss << "Received unknown mode ";
        oss << static_cast<int>(sensingMode);
        LOG_ERROR << "Failed to get Sensing mode: " << oss.str() << std::endl;
        return OperationStatus(false, oss.str());
    }

    modeStr = it->second;
    LOG_DEBUG << __FUNCTION__ << ": received sensing mode is " << modeStr << std::endl;
    return OperationStatus(true);
#else
    (void)modeStr;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::ReadData(uint32_t maxSizeBytes, std::vector<unsigned char>& sensingData,
                                         std::vector<uint64_t>& driTsfVec) const
{
#ifdef _WIGIG_SLPI_SENSING_
    static const uint32_t sc_maxNumBursts = 256U; // by now the size is constant and is limited to support up to 256 bursts

    auto valdityRes = ValidateAvailability("read data");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    if (maxSizeBytes > CYCLIC_BUF_MAX_SIZE)
    {
        std::ostringstream oss; // to_string is not supported by GCC 4.8.3
        oss << "Received 'MaxSizeDwords' exceeds the maximum of " << CYCLIC_BUF_MAX_SIZE << " Bytes";
        LOG_ERROR << "Failed to read Sensing data: " << oss.str() << std::endl;
        return OperationStatus(false, oss.str());
    }

    LOG_DEBUG << __FUNCTION__ << ": read data request arguments are 'maxSizeBytes'=" << maxSizeBytes
        << ", max num bursts used=" << sc_maxNumBursts << std::endl;

    driTsfVec.clear();
    driTsfVec.resize(sc_maxNumBursts);
    uint32_t driTsfVecLengthBytes = driTsfVec.size() * sizeof(uint64_t);

    sensingData.clear();
    sensingData.resize(maxSizeBytes);
    uint32_t sensingDataLengthBytes = sensingData.size() * sizeof(unsigned char);

    uint32_t dataReceivedBytes = 0U, numDriTsfsReceived = 0U;
    auto res = wigig_sensing_lib_read_data(m_sensingHandle,
                                           sensingData.data(), sensingDataLengthBytes, &dataReceivedBytes,
                                           driTsfVec.data(), driTsfVecLengthBytes, &numDriTsfsReceived);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to read Sensing data: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    LOG_ASSERT(dataReceivedBytes <= sensingData.size());
    // it is important to shrink the vector to the actual size in use
    sensingData.resize(dataReceivedBytes);

    LOG_ASSERT(numDriTsfsReceived <= driTsfVec.size());
    // it is important to shrink the vector to the actual size in use
    driTsfVec.resize(numDriTsfsReceived);

    LOG_DEBUG << __FUNCTION__ << ": received " << dataReceivedBytes
        << " Bytes of data and " << numDriTsfsReceived << " DRI TSFs" << std::endl;

    return OperationStatus(true);
#else
    (void)maxSizeBytes;
    (void)sensingData;
    (void)driTsfVec;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::ReadStats(uint32_t& dropCntFromLastRead, uint32_t& numRemainingBursts) const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("read stats");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    auto res = wigig_sensing_lib_read_stats(m_sensingHandle, &dropCntFromLastRead, &numRemainingBursts);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to read Sensing stats: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    LOG_DEBUG << __FUNCTION__ << ": received drop count from the last read is " << dropCntFromLastRead
        << ", number of remaining bursts is " << numRemainingBursts << std::endl;

    return OperationStatus(true);
#else
    (void)dropCntFromLastRead;
    (void)numRemainingBursts;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::ClearData() const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("clear data");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    auto res = wigig_sensing_lib_clear_data(m_sensingHandle);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to clear sensing data: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    return OperationStatus(true);
#else
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::SetReadTimeout(uint32_t readTimeoutMsec) const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("set read timeout");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    LOG_DEBUG << __FUNCTION__ << ": set read timeout request argument is 'readTimeoutMsec'=" << readTimeoutMsec << std::endl;

    auto res = wigig_sensing_lib_set_timeout(m_sensingHandle, readTimeoutMsec);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to set timeout for read of Sensing data: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    return OperationStatus(true);
#else
    (void)readTimeoutMsec;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::CancelRead() const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("cancel read");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    auto res = wigig_sensing_lib_cancel_read(m_sensingHandle);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to cancel read of Sensing data: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    return OperationStatus(true);
#else
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::GetNumRemainingBursts(uint32_t& numRemainingBursts) const
{
#ifdef _WIGIG_SLPI_SENSING_
    auto valdityRes = ValidateAvailability("get num remaining bursts");
    if (!valdityRes.IsSuccess())
    {
        return valdityRes;
    }

    auto res = wigig_sensing_lib_get_num_avail_bursts(m_sensingHandle, &numRemainingBursts);
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to get number of remaining bursts: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    LOG_DEBUG << __FUNCTION__ << ": received number of remaining bursts is " << numRemainingBursts << std::endl;
    return OperationStatus(true);
#else
    (void)numRemainingBursts;
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}

OperationStatus SensingService::CloseSensing()
{
#ifdef _WIGIG_SLPI_SENSING_
    if (!IsRegistered())
    {   // nothing to do, return success
        LOG_DEBUG << "Sensing Close requested in un-registered state, do nothing" << std::endl;
        return OperationStatus(true);
    }

    const auto res = wigig_sensing_lib_unregister(m_sensingHandle);
    m_sensingHandle = nullptr; // always clear the handle not to block new registration
    if (res != WIGIG_SENSING_LIB_NO_ERR)
    {
        const char* errStr = wigig_sensing_lib_translate_error(res);
        LOG_ERROR << "Failed to close Sensing. UnRegister Error: " << errStr << " [" << res << "]" << std::endl;
        return OperationStatus(false, errStr);
    }

    return OperationStatus(true);
#else
    return OperationStatus(false, sensingNotSupportedStr);
#endif // _WIGIG_SLPI_SENSING_
}
