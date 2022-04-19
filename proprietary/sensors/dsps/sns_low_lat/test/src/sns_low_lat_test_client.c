/******************************************************************************
* File: sns_low_lat_client_test.c
*
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
******************************************************************************/
#define VERIFY if
#define RUN_LIMIT             1
#define SAMPLE_PERIOD_US      1250 // 800 hz
#define FLAGS                 0 // eventually enable interrupts with "1"

#include "sns_low_lat_stream.h"
#include "sns_low_lat_buffer.h"

#include <string.h>
#include <unistd.h>
#include "rpcmem.h"
#include "verify.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
static uint8* buf_accel = 0;
static uint8* buf_gyro = 0;
static int low_lat_stream_handle_accel = 0;
static int low_lat_stream_handle_gyro = 0;
static pthread_t slpi_ssr_monitor;
static bool low_lat_stream_active = false;

void sig_int_handler(int signum)
{
  void* value_ptr;

  printf("Exiting early\n");

  // Stop the SLPI SSR Monitor thread
  low_lat_stream_active = false;
  pthread_join(slpi_ssr_monitor, &value_ptr);

   sns_low_lat_stream_stop(low_lat_stream_handle_accel);
  sns_low_lat_stream_stop(low_lat_stream_handle_gyro);

   sns_low_lat_stream_deinit(low_lat_stream_handle_accel);
  sns_low_lat_stream_deinit(low_lat_stream_handle_gyro);

  if(buf_accel)
  {
    rpcmem_free(buf_accel);
  }
    if(buf_gyro)
  {
    rpcmem_free(buf_gyro);
  }
  rpcmem_deinit();

  exit(0);
}

void* fwd_invoke_thread(void* arg)
{
  int rv = 0;

  printf("- Starting the SLPI forward invoke thread\n");

  while (low_lat_stream_active)
  {
    // Block for a second
    rv = sns_low_lat_stream_forward_block(1000);
    if (rv != 0)
    {
      printf("\nUnexpected exit from SLPI!\n\n");
      break;
    }
  }

  printf("- Stopping the SLPI forward invoke thread\n");

  return NULL;
}

int main(int argc, char* argv[])
{
  int duration = 10;
  int nErr = 0, len = 0x1000, fd;
  int offset = 0;
  int low_lat_err = 0;
  struct sigaction sig_int_act;
  void* value_ptr;

  printf("Starting sns_low_lat_client_test\n\n");

  // Handle signals (like Ctrl+C)
  sig_int_act.sa_handler = sig_int_handler;
  sigaction(SIGINT, &sig_int_act, NULL);

  if (argc > 1)
  {
    duration = atoi(argv[1]);
  }

  // Initialize the shared memory region
  rpcmem_init();
  printf("- Dual initialization of DSP\n");
  printf("- Allocate %d bytes from ION heap for accel\n", len);
  //VERIFY(0 != (buf_accel = rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, len)));
  VERIFY(0 != (buf_accel = rpcmem_alloc(RPCMEM_DEFAULT_HEAP, 0, len)));
  fd = rpcmem_to_fd(buf_accel);
  printf("- Accel fd %d buffer length %u and offset %u\n", fd, len, 0);
  VERIFY(0 == sns_low_lat_stream_init(fd, len, 0, &low_lat_stream_handle_accel));
  printf("- Allocate %d bytes from ION heap for gyro\n", len);
  //VERIFY(0 != (buf_gyro = rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, len)));
  VERIFY(0 != (buf_gyro = rpcmem_alloc(RPCMEM_DEFAULT_HEAP, 0, len)));
  fd = rpcmem_to_fd(buf_gyro);
  printf("- Gyro fd %d buffer length %u and offset %u\n", fd, len, 0);
  VERIFY(0 == sns_low_lat_stream_init(fd, len, 0, &low_lat_stream_handle_gyro));


  // Start the SLPI SSR monitor thread
  low_lat_stream_active = true;
  if (0 != pthread_create(&slpi_ssr_monitor, NULL, fwd_invoke_thread, NULL))
  {
    printf("- Failed to create the forward invoke thread for SLPI SSR monitoring!");
  }

  sns_low_lat_circ_buf* circ_buf_accel = (sns_low_lat_circ_buf*) buf_accel;
  sns_low_lat_circ_buf* circ_buf_gyro = (sns_low_lat_circ_buf*) buf_gyro;

  // Accel and gyro
  uint64 accel_suid = 0x4b118d0883777081;
  uint64 gyro_suid = 0x35bcdb5f5574ab55;


  //this loop is to simulate the real world scenario with qvr service; service will start running after boot and
  //will keep running entire lifetime; so init/deinit will be called during boot/reboot, start/stop will be called
  //for every client
  for(int run=0;run<RUN_LIMIT;++run)
  {
    float gyro_freq = 0, avg_gyro_freq = 0, accel_freq = 0, avg_accel_freq = 0;
    float min_gyro_freq = 0, max_gyro_freq = 0,  min_accel_freq = 0, max_accel_freq = 0;

    printf("- Starting the accel stream\n");
    VERIFY(0 == sns_low_lat_stream_start(low_lat_stream_handle_accel, &accel_suid, 1, SAMPLE_PERIOD_US, 0));
    printf("- Starting the gyro stream\n");
    VERIFY(0 == sns_low_lat_stream_start(low_lat_stream_handle_gyro, &gyro_suid, 1, SAMPLE_PERIOD_US, FLAGS));
    printf("- Start collecting data for %d seconds: run=%d\n\n\n", duration, run);

    for(int i=0;i<duration;++i)
    {
      // Sleep in between sampling cycles
      usleep(20000);

      // Blocking poll call that will unblock once the next sample is available
      sns_low_lat_stream_poll(low_lat_stream_handle_gyro);
      sns_low_lat_circ_buf_sample cur_sample, pre_sample, *cur_buf;
      int idx, idx_pre;
      float delta_time;

      // Print the gyro sample
      idx=circ_buf_gyro->header.info.most_recent_index;
      if (idx == 0) {
        idx_pre = circ_buf_gyro->header.info.max_samples - 1;
      }
      else {
        idx_pre = idx - 1;
      }
      memcpy((void *) &cur_sample,(void *) &(circ_buf_gyro->samples[idx]),sizeof(cur_sample));
      memcpy((void *) &pre_sample,(void *) &(circ_buf_gyro->samples[idx_pre]),sizeof(pre_sample));
      delta_time = (float)(cur_sample.data.timestamp - pre_sample.data.timestamp) / 19200;
      gyro_freq = 1000 / delta_time;
      avg_gyro_freq += gyro_freq;
      if (min_gyro_freq == 0 && max_gyro_freq == 0)
	      min_gyro_freq = max_gyro_freq = gyro_freq;
      if (min_gyro_freq > gyro_freq)
	      min_gyro_freq = gyro_freq;
      if (max_gyro_freq < gyro_freq)
	      max_gyro_freq = gyro_freq;
      //printf("address %x, %x\n", &(circ_buf_gyro->samples[idx]), &(circ_buf_gyro->samples[idx_pre]));
      printf("sensor %u idx %d idx_pre %d\n", cur_sample.data.sensor_type, idx, idx_pre);
      printf("sensor %u ts %llu (%f, %f, %f)\n", pre_sample.data.sensor_type, pre_sample.data.timestamp, pre_sample.data.sample[0], pre_sample.data.sample[1], pre_sample.data.sample[2]);
      printf("sensor %u ts %llu (%f, %f, %f)\n", cur_sample.data.sensor_type, cur_sample.data.timestamp, cur_sample.data.sample[0], cur_sample.data.sample[1], cur_sample.data.sample[2]);
      printf("sensor %u freq %f \n", cur_sample.data.sensor_type, gyro_freq);
      printf("sensor %u delta timestamp %f \n", cur_sample.data.sensor_type, delta_time);

      // Print the aceel sample
      idx=circ_buf_accel->header.info.most_recent_index ;
      if (idx == 0) {
        idx_pre = circ_buf_accel->header.info.max_samples - 1;
      }
      else {
        idx_pre = idx - 1;
      }
      memcpy((void *) &cur_sample,(void *) &(circ_buf_accel->samples[idx]),sizeof(cur_sample));
      memcpy((void *) &pre_sample,(void *) &(circ_buf_accel->samples[idx_pre]),sizeof(pre_sample));
      delta_time = (float)(cur_sample.data.timestamp - pre_sample.data.timestamp) / 19200;
      accel_freq = 1000 / delta_time;
      avg_accel_freq += accel_freq;
      if (min_accel_freq == 0 && max_accel_freq == 0)
	      min_accel_freq = max_accel_freq = accel_freq;
      if (min_accel_freq > accel_freq)
	      min_accel_freq = accel_freq;
      if (max_accel_freq < accel_freq)
	      max_accel_freq = accel_freq;
      //printf("address %x, %x\n", &(circ_buf_accel->samples[idx]), &(circ_buf_accel->samples[idx_pre]));
      printf("sensor %u idx %d idx_pre %d\n", cur_sample.data.sensor_type, idx, idx_pre);
      printf("sensor %u ts %llu (%f, %f, %f)\n", pre_sample.data.sensor_type, pre_sample.data.timestamp, pre_sample.data.sample[0], pre_sample.data.sample[1], pre_sample.data.sample[2]);
      printf("sensor %u ts %llu (%f, %f, %f)\n", cur_sample.data.sensor_type, cur_sample.data.timestamp, cur_sample.data.sample[0], cur_sample.data.sample[1], cur_sample.data.sample[2]);
      printf("sensor %u freq %f \n", cur_sample.data.sensor_type, gyro_freq);
      printf("sensor %u delta timestamp %f \n", cur_sample.data.sensor_type, delta_time);
    }

    avg_gyro_freq = avg_gyro_freq / duration;
    avg_accel_freq = avg_accel_freq / duration;
    printf("sensor gyroscope avg freq %f min_freq %f, max_freq %f\n", avg_gyro_freq, min_gyro_freq, max_gyro_freq);
    printf("sensor accel avg freq %f  min_freq %f max_freq %f\n", avg_accel_freq, min_accel_freq, max_accel_freq);
    printf("\n\n- Stop collecting data\n");
    VERIFY(0 == sns_low_lat_stream_stop(low_lat_stream_handle_gyro));
    VERIFY(0 == sns_low_lat_stream_stop(low_lat_stream_handle_accel));
    sleep(5);
  }

  printf("- Deinit\n");
  VERIFY(0 == sns_low_lat_stream_deinit(low_lat_stream_handle_gyro));
  VERIFY(0 == sns_low_lat_stream_deinit(low_lat_stream_handle_accel));

bail:
  // Stop the SLPI SSR Monitor thread
  low_lat_stream_active = false;
  pthread_join(slpi_ssr_monitor, &value_ptr);

  // Free the shared memory
 if(buf_accel)
  {
    rpcmem_free(buf_accel);
  }
  if(buf_gyro)
  {
    rpcmem_free(buf_gyro);
  }
  rpcmem_deinit();
  return nErr;
}
