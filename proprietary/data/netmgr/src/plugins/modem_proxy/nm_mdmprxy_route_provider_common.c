/******************************************************************************

    N M _ M D M P R X Y _ R O U T E _ P R O V I D E R _ C O M M O N . C

******************************************************************************/

/******************************************************************************

  @file    nm_mdmproxy_route_provider_common.c
  @brief   Modem proxy plugin

  DESCRIPTION
  Modem proxy module is responsible for managing routing rules and packet
  marking rules for forwarding packets to modem.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <sys/socket.h> /* Needs to be removed */
#include <linux/if.h>
#include <string.h>

#include "netmgr.h"
#include "netmgr_kif.h"

#include "nm_mdmprxy.h"
#include "nm_mdmprxy_mark_provider_factory.h"
#include "nm_mdmprxy_route_provider_factory.h"
#include "nm_mdmprxy_common.h"
#include "nm_mdmprxy_route_provider_common.h"
#include "netmgr_netd_client.h"

/* Definition of shared variables among ndc and hal route providers */
int iiface_refcount = 0;
int iiface_handle = -1;

static nm_mdmprxy_mark_prov_t *mark_prov_inst;

/*===========================================================================
                             LOCAL DECLARATIONS
===========================================================================*/
nm_mdmprxy_cfg_t nm_mdmprxy_global_link_status[NM_MDMPRXY_MAXDEVSTR];

/* Commands which will be installed at powerup */
static const struct nm_mdmprxy_cmd static_rule_list[] =
{
  {
    "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_pkt_marker -m mark --mark 0x%x"
    " -j nm_mdmprxy_pkt_forwarder",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_pkt_marker -m mark --mark 0x%x"
    " -j nm_mdmprxy_pkt_forwarder",
    nm_mdmprxy_markjump_formatter },

  /* Packets arriving on the transparent socket are meant for modem consumption.
     If we receive any non-modem packets on the transparent sockets with mark
     set to the packet mark from netd we need to drop the packet. Such packets
     are not expected to come on transparent sockets and if they do they should
     be considered spurious */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_pkt_marker ! -p %s -m mark --mark 0x%x -j DROP",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_pkt_marker ! -p %s -m mark --mark 0x%x -j DROP",
    nm_mdmprxy_mark_drop_formatter },

  /* Remove marking from iWLAN marked packets to prevent confusion with QoS marking */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_mngl_post -m mark --mark 0x%x"
                          " -j MARK --set-mark 0x0",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_post -m mark --mark 0x%x"
                          " -j MARK --set-mark 0x0",
    nm_mdmprxy_markjump_formatter }
};

static const struct nm_mdmprxy_cmd per_iiface_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I PREROUTING -i %s -j nm_mdmprxy_pkt_marker",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -i %s -j nm_mdmprxy_pkt_marker",
    nm_mdmprxy_ifacejump_formatter },

  /* nm_mdmprxy_icmp_pkt_marker chain hosts marking rules for icmp ping packets
     originating from the modem. The mark provider will install the marking rules. The
     route provider just needs to install the jump rule in PREROUTING chain to the
     nm_mdmprxy_icmp_pkt_marker chain */
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A PREROUTING -j nm_mdmprxy_icmp_pkt_marker",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -j nm_mdmprxy_icmp_pkt_marker",
    nm_mdmprxy_basic_formatter }
};

static const struct nm_mdmprxy_cmd per_oiface_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I nm_mdmprxy_pkt_forwarder -d %s"
                          " -j TEE --gateway %s --oif %s",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_pkt_forwarder -d %s"
                          " -j TEE --gateway %s --oif %s",
    nm_mdmprxy_tee_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_pkt_forwarder -d %s -j DROP",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_pkt_forwarder -d %s -j DROP",
    nm_mdmprxy_dst_drop_formatter }
};

static const struct nm_mdmprxy_cmd per_iface_forward_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t filter -I nm_mdmprxy_iface_pkt_fwder -i %s -o %s -j ACCEPT",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D nm_mdmprxy_iface_pkt_fwder -i %s -o %s -j ACCEPT",
    nm_mdmprxy_ifaceforward_incoming_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t filter -I nm_mdmprxy_iface_pkt_fwder -i %s -o %s -j ACCEPT",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D nm_mdmprxy_iface_pkt_fwder -i %s -o %s -j ACCEPT",
    nm_mdmprxy_ifaceforward_outgoing_formatter },
};

/* It is possible to have wlan0 address on the UE and DNS address on the epdg server to share
   the same subnet. In such situations packets originating on from the modem will be routed
   up the AP stack since the subnets are same. To prevent such situations we will TEE the
   incoming packet from the modem arriving from local-breakout interface and force the packet
   over wlan0 using a dummy gateway */
static const struct nm_mdmprxy_cmd local_breakout_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A PREROUTING -p tcp -i %s -j nm_mdmprxy_mngl_pre_tee",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -p tcp -i %s -j nm_mdmprxy_mngl_pre_tee",
    nm_mdmprxy_lbo_ifacejump_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A PREROUTING -p udp -i %s -j nm_mdmprxy_mngl_pre_tee",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -p udp -i %s -j nm_mdmprxy_mngl_pre_tee",
    nm_mdmprxy_lbo_ifacejump_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -I nm_mdmprxy_mngl_pre_tee -j MARK --set-mark " TOSTR(NM_MDMPRXY_LBO_MARK),
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_pre_tee -j MARK --set-mark " TOSTR(NM_MDMPRXY_LBO_MARK),
    nm_mdmprxy_basic_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_mngl_pre_tee -j TEE --gateway %s --oif %s",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_pre_tee -j TEE --gateway %s --oif %s",
    nm_mdmprxy_lbo_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -A nm_mdmprxy_mngl_pre_tee -j DROP",
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_pre_tee -j DROP",
    nm_mdmprxy_basic_formatter }
};

static const struct nm_mdmprxy_cmd local_breakout_doze_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t filter -I nm_mdmprxy_doze_mode_skip -m mark --mark 0x%x -j ACCEPT",
    "%s" NM_MDMPRXY_FLAGS " -t filter -D nm_mdmprxy_doze_mode_skip -m mark --mark 0x%x -j ACCEPT",
    nm_mdmprxy_doze_lbo_formatter }
};

static const struct nm_mdmprxy_cmd vpn_skip_rule_list[] =
{
  { "%s add from %s to %s iif lo fwmark 0x0/0x20000 goto 13000 prio 11999",
    "%s del from %s to %s iif lo fwmark 0x0/0x20000 goto 13000 prio 11999",
    nm_mdmprxy_vpn_skip_formatter }
};

static const struct nm_mdmprxy_cmd masquerade_skip_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t nat -I nm_mdmprxy_masquerade_skip -s %s -j ACCEPT",
    "%s" NM_MDMPRXY_FLAGS " -t nat -D nm_mdmprxy_masquerade_skip -s %s -j ACCEPT",
    nm_mdmprxy_masquerade_skip_formatter }
};

#ifdef FEATURE_RECOVERY_ENABLED
static const struct nm_mdmprxy_cmd remove_lbo_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -p tcp -i %s -j nm_mdmprxy_mngl_pre_tee",
    NULL,
    nm_mdmprxy_lbo_ifacejump_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -p udp -i %s -j nm_mdmprxy_mngl_pre_tee",
    NULL,
    nm_mdmprxy_lbo_ifacejump_formatter }
};

static const struct nm_mdmprxy_cmd remove_per_iiface_rule_list[] =
{
  { "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -i %s -j nm_mdmprxy_pkt_marker",
    NULL,
    nm_mdmprxy_ifacejump_formatter },

  { "%s" NM_MDMPRXY_FLAGS " -t mangle -D PREROUTING -j nm_mdmprxy_icmp_pkt_marker",
    NULL,
    nm_mdmprxy_basic_formatter }
};
#endif /* FEATURE_RECOVERY_ENABLED */

/* These rules are almost similar to the static rules list except for
   the rule 'nm_mdmprxy_pkt_forwarder'. The jump to
   'nm_mdmprxy_pkt_forwarder' chain based on packet mark rule which is
   setup by route_provider will be flushed when 'nm_mdmprxy_pkt_marker'
   chain is flushed as part of netd OOS handling

   Ideally we would like for the specific sub-modules to remove their
   corresponding rules however iptables operations are latency heavy
   so we need to avoid running a lot of rules. Flushing the
   nm_mdmprxy_pkt_marker rules in mark provider for netd OOS is
   faster than removing individual rules */
static const struct nm_mdmprxy_cmd remove_static_rules_netd_oos[] =
{
  /* Remove marking from iWLAN marked packets to prevent confusion with QoS marking */
  { NULL,
    "%s" NM_MDMPRXY_FLAGS " -t mangle -D nm_mdmprxy_mngl_post -m mark --mark 0x%x"
                          " -j MARK --set-mark 0x0",
    nm_mdmprxy_markjump_formatter }
};

struct route_prov_iptables route_prov_common_iptables_cmd_tbl;

/* Cache oem value */
static uint32_t nm_mdmprxy_route_oem_mark = 0;

/*===========================================================================
                           LOCAL DEFINITIONS
===========================================================================*/
#ifndef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_install_ipsec_tunnel_rules
===========================================================================*/
/*!
@brief
  Installs rules specific to the IPSec tunnel parameters

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int
nm_mdmprxy_route_prov_common_install_ipsec_tunnel_rules
(
  char* link_name,
  ipsec_tunnel_info_t *tunnel
)
{
  int i = 0;
  int rc = NETMGR_FAILURE;

  if (!link_name || !tunnel || !(tunnel->dest_addr) || !(tunnel->local_addr))
  {
    netmgr_log_err("%s(): invalid params!", __func__);
    goto bail;
  }

  for (i = 0; i < MAX_IPSEC_TUNNELS; i++)
  {
    if (!tunnel_rules[i])
    {
      continue;
    }

    if (!strcmp(tunnel_rules[i]->tunnel_info.local_addr, tunnel->local_addr)
          && !strcmp(tunnel_rules[i]->tunnel_info.dest_addr, tunnel->dest_addr)
          && tunnel_rules[i]->tunnel_info.tunnel_family == tunnel->tunnel_family)
    {
      /* The tunnel doze rules for this specific tunnel are already installed */
      tunnel_rules[i]->ref_count++;
      rc = NETMGR_SUCCESS;
      goto bail;
    }
  }

  /* If we reach this point it means that the rules for the tunnel are not yet setup.
     We need to install the VPN rules for the new tunnel */
  if (NETMGR_SUCCESS != nm_mdmprxy_install_vpn_skip_rule(link_name,
                                                         NETMGR_KIF_CB_IF_TECH_TYPE_EPDG,
                                                         tunnel))
  {
    netmgr_log_err("%s(): Failed to install vpn skip rule for link=%s!",
                   __func__, link_name);
    goto bail;
  }
  else
  {
    netmgr_log_med("%s(): Installed IP[%d] vpn skip rule for link=%s!",
                   __func__, tunnel->tunnel_family, link_name);
  }

  /* Add this tunnel information to the global list */
  for (i = 0; i < MAX_IPSEC_TUNNELS; i++)
  {
    if (!tunnel_rules[i])
    {
      break;
    }
  }

  if (MAX_IPSEC_TUNNELS == i)
  {
    netmgr_log_err("%s(): cannot add any more entries!", __func__);
    goto bail;
  }

  tunnel_rules[i] = (nm_mdmprxy_tunnel_rule_t*) malloc(sizeof(nm_mdmprxy_tunnel_rule_t));
  if (!tunnel_rules[i])
  {
    netmgr_log_err("%s(): failed to allocate memory!", __func__);
    goto bail;
  }

  tunnel_rules[i]->ref_count = 1;
  memcpy(&(tunnel_rules[i]->tunnel_info), tunnel, sizeof(ipsec_tunnel_info_t));

  rc = NETMGR_SUCCESS;

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_remove_ipsec_tunnel_rules
===========================================================================*/
/*!
@brief
  Removes rules specific to the IPSec tunnel parameters

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int
nm_mdmprxy_route_prov_common_remove_ipsec_tunnel_rules
(
  char* link_name,
  ipsec_tunnel_info_t *tunnel
)
{
  int i = 0;
  int rc = NETMGR_FAILURE;

  if (!link_name || !tunnel || !(tunnel->dest_addr) || !(tunnel->local_addr))
  {
    netmgr_log_err("%s(): invalid params!", __func__);
    goto bail;
  }

  for (i = 0; i < MAX_IPSEC_TUNNELS; i++)
  {
    if (!tunnel_rules[i])
    {
      continue;
    }

    if (!strcmp(tunnel_rules[i]->tunnel_info.local_addr, tunnel->local_addr)
          && !strcmp(tunnel_rules[i]->tunnel_info.dest_addr, tunnel->dest_addr)
          && tunnel_rules[i]->tunnel_info.tunnel_family == tunnel->tunnel_family)
    {
      /* Found the matching tunnel rule to uninstall */
      tunnel_rules[i]->ref_count--;
      break;
    }
  }

  if (MAX_IPSEC_TUNNELS == i)
  {
    /* If we cannot find the rule to delete, then there is nothing we can do so we return SUCCESS */
    netmgr_log_err("%s(): could not find matching tunnel rule in local store!", __func__);
    rc = NETMGR_SUCCESS;
    goto bail;
  }

  if (tunnel_rules[i]->ref_count == 0)
  {
    if (NETMGR_SUCCESS != nm_mdmprxy_remove_vpn_skip_rule(link_name,
                                                          NETMGR_KIF_CB_IF_TECH_TYPE_EPDG,
                                                          tunnel))
    {
      netmgr_log_err("%s(): Failed to remove vpn skip rule for link=%s!",
                     __func__, link_name);
      goto bail;
    }
    else
    {
      netmgr_log_med("%s(): Removed IP[%d] vpn skip rule for link=%s!",
                     __func__, tunnel->tunnel_family, link_name);
    }

    if (tunnel_rules[i])
    {
      free(tunnel_rules[i]);
      tunnel_rules[i] = NULL;
    }
  }

  rc = NETMGR_SUCCESS;

bail:
  return rc;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_uninstall_static_rules
===========================================================================*/
/*!
@brief
  Uninstall static rules for this module

@param af   - Address family
@param mark - Packet mark
@param idx  - Index in the rule list from where we need to start uninstalling
       If everything needs to be un-installed we can pass the entire list
       length

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_route_prov_common_uninstall_static_rules
(
  sa_family_t af,
  uint32_t    mark,
  int         idx
)
{
  int rc = NETMGR_FAILURE;
  int i = 0;
  int rules_len = nm_mdmprxy_rules_len(static_rule_list);
  struct nm_mdmprxy_iiface_rule_info nfo;
  size_t len;

  if (AF_INET != af && AF_INET6 != af)
  {
    netmgr_log_err("%s(): invalid address family!", __func__);
    return NETMGR_FAILURE;
  }

  if (idx > rules_len)
  {
    netmgr_log_err("%s(): starting index [%d] for un-installation cannot be greater"
                   " than rules_len [%d]", __func__, idx, rules_len);
    return NETMGR_FAILURE;
  }

  memset(&nfo, 0, sizeof(nfo));

  if (AF_INET == af)
  {
    len = strlcpy(nfo.proto, NM_MDMPRXY_ICMP_PROTO, NM_MDMPRXY_MAXPROTOSTR);
  }
  else
  {
    len = strlcpy(nfo.proto, NM_MDMPRXY_ICMP6_PROTO, NM_MDMPRXY_MAXPROTOSTR);
  }

  if (len >= NM_MDMPRXY_MAXPROTOSTR)
  {
    netmgr_log_err("%s(): truncation occurred while trying to copy"
                   " data into 'proto' field", __func__);
    return NETMGR_FAILURE;
  }

  nfo.mark = mark;

  for (i = idx; i >= 0; i--)
  {
    rc |= nm_mdmprxy_process_rule(&static_rule_list[i],
                                  NM_MDMPRXY_RUN_ANTIRULE, af, &nfo);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s() failed to apply rule [%d]", __func__, i);
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_install_static_rules
===========================================================================*/
/*!
@brief
  Install static rules for this module

@param af   - Address family
@param mark - Packet mark

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
static int nm_mdmprxy_route_prov_common_install_static_rules
(
  sa_family_t af,
  uint32_t    mark
)
{
  int rc = NETMGR_FAILURE;
  unsigned int i = 0;
  unsigned int rules_len = nm_mdmprxy_rules_len(static_rule_list);
  struct nm_mdmprxy_iiface_rule_info nfo;
  size_t len;

  if (AF_INET != af && AF_INET6 != af)
  {
    netmgr_log_err("%s(): invalid address family!", __func__);
    return NETMGR_FAILURE;
  }

  memset(&nfo, 0, sizeof(nfo));

  if (AF_INET == af)
  {
    len = strlcpy(nfo.proto, NM_MDMPRXY_ICMP_PROTO, NM_MDMPRXY_MAXPROTOSTR);
  }
  else
  {
    len = strlcpy(nfo.proto, NM_MDMPRXY_ICMP6_PROTO, NM_MDMPRXY_MAXPROTOSTR);
  }

  if (len >= NM_MDMPRXY_MAXPROTOSTR)
  {
    netmgr_log_err("%s(): truncation occurred while trying to copy"
                   " data into 'proto' field", __func__);
    return NETMGR_FAILURE;
  }

  nfo.mark = mark;

  for (i = 0; i < rules_len; i++)
  {
    if (!static_rule_list[i].rule)
    {
      continue;
    }

    rc = nm_mdmprxy_process_rule(&static_rule_list[i],
                                 NM_MDMPRXY_RUN_RULE, af, &nfo);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s() failed to apply rule [%d]", __func__, i);
      break;
    }
  }

  if ((NETMGR_SUCCESS != rc) &&
          (i <= rules_len - 1))
  {
    netmgr_log_err("%s(): rolling back installed rules!", __func__);
    nm_mdmprxy_route_prov_common_uninstall_static_rules(af, mark, i);
  }

  return rc;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_cleanup_lbo_rules
===========================================================================*/
/*!
@brief
  Function to cleanup installed LBO iptables rules
*/
/*=========================================================================*/
static int nm_mdmprxy_route_prov_common_cleanup_lbo_rules(void)
{
  int                  rc = NETMGR_SUCCESS;
  int                  reti = NETMGR_SUCCESS;
  int                  cur_index = 0;
  int                  lbo_rules_cleared = FALSE;
  unsigned int         rules_len = 0;
  unsigned int         i = 0;
  char                 result[NM_MDMPRXY_MAXCMDSTR] = "";
  uint64_t             bitmask;
  struct lbo_rule_info lbo_info;
  file_info_t          recovery_data;
  file_info_t          meta_info;

  memset(&meta_info, 0, sizeof(file_info_t));
  memset(&recovery_data, 0, sizeof(file_info_t));
  memset(&lbo_info, 0, sizeof(struct lbo_rule_info));

  /* Read meta information to get information on number of valid
     entries in recovery file */
  rc = netmgr_recovery_file_read_entry(NETMGR_RECOVERY_META_INFO_INDEX, &meta_info);
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): Failed to read meta info!", __func__);
    return NETMGR_FAILURE;
  }

  /* Read all valid entries from file and look for LBO information */
  bitmask = meta_info.recovery_info.meta_info.entry_bitmask;
  while (bitmask != 0)
  {
    if (bitmask & 0x1)
    {
      if (cur_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
      {
        break;
      }

      rc = netmgr_recovery_file_read_entry(cur_index, &recovery_data);
      if (NETMGR_SUCCESS != rc)
      {
        netmgr_log_err("%s(): failed to read entry at index [%d]!",
                       __func__, cur_index);
        reti = NETMGR_FAILURE;
        bitmask >>= 1;
        cur_index += 1;
        continue;
      }

      if (TRUE == recovery_data.is_valid && TYPE_IWLAN_LINK_INFO == recovery_data.info_type
            && TECH_TYPE_LBO == recovery_data.recovery_info.link_info.tech_type)
      {
        /* Found LBO related information in the file, use it to cleanup LBO iptables rules */
        (void) strlcpy(lbo_info.iif, recovery_data.recovery_info.link_info.link_name,
                       NM_MDMPRXY_MAXDEVSTR);

        if (!lbo_rules_cleared)
        {
          /* LBO top level rules are common to both IPv4 and IPv6 so we attempt to
             cleanup both sets of rules at the same time. In case the file has more
             than one LBO entry (one for V4 and one for V6) we will skip the cleanup
             part for the second entry but will still invalidate it in the file */
          netmgr_log_med("%s(): cleanup LBO iptables rules", __func__);
          rules_len = nm_mdmprxy_rules_len(remove_lbo_rule_list);
          for (i = 0; i < rules_len; i++)
          {
            if (!remove_lbo_rule_list[i].rule)
            {
              continue;
            }

            do
            {
              /* Remove IPv4 LBO rules from top level chains */
              rc = nm_mdmprxy_process_rule_return_result(&remove_lbo_rule_list[i],
                                                         NM_MDMPRXY_RUN_RULE,
                                                         AF_INET,
                                                         &lbo_info,
                                                         result,
                                                         sizeof(result));
            } while (NETMGR_SUCCESS == rc);

            if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
            {
              netmgr_log_med("%s(): IPv4 LBO rules removed!", __func__);
            }
            else
            {
              netmgr_log_err("%s(): command failed with err: %s", __func__, result);
            }

            do
            {
              /* Remove IPv6 LBO rules from top level chains */
              rc = nm_mdmprxy_process_rule_return_result(&remove_lbo_rule_list[i],
                                                         NM_MDMPRXY_RUN_RULE,
                                                         AF_INET6,
                                                         &lbo_info,
                                                         result,
                                                         sizeof(result));
            } while (NETMGR_SUCCESS == rc);

            if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
            {
              netmgr_log_med("%s(): IPv6 LBO rules removed!", __func__);
            }
            else
            {
              netmgr_log_err("%s(): command failed with err: %s", __func__, result);
            }
          } /* Remove LBO rules */

          /* We have attempted to cleanup both */
          lbo_rules_cleared = TRUE;
        }

        /* Invalidate the LBO entry */
        recovery_data.is_valid = FALSE;
        if (NETMGR_SUCCESS != netmgr_recovery_file_write_entry(&recovery_data))
        {
          netmgr_log_err("%s(): failed to invalidate LBO link info in recovery file", __func__);
          reti = NETMGR_FAILURE;
        }
      } /* Found valid LBO information in recovery file */
    } /* If the entry is valid */

    bitmask >>= 1;
    cur_index += 1;
  } /* while there are valid entries in the recovery file */

  if (NETMGR_SUCCESS != reti)
  {
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_iptables_deinit
===========================================================================*/
/*!
@brief
  Cleans up iiface and oiface rules
*/
/*=========================================================================*/
static void nm_mdmprxy_route_prov_common_iptables_deinit(void)
{
  int i = 0;

  /* This removes both static and iiface + oiface rules. This mechanism will be
     used till we fix the SSR handling within core */
  for (i = 0; i < NM_MDMPRXY_MAXRULES; i++)
  {
    nm_mdmprxy_uninstall_iiface_rules(i);
  }
}

/*===========================================================================
                           PUBLIC DEFINITIONS
===========================================================================*/
/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_install_iiface_rules
===========================================================================*/
/*!
@brief
  Sets up rules to forward packets to the modem by using TEE target
  to clone packets

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_install_iiface_rules
(
  int                  link,
  char*                link_name,
  int                  tech_type,
  int                  valid4,
  uint32_t             daddr4,
  int                  valid6,
  struct in6_addr      *daddr6,
  uint32_t             mask6,
  ipsec_tunnel_info_t  *tunnel
)
{
  int ip_family = AF_INET;
  struct in6_addr gwaddr6;

#ifdef FEATURE_RECOVERY_ENABLED
  NM_MDMPRXY_VAR_UNUSED(tunnel);
#endif /* FEATURE_RECOVERY_ENABLED */

  inet_pton(AF_INET6, NM_MDMPRXY_GATEWAY_V6_ADDR, &gwaddr6);

  if (iiface_refcount == 0)
  {
    /* The first reverse rmnet link is coming up. We need to activate all the dormant sockets
       to prepare for data transfer */
    if (mark_prov_inst)
    {
      netmgr_log_low("%s(): first reverse link is up, put sockets in listening mode", __func__);
      mark_prov_inst->change_socket_state(NM_MDMPRXY_SOCKET_STATE_LISTENING);
    }

    /* iiface rule list is nothing but the jump rules.
     * This is the rule to jump to the marking chain from PREROUTING and it needs to be
     * installed only for the first reverse rmnet. This is because these rules do not
     * apply to LTE scenarios.
     */
    iiface_handle = nm_mdmprxy_install_iiface_rules(NM_MDMPRXY_IIFACE, nm_mdmprxy_route_oem_mark);
    if (NETMGR_FAILURE == iiface_handle)
    {
      netmgr_log_err("%s(): failed to install iiface rules!", __func__);
      return NETMGR_FAILURE;
    }
  }

  /* Install the destination based rules, if any */
  if (valid4)
  {
    struct in_addr tempv4addr = {daddr4};
    if (NETMGR_SUCCESS != nm_mdmprxy_install_masquerade_skip_rule( (const char*) link_name,
                                                                  tech_type,
                                                                  ip_family,
                                                                  &tempv4addr))
    {
      netmgr_log_err("%s(): failed to install masquerade rules for link [%s], iiface_handle [%d]",
                     __func__, link_name, iiface_handle);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Installed masquerade skip rules for link [%s]", __func__, link_name);
    }
  }

  /* Install TEE rule for local breakout */
  if (NETMGR_KIF_CB_IF_TECH_TYPE_LBO == tech_type)
  {
    if (valid4)
    {
      ip_family = AF_INET;
    }
    else if (valid6)
    {
      ip_family = AF_INET6;
    }
    else
    {
      netmgr_log_err("%s(): bad ip_family!", __func__);
      return NETMGR_FAILURE;
    }

    if (NETMGR_SUCCESS != nm_mdmprxy_install_local_breakout_rules(link_name,
                                                                  NM_MDMPRXY_IIFACE,
                                                                  NM_MDMPRXY_GATEWAY_V4_ADDR,
                                                                  &gwaddr6,
                                                                  ip_family))
    {
      netmgr_log_err("%s(): Failed to install TEE rule for local-breakout iface!", __func__);
      return NETMGR_FAILURE;
    }

    if (NETMGR_SUCCESS != nm_mdmprxy_install_local_breakout_doze_rules(ip_family))
    {
      netmgr_log_err("%s(): Failed to install doze rules for local-breakout iface!", __func__);
      return NETMGR_FAILURE;
    }
  }

  /* Install the destination based rules, if any */
  if (valid4)
  {
    if (NETMGR_SUCCESS != nm_mdmprxy_install_oiface_rules(iiface_handle,
                                                          link,
                                                          (const char*) link_name,
                                                          TRUE,
                                                          NM_MDMPRXY_GATEWAY_V4_ADDR,
                                                          daddr4,
                                                          FALSE,
                                                          NULL,
                                                          NULL,
                                                          0))
    {
      netmgr_log_err("%s(): failed to install IPv4 oif rules for link [%s], iiface_handle [%d]",
                     __func__, link_name, iiface_handle);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Installed IPv4 oif rules for link [%s]", __func__, link_name);
    }
  }
  else if (valid6)
  {
    if (NETMGR_SUCCESS != nm_mdmprxy_install_oiface_rules(iiface_handle,
                                                          link,
                                                          (const char*) link_name,
                                                          FALSE,
                                                          NM_MDMPRXY_V4ADDR_INVALID,
                                                          NM_MDMPRXY_V4ADDR_INVALID,
                                                          TRUE,
                                                          &gwaddr6,
                                                          daddr6,
                                                          mask6))
    {
      netmgr_log_err("%s(): failed to install IPv6 oif rules for link [%s], iiface_handle [%d]",
                     __func__, link_name, iiface_handle);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Installed IPv6 oif rules for link [%s]", __func__, link_name);
    }
  }
  else
  {
    netmgr_log_err("%s(): Unhandled AF; skipping config", __func__);
    return NETMGR_FAILURE;
  }

#ifndef FEATURE_RECOVERY_ENABLED
  if ((NETMGR_KIF_CB_IF_TECH_TYPE_EPDG == tech_type)
	  && (netmgr_main_cfg.output_mark_enable == NETMGR_FALSE))
  {
    /* Install rules based on the IPsec tunnel being setup */
    if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_install_ipsec_tunnel_rules(link_name,
                                                                                  tunnel))
    {
      netmgr_log_err("%s(): failed to setup tunnel rules for link [%s]!", __func__, link_name);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Installed IP[%d] tunnel rules for link=%s!",
                     __func__, tunnel->tunnel_family, link_name);
    }
  }
#endif /* FEATURE_RECOVERY_ENABLED */

  iiface_refcount++;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_remove_iiface_rules
===========================================================================*/
/*!
@brief
  Cleans up iptables rules for the given link

@return
  NETMGR_SUCCESS/NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_remove_iiface_rules
(
  int                            link,
  char                           *link_name,
  int                            tech_type,
  int                            ip_family,
  ipsec_tunnel_info_t            *tunnel,
  struct netmgr_kif_addrstorage  *link_addr
)
{
  struct in6_addr gwaddr6;

  NM_MDMPRXY_VAR_UNUSED(link_addr);

#ifdef FEATURE_RECOVERY_ENABLED
  NM_MDMPRXY_VAR_UNUSED(tunnel);
#endif /* FEATURE_RECOVERY_ENABLED */

  inet_pton(AF_INET6, NM_MDMPRXY_GATEWAY_V6_ADDR, &gwaddr6);

  /* Remove destination based rules, if any */
  if (NETMGR_SUCCESS != nm_mdmprxy_uninstall_oiface_rules(iiface_handle, link, ip_family))
  {
    netmgr_log_err("%s(): failed to uninstall oif rules for link [%s]", __func__, link_name);
    return NETMGR_FAILURE;
  }
  else
  {
    netmgr_log_err("%s(): successfully uninstalled oif rules for link [%s]", __func__, link_name);
  }

  /* If this is the last ePDG call going down, remove tunnel doze rules */
  if ((NETMGR_KIF_CB_IF_TECH_TYPE_EPDG == tech_type) && (netmgr_main_cfg.output_mark_enable == NETMGR_FALSE))
  {
#ifndef FEATURE_RECOVERY_ENABLED
    /* Remove rules based on the IPsec tunnel being setup */
    if (NETMGR_SUCCESS != nm_mdmprxy_route_prov_common_remove_ipsec_tunnel_rules(link_name, tunnel))
    {
      netmgr_log_err("%s(): failed to remove tunnel rules for link [%s]!", __func__, link_name);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Removed IP[%d] tunnel rules for link=%s!",
                     __func__, tunnel->tunnel_family, link_name);
    }
#endif /* FEATURE_RECOVERY_ENABLED */
  }

  /* Remove destination based rules, if any */
  netmgr_log_err("%s(%d) link_addr: %d family %d \n", __func__, __LINE__, link_addr->type, ip_family);
  if (ip_family == AF_INET)
  {
    struct in_addr tempv4addr = {link_addr->_in4.addr};
    if (NETMGR_SUCCESS != nm_mdmprxy_remove_masquerade_skip_rule( (const char*) link_name,
                                                          tech_type,
                                                          ip_family,
                                                          &tempv4addr))
    {
      netmgr_log_err("%s(): failed to remove masquerade rules for link [%s], iiface_handle [%d]",
                     __func__, link_name, iiface_handle);
      return NETMGR_FAILURE;
    }
    else
    {
      netmgr_log_med("%s(): Removed masquerade skip rules for link [%s]", __func__, link_name);
    }
  }

  /* Remove TEE rule for local breakout */
  if (NETMGR_KIF_CB_IF_TECH_TYPE_LBO == tech_type)
  {
    if (NETMGR_SUCCESS
          != nm_mdmprxy_remove_local_breakout_rules(link_name,
                                                    NM_MDMPRXY_IIFACE,
                                                    NM_MDMPRXY_GATEWAY_V4_ADDR,
                                                    &gwaddr6,
                                                    ip_family,
                                                    (int) nm_mdmprxy_rules_len(local_breakout_rule_list) - 1))
    {
      netmgr_log_err("%s(): Failed to remove TEE rule for local-breakout iface!", __func__);
      return NETMGR_FAILURE;
    }

    if (NETMGR_SUCCESS
          != nm_mdmprxy_remove_local_breakout_doze_rules(ip_family,
                                                         (int) nm_mdmprxy_rules_len(local_breakout_doze_rule_list) - 1))
    {
      netmgr_log_err("%s(): Failed to remove doze rules for local-breakout iface!", __func__);
      return NETMGR_FAILURE;
    }
  }

  /* TODO: The original design was assuming that the refcount being = 1 means that the iface
     being cleaned up was an iiface. However, there is no guarantee that the last iface being
     disconnected is the iiface. If there are other oifaces still up and we get a disconnect
     on the iiface earlier to getting disconnect on oiface then we  are not cleaning up  the
     associated oiface rules */
  if (iiface_refcount == 1)
  {
    /* If this is the last rev rmnet iface then remove the jump rule from PREROUTING */
    netmgr_log_low("%s(): last reverse rmnet iface [%s] is down! Removing iiface rules",
                   __func__, link_name);
    nm_mdmprxy_uninstall_iiface_rules(iiface_handle);

    /* The last reverse rmnet link has gone down, we can shutdown all the listening
       remote sockets */
    if (mark_prov_inst)
    {
      netmgr_log_low("%s(): last reverse link is down, shutdown the listening sockets", __func__);
      mark_prov_inst->change_socket_state(NM_MDMPRXY_SOCKET_STATE_SHUTDOWN);
    }

    iiface_handle = -1;
  }

  /* Decrement the iiface_refcount only when its non-zero
     This will prevent inadvertant decrement past zero */
  if (iiface_refcount != 0)
  {
    iiface_refcount--;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_flush_doze_mode_rules
===========================================================================*/
/*!
@brief
  Global OOS doze rule clean-up
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_flush_doze_mode_rules(sa_family_t af)
{
  if (AF_INET != af && AF_INET6 != af)
  {
    netmgr_log_err("%s(): Unknown address family", __func__);
    return;
  }

  if (NETMGR_SUCCESS != nm_mdmprxy_flush_doze_rules(af))
  {
    netmgr_log_err("%s(): failed to remove doze rules for af=%d",
                   __func__, af);
  }
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_link_is_hdlr
===========================================================================*/
/*!
@brief
  Per link IS handler
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_link_is_hdlr(void* arg)
{
  /* Currently we don't have anything to handle in per link modem IS handler
     Eventually we want to move iiface + oiface rule + any other link specific
     rules to this handler */
  NM_MDMPRXY_VAR_UNUSED(arg);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_oos_hdlr
===========================================================================*/
/*!
@brief
  Global OOS handler
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_oos_hdlr(void)
{
  /* Un-initialize iptables */
  nm_mdmprxy_route_prov_common_iptables_deinit();

  nm_mdmprxy_route_prov_common_flush_doze_mode_rules(AF_INET);

  nm_mdmprxy_route_prov_common_flush_doze_mode_rules(AF_INET6);

#ifndef FEATURE_RECOVERY_ENABLED
  nm_mdmprxy_flush_vpn_skip_rules();
#endif /* FEATURE_RECOVERY_ENABLED */

  netmgr_log_low("%s(): route provider OOS handler complete", __func__);
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_is_hdlr
===========================================================================*/
/*!
@brief
  Global IS handler

@note
  - Dependencies
    - Global IS handler for the common module has been called
*/
/*=========================================================================*/
void nm_mdmprxy_route_prov_common_is_hdlr(void)
{
  /* All the links would have gone down during SSR cleanup so we also need
     to reset our internal counters */
  iiface_refcount = 0;

  /* Cleanup tunnel information list */
  memset(tunnel_rules, 0, sizeof(tunnel_rules));

  netmgr_log_low("%s(): route provider IS handler complete", __func__);
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_link_oos_hdlr
===========================================================================*/
/*!
@brief
  Per link OOS handler
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_link_oos_hdlr(void* arg)
{
  struct in6_addr gwaddr6;
  netmgr_kif_cb_link_ssr_info_t *ifinfo =
         (netmgr_kif_cb_link_ssr_info_t *) arg;
  int rc = NETMGR_SUCCESS;

  if (!ifinfo)
  {
    netmgr_log_err("%s(): Invalid params...skipping deinit!", __func__);
    goto bail;
  }

  inet_pton(AF_INET6, NM_MDMPRXY_GATEWAY_V6_ADDR, &gwaddr6);

  if (NETMGR_KIF_CB_IF_TECH_TYPE_LBO == ifinfo->tech_type)
  {
    if (NETMGR_SUCCESS
          != nm_mdmprxy_remove_local_breakout_rules(ifinfo->name,
                                                    NM_MDMPRXY_IIFACE,
                                                    NM_MDMPRXY_GATEWAY_V4_ADDR,
                                                    &gwaddr6,
                                                    AF_INET,
                                                    (int) nm_mdmprxy_rules_len(local_breakout_rule_list) - 1))
    {
      netmgr_log_err("%s(): Failed to remove V4 TEE rule for local-breakout iface!",
                      __func__);
    }

    if (NETMGR_SUCCESS
          != nm_mdmprxy_remove_local_breakout_rules(ifinfo->name,
                                                    NM_MDMPRXY_IIFACE,
                                                    NM_MDMPRXY_GATEWAY_V4_ADDR,
                                                    &gwaddr6,
                                                    AF_INET6,
                                                    (int) nm_mdmprxy_rules_len(local_breakout_rule_list) - 1))
    {
      netmgr_log_err("%s(): Failed to remove V6 TEE rule for local-breakout iface!",
                      __func__);
    }
  }

bail:
  netmgr_log_low("%s(): complete", __func__);
  return rc;
}

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_netd_oos_hdlr
===========================================================================*/
/*!
@brief
  Netd OOS handler

@return
  NETMGR_SUCCESS / NETMGR_FAILURE
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_netd_oos_hdlr(void)
{
  int rc = NETMGR_SUCCESS;
  int i = 0;
  int rules_len = nm_mdmprxy_rules_len(remove_static_rules_netd_oos);
  struct nm_mdmprxy_iiface_rule_info nfo;

  if (NM_MDMPRXY_INVALID_PACKET_MARK == nm_mdmprxy_route_oem_mark)
  {
    netmgr_log_err("%s(): packet mark is already invalidated, "
                   "nothing to do!", __func__);
    return NETMGR_SUCCESS;
  }

  memset(&nfo, 0, sizeof(nfo));
  nfo.mark = nm_mdmprxy_route_oem_mark;

  /* Un-install static rules for IPv4 */
  for (i = (rules_len - 1); i >= 0; i--)
  {
    rc |= nm_mdmprxy_process_rule(&remove_static_rules_netd_oos[i],
                                  NM_MDMPRXY_RUN_ANTIRULE, AF_INET, &nfo);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): failed to apply IPv4 rule [%d]", __func__, i);
    }
  }

  /* Un-install static rules for IPv6 */
  for (i = (rules_len - 1); i >= 0; i--)
  {
    rc |= nm_mdmprxy_process_rule(&remove_static_rules_netd_oos[i],
                                  NM_MDMPRXY_RUN_ANTIRULE, AF_INET6, &nfo);
    if (NETMGR_SUCCESS != rc)
    {
      netmgr_log_err("%s(): failed to apply IPv6 rule [%d]", __func__, i);
    }
  }

  return rc;
}

#ifdef FEATURE_RECOVERY_ENABLED
/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_deinit
===========================================================================*/
/*!
@brief
  Function to cleanup the installed rules

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_deinit(void)
{
  int rc = NETMGR_SUCCESS;
  unsigned int i = 0;
  unsigned int rules_len = 0;
  char result[NM_MDMPRXY_MAXCMDSTR] = "";
  struct nm_mdmprxy_iiface_rule_info nfo;
  size_t bytes = 0;

  memset(&nfo, 0, sizeof(struct nm_mdmprxy_iiface_rule_info));
  bytes = strlcpy(nfo.iif, NM_MDMPRXY_IIFACE, NM_MDMPRXY_MAXDEVSTR);
  if (bytes >= NM_MDMPRXY_MAXDEVSTR)
  {
    netmgr_log_err("%s(): truncation occurred!", __func__);
    return NETMGR_FAILURE;
  }

  rules_len = nm_mdmprxy_rules_len(remove_per_iiface_rule_list);
  for (i = 0; i < rules_len; i++)
  {
    if (!remove_per_iiface_rule_list[i].rule)
    {
      continue;
    }

    do
    {
      /* Remove top level rules from IPv4 chains. In the event where more than one top level
         rule got added due to multiple rounds of daemon crash and restart we need to delete
         each of those rules one-by-one till the state is clean. */
      rc = nm_mdmprxy_process_rule_return_result(&remove_per_iiface_rule_list[i],
                                                 NM_MDMPRXY_RUN_RULE,
                                                 AF_INET,
                                                 &nfo,
                                                 result,
                                                 sizeof(result));
    } while (NETMGR_SUCCESS == rc);

    if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
    {
      netmgr_log_med("%s(): IPv4 iiface rules removed!", __func__);
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
      rc = nm_mdmprxy_process_rule_return_result(&remove_per_iiface_rule_list[i],
                                                 NM_MDMPRXY_RUN_RULE,
                                                 AF_INET6,
                                                 &nfo,
                                                 result,
                                                 sizeof(result));
    } while (NETMGR_SUCCESS == rc);

    if (NETMGR_SUCCESS == nm_mdmprxy_common_check_error(result))
    {
      netmgr_log_med("%s(): IPv6 iiface rules removed!", __func__);
    }
    else
    {
      netmgr_log_err("%s(): command failed with err: %s", __func__, result);
    }
  }

  rc = nm_mdmprxy_route_prov_common_cleanup_lbo_rules();
  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to cleanup LBO iptables rules!", __func__);
  }

  netmgr_log_low("%s(): cleanup complete!", __func__);
  return NETMGR_SUCCESS;
}
#endif /* FEATURE_RECOVERY_ENABLED */

/*===========================================================================
  FUNCTION  nm_mdmprxy_route_prov_common_init
===========================================================================*/
/*!
@brief
  Init function

@note
  - Dependencies
    - Global nm_mdmprxy_common_init
*/
/*=========================================================================*/
int nm_mdmprxy_route_prov_common_init(uint32_t packetMark)
{
  iiface_refcount = 0;

  memset(&route_prov_common_iptables_cmd_tbl, 0, sizeof(route_prov_common_iptables_cmd_tbl));
  memset(tunnel_rules, 0, sizeof(tunnel_rules));

  mark_prov_inst = nm_mdmprxy_get_mark_provider();
  if (!mark_prov_inst)
  {
    netmgr_log_err("%s(): failed to get mark provider instance!", __func__);
  }

  /* Copy all the command arrays to the iptables helper */
  route_prov_common_iptables_cmd_tbl.per_iiface_rule_list     = per_iiface_rule_list;
  route_prov_common_iptables_cmd_tbl.per_iiface_rule_list_len = (int) nm_mdmprxy_rules_len(per_iiface_rule_list);

  route_prov_common_iptables_cmd_tbl.per_oiface_rule_list     = per_oiface_rule_list;
  route_prov_common_iptables_cmd_tbl.per_oiface_rule_list_len = (int) nm_mdmprxy_rules_len(per_oiface_rule_list);

  route_prov_common_iptables_cmd_tbl.per_iface_forward_rule_list     = per_iface_forward_rule_list;
  route_prov_common_iptables_cmd_tbl.per_iface_forward_rule_list_len = (int) nm_mdmprxy_rules_len(per_iface_forward_rule_list);

  route_prov_common_iptables_cmd_tbl.local_breakout_rule_list     = local_breakout_rule_list;
  route_prov_common_iptables_cmd_tbl.local_breakout_rule_list_len = (int) nm_mdmprxy_rules_len(local_breakout_rule_list);

  route_prov_common_iptables_cmd_tbl.local_breakout_doze_rule_list     = local_breakout_doze_rule_list;
  route_prov_common_iptables_cmd_tbl.local_breakout_doze_rule_list_len = (int)nm_mdmprxy_rules_len(local_breakout_doze_rule_list);

  route_prov_common_iptables_cmd_tbl.vpn_skip_rule_list     = vpn_skip_rule_list;
  route_prov_common_iptables_cmd_tbl.vpn_skip_rule_list_len = (int) nm_mdmprxy_rules_len(vpn_skip_rule_list);

  route_prov_common_iptables_cmd_tbl.masquerade_skip_rule_list     = masquerade_skip_rule_list;
  route_prov_common_iptables_cmd_tbl.masquerade_skip_rule_list_len = (int) nm_mdmprxy_rules_len(masquerade_skip_rule_list);

  nm_mdmprxy_register_route_prov_cmd_tbl(&route_prov_common_iptables_cmd_tbl);

  if (NETMGR_SUCCESS
        != nm_mdmprxy_route_prov_common_install_static_rules(AF_INET, packetMark))
  {
    netmgr_log_err("%s(): failed to setup static rules for IPv4", __func__);
    return NETMGR_FAILURE;
  }

  if (NETMGR_SUCCESS
        != nm_mdmprxy_route_prov_common_install_static_rules(AF_INET6, packetMark))
  {
    netmgr_log_err("%s(): failed to setup static rules for IPv6, cleanup "
                   "installed IPv4 rules", __func__);
    nm_mdmprxy_route_prov_common_uninstall_static_rules(AF_INET, packetMark,
                                                        (nm_mdmprxy_rules_len(static_rule_list) -1 ));
    return NETMGR_FAILURE;
  }

  nm_mdmprxy_route_oem_mark = packetMark;

  return NETMGR_SUCCESS;
}
