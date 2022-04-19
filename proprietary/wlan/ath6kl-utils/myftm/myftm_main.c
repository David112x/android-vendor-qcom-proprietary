/*==========================================================================

                     FTM Main Task Source File

Description
  Unit test component file for regsitering the routines to Diag library
  for BT and FTM commands

# Copyright (c) 2012-2020  Qualcomm Technologies, Inc.
# All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/18/10   rakeshk  Created a source file to implement routines for
                    registering the callback routines for FM and BT FTM
                    packets
07/06/10   rakeshk  changed the name of FM common header file in inclusion
07/07/10   rakeshk  Removed the sleep and wake in the main thread loop
===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <net/if.h>
#include <stdint.h>
#include <dirent.h>
#include <pwd.h>
#include <cutils/sched_policy.h>
#include <ctype.h>
#include <getopt.h>

#include "myftm_dbg.h"
#include "myftm_wlan.h"
#include "myftm_qtip.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcat g_strlcat
#define strlcpy g_strlcpy
#endif

static char *progname = NULL;
unsigned int g_dbg_level = FTM_DBG_TRACE | FTM_DBG_ERROR;
extern int g_dbg_scpc;

static void usage(void)
{
	fprintf(stderr, "\nusage: %s [options] \n"
		"   -n, --nodaemon      do not run as a daemon\n"
		"   -d    show more debug messages (-dd for even more)\n"
#ifdef CONFIG_FTM_BT
		"   -b, --board-type    Board Type\n"
#endif
#ifdef CONFIG_FTM_WLAN
		"   -i <wlan interface>\n"
		"       --interface=<wlan interface>\n"
		"       wlan adapter name (wlan, eth, etc.) default wlan\n"
#endif
		"   -B  <dbs/phya/phyb/sbs> (Phy RF Mode)\n"
		"   -H  <0/1/2> (RX Mode)\n"
		"   -I  <0/1> (Phy Id)\n"
		"   -J  (TLV 2.0 Messages)\n"
		"   -N  <BSSID>\n"
		"   -O  <STA Addr>\n"
		"   -o  <BT Addr>\n"
		"   -Q  <Secondary Frequncy>\n"
		"   -U  <0/1> (ShortGuard)\n"
		"   -X  <TX Sta Addr>\n"
		"   -Y  <RX Sta Addr>\n"
		"   --sar <0/1/2/3> \n"
		"   --sarindex <0/1/2/3/4/0xff> \n"
		"   --sarchain <0/1> \n"
		"   --sarcck2glimit <0 - 0xff> \n"
		"   --sarofdm2glimit <0 - 0xff> \n"
		"   --sarofdm5glimit <0 - 0xff> \n"
		"   --dpdflag (enable) \n"
		"   --regdomain <00:01> (Two reg domain values)\n"
		"   --dpdstatus \n"
		"   --rateBw <set bandwidth>\n"
		"   --nss <set nss>\n"
		"   --gI <set shortgi>\n"
		"   --dcm <set ofdma dcm>\n"
		"   --ppdutype <set ofdma ppdutype>\n"
		"   --linkdir  <set ofdma link direction>\n"
		"   --toneplan <set ofdma toneplan params>\n"
		"   --fecpad <set prefecpad params>\n"
		"   --ldpc <set extra symbol for ldpc>\n"
		"   --ulofdmat <set ofdma uplink transmmit config>\n"
		"   --dutycl <set duty cycle>\n"
		"   --regw <set register address>\n"
		"   --regd <read register address>\n"
		"   --regval <write value to register address>\n"
		"   --lowpower <set low power config for phyid>\n"
		"   --lpw_mode <set low power mode>\n"
		"   --phyidmask <set phyid mask>\n"
		"   --lpwfmsk   <set feature mask>\n"
		"   --help          display this help and exit\n"
		, progname);
		exit(EXIT_FAILURE);
}

/*===========================================================================
FUNCTION   main

DESCRIPTION
  Initialises the Diag library and registers the PKT table for FM and BT
  and daemonises

DEPENDENCIES
  NIL

RETURN VALUE
  NIL, Error in the event buffer will mean a NULL App version and Zero HW
  version

SIDE EFFECTS
  None

===========================================================================*/

int main(int argc, char *argv[])
{
	int c;
	static struct option options[] = {
		{"help", no_argument, NULL, 'h'},
#ifdef CONFIG_FTM_WLAN
		{"interface", required_argument, NULL, 'i'},
#endif
#ifdef CONFIG_FTM_BT
		{"board-type", required_argument, NULL, 'b'},
#endif
		{"nodaemon", no_argument, NULL, 'n'},
		{"SetWifiEnable", required_argument, NULL, 'e'},
		{"GetWifiEnable", no_argument, NULL, 'E'},
		{"SetWifiMode", required_argument, NULL, 'm'},
		{"SetWlanMode", required_argument, NULL, 'M'},
		{"GetWifiModeSupport", required_argument, NULL, 'S'},
		{"WlanATSetWifiBand", required_argument, NULL, 'w'},
		{"WlanATSetWifiBand", no_argument, NULL, 'W'},
		{"WlanATSetRate", required_argument, NULL, 'r'},
		{"WlanATSetRate", required_argument, NULL, 'R'},
		{"WlanATSetGainIdx", required_argument, NULL, 'G'},
		{"WlanATSetDacGain", required_argument, NULL, 'D'},
		{"WlanATSetPaCfg", required_argument, NULL, 'C'},
		{"WlanATSetNumPkt", required_argument, NULL, 'j'},
		{"WlanATSetAgg", required_argument, NULL, 'k'},
		{"WlanATSetWifiFreq", required_argument, NULL, 'f'},
		{"WlanATSetWifiTxPower", required_argument,NULL, 'p'},
		{"WlanATGetWifiTxPower", no_argument,NULL, 'P'},
		{"WlanATSetWifiAntenna", required_argument, NULL, 'a'},
		{"WlanATGetWifiAntenna", no_argument, NULL, 'A'},
		{"WlanATSetWifiPktSize", required_argument, NULL, 's'},
		{"WlanATSetWifiTPC", required_argument, NULL, 'c'},
		{"WlanATSetWifiTX", required_argument, NULL, 't'},
		{"WlanATGetWifiTX", no_argument, NULL, 'T'},
		{"WlanATSetWifiRX", required_argument, NULL, 'x'},
		{"WlanATSetStbc", required_argument, NULL, 'y'},
		{"WlanATSetLdpc", required_argument, NULL, 'z'},
		{"WlanATSetLPreamble", no_argument, NULL, 'l'},
		{"unittest", no_argument, NULL, 'u'},
		{"SCPCcal", required_argument, NULL, 'L'},
		{"qtipserver", no_argument, NULL, 'q'},
		{"WlanATSetDBS", required_argument, NULL, 'B'},
		{"WlanATSetWifiRXMode", required_argument, NULL, 'H'},
		{"WlanATSetPhyid", required_argument, NULL, 'I'},
		{"TLV2.0", no_argument, NULL, 'J'},
		{"WlanATSetBssid", required_argument, NULL, 'N'},
		{"WlanATSetSTAAddr", required_argument, NULL, 'O'},
		{"WlanATSetBTAddr", required_argument, NULL, 'o'},
		{"WlanATSetWifiFreq2", required_argument, NULL, 'Q'},
		{"WlanATSetTXSta", required_argument, NULL, 'X'},
		{"WlanATSetRXSta", required_argument, NULL, 'Y'},
		{"WlanATSetShortGuard", required_argument, NULL, 'U'},
		{"sar", required_argument, NULL, MYFTM_OPT_CMD_SAR},
		{"sarindex", required_argument, NULL, MYFTM_OPT_PRM_SAR_INDEX8},
		{"sarchain", required_argument, NULL, MYFTM_OPT_PRM_SAR_CHAIN},
		{"sarcck2glimit", required_argument, NULL,
				MYFTM_OPT_PRM_SAR_CCK2GLIMIT},
		{"sarofdm2glimit", required_argument, NULL,
				MYFTM_OPT_PRM_SAR_OFDM2GLIMIT},
		{"sarofdm5glimit", required_argument, NULL,
				MYFTM_OPT_PRM_SAR_OFDM5GLIMIT},
		{"dpdflag", no_argument, NULL, MYFTM_OPT_PRM_FLAG_DPD},
		{"regdomain", required_argument, NULL, MYFTM_OPT_CMD_SETREGDMN},
		{"dpdstatus", no_argument, NULL, MYFTM_OPT_CMD_DPDSTATUS},
		{"rateBw", required_argument, NULL, MYFTM_OPT_CMD_SET_RATEBW},
		{"nss", required_argument, NULL, MYFTM_OPT_CMD_SET_NSS},
		{"gI", required_argument, NULL, MYFTM_OPT_CMD_SET_GI},
		{"dcm", required_argument, NULL, MYFTM_OPT_CMD_SET_OFDM_ADCM},
		{"ppdutype", required_argument, NULL, MYFTM_OPT_CMD_SET_OFDM_PPDU_TYPE},
		{"linkdir", required_argument, NULL, MYFTM_OPT_CMD_SET_OFDM_LINKDIR},
		{"toneplan", required_argument, NULL, MYFTM_OPT_CMD_SET_OFDMA_TONEPLAN},
		{"fecpad", required_argument, NULL, MYFTM_OPT_CMD_SET_PREFECPAD},
		{"ldpc", required_argument, NULL, MYFTM_OPT_CMD_SET_LDPCEXTRASYMBOL},
		{"ulofdmat", no_argument, NULL, MYFTM_OPT_CMD_SET_OFDMAUL_TXCONFIG},
		{"dutycl", required_argument, NULL, MYFTM_OPT_CMD_SET_DUTYCYCLE},
		{"regw", required_argument, NULL, MYFTM_OPT_CMD_WRITE_REGISTER},
		{"regd", required_argument, NULL, MYFTM_OPT_CMD_READ_REGISTER},
		{"regval", required_argument, NULL, MYFTM_OPT_CMD_WRT_REGISTER_VAL},
		{"lowpower", no_argument, NULL, MYFTM_OPT_CMD_SET_LOWPOWER},
		{"lpw_mode", required_argument, NULL, MYFTM_OPT_CMD_SET_LOWPOWER_MODE},
		{"phyidmask", required_argument, NULL, MYFTM_OPT_CMD_SET_PHYIDMASK},
		{"lpwfmsk", required_argument, NULL, MYFTM_OPT_CMD_SET_LOWPOWER_FEATUREMASK},
		{0, 0, 0, 0}
	};
	int daemonize = 0;
	int int_parm = 0;
	uint32_t t_val = 0;
	WLAN_AT_WIFREQ_STRU freq_parm;
	progname = argv[0];

	printf("Version Flags: 1.0\n");
	WlanATinit();

	while (1) {
		c = getopt_long(argc, argv,
				"hdi:nb:e:Em:S:w:Wr:Rg:f:p:Pqt:Tua:Ax:s:C:c:D"
				":k:G:j:y:z:lL:M:B:H:I:JN:O:o:Q:X:Y:U:rateBw:nss:gI:dcm:ppdutype:linkdir:toneplan:"
				"fecpad:ldpc:dutycl:lpw_mode:phyidmask:lowpower "
				"lpwfmsk:ulofdmat ",options, NULL);

		if (c < 0)
			break;

		switch (c) {
#ifdef CONFIG_FTM_WLAN
			case 'i':
				strlcpy(g_ifname, optarg, IFNAMSIZ);
				break;
#endif
			case 'n':
				daemonize = 0;
				break;
			case 'd':
#ifdef DEBUG
				g_dbg_level = g_dbg_level << 1 | 0x1;
#else
				printf("Debugging disabled, please build with -DDEBUG option. No debug level set\n");
				/* no need to EXIT. comment it out. */
				//exit(EXIT_FAILURE);
#endif
				/* enable scpc code debugging */
				g_dbg_scpc = 1;
				break;
			case 'e':
				fprintf(stderr, "SetWifiEnable using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiEnable(int_parm);
				break;
			case 'E':
				fprintf(stderr, "GetWifiEnable using athtestcmdlib\n");
				int_parm = WlanATGetWifiEnable();
				fprintf(stderr, "WlanATGetWifiEnable return as %d\n", int_parm);
				break;
			case 'm':
				fprintf(stderr, "SetWifiMode using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiMode(int_parm);
				break;
			case 'w':
				fprintf(stderr, "WlanATSetWifiBand using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiBand(int_parm);
				break;
			case 'f':
				fprintf(stderr, "WlanATSetWifiFreq using athtestcmdlib\n");
				int_parm = atoi(optarg);
				freq_parm.value = int_parm;
				freq_parm.offset = 0;
				WlanATSetWifiFreq(&freq_parm);
				break;
			case 'p':
				fprintf(stderr, "WlanATSetWifiTxPower using athtestcmdlib\n");
				WlanATSetWifiTxPower(optarg);
				break;
			case 'P':
				fprintf(stderr, "WlanATGetWifiTxPower using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATGetWifiTxPower();
				break;
			case 'r':
				fprintf(stderr, "WlanATSetRate using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetRate(int_parm);
				break;
			case 'R':
				fprintf(stderr, "WlanATGetRate using athtestcmdlib\n");
				WlanATGetRate();
				break;
			case 'a':
				fprintf(stderr, "WlanATSetWifiAntenna using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiAntenna(int_parm);
				break;
			case 'A':
				fprintf(stderr, "WlanATGetWifiAntenna using athtestcmdlib\n");
				WlanATGetWifiAntenna();
				break;
			case 't':
				printf("WlanATSetWifiTX using athtestcmdlib\n");
				int_parm = atoi(optarg);
				if (0 != WlanATSetWifiTX(int_parm)) {
					fprintf(stderr, "WlanATSetWifiTX failed!\n");
					exit(EXIT_FAILURE);
				}
				printf("WlanATSetWifiTX done with sucess\n");
				exit(EXIT_SUCCESS);
				break;
			case 'x':
				fprintf(stderr, "WlanATSetWifiRX using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiRX(int_parm);
				exit(EXIT_SUCCESS);
				break;
			case 'u':
				fprintf(stderr, "unittest using athtestcmdlib\n");
				unittest();
				exit(EXIT_SUCCESS);
				break;
			case 's':
				fprintf(stderr, "WlanATSetWifiPktSize using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiPktSize(int_parm);
				break;
			case 'c':
				fprintf(stderr, "WlanATSetWifiTPC using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiTPC(int_parm);
				break;
			case 'C':
				fprintf(stderr, "WlanATSetPaCfg using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetPaCfg(int_parm);
				break;
			case 'D':
				fprintf(stderr, "WlanATSetDacGain using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetDacGain(int_parm);
				break;
			case 'G':
				fprintf(stderr, "WlanATSetGainIdx using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetGainIdx(int_parm);
				break;
			case 'j':
				fprintf(stderr, "WlanATSetNumPkt using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetNumPkt(int_parm);
				break;
			case 'k':
				fprintf(stderr, "WlanATSetAgg using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetAgg(int_parm);
				break;
			case 'y':
				fprintf(stderr, "WlanATSetStbc using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetStbc(int_parm);
				break;
			case 'z':
				fprintf(stderr, "WlanATSetLdpc using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetLdpc(int_parm);
				break;
			case 'M':
				fprintf(stderr, "WlanATSetWlanMode using athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWlanMode(int_parm);
				break;
			case 'l':
				fprintf(stderr, "WlanATSetLPreamble using athtestcmdlib\n");
				WlanATSetLPreamble();
				break;
			case 'L':
				printf("SCPC cal request\n");
				int_parm = atoi(optarg);
				if (0 != myftm_wlan_set_scpc_cal(int_parm)) {
					fprintf(stderr, "myftm_wlan_set_scpc_cal failed!\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'q':
				fprintf(stderr, "Starting QTIP server.\n");
				qtip();
				break;

			case 'B':
				fprintf(stderr, "WlanATSetDBS using "
					"athtestcmdlib\n");
				WlanATSetDBS(optarg);
				exit(EXIT_SUCCESS);
				break;

			case 'H':
				fprintf(stderr, "WlanATSetWifiRXMode using "
					"athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetWifiRXMode(int_parm);
				break;

			case 'I':
				fprintf(stderr, "WlanATSetPhyid using "
					"athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetPhyid(int_parm);
				break;

			case 'J':
				fprintf(stderr, "using TLV2.0\n");
				tlv2_enabled = 1;
				break;

			case 'N':
				fprintf(stderr, "Set Bssid using "
					"athtestcmdlib\n");
				WlanATSetBssid(optarg);
				break;

			case 'O':
				fprintf(stderr, "Set STA Addr using "
					"athtestcmdlib\n");
				WlanATSetSTAAddr(optarg);
				break;

			case 'o':
				fprintf(stderr, "Set BT using athtestcmdlib\n");
				WlanATSetBTAddr(optarg);
				break;

			case 'Q':
				fprintf(stderr, "WlanATSetWifiFreq2 using "
					"athtestcmdlib\n");
				int_parm = atoi(optarg);
				freq_parm.value = int_parm;
				freq_parm.offset = 0;
				WlanATSetWifiFreq2(&freq_parm);
				break;

			case 'U':
				fprintf(stderr, "WlanATSetShortGuard using "
					"athtestcmdlib\n");
				int_parm = atoi(optarg);
				WlanATSetShortGuard(int_parm);
				break;

			case 'X':
				fprintf(stderr, "Set TX using athtestcmdlib\n");
				WlanATSetTXSta(optarg);
				break;

			case 'Y':
				fprintf(stderr, "Set RX using athtestcmdlib\n");
				WlanATSetRXSta(optarg);
				break;

			case MYFTM_OPT_CMD_SAR:
				fprintf(stderr, "Command SAR\n");
				int_parm = atoi(optarg);
				WlanATCmdSAR(int_parm);
				break;

			case MYFTM_OPT_PRM_SAR_INDEX8:
				fprintf(stderr, "SAR Index %s \n", optarg);
				WlanATCmdSARIndex(optarg);
				break;

			case MYFTM_OPT_PRM_SAR_CHAIN:
				fprintf(stderr, "SAR CHAIN \n");
				int_parm = atoi(optarg);
				WlanATCmdSARChain(int_parm);
				break;

			case MYFTM_OPT_PRM_SAR_CCK2GLIMIT:
				fprintf(stderr, "SAR CCK2GLIMIT %s \n", optarg);
				WlanATCmdSARCCK2gLimit(optarg);
				break;

			case MYFTM_OPT_PRM_SAR_OFDM2GLIMIT:
				fprintf(stderr, "SAR OFDM2GLIMIT %s \n", optarg);
				WlanATCmdSAROFDM2gLimit(optarg);
				break;

			case MYFTM_OPT_PRM_SAR_OFDM5GLIMIT:
				fprintf(stderr, "SAR OFDM5GLIMIT %s \n", optarg);
				WlanATCmdSAROFDM5gLimit(optarg);
				break;

			case MYFTM_OPT_PRM_FLAG_DPD:
				fprintf(stderr, "Enable DPD Flag\n");
				WlanATCmdFlagDPDEnable();
				break;

			case MYFTM_OPT_CMD_SETREGDMN:
				fprintf(stderr, "Command REGDMN\n");
				WlanATCmdSETREGDMN(optarg);
				break;

			case MYFTM_OPT_CMD_DPDSTATUS:
				fprintf(stderr, "Command dpdstatus\n");
				WlanATCmdDPDStatus();
				break;

			case MYFTM_OPT_CMD_SET_RATEBW:
				fprintf(stderr, "Command rateBw\n");
				WlanATCmdSETRateBW(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_NSS:
				fprintf(stderr, "Command nss\n");
				WlanATCmdSETNSS(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_GI:
				fprintf(stderr, "Command gI\n");
				WlanATCmdSETGI(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_OFDM_ADCM:
				fprintf(stderr, "Command ofdm adcm\n");
				WlanATCmdSETADCM(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_OFDM_PPDU_TYPE:
				fprintf(stderr, "Command ofdm ppdu type\n");
				WlanATCmdSETPPDUTYPE(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_OFDM_LINKDIR:
				fprintf(stderr, "Command odfm linkdir\n");
				WlanATCmdSETLINKDIR(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_OFDMA_TONEPLAN:
				fprintf(stderr, "WlanATCmdSET_TONEPLAN using athtestcmdlib\n");
				if (0 != WlanATCmdSET_TONEPLAN(optarg)) {
					fprintf(stderr, "WlanATCmdSET_TONEPLAN failed!\n");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "WlanATCmdSET_TONEPLAN done with success\n");
				exit(EXIT_SUCCESS);
				break;

			case MYFTM_OPT_CMD_SET_PREFECPAD:
				fprintf(stderr, "Command prefecpad\n");
				WlanATCmdSET_PREFECPAD(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_LDPCEXTRASYMBOL:
				fprintf(stderr, "Command ldpc extrasymbol\n");
				WlanATCmdSET_LDPCEXTRASYMBOL(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_SET_OFDMAUL_TXCONFIG:
				fprintf(stderr, "Command odfma uplink Tx config\n");
				if (0 != WlanATCmdSET_OFDMAULTX()) {
					fprintf(stderr, "WlanATCmdSET_OFDMAULTX failed!\n");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "WlanATCmdSET_OFDMAULTX done with success\n");
				exit(EXIT_SUCCESS);
				break;

			case MYFTM_OPT_CMD_SET_DUTYCYCLE:
				fprintf(stderr, "Command Duty Cycle\n");
				WlanATCmdSET_DUTYCYCLE(atoi(optarg));
				break;

			case MYFTM_OPT_CMD_WRT_REGISTER_VAL:
				fprintf(stderr, "Command to write value at register \n");

				t_val = strtol(optarg, NULL, 0);
				if (t_val >= 0)
					WlanATCmdWRITE_REGVAL(t_val);
				else
					fprintf(stderr, "setting WlanATCmdWRITE_REGVAL to 0\n");
				break;

			case MYFTM_OPT_CMD_WRITE_REGISTER:
				fprintf(stderr, "Command to write register \n");

				t_val = strtol(optarg, NULL, 0);
				if ((t_val <= 0) || (0 != WlanATCmdWRITE_REGISTER(t_val))) {
					fprintf(stderr, "WlanATCmdWRITE_REGISTER failed!\n");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "WlanATCmdWRITE_REGISTER done with success\n");
				exit(EXIT_SUCCESS);
				break;

			case MYFTM_OPT_CMD_READ_REGISTER:
				fprintf(stderr, "Command to read register \n");

				t_val = strtol(optarg, NULL, 0);
				if ((t_val <= 0) || (0 != WlanATCmdREAD_REGISTER(t_val))) {
					fprintf(stderr, "WlanATCmdREAD_REGISTER failed!\n");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "WlanATCmdREAD_REGISTER done with success\n");
				exit(EXIT_SUCCESS);
				break;

			case MYFTM_OPT_CMD_SET_LOWPOWER:
				fprintf(stderr, "Command to send LOW POWER config\n");
				if (0 != WlanATCmdSET_LOWPOWER()) {
					fprintf(stderr, "WlanATCmdSET_LOWPOWER failed!\n");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr, "WlanATCmdSET_LOWPOWER done with success\n");
				exit(EXIT_SUCCESS);
				break;

			case MYFTM_OPT_CMD_SET_LOWPOWER_MODE:
				fprintf(stderr, "Command to configured LOW POWER MODE\n");
				WlanATCmdSET_LOWPOWER_MODE(optarg);
				break;

			case MYFTM_OPT_CMD_SET_PHYIDMASK:
				fprintf(stderr, "Command to set PHYID MASK\n");
				WlanATCmdSET_PHYIDMASK(atoi(optarg));
				break;
			case MYFTM_OPT_CMD_SET_LOWPOWER_FEATUREMASK:
				fprintf(stderr, "Command to set Low Power Feature Mask \n");

				t_val = strtol(optarg, NULL, 0);
				if (t_val >= 0)
					WlanATCmdSET_LOWEPOWER_FEATUREMASK(t_val);
				else
					fprintf(stderr, "setting LOWEPOWER_FEATUREMASK to 0\n");
				break;

			case 'h':
			default:
				usage();
				break;
		}
	}

	if (optind < argc)
		usage();

	if (daemonize && daemon(0, 0)) {
		perror("daemon");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

