/******************************************************************************

                   N M _ M D M P R X Y _ C O M M O N _ . C

******************************************************************************/

/******************************************************************************

  @file    nm_mdmprxy_common.c
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy plugin module is responsible for managing routing rules
  and packet marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2015-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <sys/socket.h>
#include <linux/if.h>

#include "netmgr.h"
#include "netmgr_kif.h"
#include "netmgr_kif_cb.h"

#include "nm_mdmprxy.h"
#include "nm_mdmprxy_iptables_helper.h"
#include "nm_mdmprxy_common.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

typedef struct
{
  boolean is_v4_iptable_rules_installed;
  boolean is_v6_iptable_rules_installed;
  boolean is_v4_iptable_jump_rule_installed;
  boolean is_v6_iptable_jump_rule_installed;
  boolean is_v4_audio_forwarding_rules_installed;
  boolean is_v6_audio_forwarding_rules_installed;
  boolean is_v4_audio_forwarding_rules_install_preference;
  boolean is_v6_audio_forwarding_rules_install_preference;
} nm_mdmprxy_cfg_t;

static nm_mdmprxy_cfg_t global_config;

int iiface_refcount_v4 = 0;
int iiface_refcount_v6 = 0;

#define NM_MDMPRXY_GET_IPTABLES_RULES_INSTALLED(family)                  \
  ((AF_INET == family) ?                                                 \
   global_config.is_v4_iptable_rules_installed :                         \
   global_config.is_v6_iptable_rules_installed)

#define NM_MDMPRXY_SET_IPTABLES_RULES_INSTALLED(family, status)          \
  *(((AF_INET == family) ?                                               \
    &global_config.is_v4_iptable_rules_installed :                       \
    &global_config.is_v6_iptable_rules_installed)) = status

#define NM_MDMPRXY_GET_IPTABLES_JUMP_RULES_INSTALLED(family)             \
  ((AF_INET == family) ?                                                 \
   global_config.is_v4_iptable_jump_rule_installed :                     \
   global_config.is_v6_iptable_jump_rule_installed)

#define NM_MDMPRXY_SET_IPTABLES_JUMP_RULES_INSTALLED(family, status)     \
  *(((AF_INET == family) ?                                               \
    &global_config.is_v4_iptable_jump_rule_installed :                   \
    &global_config.is_v6_iptable_jump_rule_installed)) = status

static const struct nm_mdmprxy_cmd static_rule_list[] =
{
  /* Custom chain inside raw table to host prerouting rules */
  { "%s" NM_MDMPRXY_FLAGS " -t raw -N nm_mdmprxy_raw_pre",
    "%s" NM_MDMPRXY_FLAGS " -t raw -X nm_mdmprxy_raw_pre",
    nm_mdmprxy_basic_formatter },

  /* Custom chain inside mangle table to host postrouting rules */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mngl_post",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_post",
    nm_mdmprxy_basic_formatter },

  /* Custom chain inside mangle table to host spi matching rules*/
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mngl_pre_spi",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_spi",
    nm_mdmprxy_basic_formatter },

  /* Custom chain inside mangle table to host audio port-forwarding rules */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mngl_pre_ex",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_ex",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain which hosts the rules for marking packet with modem mark */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_pkt_marker",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_marker",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain which hosts the rules for forwarding modem marked packets */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_pkt_forwarder",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_forwarder",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain to host ICMP related conntrack rules */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_icmp_pkt_marker",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_icmp_pkt_marker",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain to host either socket marking or modem port-range based rules */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mark_prov_chain",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mark_prov_chain",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain to host TEE rules for local-breakout interface */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mngl_pre_tee",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_tee",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain which hosts rules to forward packets between interfaces
     ex. r_rmnet_dataX and wlan0 */
  { "%s" NM_MDMPRXY_FLAGS " -t filter -N nm_mdmprxy_iface_pkt_fwder",
    "%s" NM_MDMPRXY_FLAGS " -t filter -X nm_mdmprxy_iface_pkt_fwder",
    nm_mdmprxy_basic_formatter },

  /* Iptables chain to host doze mode skip rules */
  { "%s" NM_MDMPRXY_FLAGS " -t filter -N nm_mdmprxy_doze_mode_skip",
    "%s" NM_MDMPRXY_FLAGS " -t filter -X nm_mdmprxy_doze_mode_skip",
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_pkt_skmark",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_skmark",
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -N nm_mdmprxy_mngl_pre",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre",
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t nat -N nm_mdmprxy_masquerade_skip",
    "%s" NM_MDMPRXY_FLAGS " -t nat -X nm_mdmprxy_masquerade_skip",
    nm_mdmprxy_static_rule_masquerade_formatter },

  /* Disable connection tracking for SIP port */
  { "%s" NM_MDMPRXY_FLAGS " -t raw -A nm_mdmprxy_raw_pre -p tcp --match multiport"
                          " --ports " TOSTR(NM_MDMPRXY_IMS_SIP_PORT) " -j NOTRACK",
    "%s" NM_MDMPRXY_FLAGS " -t raw -D nm_mdmprxy_raw_pre -p tcp --match multiport"
                          " --ports " TOSTR(NM_MDMPRXY_IMS_SIP_PORT) " -j NOTRACK",
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t raw -A nm_mdmprxy_raw_pre -p udp --match multiport"
                          " --ports " TOSTR(NM_MDMPRXY_IMS_SIP_PORT) " -j NOTRACK",
    "%s" NM_MDMPRXY_FLAGS " -t raw -D nm_mdmprxy_raw_pre -p udp --match multiport"
                          " --ports " TOSTR(NM_MDMPRXY_IMS_SIP_PORT) " -j NOTRACK",
    nm_mdmprxy_basic_formatter }
};

static const struct nm_mdmprxy_cmd static_jump_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I POSTROUTING -j nm_mdmprxy_mngl_post",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D POSTROUTING -j nm_mdmprxy_mngl_post",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t raw -I PREROUTING -j nm_mdmprxy_raw_pre",
    "%s" NM_MDMPRXY_FLAGS " -t raw -D PREROUTING -j nm_mdmprxy_raw_pre",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I FORWARD -j nm_mdmprxy_iface_pkt_fwder",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D FORWARD -j nm_mdmprxy_iface_pkt_fwder",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I OUTPUT -j nm_mdmprxy_doze_mode_skip",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D OUTPUT -j nm_mdmprxy_doze_mode_skip",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I INPUT -j nm_mdmprxy_doze_mode_skip",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D INPUT -j nm_mdmprxy_doze_mode_skip",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t nat -I POSTROUTING 1 -j nm_mdmprxy_masquerade_skip",
    "%s" NM_MDMPRXY_FLAGS " -t nat -D POSTROUTING 1 -j nm_mdmprxy_masquerade_skip",
    nm_mdmprxy_static_rule_masquerade_formatter
  }
};

static const struct nm_mdmprxy_cmd static_jump_rule_list_netd_restart[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t raw -I PREROUTING -j nm_mdmprxy_raw_pre",
    "%s" NM_MDMPRXY_FLAGS " -t raw -D PREROUTING -j nm_mdmprxy_raw_pre",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I FORWARD -j nm_mdmprxy_iface_pkt_fwder",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D FORWARD -j nm_mdmprxy_iface_pkt_fwder",
    nm_mdmprxy_basic_formatter
  },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I INPUT -j nm_mdmprxy_doze_mode_skip",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D INPUT -j nm_mdmprxy_doze_mode_skip",
    nm_mdmprxy_basic_formatter
  }
};

static const struct nm_mdmprxy_cmd tcp_mss_rule_list[] =
{
  /* Match TCP SYN packets coming from the apps iwlan call
     and clamp the outgoing MSS on that packet. This rule
     is required to ensure incoming packets from a server
     does not exceed the 1140 MSS */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I nm_mdmprxy_mngl_post -s %s -p tcp"
                          " --syn -j TCPMSS --set-mss %d",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_post -s %s -p tcp"
                          " --syn -j TCPMSS --set-mss %d",
    nm_mdmprxy_tcp_mss_formatter },

  /* Modify the MSS in the SYN-ACK response from server.
     This rule is required to ensure outgoing packets from
     UE do not exceed 1140 MSS */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I nm_mdmprxy_mngl_pre -d %s -p tcp"
                          " --tcp-flags SYN,ACK SYN,ACK"
                          " -j TCPMSS --set-mss %d",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_pre -d %s -p tcp"
                          " --tcp-flags SYN,ACK SYN,ACK"
                          " -j TCPMSS --set-mss %d",
    nm_mdmprxy_tcp_mss_formatter }
};

#ifdef FEATURE_RECOVERY_ENABLED
static const struct nm_mdmprxy_cmd cleanup_tables_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t raw -F nm_mdmprxy_raw_pre",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mngl_post",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mngl_pre_spi",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mngl_pre_ex",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_pkt_marker",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_pkt_forwarder",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_icmp_pkt_marker",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mark_prov_chain",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_pkt_skmark",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mngl_pre_tee",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -F nm_mdmprxy_doze_mode_skip",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -F nm_mdmprxy_iface_pkt_fwder",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -F nm_mdmprxy_mngl_pre",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t raw -X nm_mdmprxy_raw_pre",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_post",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_spi",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_ex",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_marker",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_forwarder",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_icmp_pkt_marker",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mark_prov_chain",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_pkt_skmark",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre_tee",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -X nm_mdmprxy_doze_mode_skip",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -X nm_mdmprxy_iface_pkt_fwder",
    NULL,
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -X nm_mdmprxy_mngl_pre",
    NULL,
    nm_mdmprxy_basic_formatter }
};

static char errors_to_ignored[][NM_MDMPRXY_MAXCMDSTR] =
{
  "RTNETLINK answers: No such file or directory",
  "iptables: No chain/target/match by that name."
};

#define IGNORED_ERRORS_LEN ((int) (sizeof(errors_to_ignored)/sizeof(errors_to_ignored[0])))

#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
                           LOCAL DEFINITIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  nm_mdmprxy_common_uninstall_static_rules
===========================================================================*/
/*!
@brief
  Uninstall static iptables rules

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_mdmprxy_common_uninstall_static_rules(sa_family_t af, int start)
{
  int i = 0;
  int rc = NETMGR_SUCCESS;

  if (FALSE == NM_MDMPRXY_GET_IPTABLES_RULES_INSTALLED(af))
  {
    netmgr_log_med("%s(): static rules already uninstalled for ip_family [%d]",
                   __func__, af);
    rc = NETMGR_SUCCESS;
    goto bail;
  }

  if (start >= nm_mdmprxy_rules_len(static_rule_list))
  {
    goto bail;
  }

  for (i = start; i >= 0; i--)
  {
    rc |= nm_mdmprxy_process_rule(&static_rule_list[i], NM_MDMPRXY_RUN_ANTIRULE, af, NULL);
  }

bail:
  netmgr_log_low("%s(): static rule uninstalled for ip_family [%d]", __func__, af);
  NM_MDMPRXY_SET_IPTABLES_RULES_INSTALLED(af, FALSE);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_install_static_rules
===========================================================================*/
/*!
@brief
  Install static iptables rules

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int nm_mdmprxy_common_install_static_rules(sa_family_t af)
{
  int i = 0;
  int rc = NETMGR_FAILURE;

  if (TRUE == NM_MDMPRXY_GET_IPTABLES_RULES_INSTALLED(af))
  {
    netmgr_log_med("%s(): static rules already installed for ip_family [%d]",
                   __func__, af);
    return NETMGR_SUCCESS;
  }

  for (i = 0; i < (int) nm_mdmprxy_rules_len(static_rule_list); i++)
  {
    if (!static_rule_list[i].rule)
    {
      continue;
    }

    rc = nm_mdmprxy_process_rule(&static_rule_list[i], NM_MDMPRXY_RUN_RULE, af, NULL);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply rule [%d]", __func__, i);
      goto bail;
    }
  }

  netmgr_log_low("%s(): static rules installation complete for ip_family [%d]", __func__, af);
  NM_MDMPRXY_SET_IPTABLES_RULES_INSTALLED(af, TRUE);

  rc = NETMGR_SUCCESS;
  return rc;

bail:
  nm_mdmprxy_common_uninstall_static_rules(af, i);
  return rc;
}

/*===========================================================================
  FUNCTION  _nm_mdmprxy_common_uninstall_jump_rules
===========================================================================*/
/*!
@brief
  In the main iptables chains, remove jump rule to custom chains

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static int _nm_mdmprxy_common_uninstall_jump_rules
(
  sa_family_t  af,
  int          start,
  boolean      force
)
{
  int i = 0;
  int rc = NETMGR_SUCCESS;

  /* We need to remove the jump rules only when the last iWLAN call goes down.
     We also need to ensure that once the rules are removed we don't attempt to
     remove them again */

  /* If this is an SSR scenario, we need to forcefully remove all the jump rules so
     that the iptables chains can be cleaned up properly. 'force' will be set to TRUE
     for SSR */
  if (AF_INET == af)
  {
    if (force)
    {
      /* Reset the refcount to one so that rules can be cleaned up properly */
      iiface_refcount_v4 = 1;
    }

    if (iiface_refcount_v4 == 0)
    {
      netmgr_log_med("%s(): jump rules already removed for ip_family [%d]",
                   __func__, af);
      return NETMGR_SUCCESS;
    }
    else if (iiface_refcount_v4 > 1)
    {
      iiface_refcount_v4--;
      return NETMGR_SUCCESS;
    }
  }
  else if (AF_INET6 == af)
  {
    if (force)
    {
      /* Reset the refcount to one so that rules can be cleaned up properly */
      iiface_refcount_v6 = 1;
    }

    if (iiface_refcount_v6 == 0)
    {
      netmgr_log_med("%s(): jump rules already removed for ip_family [%d]",
                   __func__, af);
      return NETMGR_SUCCESS;
    }
    else if (iiface_refcount_v6 > 1)
    {
      iiface_refcount_v6--;
      return NETMGR_SUCCESS;
    }
  }
  else
  {
    netmgr_log_err("%s(): Bad AF", __func__);
    return NETMGR_FAILURE;
  }

  if (start >= (int) nm_mdmprxy_rules_len(static_jump_rule_list))
  {
    netmgr_log_err("%s(): Invalid input!", __func__);
    return NETMGR_FAILURE;
  }

  for (i = start; i >= 0; i--)
  {
    rc |= nm_mdmprxy_process_rule(&static_jump_rule_list[i], NM_MDMPRXY_RUN_ANTIRULE, af, NULL);
  }

  netmgr_log_low("%s(): jump rules removed for ip_family [%d]", __func__, af);
  NM_MDMPRXY_SET_IPTABLES_JUMP_RULES_INSTALLED(af, FALSE);

  if (AF_INET == af)
  {
    if (iiface_refcount_v4 > 0)
    {
      iiface_refcount_v4--;
    }
  }
  else
  {
    if (iiface_refcount_v6 > 0)
    {
      iiface_refcount_v6--;
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  _nm_mdmprxy_common_clear_tcp_mss
===========================================================================*/
/*!
@brief
  Remove iptables TCP MSS clamping rule.

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
static int _nm_mdmprxy_common_clear_tcp_mss
(
  int                           af,
  const char                    *ifname,
  struct netmgr_kif_addrstorage *addr,
  int                           start
)
{
  struct tcp_mss_rule_info nfo;
  int rc = NETMGR_SUCCESS;
  int i = 0;

  if (!addr || !ifname)
  {
    rc = NETMGR_FAILURE;
    goto bail;
  }

  if (AF_INET != af && AF_INET6 != af)
  {
    rc = NETMGR_FAILURE;
    goto bail;
  }

  if (start >= (int) nm_mdmprxy_rules_len(tcp_mss_rule_list))
  {
    netmgr_log_err("%s(): invalid input!", __func__);
    rc = NETMGR_FAILURE;
    goto bail;
  }

  memset(&nfo, 0, sizeof(nfo));
  for(i = start; i >= 0; i--)
  {
    if (!tcp_mss_rule_list[i].antirule)
    {
      continue;
    }

    nfo.mss   = NM_MDMPRXY_TCP_MSS;

    if (AF_INET == af)
    {
      nfo.saddr.saddr4.s_addr = addr->_in4.addr;
      nfo.mask = addr->_in4.mask;
    }
    else
    {
      memcpy(&(nfo.saddr.saddr6), &addr->_in6.addr, sizeof(nfo.saddr.saddr6));
      nfo.mask = addr->_in6.mask;
    }

    rc |= nm_mdmprxy_process_rule(&tcp_mss_rule_list[i], NM_MDMPRXY_RUN_ANTIRULE, af, &nfo);
  }

  if (NETMGR_SUCCESS == rc)
  {
    netmgr_log_med("%s(): Finished removing tcp mss clamp rule for af [%d] link [%s]",
                   __func__, af, ifname);
  }

bail:
  return rc;
}

/*===========================================================================
                            PUBLIC FUNCTIONS
===========================================================================*/
#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_common_check_error
===========================================================================*/
/*!
@brief
  Check whether the error can be ignored

@return
  NETMGR_SUCCESS if the error can be ignored
  NETMGR_FAILURE if the error cannot be ignored
*/
/*=========================================================================*/
int nm_mdmprxy_common_check_error(char *error)
{
  int i = 0;

  if (!error)
  {
    return NETMGR_FAILURE;
  }

  for (i = 0; i < IGNORED_ERRORS_LEN; i++)
  {
    if (!strstr(error, errors_to_ignored[i]))
    {
      return NETMGR_SUCCESS;
    }
  }

  return NETMGR_FAILURE;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_uninstall_jump_rules
===========================================================================*/
/*!
@brief
  In the main iptables chains, remove jump rule to custom chains

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_uninstall_jump_rules
(
  sa_family_t  af
)
{
  int rc = NETMGR_SUCCESS;
  int num_rules = nm_mdmprxy_rules_len(static_jump_rule_list);

  rc = _nm_mdmprxy_common_uninstall_jump_rules(af, (num_rules - 1),
                                               FALSE);

  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_install_jump_rules
===========================================================================*/
/*!
@brief
  In the main iptables chains, install jump rule to custom chains

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_install_jump_rules(sa_family_t af)
{
  int i = 0;
  int rc = NETMGR_FAILURE;

  /* Install jump rules only for the first iWLAN call of the type
     Do not attempt to re-install rules for subsequent calls */
  if (AF_INET == af)
  {
    if (iiface_refcount_v4 > 0)
    {
      netmgr_log_med("%s(): jump rules already installed for ip_family [%d]",
                   __func__, af);
      iiface_refcount_v4++;
      return NETMGR_SUCCESS;
    }
  }
  else if (AF_INET6 == af)
  {
    if (iiface_refcount_v6 > 0)
    {
      netmgr_log_med("%s(): jump rules already installed for ip_family [%d]",
                   __func__, af);
      iiface_refcount_v6++;
      return NETMGR_SUCCESS;
    }
  }
  else
  {
    netmgr_log_err("%s(): Bad AF", __func__);
    return NETMGR_FAILURE;
  }

  for (i = 0; i < (int) nm_mdmprxy_rules_len(static_jump_rule_list); i++)
  {
    if (!static_jump_rule_list[i].rule)
    {
      continue;
    }

    rc = nm_mdmprxy_process_rule(&static_jump_rule_list[i], NM_MDMPRXY_RUN_RULE, af, NULL);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply rule [%d]", __func__, i);
      goto bail;
    }
  }

  netmgr_log_low("%s(): jump rules installation complete for ip_family [%d]", __func__, af);

  if (AF_INET == af)
  {
    iiface_refcount_v4++;
  }
  else
  {
    iiface_refcount_v6++;
  }

  rc = NETMGR_SUCCESS;
  return rc;

bail:
  _nm_mdmprxy_common_uninstall_jump_rules(af, i, FALSE);
  return rc;
}


/*===========================================================================
  FUNCTION  nm_mdmprxy_common_install_jump_rules_netd_restart
===========================================================================*/
/*!
@brief
  In the main iptables chains, install jump rule to custom chains
  on netd restart

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_install_jump_rules_netd_restart(sa_family_t af)
{
  int i = 0;
  int rc = NETMGR_FAILURE;

  /* Install jump rules only for the first iWLAN call of the type
     Do not attempt to re-install rules for subsequent calls */

  for (i = 0; i < (int) nm_mdmprxy_rules_len(static_jump_rule_list_netd_restart); i++)
  {
    if (!static_jump_rule_list_netd_restart[i].rule)
    {
      continue;
    }

    rc = nm_mdmprxy_process_rule(&static_jump_rule_list_netd_restart[i],
                                 NM_MDMPRXY_RUN_RULE,
                                 af, NULL);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply rule [%d]", __func__, i);
      goto bail;
    }
  }

  netmgr_log_low("%s(): netd restart jump rules installation "
                 "complete for ip_family [%d]", __func__, af);

  rc = NETMGR_SUCCESS;
  return rc;

bail:
  //_nm_mdmprxy_common_uninstall_jump_rules(af, i, FALSE);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_clear_tcp_mss
===========================================================================*/
/*!
@brief
  Remove iptables TCP MSS clamping rule.

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_clear_tcp_mss
(
  int                           af,
  int                           tech_type,
  const char                    *ifname,
  struct netmgr_kif_addrstorage *addr
)
{
  int rc = NETMGR_SUCCESS;
  int num_rules = nm_mdmprxy_rules_len(tcp_mss_rule_list);

  if (!addr)
  {
    rc = NETMGR_FAILURE;
    goto bail;
  }

  if (NETMGR_KIF_CB_IF_TECH_TYPE_LBO == tech_type)
  {
    /* Nothing to do for local breakout */
    goto bail;
  }

  rc = _nm_mdmprxy_common_clear_tcp_mss(af, ifname, addr, (num_rules - 1));

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_set_tcp_mss
===========================================================================*/
/*!
@brief
  Install iptables rule to set the TCP MSS to NM_MDMPRXY_TCP_MSS to avoid
  fragmentation of incoming ESP tunneled packets as such fragments might
  be discarded on some networks due to security concerns.

@return
  NETMGR_SUCCESS
  NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_common_set_tcp_mss
(
  int                           af,
  int                           tech_type,
  const char                    *ifname,
  struct netmgr_kif_addrstorage *addr
)
{
  struct tcp_mss_rule_info nfo;
  int rc = NETMGR_SUCCESS;
  int i = 0;

  if (NETMGR_KIF_CB_IF_TECH_TYPE_LBO == tech_type)
  {
    /* Nothing to do for local breakout */
    return NETMGR_SUCCESS;
  }

  if (!addr || !ifname)
  {
    rc = NETMGR_FAILURE;
    goto bail;
  }

  if (AF_INET != af && AF_INET6 != af)
  {
    rc = NETMGR_FAILURE;
    goto bail;
  }

  memset(&nfo, 0, sizeof(nfo));

  for ( i = 0; i < nm_mdmprxy_rules_len(tcp_mss_rule_list); i++)
  {
    if (!tcp_mss_rule_list[i].rule)
    {
      continue;
    }

    nfo.mss   = NM_MDMPRXY_TCP_MSS;

    if (AF_INET == af)
    {
      nfo.saddr.saddr4.s_addr = addr->_in4.addr;
      nfo.mask = addr->_in4.mask;
    }
    else
    {
      memcpy(&(nfo.saddr.saddr6), &addr->_in6.addr, sizeof(nfo.saddr.saddr6));
      nfo.mask = addr->_in6.mask;
    }

    rc = nm_mdmprxy_process_rule(&tcp_mss_rule_list[i], NM_MDMPRXY_RUN_RULE, af, &nfo);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply rule [%d], af [%d]", __func__, i, af);
      goto bail;
    }
  }

  netmgr_log_med("%s(): Finished installing tcp  mss clamp rule for af [%d] link [%s]",
                 __func__, af, ifname);
  rc = NETMGR_SUCCESS;
  return rc;

bail:
  _nm_mdmprxy_common_clear_tcp_mss(af, ifname, addr, i);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_oos_hdlr
===========================================================================*/
/*!
@brief
  Handler for modem OOS event

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_oos_hdlr(boolean force)
{
  int rc = NETMGR_SUCCESS;
  int num_rules = nm_mdmprxy_rules_len(static_jump_rule_list);

  /* Remove jump rules, if any */
  rc |= _nm_mdmprxy_common_uninstall_jump_rules(AF_INET, num_rules - 1,
                                                force);
  rc |= _nm_mdmprxy_common_uninstall_jump_rules(AF_INET6, num_rules - 1,
                                                force);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_init
===========================================================================*/
/*!
@brief
  Handler for modem IS event

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_init(void* arg)
{
  int rc = NETMGR_SUCCESS;

  NM_MDMPRXY_VAR_UNUSED(arg);

  iiface_refcount_v4 = 0;
  iiface_refcount_v6 = 0;

  memset(&global_config, 0, sizeof(nm_mdmprxy_cfg_t));
  rc = nm_mdmprxy_common_install_static_rules(AF_INET);
  rc |= nm_mdmprxy_common_install_static_rules(AF_INET6);

  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_common_deinit
===========================================================================*/
/*!
@brief
  Cleanup function

@return
  NETMGR_SUCCESS on successful command execution
  NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int nm_mdmprxy_common_deinit(int restart)
{
#if defined(FEATURE_RECOVERY_ENABLED)
  int i = 0;
  int num_rules = 0;
  int rc = NETMGR_SUCCESS;
  char result[NM_MDMPRXY_MAXCMDSTR] = "";

  if (restart)
  {
    iiface_refcount_v4 = 0;
    iiface_refcount_v6 = 0;
  }

  /* Remove IPv4 rules from top level chains */
  num_rules = nm_mdmprxy_rules_len(static_jump_rule_list);
  for (i = 0; i < num_rules; i++)
  {
    if (!static_jump_rule_list[i].rule)
    {
      continue;
    }

    do
    {
      /* Remove top level rules from IPv4 chains. In the event where more than one top level
         rule got added due to multiple rounds of daemon crash and restart we need to delete
         each of those rules one-by-one till the state is clean. */
      rc = nm_mdmprxy_process_rule_return_result(&static_jump_rule_list[i],
                                                 NM_MDMPRXY_RUN_ANTIRULE,
                                                 AF_INET,
                                                 NULL,
                                                 result,
                                                 sizeof(result));
    } while (NETMGR_SUCCESS == rc);

    if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
    {
      netmgr_log_med("%s(): IPv4 static jump rules removed!", __func__);
    }
    else
    {
      netmgr_log_err("%s(): command failed with err: %s", __func__, result);
    }

    do
    {
      /* Remove top level rules from IPv6 chains. In the event where more than one top level
         rule got added due to multiple rounds of daemon crash and restart we need to delete
         each of those rules one-by-one till the state is clean. */
      rc = nm_mdmprxy_process_rule_return_result(&static_jump_rule_list[i],
                                                 NM_MDMPRXY_RUN_ANTIRULE,
                                                 AF_INET6,
                                                 NULL,
                                                 result,
                                                 sizeof(result));
    } while (NETMGR_SUCCESS == rc);

    if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
    {
      netmgr_log_med("%s(): IPv6 static jump rules removed!", __func__);
    }
    else
    {
      netmgr_log_err("%s(): command failed with err: %s", __func__, result);
    }
  }

  num_rules = nm_mdmprxy_rules_len(cleanup_tables_rule_list);
  for (i = 0; i < num_rules; i++)
  {
    if (!cleanup_tables_rule_list[i].rule)
    {
      continue;
    }

    /* Flush all the IPv4 custom chains */
    rc = nm_mdmprxy_process_rule(&cleanup_tables_rule_list[i], NM_MDMPRXY_RUN_RULE, AF_INET, NULL);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply IPv4 flush rule [%d]", __func__, i);
    }

    /* Flush all the IPv6 custom chains */
    rc = nm_mdmprxy_process_rule(&cleanup_tables_rule_list[i], NM_MDMPRXY_RUN_RULE, AF_INET6, NULL);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): Failed to apply IPv6 flush rule [%d]", __func__, i);
    }
  }

  netmgr_log_low("%s(): cleanup complete!", __func__);
  return NETMGR_SUCCESS;

#else
  (void) restart;
  return NETMGR_SUCCESS;

#endif /* FEATURE_RECOVERY_ENABLED */
}
