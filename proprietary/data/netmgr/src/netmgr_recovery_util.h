/******************************************************************************

                N E T M G R _ R E C O V E R Y _ U T I L . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_recovery_util.h
  @brief   Network Manager Recovery File Utility header file

  DESCRIPTION
  Header file containing definitions for using the netmgr recovery file.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifdef FEATURE_RECOVERY_ENABLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "netmgr.h"
#include "netmgr_defs.h"

#define RECOVERY_FILE  "/data/vendor/netmgr/recovery/recovery_info"
#define NETMGR_RECOVERY_META_INFO_INDEX (-1)
#define NETMGR_RECOVERY_MAX_ENTRY_INDEX (64) /* Same as size of bitmask */

extern boolean recovery_debug_flag;

#define RECOVERY_DEBUG(fmt, ...)  \
  do { if (recovery_debug_flag) netmgr_log_low(fmt, __VA_ARGS__); } while (0)

typedef enum
{
  TYPE_META_INFO,
  TYPE_LINK_LOCAL_NW_SETTINGS,
  TYPE_IWLAN_NW_SETTINGS,
  TYPE_LINK_LOCAL_LINK_INFO,
  TYPE_IWLAN_LINK_INFO,
  TYPE_BEARER_INFO
} recovery_data_type_t;

typedef struct
{
  uint32_t total_entries;
  uint64_t entry_bitmask;
} meta_info_t;

typedef struct
{
  uint64_t       network_hndl;
  uint32_t       packet_mark;
} network_settings_t;

typedef enum
{
  TECH_TYPE_REGULAR,
  TECH_TYPE_LINK_LOCAL,
  TECH_TYPE_LBO,
  TECH_TYPE_EPDG
} tech_type_t;

typedef enum
{
  LINK_STATE_DOWN,
  LINK_STATE_UP
} link_state_t;

/* Bitmask for SA information */
#define SA_XFRM_CLEAN       0x0
#define SA_XFRM_STATE_OUT   0x1
#define SA_XFRM_STATE_IN    0x2
#define SA_XFRM_POLICY_OUT  0x4
#define SA_XFRM_POLICY_IN   0x8
#define SA_XFRM_POLICY_FWD  0x10

typedef struct
{
  int          ip_family;
  int          link;
  char         link_name[NETMGR_IF_NAME_MAX_LEN];
  link_state_t link_state;
  tech_type_t  tech_type;
  char         link_addr[NETMGR_MAX_STR_LENGTH];
  /* Since SA is shared for now, we can have one tunnel info struct per link
     If new feature support is added where we may have distinct IP based tunnels per
     link we need to extend the recovery info struct to have distinct tunnel info
     structs for IPv4 and IPv6 */
  struct
  {
    int          sa_state;                      /* 0x0  -  No SAs are installed
                                                   0x1  -  XFRM state in 'out' dir is set
                                                   0x3  -  XFRM state in 'out' and 'in' is set
                                                   0x7  -  XFRM state in both directions
                                                           and XFRM policy in 'out' direction
                                                           is set
                                                   0xF  -  XFRM state in both directions and
                                                           XFRM policy in 'out' and 'in' is set
                                                   0x1F -  XFRM state in 'in' and 'out'
                                                           XFRM policy in 'in', 'fwd' and 'out'
                                                           is set */
    int          ike_port;
    char         local_addr[NETMGR_MAX_STR_LENGTH];
    char         dest_addr[NETMGR_MAX_STR_LENGTH];
    char         proto[NETMGR_MAX_PROTO_STR_LENGTH];
    char         mode[NETMGR_MAX_MODE_LENGTH];
    uint32_t     spi_tx;
    uint32_t     spi_rx;
  } tunnel_info;
} recovery_info_t;

typedef struct
{
  int          ip_family;
  int          link;
  uint32_t     flow_id;
  uint8_t      filter_mask;
} qos_bearer_info_t;

/* Each entry in the binary file is of this type */
typedef struct file_info_s
{
  int                  is_valid;
  recovery_data_type_t info_type;
  union
  {
    meta_info_t        meta_info;
    network_settings_t nw_settings;
    recovery_info_t    link_info;
    qos_bearer_info_t  bearer_info;
  } recovery_info;
} file_info_t;

/*===========================================================================
                      GLOBAL FUNCTION DECLARATIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  netmgr_recovery_print_bitmask
===========================================================================*/
/*!
@brief
  Print the bitmask in binary so we can easily tell which entries are valid.
*/
/*=========================================================================*/
void netmgr_recovery_print_bitmask(uint64_t bitmask);


/*===========================================================================
  FUNCTION  netmgr_recovery_file_print_contents
===========================================================================*/
/*!
@brief
  Print all the contents of the recovery file
*/
/*=========================================================================*/
void netmgr_recovery_file_print_contents(void);


/*===========================================================================
  FUNCTION  netmgr_recovery_file_init
===========================================================================*/
/*!
@brief
  Initalize the recovery file. If the file does not exist or is empty, then
  initialize the file so that it has the meta info as the first entry. If the
  file exists then we recover the meta info (num entries, bitmask of which
  are valid). Meta info is read by calling netmgr_recovery_file_read_entry
  with index -1 (NETMGR_RECOVERY_FILE_META_INFO_INDEX).

@return
  int - NETMGR_SUCCESS if file operations and meta info is inited/recovered
  succesfully, NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_recovery_file_init(void);


/*===========================================================================
  FUNCTION  netmgr_recovery_file_close
===========================================================================*/
/*!
@brief
  Close the recovery file

@return
  void
*/
/*=========================================================================*/
void netmgr_recovery_file_close(void);


/*===========================================================================
  FUNCTION  netmgr_recovery_file_find_entry
===========================================================================*/
/*!
@brief
  Find entry in recovery file

@return
  int - NETMGR_SUCCESS if matching entry is found, NETMGR_FAILURE otherwise

@param
  file_info_t *file_info_ptr   - input/output param with file info to
                                 query as input, found entry as output param
  int         *found_index_ptr - output param with index of the found entry
*/
/*=========================================================================*/
int netmgr_recovery_file_find_entry(file_info_t *file_info_ptr,
                                    int         *found_index_ptr);

/*===========================================================================
  FUNCTION  netmgr_recovery_file_read_entry
===========================================================================*/
/*!
@brief
  Read entry in recovery file.

@return
  int - NETMGR_SUCCESS if read at index succeeds, NETMGR_FAILURE otherwise

@param
  int         index          - input param of index to read in file
  file_info_t *file_info_ptr - output param with file info at index

@note
  Using index -1 (NETMGR_RECOVERY_FILE_META_INFO_INDEX) will read meta info
*/
/*=========================================================================*/
int netmgr_recovery_file_read_entry(int index, file_info_t *file_info_ptr);


/*===========================================================================
  FUNCTION  netmgr_recovery_file_write_entry
===========================================================================*/
/*!
@brief
  Write entry in recovery file. If a matching entry is in the recovery file,
  then it is updated, otherwise, create a new entry at the first unused
  location in the file.

@return
  int - NETMGR_SUCCESS if write succeeds, NETMGR_FAILURE otherwise

@param
  file_info_t *file_info_ptr - input param with file info to write

@note
  Can't write meta_info, entry count/bitmask is maintained automatically.
*/
/*=========================================================================*/
int netmgr_recovery_file_write_entry(file_info_t *file_info_ptr);

/*===========================================================================
  FUNCTION  netmgr_recovery_file_find_and_invalidate_entry
===========================================================================*/
/*!
@brief
  Find entry in recovery file and invalidate

@return
  int - NETMGR_SUCCESS if matching entry is found and invalidated,
        NETMGR_FAILURE otherwise

@param
  file_info_t *file_info_ptr - input param with file info to invalidate
*/
/*=========================================================================*/
int netmgr_recovery_file_find_and_invalidate_entry(file_info_t *file_info_ptr);

#endif /* FEATURE_RECOVERY_ENABLED */
