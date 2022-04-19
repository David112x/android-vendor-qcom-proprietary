/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QCRIL_GSTK_TYPES_H
#define QCRIL_GSTK_TYPES_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdint.h>
#include <string>
#include <vector>


/*===========================================================================

                           DEFINES

===========================================================================*/


/*===========================================================================

                           ENUMS

===========================================================================*/
/* -----------------------------------------------------------------------------
   ENUM:      RIL_GSTK_Errno

   DESCRIPTION:
     RIL Errors applicable to GSTK and values same as RIL_Errno
-------------------------------------------------------------------------------*/
typedef enum {
    RIL_GSTK_E_SUCCESS = 0,
    RIL_GSTK_E_RADIO_NOT_AVAILABLE = 1,              /* If radio did not start or is resetting */
    RIL_GSTK_E_GENERIC_FAILURE = 2,
    RIL_GSTK_E_PASSWORD_INCORRECT = 3,               /* for PIN/PIN2 methods only! */
    RIL_GSTK_E_SIM_PIN2 = 4,                         /* Operation requires SIM PIN2 to be entered */
    RIL_GSTK_E_SIM_PUK2 = 5,                         /* Operation requires SIM PIN2 to be entered */
    RIL_GSTK_E_REQUEST_NOT_SUPPORTED = 6,
    RIL_GSTK_E_CANCELLED = 7,
    RIL_GSTK_E_SUBSCRIPTION_NOT_AVAILABLE = 12,      /* fail to find CDMA subscription from specified
                                                       location */
    RIL_GSTK_E_ILLEGAL_SIM_OR_ME = 15,               /* network selection failed due to
                                                       illegal SIM or ME */
    RIL_GSTK_E_MISSING_RESOURCE = 16,                /* no logical channel available */
    RIL_GSTK_E_NO_SUCH_ELEMENT = 17,                 /* application not found on SIM */
    RIL_GSTK_E_SUBSCRIPTION_NOT_SUPPORTED = 26,      /* Subscription not supported by RIL */
    RIL_GSTK_E_NO_MEMORY = 37,                       /* Not sufficient memory to process the request */
    RIL_GSTK_E_INTERNAL_ERR = 38,                    /* Modem hit unexpected error scenario while handling
                                                       this request */
    RIL_GSTK_E_SYSTEM_ERR = 39,                      /* Hit platform or system error */
    RIL_GSTK_E_MODEM_ERR = 40,                       /* Vendor RIL got unexpected or incorrect response
                                                       from modem for this request */
    RIL_GSTK_E_INVALID_STATE = 41,                   /* Unexpected request for the current state */
    RIL_GSTK_E_NO_RESOURCES = 42,                    /* Not sufficient resource to process the request */
    RIL_GSTK_E_SIM_ERR = 43,                         /* Received error from SIM card */
    RIL_GSTK_E_INVALID_ARGUMENTS = 44,               /* Received invalid arguments in request */
    RIL_GSTK_E_INVALID_SIM_STATE = 45,               /* Can not process the request in current SIM state */
    RIL_GSTK_E_INVALID_MODEM_STATE = 46,             /* Can not process the request in current Modem state */
    RIL_GSTK_E_OPERATION_NOT_ALLOWED = 54,           /* Not allowed the request now */
} RIL_GSTK_Errno;

/*===========================================================================

                           STRUCTURES

===========================================================================*/
typedef struct
{
  RIL_GSTK_Errno  err;
  string          rsp;
  int32_t         sw1;
  int32_t         sw2;
} RIL_GSTK_EnvelopeResponse;
#endif /* QCRIL_GSTK_TYPES_H */
