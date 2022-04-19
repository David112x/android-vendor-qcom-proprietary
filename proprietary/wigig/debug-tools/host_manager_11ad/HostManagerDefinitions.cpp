/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "HostManagerDefinitions.h"
#include <ostream>

std::ostream& operator<<(std::ostream& os, const BasebandType& basebandType)
{
    switch(basebandType)
    {
    case BASEBAND_TYPE_NONE:    return os << "BASEBAND_TYPE_NONE";
    case BASEBAND_TYPE_SPARROW: return os << "BASEBAND_TYPE_SPARROW";
    case BASEBAND_TYPE_MARLON:  return os << "BASEBAND_TYPE_MARLON";
    case BASEBAND_TYPE_TALYN:   return os << "BASEBAND_TYPE_TALYN";
    default:                    return os << "<unknown>";
    }
}

std::ostream& operator<<(std::ostream& os, const BasebandRevision& rev)
{
    switch (rev)
    {
    case BB_REV_UNKNOWN:return os << "UNKNOWN";
    case MAR_DB1:       return os << "MAR_DB1";
    case MAR_DB2:       return os << "MAR_DB2";
    case SPR_A0:        return os << "SPR_A0";
    case SPR_A1:        return os << "SPR_A1";
    case SPR_B0:        return os << "SPR_B0";
    case SPR_C0:        return os << "SPR_C0";
    case SPR_D0:        return os << "SPR_D0";
    case TLN_M_A0:      return os << "TLN_M_A0";
    case TLN_M_B0:      return os << "TLN_M_B0";
    case TLN_M_C0:      return os << "TLN_M_C0";
    default:            return os << "UNKNOWN";
    }
}
