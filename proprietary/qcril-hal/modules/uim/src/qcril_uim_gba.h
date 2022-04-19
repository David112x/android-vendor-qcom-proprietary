#ifndef QCRIL_UIM_GBA_H
#define QCRIL_UIM_GBA_H
/*===========================================================================

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include "modules/uim/UimGBAGetIMPIRequestMsg.h"
#include "modules/uim/UimGBAInitRequestMsg.h"
#include "qcril_uim.h"

/*===========================================================================

                           TYPES

===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_uim_gba_get_impi_resp

===========================================================================*/
/*!
    @brief
    Handles the response of OEM get IMPI request.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_gba_get_impi_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_gba_resp

===========================================================================*/
/*!
    @brief
    Handles the response of OEM GBA initialization request

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_gba_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGBAInitRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_request_gba_init

===========================================================================*/
void qcril_uim_request_gba_init
(
  const std::shared_ptr<UimGBAInitRequestMsg> req_ptr
);

/*===========================================================================

  FUNCTION:  qcril_uim_oem_request_get_impi

===========================================================================*/
void qcril_uim_gba_request_get_impi
(
  const std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr
);

#endif /* QCRIL_UIM_GBA_H */

