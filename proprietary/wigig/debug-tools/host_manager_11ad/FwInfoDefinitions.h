/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#pragma once

#include <cstdint>
#include <tuple>
#include <limits>
#include <ostream>

#include "Utils.h"

/*
 * Define FW info structure which would be sent as a Json encoded string
 * NOTE that the __exactly__ same struct is defined in the side that gets these structures
 */

struct FwVersion
{
    DWORD m_major;
    DWORD m_minor;
    DWORD m_subMinor;
    DWORD m_build;

    FwVersion() :
        m_major(0), m_minor(0), m_subMinor(0), m_build(0)
    {}

    bool IsWmiOnly() const
    {
        // test if the first bit is set, same as FieldDescription::MaskValue(m_subMinor, 0, 0) == 1
        return (m_subMinor & 0x1) == 1;
    }

    bool operator==(const FwVersion& rhs) const
    {
        return std::tie(m_major, m_minor, m_subMinor, m_build) == std::tie(rhs.m_major, rhs.m_minor, rhs.m_subMinor, rhs.m_build);
    }
};

inline std::ostream& operator<<(std::ostream& os, const FwVersion& fwVersion)
{
    return os << "[" << fwVersion.m_major << "." << fwVersion.m_minor << "." << fwVersion.m_subMinor << "." << fwVersion.m_build << "]";
}

struct FwTimestamp
{
    FwTimestamp(DWORD hour = 0, DWORD min = 0, DWORD sec = 0, DWORD day = 0, DWORD month = 0, DWORD year = 0) :
        m_hour(hour), m_min(min), m_sec(sec), m_day(day), m_month(month), m_year(year)
    {}

    static FwTimestamp CreateInvalidTimestamp()
    {
        return { sc_invalidValue, sc_invalidValue, sc_invalidValue, sc_invalidValue, sc_invalidValue, sc_invalidValue };
    }

    DWORD m_hour;
    DWORD m_min;
    DWORD m_sec;
    DWORD m_day;
    DWORD m_month;
    DWORD m_year;

    bool operator==(const FwTimestamp& rhs) const
    {
        return std::tie(m_hour, m_min, m_sec, m_day, m_month, m_year) == std::tie(rhs.m_hour, rhs.m_min, rhs.m_sec, rhs.m_day, rhs.m_month, rhs.m_year);
    }

    bool IsValid() const
    {
        return m_year != sc_invalidValue;
    }

private:
    static const auto sc_invalidValue = (std::numeric_limits<uint32_t>::max)();
};

inline std::ostream& operator<<(std::ostream& os, const FwTimestamp& timestamp)
{
    if (!timestamp.IsValid())
    {
        return os << "[NA]";
    }

    return os << "[20" << timestamp.m_year << "-" << timestamp.m_month << "-" << timestamp.m_day << " "
        << timestamp.m_hour << ":" << timestamp.m_min << ":" << timestamp.m_sec << "]";
}
