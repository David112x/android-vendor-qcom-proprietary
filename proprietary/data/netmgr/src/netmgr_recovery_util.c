/******************************************************************************

                 N E T M G R _ R E C O V E R Y _ U T I L . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_recovery_util.c
  @brief   Network Manager Recovery Utility

  DESCRIPTION
  Implementation to read/write from netmgr recovery binary file.
  * Each entry in the binary file is of type: file_info_t
  * The total number if valid entries in the file is given by: total_entries
  * The valid indices are not guaranteed to be in order from 0 to n-1. There
    could be invalidated entries in between, which is why we have the bitmask.

  Example Binary File Structure:
  +---------------------------------------------------------------------+
  |   file_info_t @  index -1  info_type = TYPE_META_INFO               |
  |                            total_entries = 3                        |
  |                            entry_bitmask = 0b0000_..._0000_1011     |
  +---------------------------------------------------------------------+
  |   file_info_t @  index 0   info_type = TYPE_LINK_LOCAL_NW_SETTINGS  |
  |                            is_valid = TRUE                          |
  |                            network_hndl = 0xAB                      |
  |                            packet_mark  = 0xA                       |
  +---------------------------------------------------------------------+
  |   file_info_t @  index 1   info_type = TYPE_IWLAN_NW_SETTINGS       |
  |                            is_valid = TRUE                          |
  |                            network_hndl = 0xCD                      |
  |                            packet_mark  = 0xB                       |
  +---------------------------------------------------------------------+
  |   file_info_t @  index 2   info_type = TYPE_LINK_LOCAL_LINK_INFO    |
  |                            is_valid = FALSE                         |
  |                            link = 1, ip_family = v6                 |
  +---------------------------------------------------------------------+
  |   file_info_t @  index 3   info_type = TYPE_IWLAN_LINK_INFO         |
  |                            is_valid = TRUE                          |
  |                            link = 1, ip_family = v4                 |
  +---------------------------------------------------------------------+
  |   file_info_t @  index 4   info_type = TYPE_IWLAN_LINK_INFO         |
  |                            is_valid = FALSE                         |
  |                            link = 2, ip_family = v6                 |
  +---------------------------------------------------------------------+

******************************************************************************/
/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#ifdef FEATURE_RECOVERY_ENABLED

#include "netmgr_util.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
boolean recovery_debug_flag = FALSE;

static file_info_t meta_file_info;

FILE *recovery_fp = NULL;

pthread_mutex_t netmgr_recovery_file_mutex = PTHREAD_MUTEX_INITIALIZER;

#define NETMGR_RECOVERY_FILE_LOCK()                                  \
  netmgr_log_med("%s(): locking recovery file mutex\n", __func__);   \
  pthread_mutex_lock(&netmgr_recovery_file_mutex);

#define NETMGR_RECOVERY_FILE_UNLOCK()                                \
  netmgr_log_med("%s(): unlocking recovery file mutex\n", __func__); \
  pthread_mutex_unlock(&netmgr_recovery_file_mutex);

/*===========================================================================
    FUNCTION:  netmgr_recovery_update_meta_info

    @note Caller should have the file lock acquired
===========================================================================*/
static int netmgr_recovery_update_meta_info(void)
{
  if (!recovery_fp) {
      netmgr_log_err("%s(): recovery file is not open!\n", __func__);
      return NETMGR_FAILURE;
  }

  /* Seek to the beginning of the file */
  if (-1 == fseek(recovery_fp, 0, SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate fp to beginning [%d:%s]!\n",
                   __func__, errno, strerror(errno));
    return NETMGR_FAILURE;
  }

  /* Update the meta info at the beginning */
  fwrite((void *)&meta_file_info, sizeof(file_info_t), 1, recovery_fp);
  if (fflush(recovery_fp) || ferror(recovery_fp))
  {
    netmgr_log_err("%s(): failed to update meta file info!\n", __func__);
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
    FUNCTION:  netmgr_recovery_init_meta_info
===========================================================================*/
static int netmgr_recovery_init_meta_info(void)
{
  int rc = NETMGR_FAILURE;

  if (!recovery_fp) {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  meta_file_info.is_valid = TRUE;
  meta_file_info.info_type = TYPE_META_INFO;
  meta_file_info.recovery_info.meta_info.entry_bitmask = 0;
  meta_file_info.recovery_info.meta_info.total_entries = 0;

  NETMGR_RECOVERY_FILE_LOCK();

  rc = netmgr_recovery_update_meta_info();

  NETMGR_RECOVERY_FILE_UNLOCK();

  if (NETMGR_SUCCESS != rc)
  {
    netmgr_log_err("%s(): failed to update meta_info in file!\n", __func__);
  }

  return rc;
}

/*===========================================================================
    FUNCTION:  netmgr_recovery_recover_meta_info
===========================================================================*/
static int netmgr_recovery_recover_meta_info(void)
{
  file_info_t file_info;

  if (!recovery_fp) {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  /* Index -1 indicates meta info */
  if (NETMGR_SUCCESS != netmgr_recovery_file_read_entry(NETMGR_RECOVERY_META_INFO_INDEX,
                                                        &file_info))
  {
    netmgr_log_err("%s(): failed to read file recovery meta info!\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  if (file_info.is_valid && (TYPE_META_INFO == file_info.info_type))
  {
    memcpy(&meta_file_info, &file_info, sizeof(file_info_t));
  }
  else
  {
    netmgr_log_err("%s(): Meta file info is invalid or of wrong type!\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  netmgr_recovery_file_find_match
===========================================================================*/
/*!
@brief
  Searches the recovery file for a match. If a match is found then we fill
  in the output params with the found entry info/index. If no match is found,
  then we use the found_index_ptr to hold the earliest available entry in the
  recovery file so we do not have to search again for an available index.

@note
  Caller should acquire NETMGR_RECOVERY_FILE_LOCK before invoking.

@param
  file_info_t *query_file_info_ptr - input param with file info to match
  int         *found_index_ptr     - output param with index of the found entry
  file_info_t *found_file_info_ptr - output param with file info if found
  int         *is_match_found_ptr  - output param, TRUE only if match is found

@return
  int - NETMGR_FAILURE if we encounter any file read/write/seek errors.
        NETMGR_SUCCESS if the file was succesfully traversed, then caller
        should check *is_match_found_ptr.
*/
/*=========================================================================*/
static int netmgr_recovery_file_find_match(file_info_t *query_file_info_ptr,
                                           int         *found_index_ptr,
                                           file_info_t *found_file_info_ptr,
                                           int         *is_match_found_ptr)
{
  file_info_t cur_file_info;
  int file_ptr_offset = 0;
  int cur_index = 0;

  if (!recovery_fp)
  {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  if (!query_file_info_ptr)
  {
    netmgr_log_err("%s(): invalid input param: query_file_info_ptr\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  if (!found_index_ptr)
  {
    netmgr_log_err("%s(): invalid output param: found_index_ptr\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  if (!found_file_info_ptr)
  {
    netmgr_log_err("%s(): invalid output param: found_file_info_ptr\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  if (!is_match_found_ptr)
  {
    netmgr_log_err("%s(): invalid output param: is_match_found_ptr\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  /* Clear output params */
  memset(found_file_info_ptr, 0, sizeof(file_info_t));
  *found_index_ptr = -1;
  *is_match_found_ptr = FALSE;

  /* Seek to the first entry after the meta info */
  if (-1 == fseek(recovery_fp, sizeof(file_info_t), SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate file pointer to file beginning "
                   "[%d:%s]!\n", __func__,
                   errno, strerror(errno));
    return NETMGR_FAILURE;
  }

  /* Linear search through file entries */
  while (fread((void *)&cur_file_info, sizeof(file_info_t), 1, recovery_fp) == 1)
  {
    /* TODO - Optimization: the ftell and cur_index calculation is only needed
       if a match is found or if we are saving an invalid location for later use */
    file_ptr_offset = ftell(recovery_fp);
    if (-1 == file_ptr_offset)
    {
      netmgr_log_err("%s(): ftell failed [%d:%s]!\n",
                     __func__, errno, strerror(errno));
      return NETMGR_FAILURE;
    }

    /* Subtract 2 because ftell returns the offset after the last read entry
       and we also have meta info at the start of the file*/
    cur_index = ((file_ptr_offset / (int)sizeof(file_info_t)) - 2);
    if (cur_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
    {
      netmgr_log_err("%s(): Error: reached max number of entries\n", __func__);
      return NETMGR_FAILURE;
    }

    /* Dont check invalid entries */
    RECOVERY_DEBUG("\nvalid = %d ", cur_file_info.is_valid);
    if (FALSE == cur_file_info.is_valid)
    {
      RECOVERY_DEBUG("skipping >> cur_index = %d offset = %d\n",
                     cur_index, file_ptr_offset);
      /* Save the location of the earliest available entry, so we can use it later
         if we want to create a new entry - saves us from having to look again */
      if (*found_index_ptr < 0)
      {
        *found_index_ptr = cur_index;
      }
      continue;
    }

    RECOVERY_DEBUG("comparing >> cur_index = %d offset = %d\n",
                   cur_index, file_ptr_offset);

    if (cur_file_info.info_type != query_file_info_ptr->info_type)
    {
      continue;
    }

    /* Comparison step:
       compare nw settings type for nw_setting entries (should only be one)
       compare link_id and ip_family for link info entries
       compare link_id, ip_family, flow_i for bearer info entries */
    switch (cur_file_info.info_type) {
      case TYPE_LINK_LOCAL_NW_SETTINGS:
      case TYPE_IWLAN_NW_SETTINGS:
        {
          RECOVERY_DEBUG("NW_SETTINGS\t- info_type=%d "
                         "Cached network handle: 0x%" PRIx64 ", "
                         "Cached packet mark: 0x%" PRIx32 "\n",
                         cur_file_info.info_type,
                         cur_file_info.recovery_info.nw_settings.network_hndl,
                         cur_file_info.recovery_info.nw_settings.packet_mark);

           netmgr_log_med("Match found at index [%d] offset [%d]\n",
                          cur_index, file_ptr_offset);
           *is_match_found_ptr = TRUE;
           *found_index_ptr = cur_index;
           memcpy(found_file_info_ptr, &cur_file_info, sizeof(file_info_t));
           return NETMGR_SUCCESS;

        }
        break; /* TYPE_LINK_LOCAL_NW_SETTINGS / TYPE_IWLAN_NW_SETTINGS */

      case TYPE_LINK_LOCAL_LINK_INFO:
      case TYPE_IWLAN_LINK_INFO:
        {
          RECOVERY_DEBUG("LINK_INFO\t- IP TYPE %2d : LINK %2d\n",
                         cur_file_info.recovery_info.link_info.ip_family,
                         cur_file_info.recovery_info.link_info.link);

          if ((cur_file_info.recovery_info.link_info.ip_family
                 == query_file_info_ptr->recovery_info.link_info.ip_family)
              && (cur_file_info.recovery_info.link_info.link
                 == query_file_info_ptr->recovery_info.link_info.link))
          {
            RECOVERY_DEBUG("Match found at index [%d] offset [%d]\n",
                           cur_index, file_ptr_offset);
            *is_match_found_ptr = TRUE;
            *found_index_ptr = cur_index;
            memcpy(found_file_info_ptr, &cur_file_info, sizeof(file_info_t));
            return NETMGR_SUCCESS;
          }
        }
        break; /* TYPE_LINK_LOCAL_LINK_INFO / TYPE_IWLAN_LINK_INFO */

      case TYPE_BEARER_INFO:
        {
          RECOVERY_DEBUG("TYPE_BEARER_INFO\t- IP TYPE %2d : LINK %2d : "
                         "FLOW_ID 0x%x\n",
                         cur_file_info.recovery_info.bearer_info.ip_family,
                         cur_file_info.recovery_info.bearer_info.link,
                         cur_file_info.recovery_info.bearer_info.flow_id);

          /* Flow id should match as well */
          if ((cur_file_info.recovery_info.bearer_info.ip_family
                == query_file_info_ptr->recovery_info.bearer_info.ip_family)
              && (cur_file_info.recovery_info.bearer_info.link
                == query_file_info_ptr->recovery_info.bearer_info.link)
              && (cur_file_info.recovery_info.bearer_info.flow_id
                == query_file_info_ptr->recovery_info.bearer_info.flow_id))
          {
            RECOVERY_DEBUG("Match found at index [%d] offset [%d]\n",
                           cur_index, file_ptr_offset);
            *is_match_found_ptr = TRUE;
            *found_index_ptr = cur_index;
            memcpy(found_file_info_ptr, &cur_file_info, sizeof(file_info_t));
            return NETMGR_SUCCESS;
          }
        }

        break; /* TYPE_BEARER_INFO */

      default:
        RECOVERY_DEBUG("%s(): invalid type to find [%d]!\n",
                       __func__, cur_file_info.info_type);
        break;
    }
  }

  RECOVERY_DEBUG("%s(): No Match found - output param: found_index_ptr = %d\n",
                 __func__, *found_index_ptr);
  return NETMGR_SUCCESS;
}

/*===========================================================================
                       GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
    FUNCTION:  netmgr_recovery_file_close
===========================================================================*/
void netmgr_recovery_file_close(void)
{
  if (recovery_fp)
  {
    fclose(recovery_fp);
    recovery_fp = NULL;
  }
}

/*===========================================================================
    FUNCTION:  netmgr_recovery_file_init
===========================================================================*/
int netmgr_recovery_file_init(void)
{
  struct stat sb;
  int rc = NETMGR_SUCCESS;

  if (stat(RECOVERY_FILE, &sb) == -1)
  {
    netmgr_log_err("%s(): stat failed [%d:%s]!\n",
                   __func__, errno, strerror(errno));

    if (ENOENT == errno)
    {
      netmgr_log_high("%s(): the file does not exist!\n", __func__);
      recovery_fp = fopen(RECOVERY_FILE, "w+e");
      if (!recovery_fp)
      {
        netmgr_log_err("%s(): failed to create file!\n", __func__);
        rc = NETMGR_FAILURE;
        goto bail;
      }
      netmgr_recovery_init_meta_info();
    }
    else
    {
      rc = NETMGR_FAILURE;
      goto bail;
    }
  }
  else
  {
    if (sb.st_size == 0)
    {
      netmgr_log_high("%s(): file exists but is empty!\n", __func__);
      recovery_fp = fopen(RECOVERY_FILE, "w+e");
      if (!recovery_fp)
      {
        netmgr_log_err("%s(): failed to open file!\n", __func__);
        rc = NETMGR_FAILURE;
        goto bail;
      }
      if (NETMGR_SUCCESS != netmgr_recovery_init_meta_info())
      {
        netmgr_log_err("%s(): failed to init meta info!\n", __func__);
        rc = NETMGR_FAILURE;
        goto bail;
      }
    }
    else
    {
      netmgr_log_med("%s(): file size: %zd\n", __func__, sb.st_size);
      if (!recovery_fp)
      {
        /* The file already exists with some values, open it for reading and
           writing without truncating it to zero length by using 'r+' mode */
        recovery_fp = fopen(RECOVERY_FILE, "r+e");
        if (!recovery_fp)
        {
          netmgr_log_err("%s(): failed to open recovery file [%d:%s]!\n",
                         __func__, errno, strerror(errno));
          rc = NETMGR_FAILURE;
          goto bail;
        }

        netmgr_log_med("%s(): File contents of recovery file on bootup:\n",
                       __func__)
        netmgr_recovery_file_print_contents();

        if (NETMGR_SUCCESS != netmgr_recovery_recover_meta_info())
        {
          netmgr_log_err("%s(): Failed to recover meta info from top of "
                         "recovery file! - reiniting\n", __func__);
          /* TODO : I think we might need to clear everything in the file, also
             should we return failure in this case? */
          if (NETMGR_SUCCESS != netmgr_recovery_init_meta_info())
          {
            netmgr_log_err("%s(): failed to init meta info!\n", __func__);
            rc = NETMGR_FAILURE;
            goto bail;
          }
        }
      }
    }
  }

bail:
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_recovery_print_bitmask
===========================================================================*/
/*!
@brief
  Print the bitmask in binary so we can easily tell which entries are valid.
*/
/*=========================================================================*/
void netmgr_recovery_print_bitmask(uint64_t bitmask)
{
  unsigned char *b = (unsigned char*) &bitmask;
  unsigned char byte;
  int i, j, idx = 0;
  char bitmask_buffer[NETMGR_MAX_STR_LENGTH];
  memset(bitmask_buffer, 0x0, NETMGR_MAX_STR_LENGTH);

  for (i=8-1;i>=0;i--)
  {
    for (j=7;j>=0;j--)
    {
      byte = (b[i] >> j) & 1;
      bitmask_buffer[idx] = '0' + byte;
      idx++;
      if ((j % 4 == 0))
      {
        if (!(i == 0 && j == 0))
        {
          bitmask_buffer[idx] = '_';
          idx++;
        }
      }
    }
  }
  netmgr_log_low("meta info bitmask: %s", bitmask_buffer);
}

/*===========================================================================
  FUNCTION  netmgr_recovery_file_print_contents
===========================================================================*/
/*!
@brief
  Print all the contents of the recovery file

@return
  void
*/
/*=========================================================================*/
void netmgr_recovery_file_print_contents(void)
{
  file_info_t file_info;
  int file_ptr_offset = 0;
  int entry_index = 0;

  if (!recovery_fp) {
    netmgr_log_err("%s(): recovery file is not open!", __func__);
    return;
  }

  NETMGR_RECOVERY_FILE_LOCK();

  /* Seek to the beginning of the file */
  if (-1 == fseek(recovery_fp, 0, SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate file pointer to file beginning "
                   "[%d:%s]!\n", __func__, errno, strerror(errno));
    goto bail;
  }

  netmgr_log_high("*******************************"
                  " RECOVERY FILE CONTENTS "
                  "*******************************");

  while (fread((void *)&file_info, sizeof(file_info_t), 1, recovery_fp) == 1) {
    file_ptr_offset = ftell(recovery_fp);
    if (-1 == file_ptr_offset) {
      netmgr_log_err("%s(): ftell failed [%d:%s]!\n",
                     __func__, errno, strerror(errno));
      goto bail;
    }

    entry_index = ((file_ptr_offset / (int)sizeof(file_info_t)) - 2);
    if (entry_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
    {
      netmgr_log_err("%s(): Error: reached max number of entries\n", __func__);
      goto bail;
    }

    netmgr_log_med("----------------------------------------------"
                   "----------------------------------------------");
    netmgr_log_low(">>> entry_index = [%d] | offset = [%lu bytes] | "
                   "[valid = %d]\n", entry_index,
                   file_ptr_offset - sizeof(file_info_t),
                   file_info.is_valid);

    switch (file_info.info_type)
    {
      case TYPE_META_INFO:
        {
          netmgr_log_low("TYPE_META_INFO\t- num_entries: %d "
                         "bitmask = 0x%" PRIx64 ", \n",
                         file_info.recovery_info.meta_info.total_entries,
                         file_info.recovery_info.meta_info.entry_bitmask);
          netmgr_recovery_print_bitmask(file_info.recovery_info.meta_info.entry_bitmask);
        }
        break; /* TYPE_META_INFO */

      case TYPE_LINK_LOCAL_NW_SETTINGS:
        {
          netmgr_log_low("TYPE_LINK_LOCAL_NW_SETTINGS\t- "
                         "nw handle: 0x%" PRIx64 ", "
                         "packet mark: 0x%" PRIx32 "\n",
                         file_info.recovery_info.nw_settings.network_hndl,
                         file_info.recovery_info.nw_settings.packet_mark);
        }
        break; /* TYPE_LINK_LOCAL_NW_SETTINGS */

      case TYPE_IWLAN_NW_SETTINGS:
        {
          netmgr_log_low("TYPE_IWLAN_NW_SETTINGS\t- nw handle: 0x%" PRIx64 ", "
                         "packet mark: 0x%" PRIx32 "\n",
                         file_info.recovery_info.nw_settings.network_hndl,
                         file_info.recovery_info.nw_settings.packet_mark);
        }
        break; /* TYPE_IWLAN_NW_SETTINGS */

      case TYPE_LINK_LOCAL_LINK_INFO:
        {
          netmgr_log_low("TYPE_LINK_LOCAL_LINK_INFO\t- IP TYPE %2d : "
                         "LINK %2d : LINK STATE %2d : TECH %2d : LINK ADDR %20s",
                         file_info.recovery_info.link_info.ip_family,
                         file_info.recovery_info.link_info.link,
                         file_info.recovery_info.link_info.link_state,
                         file_info.recovery_info.link_info.tech_type,
                         file_info.recovery_info.link_info.link_addr);

          netmgr_log_low("\t\t<TUNNEL INFO>: SA STATE %d : LOCAL ADDR %15s : "
                         "DEST ADDR %15s : SPI TX 0x%8" PRIx32 " : "
                         "SPI RX 0x%8" PRIx32 " : PROTO %3s : MODE %9s"
                         " : IKE PORT %5d",
                         file_info.recovery_info.link_info.tunnel_info.sa_state,
                         file_info.recovery_info.link_info.tunnel_info.local_addr,
                         file_info.recovery_info.link_info.tunnel_info.dest_addr,
                         file_info.recovery_info.link_info.tunnel_info.spi_tx,
                         file_info.recovery_info.link_info.tunnel_info.spi_rx,
                         file_info.recovery_info.link_info.tunnel_info.proto,
                         file_info.recovery_info.link_info.tunnel_info.mode,
                         file_info.recovery_info.link_info.tunnel_info.ike_port);
        }
        break; /* TYPE_LINK_LOCAL_LINK_INFO */

      case TYPE_IWLAN_LINK_INFO:
        {
          netmgr_log_low("TYPE_IWLAN_LINK_INFO\t- IP TYPE %2d : LINK %2d : "
                         "LINK STATE %2d : TECH %2d : LINK ADDR %20s",
                       file_info.recovery_info.link_info.ip_family,
                       file_info.recovery_info.link_info.link,
                       file_info.recovery_info.link_info.link_state,
                       file_info.recovery_info.link_info.tech_type,
                       file_info.recovery_info.link_info.link_addr);

          netmgr_log_low("\t\t<TUNNEL INFO>: SA STATE %d : LOCAL ADDR %15s : "
                         "DEST ADDR %15s : SPI TX 0x%8" PRIx32 " : "
                         "SPI RX 0x%8" PRIx32 " : PROTO %3s : MODE %9s"
                         " : IKE PORT %5d",
                         file_info.recovery_info.link_info.tunnel_info.sa_state,
                         file_info.recovery_info.link_info.tunnel_info.local_addr,
                         file_info.recovery_info.link_info.tunnel_info.dest_addr,
                         file_info.recovery_info.link_info.tunnel_info.spi_tx,
                         file_info.recovery_info.link_info.tunnel_info.spi_rx,
                         file_info.recovery_info.link_info.tunnel_info.proto,
                         file_info.recovery_info.link_info.tunnel_info.mode,
                         file_info.recovery_info.link_info.tunnel_info.ike_port);
        }
        break; /* TYPE_IWLAN_LINK_INFO */

      case TYPE_BEARER_INFO:
        {
          netmgr_log_low("TYPE_BEARER_INFO\t- IP TYPE %2d : LINK %2d : "
                         "FLOW_ID 0x%x : FILTER MASK 0x%x\n",
                         file_info.recovery_info.bearer_info.ip_family,
                         file_info.recovery_info.bearer_info.link,
                         file_info.recovery_info.bearer_info.flow_id,
                         file_info.recovery_info.bearer_info.filter_mask);
        }
        break; /* TYPE_BEARER_INFO */

      default:
        netmgr_log_low("%s(): invalid type [%d]\n",
                       __func__, file_info.info_type);
        break;
    }
  }

  netmgr_log_med("----------------------------------------------"
                 "----------------------------------------------");
bail:
  netmgr_log_high("*******************************"
                  "********** EOF *********"
                  "*******************************");
  NETMGR_RECOVERY_FILE_UNLOCK();
}


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
                                    int         *found_index_ptr)
{
  int rc = NETMGR_FAILURE;
  int is_match_found = FALSE;
  file_info_t found_file_info;

  NETMGR_RECOVERY_FILE_LOCK();
  rc = netmgr_recovery_file_find_match(file_info_ptr,
                                       found_index_ptr,
                                       &found_file_info,
                                       &is_match_found);
  NETMGR_RECOVERY_FILE_UNLOCK();

  if ((NETMGR_SUCCESS == rc) && is_match_found)
  {
    memcpy(file_info_ptr, &found_file_info, sizeof(file_info_t));
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}


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
int netmgr_recovery_file_read_entry(int index, file_info_t *file_info_ptr)
{
  int rc = NETMGR_FAILURE;

  if (!recovery_fp) {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  if (!file_info_ptr) {
    netmgr_log_err("%s(): invalid output param: file_info_ptr\n", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_RECOVERY_FILE_LOCK();

  memset(file_info_ptr, 0, sizeof(file_info_t));
  if (index < -1 || index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
  {
    netmgr_log_err("%s(): index [%d] is out of range!\n", __func__, index);
    goto bail;
  }
  else if (index == NETMGR_RECOVERY_META_INFO_INDEX) /* Meta Info Case */
  {
    netmgr_log_low("%s(): index [%d]: Reading Meta Info from top of file\n",
                   __func__, index);
  }
  else /* Regular Entry Case */
  {
    /* Try to find an invalid entry before this */
    RECOVERY_DEBUG("%s(): checking index %d...", __func__, index);
    if ((meta_file_info.recovery_info.meta_info.entry_bitmask
           & (((uint64_t) 1) << (index))) == 0)
    {
      RECOVERY_DEBUG("%s(): bit [%d] is cleared in the mask - "
                     "this entry is invalid!\n", __func__, index);
      goto bail;
    }
    RECOVERY_DEBUG("%s(): index [%d] is valid in meta bitmask\n",
                   __func__, index);
  }

  /* Seek to the file entry. Add one since meta info is at beginning of file */
  if (-1 == fseek(recovery_fp, (index + 1) * sizeof(file_info_t), SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate file pointer to entry at index [%d] "
                   "[%d:%s]!\n", __func__, index ,errno, strerror(errno));
    goto bail;
  }

  /* Read file entry and copy to output param */
  if (fread((void *)file_info_ptr, sizeof(file_info_t), 1, recovery_fp) != 1)
  {
      netmgr_log_err("%s(): Read FAILED for entry at index [%d]\n",
                     __func__, index);
      goto bail;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_RECOVERY_FILE_UNLOCK();

  netmgr_log_low("%s(): EXIT with %s - index [%d]\n", __func__,
                 (rc == NETMGR_SUCCESS) ? "success" : "FAILURE",
                 index);

  return rc;
}


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
int netmgr_recovery_file_write_entry(file_info_t *file_info_ptr)
{
  int         file_ptr_offset = 0;
  int         entry_index     = -1;
  int         found_index     = -1;
  int         is_match_found  = FALSE;
  int         rc              = NETMGR_FAILURE;
  file_info_t found_file_info;

  if (!recovery_fp)
  {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  if (!file_info_ptr)
  {
    netmgr_log_err("%s(): invalid output param: file_info_ptr\n", __func__);
    return NETMGR_FAILURE;
  }

  if (TYPE_META_INFO == file_info_ptr->info_type)
  {
    netmgr_log_err("%s(): Meta info types can not be written to file\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_RECOVERY_FILE_LOCK();

  /* Check for a matching entry to update */
  if (NETMGR_SUCCESS != netmgr_recovery_file_find_match(file_info_ptr,
                                                        &found_index,
                                                        &found_file_info,
                                                        &is_match_found))
  {
    netmgr_log_err("%s(): Could not check if the entry already "
                   "exists!\n", __func__);
    goto bail;
  }

  /* If we get back SUCCESS but index is still -1, then no match was found
     create a new entry in the recovery file */
  if (is_match_found == FALSE)
  {
    if (FALSE == file_info_ptr->is_valid)
    {
      netmgr_log_err("%s(): attempted to write a new invalid file "
                     "- not writing\n", __func__);
      goto bail;
    }

    if (found_index < 0)
    {
      file_ptr_offset = ftell(recovery_fp);
      if (-1 == file_ptr_offset) {
        netmgr_log_err("%s(): ftell failed [%d:%s]!\n",
                       __func__, errno, strerror(errno));
        goto bail;
      }

      /* No match found and no invalid entries, create a new one */
      entry_index = ((file_ptr_offset / (int)sizeof(file_info_t)) - 1);
      RECOVERY_DEBUG("%s(): No Match found, creating a new entry at "
                     "end of file - new index = %d\n",
                     __func__, entry_index);
    }
    else
    {
      /* If no match is found but found_index >= 0 there is an invalid
         entry in the file that we can overwrite. */
      entry_index = found_index;
      RECOVERY_DEBUG("%s(): No Match found, reusing invalid entry at "
                     "index = %d\n", __func__, entry_index);
    }
  }
  else
  {
    entry_index = found_index;
    RECOVERY_DEBUG("%s(): Match found at index = %d - updating... \n",
                   __func__, entry_index);
  }

  if (entry_index < 0)
  {
    netmgr_log_err("%s(): Debug: entry index is negative!\n", __func__);
    goto bail;
  }

  if (entry_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
  {
    netmgr_log_err("%s(): Error: reached max number of entries\n", __func__);
    goto bail;
  }

  /* Seek to the file entry. Add one since meta info is at beginning of file */
  if (-1 == fseek(recovery_fp, (entry_index + 1) * sizeof(file_info_t), SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate file pointer to entry at index [%d] "
                   "[%d:%s]!\n", __func__, entry_index ,errno, strerror(errno));
    goto bail;
  }

  fwrite((void *)file_info_ptr, sizeof(file_info_t), 1, recovery_fp);
  if (fflush(recovery_fp) || ferror(recovery_fp))
  {
    netmgr_log_err("%s(): failed to write entry at index = %d\n",
                   __func__, entry_index);
    goto bail;
  }

  /* Update meta info if we created an entry */
  if (is_match_found == FALSE)
  {
    netmgr_log_med("%s(): Successfully created entry at index = %d\n",
                   __func__, entry_index);
    meta_file_info.recovery_info.meta_info.total_entries += 1;
    meta_file_info.recovery_info.meta_info.entry_bitmask
        |= (((uint64_t) 1) << (entry_index));
    if (NETMGR_SUCCESS != netmgr_recovery_update_meta_info())
    {
      netmgr_log_err("%s(): failed to update meta_info in file!\n", __func__);
      goto bail;
    }
  }
  else
  {
    if (FALSE == file_info_ptr->is_valid)
    {
      netmgr_log_med("%s(): Successfully invalidated entry at index = %d\n",
                     __func__, entry_index);
      meta_file_info.recovery_info.meta_info.total_entries -= 1;
      meta_file_info.recovery_info.meta_info.entry_bitmask
          &= (~(((uint64_t) 1) << (entry_index)));
      if (NETMGR_SUCCESS != netmgr_recovery_update_meta_info())
      {
        netmgr_log_err("%s(): failed to update meta_info in file!\n", __func__);
        goto bail;
      }
    }
    else
    {
      netmgr_log_med("%s(): Successfully updated entry at index = %d\n",
                     __func__, entry_index);
    }
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_RECOVERY_FILE_UNLOCK();
  netmgr_log_low("%s(): EXIT with %s\n", __func__,
                 (rc == NETMGR_SUCCESS) ? "success" : "FAILURE");

  return rc;
}

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
int netmgr_recovery_file_find_and_invalidate_entry(file_info_t *file_info_ptr)
{
  int rc = NETMGR_FAILURE;
  int is_match_found = FALSE;
  file_info_t found_file_info;
  int found_index = 0;

  if (!recovery_fp)
  {
    netmgr_log_err("%s(): recovery file is not open!\n", __func__);
    return NETMGR_FAILURE;
  }

  if (!file_info_ptr)
  {
    netmgr_log_err("%s(): invalid output param: file_info_ptr\n", __func__);
    return NETMGR_FAILURE;
  }

  if (TYPE_META_INFO == file_info_ptr->info_type)
  {
    netmgr_log_err("%s(): Meta info types can not be written to file\n",
                   __func__);
    return NETMGR_FAILURE;
  }

  /* This function will always set is_valid to false */
  file_info_ptr->is_valid = FALSE;

  NETMGR_RECOVERY_FILE_LOCK();

  /* Check for a matching entry to invalidate */
  if (NETMGR_SUCCESS != netmgr_recovery_file_find_match(file_info_ptr,
                                                        &found_index,
                                                        &found_file_info,
                                                        &is_match_found))
  {
    netmgr_log_err("%s(): Could not check if the entry already "
                   "exists!\n", __func__);
    goto bail;
  }

  if (is_match_found == FALSE)
  {
    RECOVERY_DEBUG("%s(): No Match found, nothing to invalidate\n",
                   __func__);
    rc = NETMGR_SUCCESS;
    goto bail;
  }
  else
  {
    RECOVERY_DEBUG("%s(): Match found at index = %d - invalidating... \n",
                   __func__, found_index);
  }

  if (found_index < 0)
  {
    netmgr_log_err("%s(): Debug: entry index is negative!\n", __func__);
    goto bail;
  }

  if (found_index >= NETMGR_RECOVERY_MAX_ENTRY_INDEX)
  {
    netmgr_log_err("%s(): Error: index to invalidate is out of bounds!\n", __func__);
    goto bail;
  }

  /* Seek to the file entry. Add one since meta info is at beginning of file */
  if (-1 == fseek(recovery_fp, (found_index + 1) * sizeof(file_info_t), SEEK_SET))
  {
    netmgr_log_err("%s(): cannot relocate file pointer to entry at index [%d] "
                   "[%d:%s]!\n", __func__, found_index ,errno, strerror(errno));
    goto bail;
  }

  fwrite((void *)file_info_ptr, sizeof(file_info_t), 1, recovery_fp);
  if (fflush(recovery_fp) || ferror(recovery_fp))
  {
    netmgr_log_err("%s(): failed to invalidate entry at index = %d\n",
                   __func__, found_index);
    goto bail;
  }

  netmgr_log_med("%s(): Successfully invalidated entry at index = %d\n",
                 __func__, found_index);
  meta_file_info.recovery_info.meta_info.total_entries -= 1;
  meta_file_info.recovery_info.meta_info.entry_bitmask
      &= (~(((uint64_t) 1) << (found_index)));
  if (NETMGR_SUCCESS != netmgr_recovery_update_meta_info())
  {
    netmgr_log_err("%s(): failed to update meta_info in file!\n", __func__);
    goto bail;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_RECOVERY_FILE_UNLOCK();
  netmgr_log_low("%s(): EXIT with %s\n", __func__,
                 (rc == NETMGR_SUCCESS) ? "success" : "FAILURE");

  return rc;
}

#endif /* FEATURE_RECOVERY_ENABLED */
