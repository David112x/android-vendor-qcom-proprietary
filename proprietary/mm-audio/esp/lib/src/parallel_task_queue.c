/*============================================================================
* Copyright (c) 2017, 2018 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/

//Following GNU  macro define is needed for setting the thread affinity using sched.h
#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parallel_task_queue.h"
#include "sys/resource.h"
#include <semaphore.h>
#include <sched.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>


#define TASK_SPACE 20
#define ANDROID_ESP_THREAD_AUDIO -8
static int atrace_marker_fd = -1;
#define ATRACE_MESSAGE_LENGTH 256


typedef enum {
    EMPTY = 0,
    TO_RUN,
    RUNNING,
    DONE
} task_state_t;

typedef struct {
    unsigned int id;
    task_state_t state;
    void(*function)(void*);
    void *arg;
} task_t;

typedef struct ptq_data_t {
    pthread_t *threads;
    task_t *tasks;
    int num_threads;
    int num_tasks;
    int max_num_tasks;
    int num_to_run;
    int num_done;
    int num_ec_done;
    unsigned int seqno;
    int stop_threads;
    pthread_mutex_t mtx;
    pthread_cond_t run_cv;
    pthread_cond_t done_cv;
    // task structures for 4 ec Tasks. These will hold the function and arguments to pass on to corrresponding threads when woken up
    task_t ecTask1, ecTask2, ecTask3, ecTask4;
    pthread_t *ECthreads;
    //Each Semaphore to signal eaach of 4 EC threads to start running and waiting when task is done
    sem_t ecSem1, ecSem2, ecSem3, ecSem4;
    int num_ec_threads;
    //Semaphore to keep count of tasks done. Indicate or signal to main thread to increment count and check if it can continue or need to wait
    sem_t ecDoneCounter;

} ptq_data_t;


static void atrace_begin_body(const char* name)
{
    char buf[ATRACE_MESSAGE_LENGTH];

    int len = snprintf(buf, sizeof(buf), "B|%d|%s", getpid(), name);
    if (len >= (int) sizeof(buf)) {
        printf("Truncated name in %s: %s\n", __FUNCTION__, name);
        len = sizeof(buf) - 1;
    }
    if (atrace_marker_fd != -1)
    {
        write(atrace_marker_fd, buf, len);
    }
}

static void atrace_end_body()
{
    char c = 'E';
    if (atrace_marker_fd != -1)
    {
        write(atrace_marker_fd, &c, 1);
    }
}

/* Passes on the function ptr and arguments ptr to thread 0 and signals thread to execute this task on it */
static unsigned int ptq_run_ec1(void *handle, void (*func)(void*), void *arg)
{
    unsigned int id =1 ;

    ptq_data_t *ptq_data = (ptq_data_t *)handle;

    if (ptq_data == NULL) {
        return 0;
    }

    /* Set up the shared data structure with function and arguments that needs to be run by thread 0*/
    ptq_data->ecTask1.function = func;
    ptq_data->ecTask1.arg = arg;
    ptq_data->ecTask1.state = TO_RUN;

    /* Increments the semaphore and signal the EC thread 0 i.e., waiting on this semaphore to go to value 1 */
    sem_post(&ptq_data->ecSem1);

    return id;
}

/* Actual thread function, ESP starts running when thread is created and waits for its semaphore to go to 1 to continue further */
static void *thread_func_ec1(void *data)
{
    ptq_data_t *ptq_data = (ptq_data_t *)data;
    int rc;
    int s;
        struct sched_param sched_param;
    int policy = SCHED_FIFO;

    s= prctl(PR_SET_NAME, (unsigned long)"ESP ", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_ESP_THREAD_AUDIO);

       // setting real time property
    sched_param.sched_priority = sched_get_priority_min(policy);
    rc = sched_setscheduler(0, policy, &sched_param);
    if (rc != 0) {
             printf(" Failed to set realtime priority \n");
     }

    while(1){
        /* Decrements semaphore state if >0, if not waits untill other main thread (or any other thread) makes it >0  */

        sem_wait(&ptq_data->ecSem1);

        if (ptq_data->stop_threads) {
            break;
        }
        /* Execute the task 1 */
        atrace_begin_body("ESP Thread");
        ptq_data->ecTask1.function(ptq_data->ecTask1.arg);
        atrace_end_body();

        /* Increments the semaphore and signal the main thread i.e., waiting on this semaphore to go to value >1 */
        //sem_post(&ptq_data->ecDoneCounter);
    }

done:
    pthread_exit(NULL);
}

void ptq_wait_all_ec(void *handle, unsigned int numberParallelTasks)
{
    int i, rc;
    ptq_data_t *ptq_data = (ptq_data_t *)handle;

    if (ptq_data == NULL) {
        return;
    }

    //while (!ptq_data->stop_threads && (ptq_data->num_ec_done < ptq_data->num_ec_threads)) {
    while (!ptq_data->stop_threads && (ptq_data->num_ec_done <numberParallelTasks)) {
        /* Decrements its semaphore state if >0, if not waits untill other main thread (or any other thread) makes it >0  */
        sem_wait(&ptq_data->ecDoneCounter);
        ptq_data->num_ec_done++;
    }
    if (!ptq_data->stop_threads) {

        if (ptq_data->num_ec_done != numberParallelTasks) {
            printf("Inconsistent data, num_ec_done %d should have been 4 for 4 parallel EC TASKS\n", ptq_data->num_ec_done);
        }
    }
    // set the number of tasks done shared varibalke as zero for the next set of EC data processing
    ptq_data->num_ec_done = 0;

done:
    return;
}

task_queue_obj * init_parallel_task_queue(void)
{
    int rc = 0;
    int i;
    ptq_data_t *ptq_data = NULL;
    task_queue_obj *obj = NULL;
    cpu_set_t cpuset;
    struct sched_param sched_param;
    int policy = SCHED_FIFO;
    int atrace_enabled_tags;
    int num_threads = 0;
    int num_ec_threads = 1;

    /* Memory allocation and initialization */
    ptq_data = (ptq_data_t *)malloc(sizeof(ptq_data_t));
    if (ptq_data == NULL) {
        goto err;
    }
    memset(ptq_data, 0, sizeof(ptq_data_t));

    ptq_data->max_num_tasks = TASK_SPACE;
    ptq_data->tasks = (task_t *)malloc(ptq_data->max_num_tasks * sizeof(task_t));
    if (ptq_data->tasks == NULL) {
        goto err;
    }
    memset(ptq_data->tasks, 0, ptq_data->max_num_tasks * sizeof(task_t));

    ptq_data->threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    ptq_data->ECthreads = (pthread_t *)malloc(num_ec_threads * sizeof(pthread_t));

    if (ptq_data->threads == NULL) {
        goto err;
    }
    if (ptq_data->ECthreads == NULL) {
        goto err;
    }
    memset(ptq_data->threads, 0, num_threads * sizeof(pthread_t));
    memset(ptq_data->ECthreads, 0,  num_ec_threads * sizeof(pthread_t));

    ptq_data->num_tasks = 0;
    ptq_data->num_to_run = 0;
    ptq_data->num_done = 0;
    ptq_data->seqno = 1;
    ptq_data->stop_threads = 0;
    ptq_data->num_ec_done = 0;
    ptq_data->num_ec_threads=num_ec_threads;

    /* Allocate object to be returned */
    obj = (task_queue_obj *)malloc(sizeof(task_queue_obj));
    if (obj == NULL) {
        goto err;
    }

    obj->run_ec1 = ptq_run_ec1;
    obj->wait_all_ec = ptq_wait_all_ec;

    obj->handle = ptq_data;


    /*initialize the semaphore initial values to zero, so that all threads will be waiting initially */
    sem_init(&ptq_data->ecSem1, 0, 0);
    sem_init(&ptq_data->ecSem2, 0, 0);
    sem_init(&ptq_data->ecSem3, 0, 0);
    sem_init(&ptq_data->ecSem4, 0, 0);
    sem_init(&ptq_data->ecDoneCounter, 0, 0);


    /* Initialize mutex, condition vars, and threads */
    rc = pthread_mutex_init(&ptq_data->mtx, NULL);
    if (rc != 0) {
        goto err;
    }
    rc = pthread_cond_init(&ptq_data->run_cv, NULL);
    if (rc != 0) {
        goto err1;
    }
    rc = pthread_cond_init(&ptq_data->done_cv, NULL);
    if (rc != 0) {
        goto err2;
    }

    ptq_data->num_threads = 0;


    // Create num_ec_threads, threads for EC processing and assign core Affinity
    for (i=0; i < num_ec_threads; i++) {
        if(0==i)
        {
            // Create EC thread0
            rc = pthread_create(&ptq_data->ECthreads[0], NULL, thread_func_ec1, ptq_data);
            if (rc != 0) {
                goto err3;
            }
        }
        //CPU affinity assignment for EC threads
        CPU_ZERO(&cpuset);
        CPU_SET(3, &cpuset);
#ifdef LINUX_ENABLED
        if (pthread_setaffinity_np(ptq_data->ECthreads[i], sizeof(cpuset), &cpuset) != 0)
            printf("\n *CPU Affinity allocation failed for ESP thread %d , Default Assignment \n", i);

        pthread_getaffinity_np(ptq_data->ECthreads[i], sizeof(cpuset), &cpuset);
        printf("Set returned by pthread_getaffinity_np() for ESP thread %d contained:\n", i);
#else
        if(sched_setaffinity(ptq_data->ECthreads[i], sizeof(cpuset), &cpuset) !=0)
            printf("\n *CPU Affinity allocation failed for ESP thread %d , Default Assignment \n",i);

        sched_getaffinity(ptq_data->ECthreads[i],sizeof(cpuset), &cpuset);
        printf("Set returned by sched_getaffinity() for ESP thread %d contained:\n",i);
#endif
        for (int j = 0 ; j < CPU_SETSIZE ; j++)
            if (CPU_ISSET(j, &cpuset))
                printf("    CPU %d \n", j);
    }//for close for EC threads creation and affinity assignment

    atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
    if (atrace_marker_fd == -1) {
        printf ("Error opening trace file: %s (%d)", strerror(errno), errno);
        atrace_enabled_tags = 0;
    }

    return obj;

err3:
    /* Stop any running threads */
    ptq_data->stop_threads = 1;
    (void)pthread_cond_broadcast(&ptq_data->run_cv);
    (void)pthread_cond_broadcast(&ptq_data->done_cv);
    for (i=0; i < ptq_data->num_threads; i++) {
        (void)pthread_join(ptq_data->threads[i], NULL);
    }
    //Send signals to EC threads to join main if any created
    sem_post(&ptq_data->ecSem1);
    sem_post(&ptq_data->ecSem2);
    sem_post(&ptq_data->ecSem3);
    sem_post(&ptq_data->ecSem4);
    for (i=0; i < num_ec_threads; i++) {
        (void)pthread_join(ptq_data->ECthreads[i], NULL);
    }
    (void) pthread_cond_destroy(&ptq_data->done_cv);
err2:
    (void) pthread_cond_destroy(&ptq_data->run_cv);

err1:
    (void) pthread_mutex_destroy(&ptq_data->mtx);

err:
    if (ptq_data != NULL) {
        free(ptq_data->tasks);
        free(ptq_data->threads);
        free(ptq_data);
    }
    if (obj != NULL) {
        free(obj);
    }
    return NULL;
}


void deinit_parallel_task_queue(task_queue_obj *obj)
{
    if (obj != NULL) {
        if (obj->handle != NULL) {

            ptq_data_t *ptq_data = (ptq_data_t *)obj->handle;
            int i;
            /* Stop any running threads */
            ptq_data->stop_threads = 1;

            //Send signals to EC threads to join main
            sem_post(&ptq_data->ecSem1);
            sem_post(&ptq_data->ecSem2);
            sem_post(&ptq_data->ecSem3);
            sem_post(&ptq_data->ecSem4);
            sem_post(&ptq_data->ecDoneCounter);

            for (i=0; i <ptq_data-> num_ec_threads; i++) {
                (void)pthread_join(ptq_data->ECthreads[i], NULL);
            }

            //send signals to sva threads to join
            (void)pthread_cond_broadcast(&ptq_data->run_cv);
            (void)pthread_cond_broadcast(&ptq_data->done_cv);
            for (i=0; i < ptq_data->num_threads; i++) {
                (void)pthread_join(ptq_data->threads[i], NULL);
            }
            (void) pthread_cond_destroy(&ptq_data->done_cv);
            (void) pthread_cond_destroy(&ptq_data->run_cv);
            (void) pthread_mutex_destroy(&ptq_data->mtx);

            sem_destroy(&ptq_data->ecSem1);
            sem_destroy(&ptq_data->ecSem2);
            sem_destroy(&ptq_data->ecSem3);
            sem_destroy(&ptq_data->ecSem4);
            sem_destroy(&ptq_data->ecDoneCounter);

            free(ptq_data->tasks);
            free(ptq_data->threads);
            free(ptq_data);

        }
        free(obj);
    }

}
