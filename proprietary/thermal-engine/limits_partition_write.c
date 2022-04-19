/*===========================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "thermal.h"
#include "limits_partition_write.h"

#define PARTITION_SIZE_BYTES  4096
#define MAX_RETRY_COUNT 10
#define LIMITS_PARTITION_DELAY_USEC 5000000

static pthread_t limits_partition_thread;

static ssize_t limits_read_sysfs(int fd_sysfs,
			unsigned char *read_buffer)
{
	ssize_t len = 0, pos = 0;

	do {
		len = read(fd_sysfs, &read_buffer[(size_t)pos],
			   (size_t)PARTITION_SIZE_BYTES - (size_t)pos);
		if (len > 0)
			pos += len;
		else if (len == 0)
			break;
		else
			return -errno;
	} while (pos < (ssize_t)PARTITION_SIZE_BYTES);

	return pos;
}

static void *limits_write_data_to_partition(void *data)
{
	int fd_sysfs = -1, fd_partition = -1;
	ssize_t len = 0, pos = 0;
	ssize_t size_of_partition_data = 0;
	int retry = 0;
	char limits_cdsp_partition[] = "/dev/block/by-name/limits-cdsp";
	char limits_cdsp_sysfs[] = "/sys/module/msm_isense_cdsp/data";
	unsigned char read_buffer[PARTITION_SIZE_BYTES];
	unsigned char write_buffer[PARTITION_SIZE_BYTES];
	char write_data_into_partition = 0;

	memset(read_buffer, 0, PARTITION_SIZE_BYTES);
	memset(write_buffer, 0, PARTITION_SIZE_BYTES);

	if ((fd_sysfs = open(limits_cdsp_sysfs, O_RDONLY)) < 0) {
		msg("%s: Error opening lmh_cdsp sysfs\n", __func__);
		goto exit_handler;
	}

	if ((fd_partition = open(limits_cdsp_partition, O_WRONLY)) < 0) {
		msg("%s: Error opening limits-cdsp Partition\n", __func__);
		goto exit_handler;
	}

	do {
		len = limits_read_sysfs(fd_sysfs, &read_buffer);

		/*
		 * Read may return -EAGAIN if data is not available at the
		 * time of read. Read will return -ENODATA from Second boot.
		*/
		if (len > 0) {
			write_data_into_partition = 1;
			size_of_partition_data = len;
			break;
		} else if (len == -EAGAIN) {
			retry++;
			usleep(LIMITS_PARTITION_DELAY_USEC);
			lseek(fd_sysfs, 0, SEEK_SET);
		} else if (len == -ENODATA) {
			break;
		} else {
			msg("%s: Error Reading lmh_cdsp sysfs\n",
				__func__);
			goto exit_handler;
		}
	} while (retry < MAX_RETRY_COUNT);

	if (write_data_into_partition)
	{
		len = 0;
		pos = 0;

		memcpy(write_buffer, read_buffer, PARTITION_SIZE_BYTES);

		do {
			len = write(fd_partition, &write_buffer[(size_t)pos],
				    (size_t)size_of_partition_data - (size_t)pos);
			if (len > 0) {
				pos += len;
			} else if (len == 0) {
				break;
			} else {
				msg("%s: Error writing limits-cdsp Partition\n",
					__func__);
				goto exit_handler;
			}
		} while (pos < size_of_partition_data
			 && pos < (ssize_t)PARTITION_SIZE_BYTES);

		info("%s: Data Written into Partition", __func__);
	}

exit_handler:
	if (fd_sysfs)
		close(fd_sysfs);

	if (fd_partition)
		close(fd_partition);

	info("%s: thread exit\n", __func__);

	return NULL;
}

void limits_partition_init(void)
{
	int rc = 0;

	rc = pthread_create(&limits_partition_thread, NULL,
		limits_write_data_to_partition, NULL);
	if (rc != 0)
		msg("%s: Error creating limits partition thread\n", __func__);

	return;
}
