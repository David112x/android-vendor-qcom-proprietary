/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "BaseProfile.h"

#define QDP_RADIOTECH_3GPP2 0
#define QDP_RADIOTECH_3GPP 1
#define QDP_RADIOTECH_COMMON 2

void dataprofile_get_legacy_params(const BaseProfile::params_t in_params,
   char ** out_params);

void dataprofile_release_legacy_params(char ** in_params);

//TODO: ENABLE THIS. CURRENTLY NOT UNDER USE
//bool dataprofile_is_attach_required(const uint16_t profile_id);
