/******************************************************************************

                          N E T M G R _ R M N E T . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_rmnet.h
  @brief   Network Manager RmNet Data configuration internal header file

  DESCRIPTION
  Network Manager RmNet Data configuration internal header file

******************************************************************************/
/*===========================================================================

  Copyright (c) 2013-2015, 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_RMNET_H__
#define __NETMGR_RMNET_H__

#define NETMGR_RMNET_SUCCESS          0
#define NETMGR_RMNET_BAD_ARGS         1
#define NETMGR_RMNET_LIB_FAILURE      2

#define NETMGR_RMNET_NO_FEATURE_FLAGS   -1

#define NETMGR_RMNET_ADD_FLOW         1
#define NETMGR_RMNET_DEL_FLOW         0

#define NETMGR_RMNET_DL_MARKER_V1_ON         1
#define NETMGR_RMNET_DL_MARKER_V2_ON         2

#define NETMGR_RMNET_INGRESS_DEAGGREGATION_FLAG         (1U << 0)
#define NETMGR_RMNET_INGRESS_MAP_COMMANDS_FLAG          (1U << 1)
#define NETMGR_RMNET_INGRESS_MAP_CKSUMV4_FLAG           (1U << 2)
#define NETMGR_RMNET_EGRESS_MAP_CKSUMV4_FLAG            (1U << 3)
#define NETMGR_RMNET_INGRESS_COALESCE_FLAG              (1U << 4)
#define NETMGR_RMNET_INGRESS_MAP_CKSUMV5_FLAG           (1U << 5)
#define NETMGR_RMNET_EGRESS_MAP_CKSUMV5_FLAG            (1U << 6)
#define NETMGR_RMNET_EGRESS_AGGREGATION_FLAG            (1U << 31)
#define NETMGR_RMNET_INGRESS_FORMAT_DL_MARKER_FLAG      (1U << 30)
#define NETMGR_RMNET_INGRESS_FORMAT_DL_MARKER_FLAG_V2   (1U << 29)
#define NETMGR_RMNET_EGRESS_FORMAT_PACKET_PRIORITY      (1U << 28)

#define NETMGR_RMNET_INGRESS_FORMAT_PS_NOTIF_FLAG       (1U << 26)

/*===========================================================================
  FUNCTION  netmgr_rmnet_create_muxed_channels
===========================================================================*/
/*!
@brief
  Create RmNet Data logical channels

@arg *phys_dev physical device corresponding to a transport to modem
@arg base_rmnet initial virtual RmNet device number
@arg base_mux_id initial mux id to use when tying virtual devices to physical
@arg num_virt_dev Number of virtual devices to create. These typicall correspond
     to logical Mux channels
@arg dev_prefix virtual network device name prefix

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_create_muxed_channels
(
  const char        *phys_dev,
  unsigned int      base_rmnet,
  unsigned int      base_mux_id,
  unsigned int      num_virt_dev,
  const char        *dev_prefix
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_feature_flags
===========================================================================*/
/*!
@brief
  Set specific feature flags for ingress and egress

@arg *phys_dev physical device corresponding to a transport to modem
@arg ingress_flags ingress data format bitmask for physical device
@arg egress_flags  egress data format bitmask for physical device
@arg tail_spacing  tail spacing for the packet on the physical device

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_set_feature_flags
(
  const char        *phys_dev,
  unsigned int      ingress_flags,
  unsigned int      egress_flags,
  uint32_t          tail_spacing
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_query_vnd_name
===========================================================================*/
/*!
@brief
  Query RmNet Data kernel module for the name of a VND

@arg dev_id numerical ID for VND
@arg name buffer to store VND name
@arg name_len length of name buffer in bytes

@return
  int - NETMGR_RMNET_SUCCESS if successful
        NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
        NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_query_vnd_name
(
  uint32_t  dev_id,
  char     *name,
  uint32_t  name_len
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (egress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_edf_to_ioctl_edf
(
  uint32_t flags
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
===========================================================================*/
/*!
@brief
  Convert RmNet Data format flags to IOCTL format flags (ingress)
*/
/*=========================================================================*/
uint32_t
netmgr_rmnet_convert_rmnet_idf_to_ioctl_idf
(
  uint32_t flags
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_add_del_flow
===========================================================================*/
/*!
@brief
  Add / Delete a modem flow handle - tc flow handle mapping in the kernel.

@arg link id of the virtual network device to create the link
@arg mdm_flow_hndl modem flow id
@arg tc_flow_hndl tc flow id
@arg set_flow option to add or delete a flow pair mapping

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_add_del_flow
(
  int       link,
  uint32_t  mdm_flow_hndl,
  uint32_t  tc_flow_hndl,
  uint8_t   set_flow
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_get_ingress_data_format
===========================================================================*/
/*!
@brief
  Gets the ingress data format for a particular device.

@arg link id of the virtual network device to create the link
@arg flow_id flow id of the modem

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_get_ingress_data_format
(
  const char        *phys_dev,
  uint32_t          *ingress_flags
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_get_device_qmi_offset
===========================================================================*/
/*!
@brief
  Gets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset of the rmnet data device

@return
  int - NETMGR_SUCCESS if successful
        NETMGR_FAILURE if the operation fails
*/
/*=========================================================================*/
int
netmgr_rmnet_get_device_qmi_offset
(
  uint32_t          *qmi_offset
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_device_qmi_offset
===========================================================================*/
/*!
@brief
  Sets the netmgr_rmnet_data_device_qmi_offset.

@arg qmi_offset qmi offset to be set for the rmnet data device

@return
  void
*/
/*=========================================================================*/
void
netmgr_rmnet_set_device_qmi_offset
(
  uint32_t          qmi_offset
);


/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_ep_params
===========================================================================*/
/*!
@brief
  Configure ep parameters for IPA based targets

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

int
netmgr_rmnet_configure_ep_params
(
  void
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_data
===========================================================================*/
/*!
@brief


@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_data
(
  void
);


/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_all_configuration
===========================================================================*/
/*!
@brief


@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_remove_all_configuration
(
  void
);

/*===========================================================================
  FUNCTION netmgr_rmnet_update_ul_aggregation
===========================================================================*/
/*!
@brief
  Updates the aggregation parameters on an RmNet Data egress device
  First, egress flags are retrieved. If ul_agg_count is less than or equal to 1,
  then the aggregation flag is removed. Otherwise, the aggregation flag is ORed.
  The new flags and aggregation sizes are then passed up to RmNet Data

@arg *phys_dev physical device corresponding to a transport to modem
@arg ul_agg_count New aggregation packet count
@arg ul_agg_size New aggregation buffer size count

@return
  int - NETMGR_RMNET_SUCCESS if successful
  NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
  NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_update_ul_aggregation
(
  const char *phys_dev,
  unsigned int ul_agg_count,
  unsigned int ul_agg_size
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_link
===========================================================================*/
/*!
@brief Creates a single rmnet embedded link with RTM_NETLINK messages to
       underlying rmnet kernel driver using librmnetctl library

@param
  int - link_num - number of the link (ex: rmnet_dataX)

@return
  int - NETMGR_SUCCESS if link was created, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_link
(
  int link_num
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_configure_embedded_links
===========================================================================*/
/*!
@brief Creates rmnet embedded links with RTM_NETLINK messages to underlying
       rmnet kernel driver using librmnetctl library


@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_configure_embedded_links
(
  void
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_change_embedded_link
===========================================================================*/
/*!
@brief Changes flag config of an existing link with RTM_NETLINK messages to
       underlying rmnet kernel driver with use of librmnetctl library

@param
  int - link number

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_change_embedded_link
(
  int link_num
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_change_embedded_links
===========================================================================*/
/*!
@brief Changes flag config of an existing link with RTM_NETLINK messages to
       underlying rmnet kernel driver with use of librmnetctl library

@param
  int - flag configuration for links

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_change_embedded_links
(
  int flag
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_embedded_link
===========================================================================*/
/*!
@brief Removes rmnet embedded link with RTM_NETLINK messages from underlying
       rmnet kernel driver with the help of librmnetctl library

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_remove_embedded_link
(
  int link_num
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_remove_embedded_links
===========================================================================*/
/*!
@brief Removes rmnet embedded links with RTM_NETLINK messages from underlying
       rmnet kernel driver with the help of librmnetctl library

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_rmnet_remove_embedded_links
(
  void
);

/*===========================================================================
  FUNCTION  netmgr_rmnet_ext_init
===========================================================================*/
/*!
@brief Init Netmgr rmnet extention

@return
  int - NETMGR_SUCCESS/NETMGR_FAILURE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_rmnet_ext_init ();

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_ko_loading_property
===========================================================================*/
/*!
@brief
  Sets android properties to trigger loading of netmgr kernel modules

@param
  prop_name - Name of the property

@param
  prop_value - value to be set in the property
               0 - for disabling
               1 - for enabling

@return
  int - NETMGR_SUCCESS if properties were set correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_rmnet_set_ko_loading_property(const char *prop_name, const char *prop_value);

/*===========================================================================
  FUNCTION  netmgr_rmnet_set_shsusr_loading_property
===========================================================================*/
/*!
@brief
  Sets android properties to trigger loading of shsusrd module

@param
  prop_name - Name of the property

@param
  prop_value - value to be set in the property
               0 - for disabling
               1 - for enabling

@return
  int - NETMGR_SUCCESS if properties were set correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_rmnet_set_shsusr_loading_property(const char *prop_name,
                                             const char *prop_value);

/*===========================================================================
  FUNCTION netmgr_rmnet_update_embedded_ul_aggregation
===========================================================================*/
/*!
@brief
  Updates the aggregation parameters on an RmNet Data egress device

@arg *phys_dev physical device corresponding to a transport to modem
@arg ul_agg_count New aggregation packet count
@arg ul_agg_size New aggregation buffer size count
@arg ul_agg_time New aggregation time limit in nanoseconds

@return
  int - NETMGR_RMNET_SUCCESS if successful
  NETMGR_RMNET_BAD_ARGS if null/invalid arguments are passed
  NETMGR_RMNET_LIB_FAILURE if failed to configure kernel
*/
/*=========================================================================*/
int
netmgr_rmnet_update_embedded_ul_aggregation
(
  const char *phys_dev,
  unsigned int ul_agg_count,
  unsigned int ul_agg_size,
  unsigned int ul_agg_time,
  unsigned int ul_agg_features
);

#endif  /* __NETMGR_RMNET_H__ */

