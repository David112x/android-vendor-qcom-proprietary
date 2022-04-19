#ifndef SYSTEM_COMMON_SERVICE_01_H
#define SYSTEM_COMMON_SERVICE_01_H
/**
  @file system_common_v01.h

  @brief This is the public header file which defines the system_common service Data structures.

  This header file defines the types and structures that were defined in
  system_common. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header: //commercial/MPSS.HE.1.0.c1/Main/modem_proc/qmimsgs/system_common/api/system_common_v01.h#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7 
   It was generated on: Mon May 14 2018 (Spin 0)
   From IDL File: system_common_v01.idl */

/** @defgroup system_common_qmi_consts Constant values defined in the IDL */
/** @defgroup system_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup system_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup system_common_qmi_messages Structures sent as QMI messages */
/** @defgroup system_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup system_common_qmi_accessor Accessor for QMI service object */
/** @defgroup system_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup system_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SYSTEM_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SYSTEM_COMMON_V01_IDL_MINOR_VERS 0x02
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SYSTEM_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */

/** @addtogroup system_common_qmi_enums
    @{
  */
typedef enum {
  SYSTEM_BAND_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  SYSTEM_BAND_BC0_V01 = 0, 
  SYSTEM_BAND_BC1_V01 = 1, 
  SYSTEM_BAND_BC3_V01 = 3, 
  SYSTEM_BAND_BC4_V01 = 4, 
  SYSTEM_BAND_BC5_V01 = 5, 
  SYSTEM_BAND_BC6_V01 = 6, 
  SYSTEM_BAND_BC7_V01 = 7, 
  SYSTEM_BAND_BC8_V01 = 8, 
  SYSTEM_BAND_BC9_V01 = 9, 
  SYSTEM_BAND_BC10_V01 = 10, 
  SYSTEM_BAND_BC11_V01 = 11, 
  SYSTEM_BAND_BC12_V01 = 12, 
  SYSTEM_BAND_BC13_V01 = 13, 
  SYSTEM_BAND_BC14_V01 = 14, 
  SYSTEM_BAND_BC15_V01 = 15, 
  SYSTEM_BAND_BC16_V01 = 16, 
  SYSTEM_BAND_BC17_V01 = 17, 
  SYSTEM_BAND_BC18_V01 = 18, 
  SYSTEM_BAND_BC19_V01 = 19, 
  SYSTEM_BAND_BC_MAX_V01 = 20, 
  SYSTEM_BAND_GSM_450_V01 = 40, 
  SYSTEM_BAND_GSM_480_V01 = 41, 
  SYSTEM_BAND_GSM_750_V01 = 42, 
  SYSTEM_BAND_GSM_850_V01 = 43, 
  SYSTEM_BAND_GSM_EGSM_900_V01 = 44, 
  SYSTEM_BAND_GSM_PGSM_900_V01 = 45, 
  SYSTEM_BAND_GSM_RGSM_900_V01 = 46, 
  SYSTEM_BAND_GSM_DCS_1800_V01 = 47, 
  SYSTEM_BAND_GSM_PCS_1900_V01 = 48, 
  SYSTEM_BAND_WCDMA_I_IMT_2000_V01 = 80, 
  SYSTEM_BAND_WCDMA_II_PCS_1900_V01 = 81, 
  SYSTEM_BAND_WCDMA_III_1700_V01 = 82, 
  SYSTEM_BAND_WCDMA_IV_1700_V01 = 83, 
  SYSTEM_BAND_WCDMA_V_850_V01 = 84, 
  SYSTEM_BAND_WCDMA_VI_800_V01 = 85, 
  SYSTEM_BAND_WCDMA_VII_2600_V01 = 86, 
  SYSTEM_BAND_WCDMA_VIII_900_V01 = 87, 
  SYSTEM_BAND_WCDMA_IX_1700_V01 = 88, 
  SYSTEM_BAND_WCDMA_XI_1500_V01 = 90, 
  SYSTEM_BAND_WCDMA_XIX_850_V01 = 91, 
  SYSTEM_BAND_WLAN_US_2400_V01 = 110, 
  SYSTEM_BAND_WLAN_JAPAN_2400_V01 = 111, 
  SYSTEM_BAND_WLAN_EUROPE_2400_V01 = 112, 
  SYSTEM_BAND_WLAN_FRANCE_2400_V01 = 113, 
  SYSTEM_BAND_WLAN_SPAIN_2400_V01 = 114, 
  SYSTEM_BAND_WLAN_US_5000_V01 = 115, 
  SYSTEM_BAND_WLAN_JAPAN_5000_V01 = 116, 
  SYSTEM_BAND_WLAN_EUROPE_5000_V01 = 117, 
  SYSTEM_BAND_WLAN_FRANCE_5000_V01 = 118, 
  SYSTEM_BAND_WLAN_SPAIN_5000_V01 = 119, 
  SYSTEM_BAND_LTE_EUTRAN_BAND1_V01 = 120, 
  SYSTEM_BAND_LTE_EUTRAN_BAND2_V01 = 121, 
  SYSTEM_BAND_LTE_EUTRAN_BAND3_V01 = 122, 
  SYSTEM_BAND_LTE_EUTRAN_BAND4_V01 = 123, 
  SYSTEM_BAND_LTE_EUTRAN_BAND5_V01 = 124, 
  SYSTEM_BAND_LTE_EUTRAN_BAND6_V01 = 125, 
  SYSTEM_BAND_LTE_EUTRAN_BAND7_V01 = 126, 
  SYSTEM_BAND_LTE_EUTRAN_BAND8_V01 = 127, 
  SYSTEM_BAND_LTE_EUTRAN_BAND9_V01 = 128, 
  SYSTEM_BAND_LTE_EUTRAN_BAND10_V01 = 129, 
  SYSTEM_BAND_LTE_EUTRAN_BAND11_V01 = 130, 
  SYSTEM_BAND_LTE_EUTRAN_BAND12_V01 = 131, 
  SYSTEM_BAND_LTE_EUTRAN_BAND13_V01 = 132, 
  SYSTEM_BAND_LTE_EUTRAN_BAND14_V01 = 133, 
  SYSTEM_BAND_LTE_EUTRAN_BAND17_V01 = 136, 
  SYSTEM_BAND_LTE_EUTRAN_BAND18_V01 = 137, 
  SYSTEM_BAND_LTE_EUTRAN_BAND19_V01 = 138, 
  SYSTEM_BAND_LTE_EUTRAN_BAND20_V01 = 139, 
  SYSTEM_BAND_LTE_EUTRAN_BAND21_V01 = 140, 
  SYSTEM_BAND_LTE_EUTRAN_BAND23_V01 = 142, 
  SYSTEM_BAND_LTE_EUTRAN_BAND24_V01 = 143, 
  SYSTEM_BAND_LTE_EUTRAN_BAND25_V01 = 144, 
  SYSTEM_BAND_LTE_EUTRAN_BAND26_V01 = 145, 
  SYSTEM_BAND_LTE_EUTRAN_BAND27_V01 = 146, 
  SYSTEM_BAND_LTE_EUTRAN_BAND28_V01 = 147, 
  SYSTEM_BAND_LTE_EUTRAN_BAND29_V01 = 148, 
  SYSTEM_BAND_LTE_EUTRAN_BAND30_V01 = 149, 
  SYSTEM_BAND_LTE_EUTRAN_BAND31_V01 = 150, 
  SYSTEM_BAND_LTE_EUTRAN_BAND32_V01 = 151, 
  SYSTEM_BAND_LTE_EUTRAN_BAND33_V01 = 152, 
  SYSTEM_BAND_LTE_EUTRAN_BAND34_V01 = 153, 
  SYSTEM_BAND_LTE_EUTRAN_BAND35_V01 = 154, 
  SYSTEM_BAND_LTE_EUTRAN_BAND36_V01 = 155, 
  SYSTEM_BAND_LTE_EUTRAN_BAND37_V01 = 156, 
  SYSTEM_BAND_LTE_EUTRAN_BAND38_V01 = 157, 
  SYSTEM_BAND_LTE_EUTRAN_BAND39_V01 = 158, 
  SYSTEM_BAND_LTE_EUTRAN_BAND40_V01 = 159, 
  SYSTEM_BAND_LTE_EUTRAN_BAND41_V01 = 160, 
  SYSTEM_BAND_LTE_EUTRAN_BAND42_V01 = 161, 
  SYSTEM_BAND_LTE_EUTRAN_BAND43_V01 = 162, 
  SYSTEM_BAND_LTE_EUTRAN_BAND125_V01 = 163, 
  SYSTEM_BAND_LTE_EUTRAN_BAND126_V01 = 164, 
  SYSTEM_BAND_LTE_EUTRAN_BAND127_V01 = 165, 
  SYSTEM_BAND_LTE_EUTRAN_BAND252_V01 = 166, 
  SYSTEM_BAND_LTE_EUTRAN_BAND255_V01 = 167, 
  SYSTEM_BAND_LTE_EUTRAN_BAND66_V01 = 168, 
  SYSTEM_BAND_LTE_EUTRAN_BAND250_V01 = 169, 
  SYSTEM_BAND_TDS_BANDA_V01 = 170, 
  SYSTEM_BAND_TDS_BANDB_V01 = 171, 
  SYSTEM_BAND_TDS_BANDC_V01 = 172, 
  SYSTEM_BAND_TDS_BANDD_V01 = 173, 
  SYSTEM_BAND_TDS_BANDE_V01 = 174, 
  SYSTEM_BAND_TDS_BANDF_V01 = 175, 
  SYSTEM_BAND_LTE_EUTRAN_BAND46_V01 = 176, 
  SYSTEM_BAND_LTE_EUTRAN_BAND47_V01 = 177, 
  SYSTEM_BAND_LTE_EUTRAN_BAND48_V01 = 178, 
  SYSTEM_BAND_LTE_EUTRAN_BAND71_V01 = 179, 
  SYSTEM_BAND_LTE_EUTRAN_BAND67_V01 = 180, 
  SYSTEM_BAND_LTE_EUTRAN_BAND68_V01 = 181, 
  SYSTEM_BAND_LTE_EUTRAN_BAND49_V01 = 182, 
  SYSTEM_BAND_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}system_band_enum_v01;
/**
    @}
  */

/* Conditional compilation tags for message removal */ 

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object system_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

