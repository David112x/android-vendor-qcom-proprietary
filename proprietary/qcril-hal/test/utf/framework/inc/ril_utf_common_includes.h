/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_common_includes.h
  @brief   common include headers used in RIL UTF

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_COMMON_INC_H
#define _RIL_UTF_COMMON_INC_H

#define __STDC_LIMIT_MACROS

#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>

#ifndef RIL_UTF_IOE
#include "cutils/properties.h"
#include "ril.h"
#include "qcril_log.h"
#include "qcrili.h"
#ifdef  __cplusplus
#include "modules/voice/qcril_qmi_voice.h"
#endif
#include "qcril_qmi_ims.h"
#include "qcril_qmi_imss.h"
#include "qcril_qmi_nas.h"
#include "qcril_db.h"
#endif

#endif //_RIL_UTF_COMMON_INC_H
