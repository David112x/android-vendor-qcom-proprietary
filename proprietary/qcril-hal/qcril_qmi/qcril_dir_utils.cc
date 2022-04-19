/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qcril_file_utils.c
  @brief   Provides interface to communicate with files

  DESCRIPTION

******************************************************************************/
#include "qcril_dir_utils.h"
#include "qcril_memory_management.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "qcril_log.h"

/*===========================================================================

  FUNCTION qcril_dir_retrieve_all_files

===========================================================================*/
/*!
    @brief
    retrieve all file names under certain directory

    @return
    0 for success
*/
/*=========================================================================*/
int qcril_dir_retrieve_all_files
(
    const char  *dir,
    list<string> &file_names,
    list<string> &dir_names
)
{
    DIR           *dir_desc;
    struct dirent *entry;
    int           root_dir_name_len = 0;
    int           ret               = E_FAILURE;

    do {
        if (!dir)
        {
            break;
        }

        root_dir_name_len = strlen(dir);
        dir_desc = opendir(dir);

        if (dir_desc)
        {
            while ((entry = readdir(dir_desc)))
            {
                if (strcmp(entry->d_name, "..") && strcmp(entry->d_name,"."))
                {
                    if (entry->d_type == DT_DIR)
                    {
                        string dir_entry;
                        dir_entry.append(dir);
                        dir_entry.append(entry->d_name);
                        dir_entry.append("/");
                        dir_names.push_back(dir_entry);
                        QCRIL_LOG_INFO("retrieved dir: %s", dir_entry.c_str());
                    }
                    else
                    {
                        string file_entry;
                        file_entry.append(dir);
                        file_entry.append(entry->d_name);
                        file_names.push_back(file_entry);
                        QCRIL_LOG_INFO("retrieved file: %s", file_entry.c_str());
                    }
                }
            }

            closedir (dir_desc);
            ret = E_SUCCESS;
        }
        else
        {
            char errstr[200];
            strerror_r(errno, errstr, sizeof(errstr));
            QCRIL_LOG_ESSENTIAL("QCRIL_ERROR:IO: Failed to open directory %s with error: %d (%s)\n",
                    dir, errno, errstr);
        }

    }while (0);

    return ret;
};

/*===========================================================================

  FUNCTION qcril_dir_retrieve_all_files_recursively

===========================================================================*/
/*!
    @brief
    retrieve all files under current dir recursively

    @return
    0 for success
*/
/*=========================================================================*/
int qcril_dir_retrieve_all_files_recursively
(
    const char  *dir,
    list<string> &file_names
)
{
    int ret = E_FAILURE;

    list<string> dir_names;

    QCRIL_LOG_FUNC_ENTRY();

    do {
        if (!dir)
        {
            break;
        }

        dir_names.push_back(dir);

        while (!dir_names.empty())
        {
            string dir_index = dir_names.front();
            QCRIL_LOG_DEBUG("processing dir %s ",dir_index.c_str());
            ret = qcril_dir_retrieve_all_files(dir_index.c_str(),
                                         file_names,
                                         dir_names);
            dir_names.pop_front();

            QCRIL_LOG_DEBUG("cur_num_of_dirs %d", dir_names.size());
            QCRIL_LOG_DEBUG("cur_num_of_files %d", file_names.size());
        }
    } while (0);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
}

