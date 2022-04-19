#ifndef CNE_UTILS_H
#define CNE_UTILS_H

/**----------------------------------------------------------------------------
  @file CneUtils.h

  This header file provides various utility routines.

-----------------------------------------------------------------------------*/

/*=============================================================================
               Copyright (c) 2009,2010,2017 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneUtils.h#1 $
  $DateTime: 2009/08/31 14:29:20 $
  $Author: syadagir $
  $Change: 1012856 $

  when        who  what, where, why
  ----------  ---  -------------------------------------------------------
  2009-07-15  ysk  First revision.

============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <map>
#include <string>
#include "CneDefs.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define CNE_EXEC_NEW_BUFFSIZE 256
#define CNE_EXEC_MAX_CMD_ARGS 64

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
typedef enum chipsetType_t {
    WLAN_CHIPSET_UNKNOWN = 0,
    WLAN_CHIPSET_NON_QCA,
    WLAN_CHIPSET_PRONTO,
    WLAN_CHIPSET_ROME,
    WLAN_CHIPSET_HELIUM,
    WLAN_CHIPSET_HASTINGS,
    WLAN_CHIPSET_MAX = WLAN_CHIPSET_HASTINGS + 1
}chipsetType_e;

typedef enum _CneForkExecRC {
    CNE_EXEC_OK = 0,         /* forkExec() completed successfully         */
    CNE_EXEC_BAD_ARGS = 1,   /* Caller gave bad arguments                 */
    CNE_EXEC_SYSERR = 2,     /* A system call failed                      */
    CNE_EXEC_TIMEOUT = 3,    /* Command took too long to execute          */
    CNE_EXEC_NONZERORC = 4,  /* Command exited with a non-zero return code*/
    CNE_EXEC_WAITPIDER = 5,  /* waitpid() gave an unexpected return code  */
    CNE_EXEC_UNSUPPORT_CMD = 6 /* Command to be executed is not supported */
}CneForkExecRC;

typedef enum _CneForkExecTimeOutAction {
    CNE_EXEC_TIMEOUT_KILL = 0,         /* Kill the child process and return CNE_EXEC_TIMEOUT*/
    CNE_EXEC_TIMEOUT_PASS = 1,         /* Leave the child running and return CNE_EXEC_TIMEOUT*/
    CNE_EXEC_TIMEOUT_IGNORE = 2        /* Block until child exits*/
}CneForkExecTimeOutAction;

/* ----- forkExec options structure ---------------------------------------- */
typedef struct _CneForkExecOpt
{
    int *rc;                    /* OPTIONAL: store the rc of the child here     */
    char *stdout;               /* OPTIONAL: Buffer to store stdout of the child*/
    int stdout_length;          /* OPTIONAL: Size of the stdout buffer          */
    char *stderr;               /* OPTIONAL: Buffer to store stderr of the child*/
    int stderr_length;          /* OPTIONAL: Size of the stderr buffer          */
    int stdout_to_stderr;       /* OPTIONAL: Redirect stdout to stderr buffer   */
    int stderr_to_stdout;       /* OPTIONAL: Redirect stderr to stdout buffer   */
    struct timeval tv_timeout;  /* OPTIONAL: Command timeout value(Default 2sec)*/
    CneForkExecTimeOutAction timeout_action;         /* OPTIONAL: What to do if the child times out  */

    _CneForkExecOpt() {
        rc = nullptr;
        stdout = nullptr;
        stdout_length = 0;
        stderr = nullptr;
        stderr_length = 0;
        stdout_to_stderr = 0;
        stderr_to_stdout = 0;
        tv_timeout.tv_sec = 2;
        tv_timeout.tv_usec = 0;
        timeout_action = CNE_EXEC_TIMEOUT_KILL;
    }
} CneForkExecOpt;

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/

class CneUtils {
public:

    /**
     @brief Will return the Uint16 bit value pointed by the byte pointer
               passed in

     @param  bytePtr: byte pointer from which the next 16 bits will be refered.
     @see    None
     @return  Uint16
    */
  static uint16_t getUint16(const uint8_t* bytePtr);

    /**
     @brief Will return the Uint32 bit value pointed by the byte pointer
               passed in

     @param  bytePtr: byte pointer from which the next 32 bits will be refered.
     @see    None
     @return  Uint32
    */
  static uint32_t getUint32(const uint8_t* bytePtr);

  /**
   * @brief get a string representation of a CnE command
   *
   * @param[in] getCmd cne_cmd_enum_type
   *
   * @return char const*
   */
  static char const* getCneCmdStr(cne_cmd_enum_type getCmd);

  /**
   * @brief get a string representation of a CnE message
   *
   * @param[in] getMsg cne_msg_enum_type
   *
   * @return char const*
   */
  static char const* getCneMsgStr(cne_msg_enum_type getMsg);

  /**
   * @brief get a string representation of a network state
   *
   * @param[in] getState cne_network_state_enum_type
   *
   * @return char const*
   */
  static char const* getCneNwStateStr(cne_network_state_enum_type getState);

  /**
   * @brief get a string representation of an RAT type
   *
   * @param[in] getRat cne_rat_type
   *
   * @return char const*
   */
  static char const* getCneRatStr (cne_rat_type getRat);

  /**
   * @brief get a string representation of a RAT subtype
   *
   * @param[in] getRatSubType cne_rat_subtype
   *
   * @return char const*
   */
  static char const* getCneRatSubTypeStr(cne_rat_subtype getRatSubType);

  /**
   * @brief get a string representation of a CnE return type
   *
   * @param[in] getRetType CneRetType
   *
   * @return char const*
   */
  static char const* getCneRetTypeStr(CneRetType getRetType);

  /**
   * @brief Queries property to get the wlan chipset type.
   *
   * @param[in] None
   *
   * @return chipsetType_e
   */
  static chipsetType_e getWlanChipsetType();

  /**
   * @brief get a string representation of a CnE Event
   *
   * @param[in] event CneEvent
   *
   * @return char const*
   */
  inline static char const* getCneEventStr(CneEvent event) {
    return getCneCmdStr(event);
  }

  // queries the kernel to get the appname belonging to a process
  static bool GetAppName( int pid, std::string &appname );

  /**
   * @brief Execute a shell command via forked child thread
   *
   * @param[in] command, opts
   *
   * @return int
   */
   static CneForkExecRC forkExec(const char *command, const CneForkExecOpt *opts);

   /**
   * @brief gets the Wifi interface name
   *
   * @param[in] none
   *
   * @return std::string
   */
   static std::string getWifiIfaceDefName();
private:

  static bool isInitNeeded; // if true, init should be called
  static char const* const EMPTY_STRING; // sent when no string representation
                                         // is available
  static char const* const HOSTAPDCLI_EXECUTABLE;
  static char const* const HOSTAPDCLI_EXECUTABLE_PATH;
  static chipsetType_e chipsetType;

  // typedefs for map pairs
  typedef std::pair<cne_cmd_enum_type, char const* const> cmdPair;
  typedef std::pair<cne_msg_enum_type, char const* const> msgPair;
  typedef std::pair<cne_network_state_enum_type, char const* const> netStatePair;
  typedef std::pair<cne_rat_type, char const* const> ratTypePair;
  typedef std::pair<cne_rat_subtype, char const* const> ratSubtypePair;
  typedef std::pair<CneRetType, char const* const> retTypePair;

  // map enum -> string
  static std::map<cne_cmd_enum_type, char const* const> cmd;
  static std::map<cne_msg_enum_type, char const* const> msg;
  static std::map<cne_network_state_enum_type, char const* const> netState;
  static std::map<cne_rat_type, char const* const> ratType;
  static std::map<cne_rat_subtype, char const* const> ratSubtype;
  static std::map<CneRetType, char const* const> retType;

  // init maps
  static void init();

  static CneForkExecRC forkExecInternal(const char *command,
                                        const CneForkExecOpt *opts, const char* cmd_loc);
};

#endif /* CNE_UTILS_H */
