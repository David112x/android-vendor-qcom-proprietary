/******************************************************************************

                        N E T M G R _ M A I N . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_main.c
  @brief   Network Manager main function implementation

  DESCRIPTION
  Implementation of NetMgr's main function.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/14/11   jas        change netmgrd uid to radio at power up
02/08/10   ar         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h> /* open, read */
#include <signal.h>
#include <linux/capability.h>
#include <strings.h>
#include <stdint.h>

#ifndef NETMGR_OFFTARGET
#include <sys/capability.h>
#endif

#include <linux/msm_rmnet.h>        /* RmNet Kernel Constants */
#include <linux/rmnet_data.h>       /* RmNet Kernel Constants */
#include "netmgr_qmi_wda.h"
#include "netmgr_rmnet.h"

#ifdef FEATURE_DS_LINUX_ANDROID
#include <cutils/properties.h>
  #ifndef NETMGR_OFFTARGET
    #include <cutils/android_filesystem_config.h>
  #endif
#include "vndfwk-detect.h"
#endif

#ifndef PROPERTY_VALUE_MAX
  #define PROPERTY_VALUE_MAX 256
#endif

#include "ds_util.h"
#include "ds_string.h"
#include "ds_trace.h"
#include "netmgr_defs.h"
#include "netmgr_exec.h"
#include "netmgr_kif.h"
#include "netmgr_qmi.h"
#include "netmgr_tc.h"
#include "netmgr_platform.h"
#include "netmgr_main.h"
#include "netmgr_qmi_dpm.h"
#include "netmgr_qmi_dfs.h"
#include "netmgr_qmi_dms.h"
#include "netmgr_cmdq.h"

#include "netmgr_cb.h"
#include "netmgr_main_cb.h"
#include "netmgr_netd_client.h"
#include "netmgr_target.h"

#ifdef NETMGR_TEST
#include "netmgr_test.h"
#endif

/*===========================================================================
                     FORWARD DECLARATIONS
===========================================================================*/
char l2s_proc_name[netmgr_l2s_pname_size];
int  l2s_pid;

extern void nm_link_local_routing_main_register_plugin(void);
extern void nm_link_local_routing_main_init(int restart);

extern void nm_mdmprxy_register_plugin(void);
extern void nm_mdmprxy_init(int restart);

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_MAIN_DEFAULT_NINT         NETMGR_MAX_LINK

#ifdef FEATURE_DS_LINUX_ANDROID
/* Macros from Android property database */
#ifdef FEATURE_DATA_IWLAN
  #define NETMGR_MAIN_PROPERTY_IWLAN          "persist.vendor.data.iwlan.enable"
  #define NETMGR_MAIN_PROPERTY_IWLAN_SIZE     (5)
  #define NETMGR_MAIN_PROPERTY_IWLAN_DEFAULT  NETMGR_TRUE /* true or false */
#endif /* FEATURE_DATA_IWLAN */
#endif /* FEATURE_DS_LINUX_ANDROID */

#define NETMGR_MAIN_SHS_KO_PROPERTY   "persist.vendor.data.shs_ko_load"
#define NETMGR_MAIN_PERF_KO_PROPERTY  "persist.vendor.data.perf_ko_load"
#define NETMGR_MAIN_PERF_KO_PROPERTY_MAX 3
#define NETMGR_MAIN_KO_PROPERTY_SET_ONE "1"
#define NETMGR_MAIN_KO_PROPERTY_UNSET "0"
#define NETMGR_MAIN_SHSUSR_LOAD_PROPERTY   "persist.vendor.data.shsusr_load"
#define NETMGR_MAIN_SHSUSR_PROPERTY_ENABLE  "1"
#define NETMGR_MAIN_SHSUSR_PROPERTY_DISABLE "0"

/* Default network interface name prefix; may be overridden */
LOCAL char netmgr_iname_default[] = "rmnet";

#define NETMGR_BASE_TEN 10
#define NETMGR_DATA_FMT_DFLT_LLP 2

#ifdef FEATURE_DS_LINUX_ANDROID
  #define NETMGR_LOAD_PROP(X, Y, Z) \
    do { \
      memset(Y, 0, sizeof(Y)); \
      property_get(X, Y, Z); \
      netmgr_log_med("Read property %s; Got \"%s\"; (Default: \"%s\")\n", X, Y, Z);\
    } while(0);
#else
  #define NETMGR_LOAD_PROP(X, Y, Z) \
    do { \
      (void)strlcpy(Y, Z, sizeof(Y)); \
      netmgr_log_med("Read property %s; Using default \"%s\")\n", X, Y);\
    } while(0);
#endif

#define NETMGR_SYSCMD_SIZ  (256)

/*---------------------------------------------------------------------------
   Program configuration info
---------------------------------------------------------------------------*/

struct netmgr_main_cfg_s netmgr_main_cfg = {
  NETMGR_MAIN_RUNMODE_DEFAULT,               /* runmode */
  DS_LOG_MODE_DFLT,                          /* logmode */
  -1,                                        /* logthreshold */
  -1,                                        /* nint */
  netmgr_iname_default,                      /* iname */
  NETMGR_KIF_SKIP,                           /* skip */
  NULL,                                      /* dirpath */
  NULL,                                      /* modscr */
  FALSE,                                     /* debug */
  FALSE,                                     /* runtests */
  FALSE,                                     /* initialized */
  NETMGR_LINK_RMNET_0,                       /* def_link */
  IWLAN_DISABLE,                             /* iwlan_enabled */
  FALSE,                                     /* iwlan_ims_enabled */
  FALSE,                                     /* iwlan_traffic_sel_enabled */
  FALSE,                                     /* iwlan_client_enable */
  FALSE,                                     /* rmnet_data_enabled */
  {0,},                                      /* phys_net_dev */
  {0,},                                      /* modems_enabled */
  FALSE,                                     /* single_qmux_ch_enabled */
  {0,},                                      /* qmux_ch_name */
  {0,},                                      /* smd_ch_name */
  0,                                         /* epid */
  DS_EP_TYPE_EMBEDDED,                       /* ep_type Default DS_EP_TYPE_EMBEDDED */
  0,                                         /* ipa_cons_ep_num */
  0,                                         /* ipa_prod_ep_num */
  FALSE,                                     /* dpm_enabled */
  TRUE,                                      /* use_qmuxd */
  10000,                                     /* dpm_retry_timeout */
  FALSE,                                     /* wda_data_format */
  FALSE,                                     /* tcp_ack_prio */
  FALSE,                                     /* in band flow control */
  FALSE,                                     /* QOS is disabled*/
  0,                                         /* kernel flow control */
  0,                                         /* Kernel QMAP flow control */
  0,                                         /* QMI Power collapse */
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,3000000,0,0}, /* data format netmgr_data_format_s*/
  NETMGR_MODEM_NORMAL_STATE,                 /* modem ssr state */
  0,                                         /* pnd rps mask */
  0,                                         /* vnd rps mask */
  0,                                         /* clat rps mask*/
  0,                                         /* netdev budget */
  FALSE,                                     /* low latency filters */
  TRUE,                                      /* Drop SSDP */
  0,                                         /* Process ready requests */
  FALSE,                                     /* hybrid qos */
  FALSE,                                     /* data format configured */
  0,                                         /* netdev max backlog */
  0,                                         /* disable tcp hystart detect */
  0,                                         /* disable hystart */
  0,                                         /* intitial ssthresh */
  0,                                         /* ipv4 frag low threshold */
  0,                                         /* ipv4 frag high threshold */
  FALSE,                                     /* qos_via_idl */
  0,                                         /* tc_ul_baserate */
  NETMGR_TC_CEILING_MBPS,                    /* tc_ul_ceiling_mbps */
  FALSE,                                     /* dynamic_tc_ul_baserate */
  0,                                         /* tc_ul_burst */
  0,                                         /* num_data_ports */
  0,                                         /* num_rev_data_ports */
  0,                                         /* wakelock_timeout*/
  0,                                         /* mtu */
  0,                                         /* iwlan_rekey */
  { {NETMGR_MAIN_IKE_PORT_INVALID, FALSE,{0},{0}},  /* IKE ports */
    {NETMGR_MAIN_IKE_PORT_INVALID, FALSE,{0},{0}},
    {NETMGR_MAIN_IKE_PORT_INVALID, FALSE,{0},{0}},
    {NETMGR_MAIN_IKE_PORT_INVALID, FALSE,{0},{0}},
    {NETMGR_MAIN_IKE_PORT_INVALID, FALSE,{0},{0}} },
  {0,0,{0}},                                  /* cmd_exec options */
  0,                                          /* new rmnet driver */
  0,                                          /* rtnl based tc */
  0,                                          /* tcp_mtu_probing*/
  0,                                          /* Enable dl_marker parsing */
  0,                                          /* Max configurable MTU */
  0,                                          /* rmnet perf*/
  0,                                          /* rmnet_ shs*/
  0,                                          /* disable txcsum for xlat */
  0,                                          /* rsc */
  0,                                          /* rsb */
  0,                                          /* MHI' */
  0,                                          /* skip_buffered_qos_modify */
  0,                                          /* num of static forward links */
  0,                                          /* num of static reverse links */
  0,                                          /* dynamic link creation */
  0,                                          /* rev_mux_offset */
  0,                                          /* fwd_link_start */
  0,                                          /* rev_link_start */
  0,                                          /* max_links_per_modem */
  0,                                          /* max_links_in_total */
  0,                                          /* output_mark_enable */
  0,                                          /* uplink priority port */
};  /* Initialize everything to invalid values */

pthread_mutex_t netmgr_main_cfg_lock = PTHREAD_MUTEX_INITIALIZER;

/* Array of strings used to identify which port is used by RmNET.
 * Order of strings is important as we use index of the string element
 * internally. First field is for USB driver port match, which may be
 * empty string if unused
 * Structure: {data_ctl_port, qmi_conn_id, link_id, modem_wait, enabled, on_demand} */
netmgr_ctl_port_config_type
netmgr_ctl_port_array[NETMGR_MAX_LINK+1] =
{
  /* SMD/BAM transport */
  {"DATA5_CNTL",  QMI_PORT_RMNET_0,          NETMGR_LINK_RMNET_0,      TRUE,  TRUE, FALSE},
  {"DATA6_CNTL",  QMI_PORT_RMNET_1,          NETMGR_LINK_RMNET_1,      FALSE, TRUE, FALSE},
  {"DATA7_CNTL",  QMI_PORT_RMNET_2,          NETMGR_LINK_RMNET_2,      FALSE, TRUE, FALSE},
  {"DATA8_CNTL",  QMI_PORT_RMNET_3,          NETMGR_LINK_RMNET_3,      FALSE, TRUE, FALSE},
  {"DATA9_CNTL",  QMI_PORT_RMNET_4,          NETMGR_LINK_RMNET_4,      FALSE, TRUE, FALSE},
  {"DATA12_CNTL", QMI_PORT_RMNET_5,          NETMGR_LINK_RMNET_5,      FALSE, TRUE, FALSE},
  {"DATA13_CNTL", QMI_PORT_RMNET_6,          NETMGR_LINK_RMNET_6,      FALSE, TRUE, FALSE},
  {"DATA14_CNTL", QMI_PORT_RMNET_7,          NETMGR_LINK_RMNET_7,      FALSE, TRUE, FALSE},
  {"DATA15_CNTL", QMI_PORT_RMNET_8,          NETMGR_LINK_RMNET_8,      FALSE, TRUE, FALSE},
  {"DATA16_CNTL", QMI_PORT_RMNET_9,          NETMGR_LINK_RMNET_9,      FALSE, TRUE, FALSE},
  {"DATA17_CNTL", QMI_PORT_RMNET_10,         NETMGR_LINK_RMNET_10,     FALSE, TRUE, FALSE},
  {"DATA18_CNTL", QMI_PORT_RMNET_11,         NETMGR_LINK_RMNET_11,     FALSE, TRUE, FALSE},
  {"DATA19_CNTL", QMI_PORT_RMNET_12,         NETMGR_LINK_RMNET_12,     FALSE, TRUE, FALSE},
  {"DATA20_CNTL", QMI_PORT_RMNET_13,         NETMGR_LINK_RMNET_13,     FALSE, TRUE, FALSE},
  {"DATA21_CNTL", QMI_PORT_RMNET_14,         NETMGR_LINK_RMNET_14,     FALSE, TRUE, FALSE},
  {"DATA22_CNTL", QMI_PORT_RMNET_15,         NETMGR_LINK_RMNET_15,     FALSE, TRUE, FALSE},
  {"DATA23_CNTL", QMI_PORT_RMNET_16,         NETMGR_LINK_RMNET_16,     FALSE, TRUE, FALSE},

  /* SDIO/USB transport */
  {"MDM0_CNTL",   QMI_PORT_RMNET_SDIO_0,     NETMGR_LINK_RMNET_17,     TRUE,  TRUE, FALSE},
  {"MDM1_CNTL",   QMI_PORT_RMNET_SDIO_1,     NETMGR_LINK_RMNET_18,     FALSE, TRUE, FALSE},
  {"MDM2_CNTL",   QMI_PORT_RMNET_SDIO_2,     NETMGR_LINK_RMNET_19,     FALSE, TRUE, FALSE},
  {"MDM3_CNTL",   QMI_PORT_RMNET_SDIO_3,     NETMGR_LINK_RMNET_20,     FALSE, TRUE, FALSE},
  {"MDM4_CNTL",   QMI_PORT_RMNET_SDIO_4,     NETMGR_LINK_RMNET_21,     FALSE, TRUE, FALSE},
  {"MDM5_CNTL",   QMI_PORT_RMNET_SDIO_5,     NETMGR_LINK_RMNET_22,     FALSE, TRUE, FALSE},
  {"MDM6_CNTL",   QMI_PORT_RMNET_SDIO_6,     NETMGR_LINK_RMNET_23,     FALSE, TRUE, FALSE},
  {"MDM7_CNTL",   QMI_PORT_RMNET_SDIO_7,     NETMGR_LINK_RMNET_24,     FALSE, TRUE, FALSE},
  {"MDM8_CNTL",   QMI_PORT_RMNET_SDIO_8,     NETMGR_LINK_RMNET_25,     FALSE, TRUE, FALSE},
  {"MDM9_CNTL",   QMI_PORT_RMNET_SDIO_9,     NETMGR_LINK_RMNET_26,     FALSE, TRUE, FALSE},
  {"MDM10_CNTL",  QMI_PORT_RMNET_SDIO_10,    NETMGR_LINK_RMNET_27,     FALSE, TRUE, FALSE},
  {"MDM11_CNTL",  QMI_PORT_RMNET_SDIO_11,    NETMGR_LINK_RMNET_28,     FALSE, TRUE, FALSE},
  {"MDM12_CNTL",  QMI_PORT_RMNET_SDIO_12,    NETMGR_LINK_RMNET_29,     FALSE, TRUE, FALSE},
  {"MDM13_CNTL",  QMI_PORT_RMNET_SDIO_13,    NETMGR_LINK_RMNET_30,     FALSE, TRUE, FALSE},
  {"MDM14_CNTL",  QMI_PORT_RMNET_SDIO_14,    NETMGR_LINK_RMNET_31,     FALSE, TRUE, FALSE},
  {"MDM15_CNTL",  QMI_PORT_RMNET_SDIO_15,    NETMGR_LINK_RMNET_32,     FALSE, TRUE, FALSE},
  {"MDM16_CNTL",  QMI_PORT_RMNET_SDIO_16,    NETMGR_LINK_RMNET_33,     FALSE, TRUE, FALSE},

#ifdef FEATURE_DATA_IWLAN
  /* SMD/BAM reverse transport */
  {"DATA23_CNTL", QMI_PORT_REV_RMNET_0,      NETMGR_LINK_REV_RMNET_0,  FALSE, TRUE, FALSE},
  {"DATA24_CNTL", QMI_PORT_REV_RMNET_1,      NETMGR_LINK_REV_RMNET_1,  FALSE, TRUE, FALSE},
  {"DATA25_CNTL", QMI_PORT_REV_RMNET_2,      NETMGR_LINK_REV_RMNET_2,  FALSE, TRUE, FALSE},
  {"DATA26_CNTL", QMI_PORT_REV_RMNET_3,      NETMGR_LINK_REV_RMNET_3,  FALSE, TRUE, FALSE},
  {"DATA27_CNTL", QMI_PORT_REV_RMNET_4,      NETMGR_LINK_REV_RMNET_4,  FALSE, TRUE, FALSE},
  {"DATA28_CNTL", QMI_PORT_REV_RMNET_5,      NETMGR_LINK_REV_RMNET_5,  FALSE, TRUE, FALSE},
  {"DATA29_CNTL", QMI_PORT_REV_RMNET_6,      NETMGR_LINK_REV_RMNET_6,  FALSE, TRUE, FALSE},
  {"DATA30_CNTL", QMI_PORT_REV_RMNET_7,      NETMGR_LINK_REV_RMNET_7,  FALSE, TRUE, FALSE},
  {"DATA31_CNTL", QMI_PORT_REV_RMNET_8,      NETMGR_LINK_REV_RMNET_8,  FALSE, TRUE, FALSE},
  {"DATA32_CNTL", QMI_PORT_REV_RMNET_9,      NETMGR_LINK_REV_RMNET_9,  FALSE, TRUE, FALSE},
  {"DATA33_CNTL", QMI_PORT_REV_RMNET_10,     NETMGR_LINK_REV_RMNET_10, FALSE, TRUE, FALSE},
  {"DATA34_CNTL", QMI_PORT_REV_RMNET_11,     NETMGR_LINK_REV_RMNET_11, FALSE, TRUE, FALSE},
  {"DATA35_CNTL", QMI_PORT_REV_RMNET_12,     NETMGR_LINK_REV_RMNET_12, FALSE, TRUE, FALSE},
  {"DATA36_CNTL", QMI_PORT_REV_RMNET_13,     NETMGR_LINK_REV_RMNET_13, FALSE, TRUE, FALSE},
  {"DATA37_CNTL", QMI_PORT_REV_RMNET_14,     NETMGR_LINK_REV_RMNET_14, FALSE, TRUE, FALSE},
  {"DATA38_CNTL", QMI_PORT_REV_RMNET_15,     NETMGR_LINK_REV_RMNET_15, FALSE, TRUE, FALSE},

  /* USB reverse transport */
  {"RMDM0_CNTL",  QMI_PORT_REV_RMNET_USB_0,  NETMGR_LINK_REV_RMNET_16,  FALSE, TRUE, FALSE},
  {"RMDM1_CNTL",  QMI_PORT_REV_RMNET_USB_1,  NETMGR_LINK_REV_RMNET_17, FALSE, TRUE, FALSE},
  {"RMDM2_CNTL",  QMI_PORT_REV_RMNET_USB_2,  NETMGR_LINK_REV_RMNET_18, FALSE, TRUE, FALSE},
  {"RMDM3_CNTL",  QMI_PORT_REV_RMNET_USB_3,  NETMGR_LINK_REV_RMNET_19, FALSE, TRUE, FALSE},
  {"RMDM4_CNTL",  QMI_PORT_REV_RMNET_USB_4,  NETMGR_LINK_REV_RMNET_20, FALSE, TRUE, FALSE},
  {"RMDM5_CNTL",  QMI_PORT_REV_RMNET_USB_5,  NETMGR_LINK_REV_RMNET_21, FALSE, TRUE, FALSE},
  {"RMDM6_CNTL",  QMI_PORT_REV_RMNET_USB_6,  NETMGR_LINK_REV_RMNET_22, FALSE, TRUE, FALSE},
  {"RMDM7_CNTL",  QMI_PORT_REV_RMNET_USB_7,  NETMGR_LINK_REV_RMNET_23, FALSE, TRUE, FALSE},
  {"RMDM8_CNTL",  QMI_PORT_REV_RMNET_USB_8,  NETMGR_LINK_REV_RMNET_24, FALSE, TRUE, FALSE},
  {"RMDM9_CNTL",  QMI_PORT_REV_RMNET_USB_9,  NETMGR_LINK_REV_RMNET_25, FALSE, TRUE, FALSE},
  {"RMDM10_CNTL", QMI_PORT_REV_RMNET_USB_10, NETMGR_LINK_REV_RMNET_26, FALSE, TRUE, FALSE},
  {"RMDM11_CNTL", QMI_PORT_REV_RMNET_USB_11, NETMGR_LINK_REV_RMNET_27, FALSE, TRUE, FALSE},
  {"RMDM12_CNTL", QMI_PORT_REV_RMNET_USB_12, NETMGR_LINK_REV_RMNET_28, FALSE, TRUE, FALSE},
  {"RMDM13_CNTL", QMI_PORT_REV_RMNET_USB_13, NETMGR_LINK_REV_RMNET_29, FALSE, TRUE, FALSE},
  {"RMDM14_CNTL", QMI_PORT_REV_RMNET_USB_14, NETMGR_LINK_REV_RMNET_30, FALSE, TRUE, FALSE},
  {"RMDM15_CNTL", QMI_PORT_REV_RMNET_USB_15, NETMGR_LINK_REV_RMNET_31, FALSE, TRUE, FALSE},
#endif /* FEATURE_DATA_IWLAN */

  /* Must be last record for validation */
  {"",            "",                       NETMGR_LINK_MAX,          FALSE, FALSE, FALSE}
};

/* Table of transport device name prefix per Modem */
#ifdef FEATURE_DATA_IWLAN
  netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[NETMGR_MAX_MODEMS][NETMGR_MAX_LINK_TYPES] =
  {
    {
      { NETMGR_MAIN_RMNET_SMD_PREFIX,      NETMGR_LINK_RMNET_0,      NETMGR_LINK_RMNET_16      },
      { NETMGR_MAIN_REV_RMNET_SMD_PREFIX,  NETMGR_LINK_REV_RMNET_0,  NETMGR_LINK_REV_RMNET_15  },
    },
    {
      { NETMGR_MAIN_RMNET_SDIO_PREFIX,     NETMGR_LINK_RMNET_17,      NETMGR_LINK_RMNET_33     },
      { NETMGR_MAIN_REV_RMNET_USB_PREFIX,  NETMGR_LINK_REV_RMNET_16,  NETMGR_LINK_REV_RMNET_31 },
    }
  };
#else
  netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[] =
  {
    { NETMGR_MAIN_RMNET_SMD_PREFIX,  NETMGR_LINK_RMNET_0,   NETMGR_LINK_RMNET_16 },
    { NETMGR_MAIN_RMNET_SDIO_PREFIX, NETMGR_LINK_RMNET_17,  NETMGR_LINK_RMNET_33 },

    /* This must be the last entry in the table */
    { "",                            NETMGR_LINK_NONE,     NETMGR_LINK_NONE     }
  };
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_signal_handler
===========================================================================*/
/*!
@brief
  Callback registered as OS signal handler.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Toggles generation of debug messages
*/
/*=========================================================================*/
LOCAL void netmgr_signal_handler( int sig )
{
  switch (sig)
  {
  case SIGUSR1:
    /* On USR1 signal, toggle the debug flags */
    netmgr_main_cfg.debug = (netmgr_main_cfg.debug)? FALSE : TRUE;
    function_debug = (function_debug)? FALSE : TRUE;

    netmgr_cb_runall(NETMGR_MAIN_CB_TABLE, NETMGR_MAIN_CB_SIGUSR1, NULL);

    netmgr_log_med("Signal Handler - Setting debug flag: %d\n",netmgr_main_cfg.debug);
    netmgr_log_med("Runmode: 0x%x\n", netmgr_main_cfg.runmode);
    {
      /* Display security credentials */
      struct __user_cap_data_struct cap_data;
      struct __user_cap_header_struct cap_hdr;
      cap_hdr.version = _LINUX_CAPABILITY_VERSION;
      cap_hdr.pid = 0; /* 0 is considered self pid */
      (void)capget(&cap_hdr, &cap_data);
      netmgr_log_med("Running as: uid[%d] gid[%d] caps_perm/eff[0x%x/0x%x]\n",
                     getuid(), getgid(), cap_data.permitted, cap_data.effective);
    }

    /* Dump DPM related data channel information */
    netmgr_log_med("DPM enabled %d, epid %d, consumer pipe %d, producer pipe %d\n",
        netmgr_main_cfg.dpm_enabled,
        netmgr_main_cfg.epid,
        netmgr_main_cfg.consumer_pipe_num,
        netmgr_main_cfg.producer_pipe_num);
    netmgr_qmi_dpm_log_state();

#ifdef FEATURE_RECOVERY_ENABLED
    netmgr_log_med("SIGUSR1 - Printing recovery file contents\n");
    netmgr_recovery_file_print_contents();
#endif /* FEATURE_RECOVERY_ENABLED */

    break;

  case SIGUSR2:
    netmgr_cb_runall(NETMGR_MAIN_CB_TABLE, NETMGR_MAIN_CB_SIGUSR2, NULL);
    netmgr_qmi_toggle_coalescing_state();
    break;

  case SIGTERM:
    /* On TERM signal, exit() so atexit cleanup functions gets called */
    exit(0);
    break;

  default:
    break;
  }
}

/*===========================================================================
  FUNCTION  netmgr_main_get_qos_enabled
===========================================================================*/
/*!
@brief
  Return value for QOS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_qos_enabled( void )
{
  return (NETMGR_MAIN_RUNMODE_QOSHDR ==
          (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_QOSHDR));
}

/*===========================================================================
  FUNCTION  netmgr_main_get_qos_hybrid_enabled
===========================================================================*/
/*!
@brief
  Return value for QOS hybrid enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_qos_hybrid_enabled( void )
{
  return netmgr_main_cfg.hybrid_qos;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline iwlan_state_t netmgr_main_get_iwlan_enabled( void )
{
  return netmgr_main_cfg.iwlan_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_ims_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN IMS enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
inline int netmgr_main_get_iwlan_ims_enabled( void )
{
  return netmgr_main_cfg.iwlan_ims_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_iwlan_traffic_sel_enabled
===========================================================================*/
/*!
@brief
  Return value for iWLAN traffic selector enabled configuration item

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_iwlan_traffic_sel_enabled( void )
{
  return netmgr_main_cfg.iwlan_traffic_sel_enabled;
}
#endif /* FEATURE_DATA_IWLAN */

#ifdef FEATURE_DS_LINUX_ANDROID
/*===========================================================================
  FUNCTION  netmgr_main_get_tcpackprio_enabled
===========================================================================*/
/*!
@brief
  Return value for TCP_ACK_PRIO enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_tcpackprio_enabled( void )
{
  return netmgr_main_cfg.tcp_ack_prio;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_baserate
===========================================================================*/
/*!
@brief
  Return value for TC_BASE_DATARATE enabled configuration item from the netmgr
  configuration property.

@return
  int -

@note

- Dependencies
   - None

- Side Effects
   - None
*/
/*=========================================================================*/
uint64_t netmgr_main_get_tc_ul_baserate( void )
{
  return netmgr_main_cfg.tc_ul_baserate;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_dynamic_tc_ul_baserate
===========================================================================*/
/*!
@brief
  Return value for dynamic_tc_ul_baserate enabled configuration item from the
  netmgr configuration property.

@return
  int -

@note

- Dependencies
   - None

- Side Effects
   - None
*/
/*=========================================================================*/
int netmgr_main_get_dynamic_tc_ul_baserate( void )
{
  return netmgr_main_cfg.dynamic_tc_ul_baserate;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_tc_ul_burst
===========================================================================*/
/*!
@brief
  Return value for TC_UL_BURST enabled configuration item from the netmgr
  configuration property.

@return
  unsigned long -

@note

- Dependencies
   - None

- Side Effects
   - None
*/
/*=========================================================================*/
unsigned long netmgr_main_get_tc_ul_burst( void )
{
  return netmgr_main_cfg.tc_ul_burst;
}
#endif /* FEATURE_DS_LINUX_ANDROID */

/*===========================================================================
    FUNCTION  netmgr_main_get_qos_via_idl_enabled
  ===========================================================================*/
/*!
  @brief
  Return value for QOS_VIA_IDL enabled configuration item from the netmgr
  configuration property.

  @return
  int - TRUE/FALSE

  @note

  - Dependencies
  - None

  - Side Effects
  - None
*/
/*=========================================================================*/
int netmgr_main_get_qos_via_idl_enabled( void )
{
  return netmgr_main_cfg.qos_via_idl;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_ibfc_enabled
===========================================================================*/
/*!
@brief
  Return value for IBFC enabled configuration item from the netmgr
  configuration property.

@return
  int - TRUE/FALSE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_main_get_ibfc_enabled( void )
{
  return netmgr_main_cfg.ibfc;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_rmnet_data_enabled
===========================================================================*/
/*!
@brief
  Return value for rmnet data driver enabled configuraition item

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_rmnet_data_enabled( void )
{
  return netmgr_main_cfg.rmnet_data_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_phys_net_dev
===========================================================================*/
/*!
@brief
  Return value for physical network device if any.

@return
  char pointer to physical network device name
  NULL if none such device exists.
*/
/*=========================================================================*/
char *netmgr_main_get_phys_net_dev( void )
{
  return netmgr_main_cfg.phys_net_dev;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_dpm_enabled
===========================================================================*/
/*!
@brief
  Return value for Data Port Mapper enablement

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_dpm_enabled( void )
{
  return netmgr_main_cfg.dpm_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_low_latency_filters_enabled
===========================================================================*/
/*!
@brief
  Return value for low latency filters through QMI DFS enablement

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_low_latency_filters_enabled( void )
{
  return netmgr_main_cfg.low_latency_filters;
}

/*===========================================================================
  FUNCTION  netmgr_main_get_skip_buffered_qos_modify
===========================================================================*/
/*!
@brief
  Return value for flag to control feature to enable/disable skip buffered
  QoS modify events.

@return
  int - TRUE/FALSE
*/
/*=========================================================================*/
inline int netmgr_main_get_skip_buffered_qos_modify( void )
{
  return netmgr_main_cfg.skip_buffered_qos_modify;
}

/*===========================================================================
  FUNCTION  netmgr_main_set_skip_buffered_qos_modify
===========================================================================*/
/*!
@brief
  Sets value for flag to control feature to enable/disable skip buffered
  QoS modify events.

@return
  void
*/
/*=========================================================================*/
inline void netmgr_main_set_skip_buffered_qos_modify( int val )
{
  netmgr_main_cfg.skip_buffered_qos_modify = val;
}

/*===========================================================================
  FUNCTION  netmgr_main_process_arg
===========================================================================*/
/*!
@brief
  Populates program configuration information for the specified argument and
  argument value.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
*/
/*=========================================================================*/
void
netmgr_main_process_arg(char argid, char * argval)
{
  switch (argid) {
    case 'b':
      /* run in background, i.e. run as a forked daemon process */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_BACK;
      fprintf(stderr, "running in background process\n");
      break;
    case 'E':
      /* use Ethernet link protocol */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_ETHERNET;
      fprintf(stderr, "using Ethernet link protocol\n");
      break;
#ifdef NETMGR_QOS_ENABLED
    case 'Q':
      /* use RmNET QoS header prepended to TX packets */
      netmgr_main_cfg.runmode |= NETMGR_MAIN_RUNMODE_QOSHDR;
      fprintf(stderr, "QOS enabled, using QMI header\n");
      break;
#endif /* NETMGR_QOS_ENABLED */
    case 's':
      /* Log to syslog. By default program will log to stderr */
      netmgr_main_cfg.logmode = (int)DS_LOG_MODE_SYSLOG;
      fprintf(stderr, "using syslog\n");
      break;
    case 'l':
      /* Logging threshold as an integer value */
      netmgr_main_cfg.logthreshold = ds_atoi(argval);
      fprintf(stderr, "using log level %d\n", ds_atoi(argval));
      break;
    case 'n':
      /* Number of interfaces to create */
      netmgr_main_cfg.nint = ds_atoi(argval);
      fprintf(stderr, "cfging %d interfaces\n", ds_atoi(argval));
      break;
    case 'i':
      /* Interface name to use */
      netmgr_main_cfg.iname = argval;
      fprintf(stderr, "using interface name %s\n", argval);
      break;
    case 'k':
      /* Load kernel driver module and DHCP client */
      netmgr_main_cfg.skip = NETMGR_KIF_LOAD;
      fprintf(stderr, "perform module load\n");
      break;
    case 'd':
      /* Directory pathname to search for script files */
      netmgr_main_cfg.dirpath = argval;
      fprintf(stderr, "using relative path %s\n", argval);
      break;
    case 'm':
      /* Name of driver module load script */
      netmgr_main_cfg.modscr = argval;
      fprintf(stderr, "using module load script %s\n", argval);
      break;
    case 'D':
      /* Verbose debug flag */
      netmgr_main_cfg.debug = TRUE;
      function_debug = TRUE;
      fprintf(stderr, "setting debug mode.\n");
      break;
    case 'T':
      /* Execute internal tests flag */
      netmgr_main_cfg.runtests = TRUE;
      fprintf(stderr, "setting runtests mode.\n");
      break;
    default:
      /* Ignore unknown argument */
      fprintf(stderr, "ignoring unknown arg '%c'\n", argid);
      break;
  }
  return;
}


/*===========================================================================
  FUNCTION  netmgr_main_parse_args
===========================================================================*/
/*!
@brief
  Parses all specified program command line arguments and populates
  configuration information.

@return
  void

@note

   - Dependencies
     - None

   - Side Effects
     - None
*/
/*=========================================================================*/
void
netmgr_main_parse_args (int argc, char ** argv)
{
  int i;
  char a;

  NETMGR_LOG_FUNC_ENTRY;

  for (i = 1; i < argc; ++i) {
    if (std_strlen(argv[i]) < 2) {
      /* Minimum length of a valid argument is 2, as each arg is
      ** prefixed by a '-' char.
      */
      continue;
    }

    if (*argv[i] != '-') {
      /* Every valid arg should begin with a '-' */
      continue;
    }

    /* Get next char in argument, which is the arg type */
    a = *(argv[i] + 1);

    /* Process based on type of argument */
    switch (a) {
      case 'l':
      case 'n':
      case 'i':
      case 'd':
      case 'm':
      case 'u':
      case 't':
        /* These arguments have an associated value immediately following
        ** the argument.
        */
        if (++i < argc) {
          netmgr_main_process_arg(a, argv[i]);
        }
        break;
      case 'b':
      case 'E':
      case 'Q':
      case 's':
      case 'k':
      case 'D':
      case 'T':
        /* These arguments do not have any value following the argument */
        netmgr_main_process_arg(a, 0);
        break;
      default:
        /* Everything else is an unknown arg that is ignored */
        fprintf(stderr, "unknown arg %s specified\n", argv[i]);
    }
  }

#if 0
  /* Verify Ethernet and QoS modes are not specified simultaneously;
   * this is currently not supported in Linux RmNET driver
   * (due to insufficent skb headroom) */
  if( (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_ETHERNET) &&
      (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_QOSHDR) ) {
    NETMGR_STOP("Ethernet protocol with QoS Header not supproted!!");
  }
#endif

  NETMGR_LOG_FUNC_EXIT;
  return;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_disable_modem_reverse_links
===========================================================================*/
/*!
@brief
 This function disables all the reverse links in the given modem.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_disable_modem_reverse_links
(
  int  modem
)
{
  int i;
  int modem_start_index = NETMGR_MAIN_GET_INST_MIN_REV(modem);
  int modem_end_index = NETMGR_MAIN_GET_INST_MAX_REV(modem);

  netmgr_log_low("disabling modem [%d] reverse ports start=[%d], end=[%d]",
                 modem,
                 modem_start_index,
                 modem_end_index);

  for (i = modem_start_index; i <= modem_end_index; i++)
  {
    netmgr_ctl_port_array[i].enabled = FALSE;
    netmgr_ctl_port_array[i].modem_wait = FALSE;
  }
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_main_disable_modem_links
===========================================================================*/
/*!
@brief
 This function disables all the links in the modem which is disabled.
 Which modem is enabled/disabled is determined by target configuration

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_disable_modem_links
(
  int   *modems_enabled,
  int   num_modems
)
{
  int i;
  int j;
  int modem_start_index;
  int modem_end_index;

  ds_assert(modems_enabled != NULL);
  ds_assert(((num_modems >= 0) && (num_modems <= NETMGR_MAX_MODEMS)));

  for(i=0; i < num_modems; i++)
  {
    netmgr_log_low("modem_enable[%d]=[%d]", i, modems_enabled[i]);
    if (TRUE != modems_enabled[i])
    {
      /* Disable the forward links on the modem */
      modem_start_index = NETMGR_MAIN_GET_INST_MIN(i);
      modem_end_index = NETMGR_MAIN_GET_INST_MAX(i);

      netmgr_log_low("disabling modem [%d] forward ports start=[%d], end=[%d]",
                     i,
                     modem_start_index,
                     modem_end_index);

      for (j = modem_start_index; j <= modem_end_index; j++)
      {
        netmgr_ctl_port_array[j].enabled = FALSE;
        netmgr_ctl_port_array[j].modem_wait = FALSE;
      }

#ifdef FEATURE_DATA_IWLAN
      /* Disable all reverse links on the modem */
      netmgr_main_disable_modem_reverse_links(i);
#endif /* FEATURE_DATA_IWLAN */
    }
  }
}


#ifdef FEATURE_DS_LINUX_ANDROID
#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_main_process_iwlan_enabled
===========================================================================*/
/*!
@brief
 This function reads the iwlan persist property to determine whether to
 enable or disable the feature.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_process_iwlan_enabled
(
  int   *modems_enabled,
  int   num_modems
)
{
#define NSWO_IWLAN_ONLY "nswo"

  int i, ret;
  char args[PROPERTY_VALUE_MAX];
  char args_encrypt_state[PROPERTY_VALUE_MAX];
  iwlan_state_t is_iwlan_enabled = IWLAN_DISABLE;
  boolean is_iwlan_ims_enabled = FALSE;

  if (!modems_enabled || (num_modems < 0) || (num_modems > NETMGR_MAX_MODEMS)) {
    netmgr_log_err("invalid parameters\n");
    return;
  }

  /* Retrieve value of NETMGR_MAIN_PROPERTY_IWLAN */
  memset(args, 0, sizeof(args));
  memset(args_encrypt_state, 0, sizeof(args_encrypt_state));

  do
  {
      /* Device encryption/decryption state can be determined by the value of vold.decrypt
         property

         If the system is running in encrypted state then the property will be set to
         "trigger_restart_min_framework"

         If the system is running in decrypted state then the property will be set to
         "trigger_restart_framework" */
      ret = property_get( "vold.decrypt", args_encrypt_state, "" );

      if (strncasecmp("trigger_restart_min_framework",
                  args_encrypt_state,
                  sizeof("trigger_restart_min_framework")) == 0)
      {
          netmgr_log_med("%s(): device is encrypted, setting iwlan prop to false",
                  __func__);
          netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
      }
      else if(netmgr_main_cfg.iwlan_enabled != IWLAN_DISABLE)
      {
          netmgr_log_med("%s(): iwlan enabled from netmgr_config.xml,"
                         "thus reading value of adb property",
                         __func__);

          ret = property_get( NETMGR_MAIN_PROPERTY_IWLAN, args,
                              NETMGR_MAIN_PROPERTY_IWLAN_DEFAULT );

          if (ret > NETMGR_MAIN_PROPERTY_IWLAN_SIZE)
          {
              netmgr_log_err("property [%s] has size [%d] that exceeds max [%d],"
                             "property value can be true, false, nswo.",
                             NETMGR_MAIN_PROPERTY_IWLAN,
                             ret,
                             NETMGR_MAIN_PROPERTY_IWLAN_SIZE);
              is_iwlan_enabled = netmgr_main_cfg.iwlan_enabled;
              break;
          }

          if( !strncasecmp( NETMGR_TRUE, args, sizeof(NETMGR_TRUE) ) )
          {
              is_iwlan_enabled = IWLAN_ENABLE;

              /* In case where adb property is not set and
                 config.xml entry contains iwlan in nswo mode,
                 then iwlan mode should be nswo */
              if( is_iwlan_enabled == IWLAN_ENABLE &&
                      netmgr_main_cfg.iwlan_enabled == NSWO_ONLY)
              {
                  netmgr_main_cfg.iwlan_enabled = NSWO_ONLY;
              }
              else
              {
                  netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;
              }
          }
          else if(!strncasecmp( NSWO_IWLAN_ONLY, args, sizeof(NSWO_IWLAN_ONLY) ) )
          {
              is_iwlan_enabled = NSWO_ONLY;
              netmgr_main_cfg.iwlan_enabled = NSWO_ONLY;
          }
          else if(!strncasecmp( NETMGR_FALSE, args, sizeof(NETMGR_FALSE) ) )
          {
              is_iwlan_enabled = IWLAN_DISABLE;
              netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
          }
      }
  }while(0);

  /* Disable reverse links on all enabled modems */
  if (IWLAN_DISABLE == is_iwlan_enabled)
  {
    for (i = 0; i < num_modems; i++)
    {
      netmgr_log_low("modem_enable[%d]=[%d]", i, modems_enabled[i]);
      if (TRUE == modems_enabled[i])
      {
        /* Disable all reverse links on the modem */
        netmgr_main_disable_modem_reverse_links(i);
      }
    }
  }

  netmgr_main_cfg.iwlan_ims_enabled = is_iwlan_ims_enabled;
}

/*===========================================================================
  FUNCTION  netmgr_main_process_iwlan_traffic_selectors_enabled
===========================================================================*/
/*!
@brief
  This function disable the feature by default now

@return
  void

*/
/*=========================================================================*/
LOCAL void
netmgr_main_process_iwlan_traffic_selectors_enabled(void)
{
  netmgr_log_low("set iwlan_traffic_sel_enabled to false by default");
  netmgr_main_cfg.iwlan_traffic_sel_enabled = FALSE;
}

#endif /* FEATURE_DATA_IWLAN */
#endif /* FEATURE_DS_LINUX_ANDROID */

/*===========================================================================
  FUNCTION  netmgr_main_update_dev_prefix_tbl
===========================================================================*/
/*!
@brief
  This functions updates the dev prefix table with appropriate values which
  will be used for enabling/disabling modem links further.

@return
  void

*/
/*=========================================================================*/
void netmgr_main_update_dev_prefix_tbl(void)
{
    int rmnet_smd_fwd_start  = (netmgr_main_cfg.fwd_link_start); /* 0 */
    int rmnet_sdio_fwd_start = (netmgr_main_cfg.fwd_link_start + netmgr_main_cfg.num_data_ports); /* 11 or 17 */
    int rmnet_smd_fwd_end    = (rmnet_sdio_fwd_start - 1); /* 10 or 16 */
    int rmnet_sdio_fwd_end   = (rmnet_sdio_fwd_start + netmgr_main_cfg.num_data_ports - 1); /* 21 or 33 */

#ifdef FEATURE_DATA_IWLAN
    int rmnet_smd_rev_start  = (netmgr_main_cfg.rev_link_start); /* 22 or 34 */
    int rmnet_sdio_rev_start = (netmgr_main_cfg.rev_link_start + netmgr_main_cfg.num_rev_data_ports); /* 22+9=31 or 34+16=50*/
    int rmnet_smd_rev_end    = (rmnet_sdio_rev_start - 1); /* 30 or 49 */
    int rmnet_sdio_rev_end   = (rmnet_sdio_rev_start + netmgr_main_cfg.num_rev_data_ports - 1); /* 31+9+-1=39 or 50+16-1=65*/

    /* SMD Modem 0 */
    netmgr_main_dev_prefix_tbl[0][0].inst_min = rmnet_smd_fwd_start;
    netmgr_main_dev_prefix_tbl[0][0].inst_max = rmnet_smd_fwd_end;

    netmgr_main_dev_prefix_tbl[0][1].inst_min = rmnet_smd_rev_start;
    netmgr_main_dev_prefix_tbl[0][1].inst_max = rmnet_smd_rev_end;

    /* SDIO Modem 1 */
    netmgr_main_dev_prefix_tbl[1][0].inst_min = rmnet_sdio_fwd_start;
    netmgr_main_dev_prefix_tbl[1][0].inst_max = rmnet_sdio_fwd_end;

    netmgr_main_dev_prefix_tbl[1][1].inst_min = rmnet_sdio_rev_start;
    netmgr_main_dev_prefix_tbl[1][1].inst_max = rmnet_sdio_rev_end;
#else
    /* SMD Modem 0 */
    netmgr_main_dev_prefix_tbl[0].inst_min = rmnet_smd_fwd_start;
    netmgr_main_dev_prefix_tbl[0].inst_max = rmnet_smd_fwd_end;

    /* SDIO Modem 1 */
    netmgr_main_dev_prefix_tbl[1].inst_min = rmnet_sdio_fwd_start;
    netmgr_main_dev_prefix_tbl[1].inst_max = rmnet_sdio_fwd_end;
#endif
}

/*===========================================================================
  FUNCTION  netmgr_main_process_target
===========================================================================*/
/*!
@brief
  Updates netmgr links based on current target configuration.

@return
  void

*/
/*=========================================================================*/
LOCAL void netmgr_main_process_target()
{
  ds_target_t target;
  const char *target_str;

  target = ds_get_target();
  target_str = ds_get_target_str(target);

  netmgr_log_med("ds_target is set to: %d [%s]", target, target_str);
  memset(netmgr_main_cfg.modem_enable, 0, sizeof(netmgr_main_cfg.modem_enable));

  if (NETMGR_SUCCESS != netmgr_config_load(NETMGR_CONFIG_FILE, target_str ))
  {
    netmgr_log_err("%s: unable to load config for target:%s",
                   __func__, target_str);
    /* Abort can misbehave sometimes; print the error twice */
    NETMGR_ABORT("%s: unable to load config for target:%s",
                 __func__, target_str);
  }
  netmgr_log_low("%s: loaded config for target:%s", __func__, target_str);

  /* epid from phys_net_dev */
  if ((DS_TARGET_FUSION_SDA855_SDX55 == target) ||
      (DS_TARGET_FUSION_SDA865_SDX55 == target) ||
      (DS_TARGET_FUSION_SDA855_SDXMARMOT == target))
    ds_get_epid(netmgr_main_cfg.phys_net_dev, &netmgr_main_cfg.ep_type,
                &netmgr_main_cfg.epid);

  netmgr_main_update_dev_prefix_tbl();
  netmgr_main_disable_modem_links(netmgr_main_cfg.modem_enable, NETMGR_MAX_MODEMS);

  netmgr_config_print();
#ifdef FEATURE_DATA_IWLAN
  netmgr_main_process_iwlan_enabled(netmgr_main_cfg.modem_enable, NETMGR_MAX_MODEMS);
  netmgr_log_med("%s():setting iwlan_enabled = [%d]", __func__, netmgr_main_cfg.iwlan_enabled);

  /* Check if traffic selector property is enabled */
  netmgr_main_process_iwlan_traffic_selectors_enabled();
#endif /* FEATURE_DATA_IWLAN */

  netmgr_config_check_output_mark_availability();
}

/*===========================================================================
  FUNCTION  netmgr_main_sm_inited
===========================================================================*/
/*!
@brief
  posts NETMGR_INITED_EV to each state machine instance

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void netmgr_main_sm_inited(void)
{
  int i=0;
  netmgr_exec_cmd_t * cmd = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  for(i=0; i<netmgr_main_cfg.max_links_in_total; i++)
  {
    /* Skip disabled links */
    if( netmgr_ctl_port_array[i].enabled == FALSE )
    {
      netmgr_log_low( "netmgr_main_sm_inited: ignoring link[%d]\n", i );
      continue;
    }

    netmgr_log_low( "netmgr_main_sm_inited: initing link[%d]\n", i );

    /* Allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);

    /* Set command object parameters */
    cmd->data.type = NETMGR_INITED_EV;
    cmd->data.link = i;
    memset(&cmd->data.info, 0, sizeof(cmd->data.info));

    /* Post command for processing in the command thread context */
    if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
      netmgr_log_err("failed to put commmand\n");
      netmgr_exec_release_cmd( cmd );
    }
  }

  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_diag_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of Diag LSM resources.  Invoked at process termination.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_diag_cleanup(void)
{
#ifdef FEATURE_DATA_LOG_QXDM
  (void) Diag_LSM_DeInit();
#endif

#ifdef FEATURE_RECOVERY_ENABLED
  (void) netmgr_recovery_file_close();
#endif /* FEATURE_RECOVERY_ENABLED */
}

/*===========================================================================
  FUNCTION  netmgr_load_data_format
===========================================================================*/
/*!
@brief
  Populates some fields of runtime data format.

@return
  0 always.
*/
/*=========================================================================*/
int
netmgr_load_data_format
(
  void
)
{
  netmgr_data_format_t *props = &(netmgr_main_cfg.data_format);

  props->qos_format = netmgr_main_get_qos_enabled() & (!netmgr_main_get_qos_hybrid_enabled());
  props->link_prot = NETMGR_DATA_FMT_DFLT_LLP;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_main_post_is_complete_ev
===========================================================================*/
/*!
@brief
  Post the IS_COMPLETE event to the executive. This ensures that we start
  processing ready requests from dsi only after at least one of the link in
  netmgr is in INITED state.

@return
  None
*/
/*=========================================================================*/
void
netmgr_main_post_is_complete_ev()
{
  netmgr_exec_cmd_t *cmd = NULL;
  int i = 0;

  NETMGR_LOG_FUNC_ENTRY
  netmgr_log_err("%s:ENTRY " , __func__);

  for (i = 0; i<netmgr_main_cfg.max_links_in_total; i++)
  {
    if (netmgr_ctl_port_array[i].enabled)
    {
      break;
    }
  }

  if (netmgr_main_cfg.max_links_in_total == i)
  {
    NETMGR_ABORT("No links are enabled in config");
    return;
  }

  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);
  cmd->cmd.execute_f = netmgr_exec_process_msg ;
  cmd->data.type = NETMGR_IS_COMPLETE_MSG ;
  cmd->data.link = i;

  if(NETMGR_FAILURE == netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("Unable to post is_complete msg");
  }
  NETMGR_LOG_FUNC_EXIT
}


/*===========================================================================
  FUNCTION  netmgr_main
===========================================================================*/
/*!
@brief
  Main entry point of the core program. Performs all program initialization.

@return
  int - NETMGR_SUCCESS always

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_main_init_cb()
{
  /* TODO Check the returns */
  netmgr_cb_create_table(NETMGR_MAIN_CB_TABLE);
  netmgr_cb_create_chain(NETMGR_MAIN_CB_TABLE, NETMGR_MAIN_CB_SIGUSR1);
  netmgr_cb_create_chain(NETMGR_MAIN_CB_TABLE, NETMGR_MAIN_CB_SIGUSR2);
  netmgr_cb_create_chain(NETMGR_MAIN_CB_TABLE, NETMGR_MAIN_CB_INITDONE);
}

/*===========================================================================
  FUNCTION netmgr_main_register_plugins
===========================================================================*/
/*!
@brief
  Registers the plugins with netmgr callback framework
  - Link-local
  - iWLAN

@return
  None
*/
/*=========================================================================*/
static void netmgr_main_register_plugins(void)
{
#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  /* TODO: Figure out a way to remove the compile time dependency */
  nm_link_local_routing_main_register_plugin();
#endif /* !defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID) */

  /* TODO: Remove dependency on iWLAN property for initializing modem proxy plugin
     and rely on configdb instead */
  /* TODO: Figure out a way to remove compile time dependencies */
#ifdef FEATURE_DATA_IWLAN
  if (IWLAN_DISABLE != netmgr_main_get_iwlan_enabled())
#endif /* FEATURE_DATA_IWLAN */
  {
#if (defined (FEATURE_DS_LINUX_ANDROID) || defined (NETMGR_OFFTARGET))
    nm_mdmprxy_register_plugin();
#endif /* (defined (FEATURE_DS_LINUX_ANDROID) || defined (NETMGR_OFFTARGET)) */
  }
}

/*===========================================================================
  FUNCTION netmgr_main_initialize_plugins
===========================================================================*/
/*!
@brief
  Initializes the plugin modules
  - Link-local
  - iWLAN

@param restart - indicates netmgr startup mode
       0 - regular bootup
       1 - crash restart

@return
  None
*/
/*=========================================================================*/
static void netmgr_main_initialize_plugins(int restart)
{
  #if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  /* TODO: Figure out a way to remove the compile time dependency */
  /* Initialize plugin modules here */
  nm_link_local_routing_main_init(restart);
#endif /* !defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID) */

  /* TODO: Remove dependency on iWLAN property for initializing modem proxy plugin
     and rely on configdb instead */
  /* TODO: Figure out a way to remove compile time dependencies */
#ifdef FEATURE_DATA_IWLAN
  if (IWLAN_DISABLE != netmgr_main_get_iwlan_enabled())
#endif /* FEATURE_DATA_IWLAN */
  {
#if (defined (FEATURE_DS_LINUX_ANDROID) || defined (NETMGR_OFFTARGET))
    nm_mdmprxy_init(restart);
#endif /* (defined (FEATURE_DS_LINUX_ANDROID) || defined (NETMGR_OFFTARGET)) */
  }
}

/* AID_QCOM_DIAG has been removed and been replaced with "oem_2901" which corresponds
   to group ID 2901. Netmgr needs to be part of this group to use diag for logging.
   Currently we don't have a definition for 2901 in the code. Till that time use a
   local define and set the correct group. */
#define NETMGR_DIAG_GRP 2901

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  netmgr_main_recovery_init
===========================================================================*/
/*!
@brief
  Initialize recovery file and determine if we are in a restart scenario.

@return
  int - TRUE if restart scenario, FALSE if not
*/
/*=========================================================================*/
static int netmgr_main_recovery_init(void)
{
  file_info_t meta_info;

  if (NETMGR_SUCCESS != netmgr_recovery_file_init())
  {
    return FALSE;
  }

  if (NETMGR_SUCCESS != netmgr_recovery_file_read_entry(NETMGR_RECOVERY_META_INFO_INDEX,
                                                        &meta_info))
  {
    return FALSE;
  }

  if ((TRUE == meta_info.is_valid)
        && (meta_info.recovery_info.meta_info.total_entries > 0))
  {
    return TRUE;
  }

  return FALSE;
}

/*===========================================================================
  FUNCTION  netmgr_main_recovery_cleanup_links
===========================================================================*/
/*!
@brief
  Cleanup the xfrm state/policy for all the link entries in the recovery file.

@return
  int - NETMGR_SUCCESS if dangling XFRM rules are cleaned up,
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
static int netmgr_main_recovery_cleanup_links(void)
{
  int rc = NETMGR_SUCCESS;
  int cur_index = 0;
  int reti = NETMGR_SUCCESS;
  file_info_t meta_info;
  uint64_t bitmask;
  file_info_t cur_file_info;
  uint32_t cleaned_entries = 0;

  if (NETMGR_SUCCESS != netmgr_recovery_file_read_entry(NETMGR_RECOVERY_META_INFO_INDEX,
                                                        &meta_info))
  {
    netmgr_log_err("%s(): Failed to read meta info!", __func__);
    return NETMGR_FAILURE;
  }

  /* Read all valid entries from file and clean up the sa rules if it
     is a iwlan link info entry */
  bitmask = meta_info.recovery_info.meta_info.entry_bitmask;
  while (bitmask != 0)
  {
    if (bitmask & 0x1)
    {
      if (cur_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
      {
        break;
      }

      reti = netmgr_recovery_file_read_entry(cur_index, &cur_file_info);

      if (NETMGR_SUCCESS != reti)
      {
        netmgr_log_err("%s(): Failed to read entry at index [%d] "
                       "meta_info bitmask = 0x%lx", __func__,
                       cur_index,
                       meta_info.recovery_info.meta_info.entry_bitmask);
        rc = NETMGR_FAILURE;
        bitmask >>= 1;
        cur_index += 1;
        continue;
      }

      if ((TRUE == cur_file_info.is_valid)
           && (cur_file_info.info_type == TYPE_IWLAN_LINK_INFO))
      {
        reti = netmgr_kif_cleanup_sa_rules(&cur_file_info);
        if (NETMGR_SUCCESS != reti)
        {
          netmgr_log_err("%s(): Failed to cleanup sa rules for link [%d] "
                         "ip_family [%d]!", __func__,
                         cur_file_info.recovery_info.link_info.link,
                         cur_file_info.recovery_info.link_info.ip_family);
          /* NOTE - Something failed to get cleaned up, this entry might never get removed
             and the file might eventaully reach the max allowable */
          rc = NETMGR_FAILURE;
          bitmask >>= 1;
          cur_index += 1;
          continue;
        }

        netmgr_log_low("%s(): Successfully cleaned up sa rules for link [%d]"
                       "ip_family [%d]!", __func__,
                       cur_file_info.recovery_info.link_info.link,
                       cur_file_info.recovery_info.link_info.ip_family);
        cleaned_entries += 1;
      }
    }

    bitmask >>= 1;
    cur_index += 1;
  }

  if (NETMGR_SUCCESS == rc)
  {
    netmgr_log_med("%s(): Cleaned up all entries from recovery file",
                   __func__)
  }
  else
  {
    netmgr_log_med("%s(): Failed to clean %d of %d entries from recovery file",
                   __func__,
                   (meta_info.recovery_info.meta_info.total_entries - cleaned_entries),
                   meta_info.recovery_info.meta_info.total_entries);
  }

  return rc;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  netmgr_main_unset_ko_loading_properties
===========================================================================*/
/*!
@brief
  Unsets android properties to trigger unloading of netmgr kernel modules

@return
  int - NETMGR_SUCCESS if properties were unset correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_main_unset_ko_loading_properties(void)
{
  ds_target_t target = ds_get_target();
  int reti = NETMGR_SUCCESS;

  if (target == DS_TARGET_MSMNILE
#ifdef FEATURE_DS_LINUX_ANDROID
      && !isRunningWithVendorEnhancedFramework()
#endif /* FEATURE_DS_LINUX_ANDROID */
     )
  {
    return reti;
  }

  /* Unset SHS KO property */
  if (netmgr_main_cfg.rmnet_shs && NETMGR_SUCCESS
      != netmgr_rmnet_set_ko_loading_property(NETMGR_MAIN_SHS_KO_PROPERTY,
                                              NETMGR_MAIN_KO_PROPERTY_UNSET))
  {
    reti = NETMGR_FAILURE;
  }

  /* Unset Perf KO property */
  if (netmgr_main_cfg.rmnet_perf && NETMGR_SUCCESS
      != netmgr_rmnet_set_ko_loading_property(NETMGR_MAIN_PERF_KO_PROPERTY,
                                              NETMGR_MAIN_KO_PROPERTY_UNSET))
  {
    reti = NETMGR_FAILURE;
  }

  return reti;
}

/*===========================================================================
  FUNCTION  netmgr_main_set_ko_loading_properties
===========================================================================*/
/*!
@brief
  Sets android properties to trigger loading of netmgr kernel modules

@return
  int - NETMGR_SUCCESS if properties were set correctly
        NETMGR_FAILURE otherwise
*/
/*=========================================================================*/
int netmgr_main_set_ko_loading_properties(int netmgr_restart)
{
  ds_target_t target = ds_get_target();
  int reti = NETMGR_SUCCESS;

  if (target == DS_TARGET_MSMNILE
#ifdef FEATURE_DS_LINUX_ANDROID
      && !isRunningWithVendorEnhancedFramework()
#endif /* FEATURE_DS_LINUX_ANDROID */
     )
  {
    return reti;
  }

  if (netmgr_restart)
  {
    /* If we are in a netmgr restart scenario, unload the ko module first
     * before attempting to load again */
    netmgr_log_high("%s(): netmgr restarted, unload ko modules!", __func__);
    netmgr_main_unset_ko_loading_properties();
  }

  /* Set SHS KO property */
  if (netmgr_main_cfg.rmnet_shs)
  {
    if( NETMGR_SUCCESS == netmgr_rmnet_set_ko_loading_property(NETMGR_MAIN_SHS_KO_PROPERTY,
                                                               NETMGR_MAIN_KO_PROPERTY_SET_ONE))
    {
        /* rmnet_shs.ko load started succesfully, start shsusrd */
        if (NETMGR_SUCCESS != netmgr_rmnet_set_shsusr_loading_property(NETMGR_MAIN_SHSUSR_LOAD_PROPERTY,
                                                                       NETMGR_MAIN_SHSUSR_PROPERTY_ENABLE))
        {
            netmgr_log_high("%s(): failed to set propery to load shsusrd!", __func__);
            reti = NETMGR_FAILURE;
        }
        else
        {
            netmgr_log_high("%s(): enabled property for shsusrd load", __func__);
        }
    }
    else
    {
        netmgr_log_high("%s(): failed to set propery to load rmnet_shs!", __func__);
        reti = NETMGR_FAILURE;
    }
  }

  /* Set Perf KO property */
  if (netmgr_main_cfg.rmnet_perf)
  {
    do {
      char rmnet_perf_val[2] = {};

      if (netmgr_main_cfg.rmnet_perf < 0 ||
          netmgr_main_cfg.rmnet_perf > NETMGR_MAIN_PERF_KO_PROPERTY_MAX)
      {
        reti = NETMGR_FAILURE;
        break;
      }

      if (snprintf(rmnet_perf_val, sizeof(rmnet_perf_val), "%d",
                   netmgr_main_cfg.rmnet_perf) <= 0)
      {
        reti = NETMGR_FAILURE;
        break;
      }

      if (NETMGR_SUCCESS !=
          netmgr_rmnet_set_ko_loading_property(NETMGR_MAIN_PERF_KO_PROPERTY,
                                               rmnet_perf_val))
      {
        reti = NETMGR_FAILURE;
      }
    } while (0);
  }

  return reti;
}

/*===========================================================================
  FUNCTION  netmgr_main
===========================================================================*/
/*!
@brief
  Main entry point of the core program. Performs all program initialization.

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
netmgr_main (int argc, char ** argv)
{
  time_t t;
  int log_flags;
  int netmgr_restart = 0;

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  #if defined(TARGET_SUPPORTS_ANDROID_WEAR)
    gid_t groups[] = { AID_NET_RAW, AID_INET, AID_SYSTEM };
  #else
#ifdef NETMGR_DIAG_GRP
    gid_t groups[] = { AID_NET_RAW, AID_INET, NETMGR_DIAG_GRP, AID_SYSTEM };
#else
    gid_t groups[] = { AID_NET_RAW, AID_INET, AID_DIAG, AID_SYSTEM };
#endif
  #endif

  size_t size = (sizeof(groups) / sizeof(groups[0]));

  /* Set the required supplemental groups */
  if (NETMGR_SUCCESS != setgroups(size, groups))
  {
    netmgr_log_err("Failed to change supplemental groups for netmgr",
                   errno, strerror(errno));
  }
#endif /* (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID)) */

  /* Create the netmgr command queue thread as the first operation during bootup
     so that it is spawned with the necessary privileges */
  if (NETMGR_SUCCESS != netmgr_cmdq_init())
  {
    netmgr_log_err("netmgr_cmdq_init(): failed!");
  }

  /* Drop netmgrd privileges to radio */
#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
  /* adjust uid/gid and capabilities */
  if (ds_change_user_cap( AID_RADIO, AID_RADIO,
                          (1ULL << CAP_NET_ADMIN) | (1ULL << CAP_NET_RAW)
#ifdef CAP_BLOCK_SUSPEND
                          | (1ULL << CAP_BLOCK_SUSPEND)
#endif
                        ) != 0)
  {
    netmgr_log_err("couldn't change uid and capabilities at power up");
    exit(EXIT_FAILURE);
  }
#endif /* (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID)) */

  netmgr_cb_init();
  /* Initialize main before anything else as NM core modules can depend on these.
   * This is a special case only and other core modules should initialize their
   * tables in their respective init functions */
  netmgr_main_init_cb();

  /* Initialize the random number generator. This will be used to randomize the IKE port
     in case new marking logic is used */
  srand((unsigned) time(&t));

  if ( TRUE == log_to_file )
  {
    /* Init file pointer to write to device file system  */
    logging_fd = fopen(NETMGR_LOG_FILE_LOCATION, "w+e");
    if (!logging_fd)
    { 
      log_to_file = FALSE;
      perror("failed");
    }
  }

#ifdef FEATURE_DATA_LOG_QXDM
  /* Initialize Diag services */
  if ( TRUE != Diag_LSM_Init(NULL) )
  {
    netmgr_log_err("failed on Diag_LSM_Init\n" );
  }
  else
  {
    atexit(netmgr_diag_cleanup);
  }
#endif
#ifdef FEATURE_DATA_LOG_SYSLOG
  /* Initialize logging as per desired mode */
  netmgr_log_init(netmgr_main_cfg.logthreshold, netmgr_main_cfg.logmode);
#endif

  NETMGR_LOG_FUNC_ENTRY;
  netmgr_l2s_setup();
  ds_log_multicast_init();

  DS_OPEN_TRACE_MARKER;

#ifdef FEATURE_RECOVERY_ENABLED
  /* Initialize recovery file and determine if we are in a restart case */
  netmgr_restart = netmgr_main_recovery_init();
#endif /* FEATURE_RECOVERY_ENABLED */

  /* TODO: Config loading should happen before any other module loads. */
  netmgr_config_init();

  /* Initialize number of active interfaces to default; may be overridden
   * by system property or command-line argument */
  netmgr_main_cfg.nint = NETMGR_MAIN_DEFAULT_NINT;

  /* Parse command line arguments and populate configuration blob */
  netmgr_main_parse_args(argc, argv);

  /* Update links state based on external subsystem usage */
  netmgr_main_process_target();

  netmgr_load_data_format();

  /* Initialize platform layer */
  netmgr_platform_init();
  netmgr_log_med( "platform init completed\n" );

  /* Initialize QMI interface module */
  netmgr_qmi_init( netmgr_main_cfg.nint, netmgr_ctl_port_array, netmgr_main_cfg.data_format);
  netmgr_log_med( "qmi init completed\n" );

  /* Initialize kernel interface module */
  netmgr_kif_init( netmgr_main_cfg.nint,
                   netmgr_ctl_port_array,
                   netmgr_main_cfg.skip,
                   netmgr_main_cfg.dirpath,
                   netmgr_main_cfg.modscr);
  netmgr_log_med( "kif init completed\n" );

  /* Initialize executive module */
  netmgr_exec_init( netmgr_main_cfg.nint, netmgr_ctl_port_array );

  if (netmgr_main_get_rmnet_data_enabled())
  {
     /* This function call is effective only for IPA based targets */
     netmgr_rmnet_configure_ep_params();
     /* Perform DPM port open if DPM is enabled for this target,
       prior to any QMI service initialization */
     if (netmgr_main_get_dpm_enabled())
     {
       netmgr_qmi_dpm_initialize_blocking();
     }
  }
  /* Initialize RmNet extention */
  if (netmgr_main_get_rmnet_data_enabled())
  {
    (void)netmgr_rmnet_ext_init();
  }

  /* Target init - this will block until modem is up */
  if (NETMGR_SUCCESS != netmgr_target_init())
  {
    netmgr_log_err("Failed to initialize target\n");
    exit(EXIT_FAILURE);
  }

  if (netmgr_main_get_rmnet_data_enabled())
  {

    if (netmgr_main_cfg.rtm_rmnet_enabled)
    {
      /* Configure new RTM_NETLINK rmnet driver*/
      if (!netmgr_main_cfg.dynamic_link_creation)
      {
        if (NETMGR_FAILURE == netmgr_rmnet_configure_embedded_links())
        {
          netmgr_l2s_msg_seq(netmgr_l2s_invalid_link,"RMNET_DATA config failed");
        }
        else
        {
          netmgr_l2s_msg_seq(netmgr_l2s_invalid_link,"RMNET_DATA configured");
        }
      }

      if (NETMGR_SUCCESS != netmgr_main_set_ko_loading_properties(netmgr_restart))
      {
        netmgr_log_err("%s(): failed to set properties to trigger "
                       "loading of kernel modules!", __func__);
      }
    }
    else
    {
      if (NETMGR_FAILURE == netmgr_rmnet_configure_embedded_data())
      {
        netmgr_l2s_msg_seq(netmgr_l2s_invalid_link,"RMNET_DATA config failed");
      }
      else
      {
        netmgr_l2s_msg_seq(netmgr_l2s_invalid_link,"RMNET_DATA configured");
      }

    }
  }

  /* Run as a daemon, if requested */
  if( netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_BACK ) {
    netmgr_daemonize();
    netmgr_log_low( "daemonize completed\n" );
    (void)sleep(1);
  }

  /* Register signal handler */
  signal(SIGUSR1, netmgr_signal_handler);
  signal(SIGUSR2, netmgr_signal_handler);
  signal(SIGTERM, netmgr_signal_handler);
  signal(SIGPIPE, SIG_IGN);

  if (!netmgr_main_cfg.rtm_rmnet_enabled)
  {
    netmgr_kif_powerup_init(netmgr_main_cfg.iname);
  }
  else
  {
    /*Skip ioctl powerup init if using netlink driver*/
    netmgr_kif_rtm_rmnet_powerup_init();
  }

#ifdef NETMGR_QOS_ENABLED
  if( netmgr_main_get_qos_enabled() )
  {
    /* Initialize traffic control module */
    netmgr_tc_init( netmgr_main_cfg.nint, netmgr_ctl_port_array, netmgr_restart );
    netmgr_log_low( "tc init completed\n" );
  }
#endif // NETMGR_QOS_ENABLED

#ifdef FEATURE_DS_LINUX_ANDROID
  if (!netmgr_main_cfg.rtm_rmnet_enabled &&
      !netmgr_main_get_qos_hybrid_enabled())
  {
    /* Check to see if qos_header_format is needed. Gets the value of the WDA
     * data format property qos_header_format to be sent to the modem.
     * qos_header_format is applicable only when baseband is not mdm2
     * and netmgr_kif_ifioctl_set_qosmode() is implemented. */
     netmgr_kif_get_qos_header_format((const char *)netmgr_main_get_phys_net_dev(),
                                      NETMGR_RMNET_START,
                                      &(netmgr_main_cfg.data_format));

     /*data format may have been updated, let qmi know*/
     netmgr_qmi_update_data_format_config(netmgr_main_cfg.data_format);
  }
#endif /* FEATURE_DS_LINUX_ANDROID */

  if (netmgr_main_get_low_latency_filters_enabled())
  {
    netmgr_qmi_dfs_enable_low_latency_filters((const char *)netmgr_main_get_phys_net_dev());
  }

  if (!netmgr_main_cfg.rtm_rmnet_enabled &&
      !netmgr_main_get_qos_hybrid_enabled())
  {
    /* Sets the qos_header_format for every virtual network device if
     * applicable. */
     netmgr_kif_set_qos_header_format(NETMGR_RMNET_START,
                                      &(netmgr_main_cfg.data_format));
  }
  netmgr_main_cfg.initialized = TRUE;
  netmgr_log_high("Initialization complete.\n");

#ifdef FEATURE_RECOVERY_ENABLED
  if (netmgr_restart)
  {
    netmgr_main_recovery_cleanup_links();

    if (netmgr_main_cfg.dynamic_link_creation)
    {
      netmgr_kif_vnd_cleanup_then_notify_clients();
    }

    /* Only in restart scenario, we need to clean up
        ssdp iptables rules before we create them later. */
    netmgr_kif_remove_ssdp_rule_on_ifaces();

    /* Clean up the packet priority rules before we add them again */
    netmgr_kif_remove_packet_priority_rules();
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  /* bring up each SM instance in INITED state */
  netmgr_main_sm_inited();

#if defined (FEATURE_DS_LINUX_ANDROID)
  NetmgrNetdClientInit();
#endif /* defined(FEATURE_DS_LINUX_ANDROID) */

  /* Register netmgr plugins. This will ensure that the plugin modules have
     registered for callback events. Once the plugins have registered for
     callback events we can call into them from state machine to facilitate
     any cleanup operations */
  netmgr_main_register_plugins();

  /* Initialize netmgr plugins */
  netmgr_main_initialize_plugins(netmgr_restart);

  netmgr_print_cb_tables();

  netmgr_main_post_is_complete_ev();

  netmgr_qmi_dms_send_info();

  /* Indefinitely wait on command processing thread. */
  netmgr_log_med( "netmgr going into wait loop" );

  netmgr_unix_listener_init();

  netmgr_exec_wait();

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

