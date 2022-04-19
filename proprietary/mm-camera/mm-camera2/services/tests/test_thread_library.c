/*============================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/* test_thread_library.c
 */

//Thread Service Library = TSL

//TSL Test includes
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "thread_services.h"
#include "debuglog.h"

//TSL Test definitions
#define CLIENT_MODULES 5
#define CLIENT_THREAD CLIENT_MODULES
#define JOBS_PER_CLIENT 5
#define INSTANCES_PER_CLIENT_JOB 5

//TST Test types
typedef void *(*job_func_t)(void *arg);
typedef void (*job_callback_t)(void *user_data);

typedef struct job_config_ {
  char job_name[128];
  int job_instances;
  job_func_t job_func;
  job_callback_t stopped_cb;
  char *job_data[INSTANCES_PER_CLIENT_JOB];
  unsigned int job_handles[INSTANCES_PER_CLIENT_JOB];
} job_config_t;

typedef struct client_config_ {
  char client_name[128];
  int client_jobs;
  job_callback_t flushed_cb;
  unsigned int job_sleep;
  job_config_t job_config[JOBS_PER_CLIENT];
} client_config_t;

typedef struct test_config_ {
  int client_modules;
  client_config_t client_config[CLIENT_MODULES];
} test_config_t;

typedef struct job_data_{
  int data_num1;
  int data_num2;
  int sleep_time;
  char *name;
} job_data_t;

//TSL Test configuration
//Session 1
void *sess1_client1_job1_func(void *arg);
void *sess1_client1_job2_func(void *arg);
void *sess1_client1_job3_func(void *arg);
void *sess1_client1_job4_func(void *arg);
void *sess1_client1_job5_func(void *arg);
void *sess1_client2_job1_func(void *arg);
void *sess1_client2_job2_func(void *arg);
void *sess1_client3_job1_func(void *arg);
void *sess1_client3_job2_func(void *arg);
void *sess1_client3_job3_func(void *arg);
void sess1_client1_job_flushed(void *arg);
void sess1_client2_job_flushed(void *arg);
void sess1_client3_job_flushed(void *arg);
void sess1_client1_job_stop(void *arg);
void sess1_client2_job_stop(void *arg);
void sess1_client3_job_stop(void *arg);
//Session 2
void *sess2_client1_job1_func(void *arg);
void *sess2_client1_job2_func(void *arg);
void *sess2_client1_job3_func(void *arg);
void *sess2_client1_job4_func(void *arg);
void *sess2_client1_job5_func(void *arg);
void sess2_client1_job_flushed(void *arg);
void sess2_client1_job_stop(void *arg);

static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t test_cond = PTHREAD_COND_INITIALIZER;

test_config_t tsl_test = {
  3,// 3 clients
  {
    { //client-1
      "Sess1-Client1",//client name
      5,//number of client jobs or threads
      sess1_client1_job_flushed,//job flushed callback
      3,//client jobs sleep time
      {
        {//client-1, job-1
          "Sess1-Client1-Job1",//job name
           1,//job instances
           sess1_client1_job1_func,//job worker func
           sess1_client1_job_stop,//job stopped func
           {"sess1_client1_job1_inst1"},//job data ptrs
           {0}//job handles
        },
        {//client-1, job-2
          "Sess1-Client1-Job2",//job name
           2,//job instances
           sess1_client1_job2_func,//job worker func
           sess1_client1_job_stop,//job stopped func
           {"sess1_client1_job2_inst1", "sess1_client1_job2_inst2"},//job data ptrs
           {0, 0}//job handles
        },
        {//client-1, job-3
          "Sess1-Client1-Job3",//job name
           3,//job instances
           sess1_client1_job3_func,//job worker func
           sess1_client1_job_stop,//job stopped func
           {"sess1_client1_job3_inst1", "sess1_client1_job3_inst2", "sess1_client1_job3_inst3"},//job data ptrs
           {0, 0, 0}//job handles
        },
        {//client-1, job-4
          "Sess1-Client1-Job4",//job name
           4,//job instances
           sess1_client1_job4_func,//job worker func
           sess1_client1_job_stop,//job stopped func
           {"sess1_client1_job4_inst1", "sess1_client1_job4_inst2", "sess1_client1_job4_inst3", "sess1_client1_job4_inst4"},//job data ptrs
           {0, 0, 0, 0}//job handles
        },
        {//client-1, job-5
          "Sess1-Client1-Job5",//job name
           5,//job instances
           sess1_client1_job5_func,//job worker func
           sess1_client1_job_stop,//job stopped func
           {"sess1_client1_job5_inst1", "sess1_client1_job5_inst2", "sess1_client1_job5_inst3", "sess1_client1_job5_inst4", "sess1_client1_job5_inst5"},//job data ptrs
           {0, 0, 0, 0, 0}//job handles
        }
      }
    },

    { //client-2
      "Sess1-Client2",//client name
      2,//number of client jobs or threads
      sess1_client2_job_flushed,//job flushed callback
      5,//client jobs sleep time
      {
        {//client-2, job-1
          "Sess1-Client2-Job1",//job name
           1,//job instances
           sess1_client2_job1_func,//job worker func
           sess1_client2_job_stop,//job stopped func
           {"sess1_client2_job1_inst1"},//job data ptrs
           {0}//job handles
        },
        {//client-2, job-2
          "Sess1-Client2-Job2",//job name
           2,//job instances
           sess1_client2_job2_func,//job worker func
           sess1_client2_job_stop,//job stopped func
           {"sess1_client2_job2_inst1", "sess1_client2_job2_inst2"},//job data ptrs
           {0, 0}//job handles
        }
      }
    },

    { //client-3
      "Sess1-Client3",//client name
      3,//number of client jobs or threads
      sess1_client3_job_flushed,//job flushed callback
      7,//client jobs sleep time
      {
        {//client-3, job-1
          "Sess1-Client3-Job1",//job name
           1,//job instances
           sess1_client3_job1_func,//job worker func
           sess1_client3_job_stop,//job stopped func
           {"sess1_client3_job1_inst1"},//job data ptrs
           {0}//job handles
        },
        {//client-3, job-2
          "Sess1-Client3-Job2",//job name
           2,//job instances
           sess1_client3_job2_func,//job worker func
           sess1_client3_job_stop,//job stopped func
           {"sess1_client3_job2_inst1", "sess1_client3_job2_inst2"},//job data ptrs
           {0, 0}//job handles
        },
        {//client-3, job-3
          "Sess1-Client3-Job3",//job name
           3,//job instances
           sess1_client3_job3_func,//job worker func
           sess1_client3_job_stop,//job stopped func
           {"sess1_client3_job3_inst1", "sess1_client3_job3_inst2", "sess1_client3_job3_inst3"},//job data ptrs
           {0, 0, 0}//job handles
        }
      }
    }
  }
};

test_config_t tsl_test_2 = {
  1,// 1 client
  {
    { //client-1
      "Sess2-Client1",//client name
      5,//number of client jobs or threads
      sess2_client1_job_flushed,//job flushed callback
      3,//client jobs sleep time
      {
        {//client-1, job-1
          "Sess2-Client1-Job1",//job name
           1,//job instances
           sess2_client1_job1_func,//job worker func
           sess2_client1_job_stop,//job stopped func
           {"sess2_client1_job1_inst1"},//job data ptrs
           {0}//job handles
        },
        {//client-1, job-2
          "Sess2-Client1-Job2",//job name
           2,//job instances
           sess2_client1_job2_func,//job worker func
           sess2_client1_job_stop,//job stopped func
           {"sess2_client1_job2_inst1", "sess2_client1_job2_inst2"},//job data ptrs
           {0, 0}//job handles
        },
        {//client-1, job-3
          "Sess2-Client1-Job3",//job name
           3,//job instances
           sess2_client1_job3_func,//job worker func
           sess2_client1_job_stop,//job stopped func
           {"sess2_client1_job3_inst1", "sess2_client1_job3_inst2", "sess2_client1_job3_inst3"},//job data ptrs
           {0, 0, 0}//job handles
        },
        {//client-1, job-4
          "Sess2-Client1-Job4",//job name
           4,//job instances
           sess2_client1_job4_func,//job worker func
           sess2_client1_job_stop,//job stopped func
           {"sess2_client1_job4_inst1", "sess2_client1_job4_inst2", "sess2_client1_job4_inst3", "sess2_client1_job4_inst4"},//job data ptrs
           {0, 0, 0, 0}//job handles
        },
        {//client-1, job-5
          "Sess2-Client1-Job5",//job name
           5,//job instances
           sess2_client1_job5_func,//job worker func
           sess2_client1_job_stop,//job stopped func
           {"sess2_client1_job5_inst1", "sess2_client1_job5_inst2", "sess2_client1_job5_inst3", "sess2_client1_job5_inst4", "sess2_client1_job5_inst5"},//job data ptrs
           {0, 0, 0, 0, 0}//job handles
        }
      }
    }
  }
};


//TSL test body
void *sess1_client1_job1_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client1_job2_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client1_job3_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client1_job4_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client1_job5_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client2_job1_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  TH_LOG_LOW("Returning...");
  return NULL;
}
void *sess1_client2_job2_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client3_job1_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client3_job2_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess1_client3_job3_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}

void sess1_client1_job_stop(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job stop callback for %s, Data1:%d, Data2:%d", job_data->name, job_data->data_num1, job_data->data_num2);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return;
}

void sess1_client2_job_stop(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job stop callback for %s, Data1:%d, Data2:%d", job_data->name, job_data->data_num1, job_data->data_num2);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return;
}

void sess1_client3_job_stop(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job stop callback for %s, Data1:%d, Data2:%d", job_data->name, job_data->data_num1, job_data->data_num2);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return;
}

void sess1_client1_job_flushed(void *arg)
{
  job_data_t *flush_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job flushed callback for %s, Data1:%d, Data2:%d", flush_data->name, flush_data->data_num1, flush_data->data_num2);

  usleep(flush_data->sleep_time * 1000);
  free(flush_data);

  return;
}

void sess1_client2_job_flushed(void *arg)
{
  job_data_t *flush_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job flushed callback for %s, Data1:%d, Data2:%d", flush_data->name, flush_data->data_num1, flush_data->data_num2);

  usleep(flush_data->sleep_time * 1000);
  free(flush_data);

  return;
}

void sess1_client3_job_flushed(void *arg)
{
  job_data_t *flush_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job flushed callback for %s, Data1:%d, Data2:%d", flush_data->name, flush_data->data_num1, flush_data->data_num2);

  usleep(flush_data->sleep_time * 1000);
  free(flush_data);

  return;
}

//sess-1 client-1 : Pump in jobs
void *sess1_client1_context(void *arg)
{
  th_err_t err;
  int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
  int num_jobs = 0;
  client_config_t *client_config = NULL;
  job_data_t *job_data = NULL;

  client_config = (client_config_t *)arg;
  if (NULL == client_config) {
    return NULL;
  }

  TH_LOG_ERR(" Sess1-Client-1 starts running, jobs:%d", client_config->client_jobs);

  for (num_jobs = 0; num_jobs < 1000; num_jobs++) {
    for (idx1 = 0; idx1 < client_config->client_jobs; idx1++) {
      for (idx2 = 0; idx2 < client_config->job_config[idx1].job_instances; idx2++) {
        job_data = (job_data_t *)malloc(sizeof(job_data_t));
        job_data->data_num1 = num_jobs+1;
        job_data->sleep_time = client_config->job_sleep;
        job_data->name = client_config->job_config[idx1].job_data[idx2];
        job_data->data_num2 = idx2+1;

        err = post_job_request(client_config->job_config[idx1].job_handles[idx2],
                               client_config->job_config[idx1].stopped_cb,
                               (void *)job_data,
                               2, 0, 0, 0);
        if (TH_SUCCESS == err) {
        }else {
          TH_LOG_ERR(" %s dispatch failed", job_data->name);
        }

        usleep(job_data->sleep_time * 1000);
      }
    }
  }

  TH_LOG_ERR(" Sess1-Client-1 exiting");

  return NULL;
}

//sess-1 client-2a : Pump in jobs, after N jobs wait for other client to flush
void *sess1_client2_context_a(void *arg)
{
  th_err_t err;
  int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
  int num_jobs = 0;
  client_config_t *client_config = NULL;
  job_data_t *job_data = NULL;

  client_config = (client_config_t *)arg;
  if (NULL == client_config) {
    return NULL;
  }

  TH_LOG_ERR(" Sess1-Client-2a starts running");

  for (num_jobs = 0; num_jobs < 200; num_jobs++) {
    job_data = (job_data_t *)malloc(sizeof(job_data_t));
    job_data->data_num1 = num_jobs+1;
    job_data->sleep_time = client_config->job_sleep;
    job_data->name = client_config->job_config[0].job_data[0];
    job_data->data_num2 = 0;

    err = post_job_request(client_config->job_config[0].job_handles[0],
                           client_config->job_config[0].stopped_cb,
                           (void *)job_data,
                           2, 0, 0, 0);
    if (TH_SUCCESS == err) {
    }else {
      TH_LOG_ERR(" Sess1-Client-2a job dispatch failed");
    }

    usleep(job_data->sleep_time * 1000);

    if (99 == num_jobs) {
      pthread_mutex_lock(&test_mutex);
      pthread_cond_signal(&test_cond);
      pthread_mutex_unlock(&test_mutex);
    }
  }

  TH_LOG_ERR(" Sess1-Client-2a exiting");

  return NULL;
}

//sess-1 client-2a : Wait till other client signals, then flush
void *sess1_client2_context_b(void *arg)
{
  th_err_t err;
  client_config_t *client_config = NULL;
  job_data_t *flush_data = NULL;

  client_config = (client_config_t *)arg;
  if (NULL == client_config) {
    return NULL;
  }

  TH_LOG_ERR(" Sess1-Client-2b starts running...");

  pthread_mutex_lock(&test_mutex);
  pthread_cond_wait(&test_cond, &test_mutex);
  pthread_mutex_unlock(&test_mutex);

  flush_data = (job_data_t *)malloc(sizeof(job_data_t));
  flush_data->sleep_time = client_config->job_sleep;
  flush_data->name = client_config->job_config[0].job_data[0];

  TH_LOG_ERR(" Sess1-Client-2b triggering flush, user_data:%p", flush_data);

  err = stop_job_request(client_config->job_config[0].job_handles[0],
                         (void *)flush_data, 0);
  if (TH_SUCCESS != err) {
    TH_LOG_ERR("Client-2b job flush failed");
  }

  TH_LOG_ERR(" Sess1-Client-2b exiting");

  return NULL;
}

//sess-1 client-3 : Pump in jobs, flush, pump in jobs
void *sess1_client3_context(void *arg)
{
  th_err_t err;
  int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
  int num_jobs = 0;
  client_config_t *client_config = NULL;
  job_data_t *job_data = NULL;
  job_data_t *flush_data = NULL;

  client_config = (client_config_t *)arg;
  if (NULL == client_config) {
    return NULL;
  }

  TH_LOG_ERR(" Sess1-Client-3 starts running, jobs:%d", client_config->client_jobs);

  for (num_jobs = 0; num_jobs < 1000; num_jobs++) {
    for (idx1 = 0; idx1 < client_config->client_jobs; idx1++) {
      for (idx2 = 0; idx2 < client_config->job_config[idx1].job_instances; idx2++) {
        job_data = (job_data_t *)malloc(sizeof(job_data_t));
        job_data->data_num1 = num_jobs+1;
        job_data->sleep_time = client_config->job_sleep;
        job_data->name = client_config->job_config[idx1].job_data[idx2];
        job_data->data_num2 = idx2+1;

        err = post_job_request(client_config->job_config[idx1].job_handles[idx2],
                               client_config->job_config[idx1].stopped_cb,
                               (void *)job_data,
                               2, 0, 0, 0);
        if (TH_SUCCESS != err) {
          TH_LOG_ERR(" %s dispatch failed", job_data->name);
        }

        usleep(job_data->sleep_time * 1000);

        if (499 == num_jobs) {
          flush_data = (job_data_t *)malloc(sizeof(job_data_t));
          flush_data->sleep_time = client_config->job_sleep;
          flush_data->name = client_config->job_config[idx1].job_data[idx2];

          err = stop_job_request(client_config->job_config[idx1].job_handles[idx2],
                                 (void *)flush_data, 0);
          if (TH_SUCCESS != err) {
            TH_LOG_ERR(" %s flush failed", flush_data->name);
          }
          usleep(1000);
        }
      }
    }
  }

  TH_LOG_ERR(" Sess1-Client-3 exiting");

  return NULL;
}

void run_session_1()
{
  int rc = 0;
  pthread_t tids[CLIENT_MODULES<<1];

  //create Sess 1 Client 1 context
  rc = pthread_create(&tids[0], NULL, sess1_client1_context, &tsl_test.client_config[0]);
  //create Sess 1 Client 2 contexts
  rc = pthread_create(&tids[2], NULL, sess1_client2_context_a, &tsl_test.client_config[1]);
  rc = pthread_create(&tids[3], NULL, sess1_client2_context_b, &tsl_test.client_config[1]);
  //create Sess 1 Client 3 context
  rc = pthread_create(&tids[4], NULL, sess1_client3_context, &tsl_test.client_config[2]);

  //close all Client contexts
  pthread_join(tids[0], NULL);
  pthread_join(tids[2], NULL);
  pthread_join(tids[3], NULL);
  pthread_join(tids[4], NULL);

  return;
}

void *sess2_client1_job1_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess2_client1_job2_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess2_client1_job3_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess2_client1_job4_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}
void *sess2_client1_job5_func(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Data1:%d, Data2:%d, Name:%s", job_data->data_num1, job_data->data_num2, job_data->name);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return NULL;
}

void sess2_client1_job_stop(void *arg)
{
  job_data_t *job_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job stop callback for %s, Data1:%d, Data2:%d", job_data->name, job_data->data_num1, job_data->data_num2);

  usleep(job_data->sleep_time * 1000);
  free(job_data);

  return;
}

void sess2_client1_job_flushed(void *arg)
{
  job_data_t *flush_data = (job_data_t *)arg;

  TH_LOG_ERR(" Job flushed callback for %s, Data1:%d, Data2:%d", flush_data->name, flush_data->data_num1, flush_data->data_num2);

  usleep(flush_data->sleep_time * 1000);
  free(flush_data);

  return;
}

//sess-2 client-1 : Pump in jobs (blocking), flush(blocking), pump in jobs(blocking)
void *sess2_client1_context(void *arg)
{
  th_err_t err;
  int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
  int num_jobs = 0;
  client_config_t *client_config = NULL;
  job_data_t *job_data = NULL;
  job_data_t *flush_data = NULL;

  client_config = (client_config_t *)arg;
  if (NULL == client_config) {
    return NULL;
  }

  TH_LOG_ERR(" Sess2-Client-1 starts running, jobs:%d", client_config->client_jobs);

  for (num_jobs = 0; num_jobs < 100; num_jobs++) {
    for (idx1 = 0; idx1 < client_config->client_jobs; idx1++) {
      for (idx2 = 0; idx2 < client_config->job_config[idx1].job_instances; idx2++) {
        job_data = (job_data_t *)malloc(sizeof(job_data_t));
        job_data->data_num1 = num_jobs+1;
        job_data->sleep_time = client_config->job_sleep;
        job_data->name = client_config->job_config[idx1].job_data[idx2];
        job_data->data_num2 = idx2+1;

        err = post_job_request(client_config->job_config[idx1].job_handles[idx2],
                               client_config->job_config[idx1].stopped_cb,
                               (void *)job_data,
                               2, 0, 0, 1);
        if (TH_SUCCESS != err) {
          TH_LOG_ERR(" %s dispatch failed", job_data->name);
        }

        usleep(job_data->sleep_time * 1000);

        if (49 == num_jobs) {
          flush_data = (job_data_t *)malloc(sizeof(job_data_t));
          flush_data->sleep_time = client_config->job_sleep;
          flush_data->name = client_config->job_config[idx1].job_data[idx2];

          err = stop_job_request(client_config->job_config[idx1].job_handles[idx2],
                                 (void *)flush_data, 1);
          if (TH_SUCCESS != err) {
            TH_LOG_ERR(" %s flush failed", flush_data->name);
          }
          usleep(1000);
        }
      }
    }
  }

  TH_LOG_ERR(" Sess2-Client-1 exiting");

  return NULL;
}

void run_session_2()
{
  int rc = 0;
  pthread_t tids[CLIENT_MODULES<<1];

  //create Sess 2 Client 1 context
  rc = pthread_create(&tids[0], NULL, sess2_client1_context, &tsl_test_2.client_config[0]);

  //close all Client contexts
  pthread_join(tids[0], NULL);

  return;
}

int main(int argc __unused, char * argv[] __unused)
{
  th_err_t err;
  int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;

  TH_LOG_ERR(" Welcome to Thread Library test...");

  for (idx1 = 0; idx1 < tsl_test.client_modules; idx1++) {
    TH_LOG_ERR("Client Name: %s", tsl_test.client_config[idx1].client_name);
    for (idx2 = 0; idx2 < tsl_test.client_config[idx1].client_jobs; idx2++) {
      TH_LOG_ERR("\tJob Name: %s", tsl_test.client_config[idx1].job_config[idx2].job_name);
    }
  }

  for (idx1 = 0; idx1 < tsl_test_2.client_modules; idx1++) {
    TH_LOG_ERR("Client Name: %s", tsl_test_2.client_config[idx1].client_name);
    for (idx2 = 0; idx2 < tsl_test_2.client_config[idx1].client_jobs; idx2++) {
      TH_LOG_ERR("\tJob Name: %s", tsl_test_2.client_config[idx1].job_config[idx2].job_name);
    }
  }

  err = init_thread_services();
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Init success");
  } else {
    goto exit;
  }

  //create thread session 1
  err = create_session(1);
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Session 1 Create success");
  } else {
    goto exit;
  }
  //register jobs for thread session 1
  for (idx1 = 0; idx1 < tsl_test.client_modules; idx1++) {
    for (idx2 = 0; idx2 < tsl_test.client_config[idx1].client_jobs; idx2++) {
      for (idx3 = 0; idx3 < tsl_test.client_config[idx1].job_config[idx2].job_instances; idx3++) {
        tsl_test.client_config[idx1].job_config[idx2].job_handles[idx3] =
                  register_job_request(1,
                                       tsl_test.client_config[idx1].job_config[idx2].job_func,
                                       tsl_test.client_config[idx1].job_config[idx2].job_name,
                                       tsl_test.client_config[idx1].flushed_cb,
                                       1);
        TH_LOG_ERR("Job Name: %s, Job Instance: %d, Handle: %x",
                                 tsl_test.client_config[idx1].job_config[idx2].job_name,
                                 idx3+1,
                                 tsl_test.client_config[idx1].job_config[idx2].job_handles[idx3]);
      }
    }
  }

  //create thread session 2
  err = create_session(2);
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Session 2 Create success");
  } else {
    goto exit;
  }
  //register jobs for thread session 2
  for (idx1 = 0; idx1 < tsl_test_2.client_modules; idx1++) {
    for (idx2 = 0; idx2 < tsl_test_2.client_config[idx1].client_jobs; idx2++) {
      for (idx3 = 0; idx3 < tsl_test_2.client_config[idx1].job_config[idx2].job_instances; idx3++) {
        tsl_test_2.client_config[idx1].job_config[idx2].job_handles[idx3] =
                  register_job_request(2,
                                       tsl_test_2.client_config[idx1].job_config[idx2].job_func,
                                       tsl_test_2.client_config[idx1].job_config[idx2].job_name,
                                       tsl_test_2.client_config[idx1].flushed_cb,
                                       1);
        TH_LOG_ERR("Job Name: %s, Job Instance: %d, Handle: %x",
                                 tsl_test_2.client_config[idx1].job_config[idx2].job_name,
                                 idx3+1,
                                 tsl_test_2.client_config[idx1].job_config[idx2].job_handles[idx3]);
      }
    }
  }

  //run thread session 1
  run_session_1();

  //run thread session 2
  run_session_2();

exit:
  err = destroy_session(1);
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Session 1 Destroy success");
  }

  err = destroy_session(2);
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Session 2 Destroy success");
  }

  err = deinit_thread_services();
  if (TH_SUCCESS == err) {
    TH_LOG_ERR("Thread Service Deinit success");
  }

  return 0;
}

