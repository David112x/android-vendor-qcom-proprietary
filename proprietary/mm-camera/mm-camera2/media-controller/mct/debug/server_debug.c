/* server_debug.c
 *
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "server_debug.h"

#define SOF_RECOVER 0


void* server_debug_dump_data_for_sof_freeze(void *status)
{
  int fd;
  char buf[1024] = {0};
  int len;
  DIR *dir;
  struct dirent* de;
  DIR *dir1;
  struct dirent* de1;
  char input[256] = {0};
  char filename[128] = {0};
  int debug_file_fd;

  strlcpy (filename, QCAMERA_DUMP_FRM_LOCATION"sof_freeze_dump.txt",
    sizeof(filename));
  debug_file_fd = open (filename, O_RDWR|O_CREAT, 0777);
  if (debug_file_fd < 0) {
    debug_file_fd = open (filename, O_RDWR);
    if (debug_file_fd < 0) {
      CLOGE(CAM_MCT_MODULE, "Failure in opening %s", filename);
      return NULL;
    }
  }
  if (*((int *)status) == SOF_RECOVER) {
    if (remove(filename) == 0) {
      CLOGE(CAM_MCT_MODULE, "file deleted successfully.");
    }
    else
      CLOGE(CAM_MCT_MODULE, "Unable to delete the file");
    return NULL;
  }
  write(debug_file_fd, "**********clock start**********\n", 32);
  dir = opendir("/sys/kernel/debug/clk");
  if (dir == NULL) {
    CLOGE(CAM_MCT_MODULE, "opendir clk fails");
    close(debug_file_fd);
    return NULL;
  }
  while ((de = readdir(dir))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;
    write(debug_file_fd, "******************************\n", 31);
    write(debug_file_fd, de->d_name, strlen(de->d_name));
    write(debug_file_fd, "\n", 1);
    write(debug_file_fd, "******************************\n", 31);
    memset(input, 0, 256);
    strlcpy(input, "/sys/kernel/debug/clk/", sizeof(input));
    strlcat(input, de->d_name, sizeof(input));
    if (de->d_type != DT_DIR)
      continue;
    dir1 = opendir(input);
    if (dir1 == NULL) {
      CLOGE(CAM_MCT_MODULE, "opendir of %s fails", input);
      close(debug_file_fd);
      closedir(dir);
      return NULL;
    }
    while ((de1 = readdir(dir1))) {
      if (!strcmp(de1->d_name, ".") || !strcmp(de1->d_name, ".."))
        continue;

      if (de1->d_type == DT_DIR)
        continue;

      if (!strcmp(de1->d_name, "enable")) {
        strlcpy(input, "/sys/kernel/debug/clk/", sizeof(input));
        strlcat(input, de->d_name, sizeof(input));
        strlcat(input, "/", sizeof(input));
        strlcat(input, "enable", sizeof(input));
        fd = open(input, O_RDONLY | O_SYNC);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "enable = ", 9);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        } else {
          CLOGE(CAM_MCT_MODULE, "open of %s fails", input);
        }
      }
      if (!strcmp(de1->d_name, "rate")) {
        strlcpy(input, "/sys/kernel/debug/clk/", sizeof(input));
        strlcat(input, de->d_name, sizeof(input));
        strlcat(input, "/", sizeof(input));
        strlcat(input, "rate", sizeof(input));
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "rate = ", 7);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        } else {
          CLOGE(CAM_MCT_MODULE, "open of %s fails", input);
        }
      }
    }
    closedir(dir1);
  }
  closedir(dir);
  write(debug_file_fd, "**********clock end**********\n", 30);
  write(debug_file_fd, "**********Regulator start**********\n", 36);
  dir = opendir("/sys/kernel/debug/regulator");
  if (dir == NULL) {
    CLOGE(CAM_MCT_MODULE, "opendir regulator fails");
    close(debug_file_fd);
    return NULL;
  }
  while ((de = readdir(dir))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
      continue;
    write(debug_file_fd, "******************************\n", 31);
    write(debug_file_fd, de->d_name, strlen(de->d_name));
    write(debug_file_fd, "\n", 1);
    write(debug_file_fd, "******************************\n", 31);
    memset(input, 0, 256);
    strlcpy(input, "/sys/kernel/debug/regulator/", sizeof(input));
    strlcat(input, de->d_name, sizeof(input));
    if (de->d_type != DT_DIR)
      continue;
    dir1 = opendir(input);
    if (dir1 == NULL) {
      CLOGE(CAM_MCT_MODULE, "opendir of %s fails", input);
      close(debug_file_fd);
      closedir(dir);
      return NULL;
    }
    while ((de1 = readdir(dir1))) {
      if (!strcmp(de1->d_name, ".") || !strcmp(de1->d_name, ".."))
        continue;

      if (de1->d_type == DT_DIR)
        continue;

      if (!strcmp(de1->d_name, "enable")) {
        strlcpy(input, "/sys/kernel/debug/regulator/", sizeof(input));
        strlcat(input, de->d_name, sizeof(input));
        strlcat(input, "/", sizeof(input));
        strlcat(input, "enable", sizeof(input));
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "enable = ", 9);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        } else {
          CLOGE(CAM_MCT_MODULE, "open of %s fails", input);
        }
      }
      if (!strcmp(de1->d_name, "voltage")) {
        strlcpy(input, "/sys/kernel/debug/regulator/", sizeof(input));
        strlcat(input, de->d_name, sizeof(input));
        strlcat(input, "/", sizeof(input));
        strlcat(input, "voltage", sizeof(input));
        fd = open(input, O_RDONLY);
        if (fd > 0) {
          memset(buf, 0, 1024);
          len = read(fd, buf, 1024);
          if (len > 0) {
            write(debug_file_fd, "voltage = ", 10);
            write(debug_file_fd, buf, len);
            write(debug_file_fd, "\n", 1);
          }
          close(fd);
        } else {
          CLOGE(CAM_MCT_MODULE, "open of %s fails", input);
        }
      }
    }
    closedir(dir1);
  }
  closedir(dir);
  write(debug_file_fd, "**********Regulator end**********\n", 34);

  write(debug_file_fd, "**********GPIO start**********\n", 31);
#if 0
  fd = open("/sys/kernel/debug/gpio", O_RDONLY);
  if (fd > 0) {
    do {
     len = read(fd, buf, 1024);
     if (len < 1024) {
       write(debug_file_fd, buf, len);
       break;
     }
     if (len == 0)
       break;

    write(debug_file_fd, buf, len);
    } while (1);
    close(fd);
  } else {
    CLOGE(CAM_MCT_MODULE, "open of gpio fails");
  }

  write(debug_file_fd, "**********GPIO end**********\n", 29);
#endif
  close(debug_file_fd);
  return NULL;
}
