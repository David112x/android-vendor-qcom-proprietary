/*============================================================================
  @file sns_espplus_testapp.c

  @brief
  Test the QTI Hybrid approach at the HAL layer.

  Copyright (c) 2013, 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ==========================================================================*/

#include "hardware/sensors.h"
#include <cutils/native_handle.h>
#include <hardware/hardware.h>
#include <hardware/gralloc1.h>

#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <inttypes.h>

#define OPTSTRING "h:"
#define COMMAND_CHARS "adsfbe12345678"

#define DEFAULT_SENSOR_TYPE       1
#define DEFAULT_WAKEUP_FLAG       'd'
#define DEFAULT_SAMPLING_RATE_HZ  5
#define DEFAULT_REPORT_RATE_HZ    0
#define DEFAULT_OUTPUT_FILE       "/data/local/sns_hal_batch.out"
#define OUTPUT_TO_STDOUT          ""
#define DEFAULT_DURATION_SECS     10

// copied from sensors_hal.h
#define QTI_SENSOR_TYPE_BASE       33171000

//#define DIRECT_MEM_SIZE           0x4000
#define EVENT_SIZE                124
#define EVENT_COUNT               10240
#define DIRECT_MEM_SIZE           EVENT_COUNT * EVENT_SIZE// Use the same size as CTS
#define DIRECT_RATE_LEVEL         SENSOR_DIRECT_RATE_NORMAL
//#define DIRECT_RATE_LEVEL         SENSOR_DIRECT_RATE_FAST
//#define DIRECT_RATE_LEVEL         SENSOR_DIRECT_RATE_VERY_FAST
#define DIRECT_RATE_LEVEL1         SENSOR_DIRECT_RATE_VERY_FAST
#define DIRECT_RATE_LEVEL2         SENSOR_DIRECT_RATE_FAST
//#define DIRECT_SENSOR             0 // accel
//#define DIRECT_SENSOR             8 // uncal gyro
//#define DIRECT_SENSOR             11 // uncal mag
#define DIRECT_SENSOR             2 // gyro
//#define DIRECT_SENSOR             10 // mag
#define DIRECT_SENSOR1            8 // uncal gyro
#define DIRECT_SLEEP_DURATION_US  10*1000000 // 10 seconds

#ifdef SNS_LA_SIM
#define HAL_LIB_PATH ""
#else
#  if defined(__LP64__)
#    if defined (USES_FULL_TREBLE)
#      define HAL_LIB_PATH "/vendor/lib64/"
#    else
#      define HAL_LIB_PATH "/vendor/lib64/hw/"
#    endif
#  else
#    if defined (USES_FULL_TREBLE)
#      define HAL_LIB_PATH "/vendor/lib/"
#    else
#      define HAL_LIB_PATH "/vendor/lib/hw/"
#    endif
#  endif
#endif

#ifdef USES_FULL_TREBLE
  #define HAL_LIB_NAME HAL_LIB_PATH"sensors.ssc.so"
#else
  #define HAL_LIB_NAME HAL_LIB_PATH"sensors."BOARD_PLATFORM".so"
#endif

#define GRALLOC_LIB_NAME HAL_LIB_PATH"gralloc."BOARD_PLATFORM".so"

#define NSEC_PER_MS         1000000
#define NSEC_PER_S          1000000000
#define HZ_TO_NSEC(hz)      (1000000000LL/(hz))


static int sensorEventsRecvd = 0;

static int direct_report_multi_client_test = 0; //1;
static int direct_report_reenable_test = 0; //1;
static int direct_report_enable_monitor_thread = 0; //1;
static int direct_report_reconfigure_test = 0; //1;
static int direct_report_double_register_test = 1;
static pthread_t direct_report_monitor;
bool channel_running = 0;

// Initialize the gralloc1 device and functions
gralloc1_device_t* gralloc1_device = NULL;
GRALLOC1_PFN_CREATE_DESCRIPTOR gralloc1_create_descriptor = NULL;
GRALLOC1_PFN_DESTROY_DESCRIPTOR gralloc1_destroy_descriptor = NULL;
GRALLOC1_PFN_SET_DIMENSIONS gralloc1_set_dimensions = NULL;
GRALLOC1_PFN_SET_FORMAT gralloc1_set_format = NULL;
GRALLOC1_PFN_SET_LAYER_COUNT gralloc1_set_layer_count = NULL;
GRALLOC1_PFN_SET_PRODUCER_USAGE gralloc1_set_producer_usage = NULL;
GRALLOC1_PFN_ALLOCATE gralloc1_allocate = NULL;
GRALLOC1_PFN_RETAIN gralloc1_retain = NULL;
GRALLOC1_PFN_RELEASE gralloc1_release = NULL;
GRALLOC1_PFN_LOCK gralloc1_lock = NULL;
GRALLOC1_PFN_UNLOCK gralloc1_unlock = NULL;

struct sensors_module_t *hal_sym;

struct sensors_poll_device_1 *dev;
struct sensors_poll_device_t *dev_old;
struct sensor_t *list = NULL;
int sensor_count;

typedef enum
  {
    ACTIVATE = 0,
    DEACTIVATE,
    BATCH,
    EXIT,
    NUM_COMMANDS
  } testapp_command_e;

typedef enum
  {
    INACTIVE = 0,
    STREAMING,
    BATCHING
  } sensor_operating_state_e;

static char *handle_to_string( struct sensor_t *list, int handle )
{
  struct sensor_t *list_p = list;

  while( list_p->name ) {
    if( list_p->handle == handle ) {
      return (char*)list_p->name;
    }
    list_p++;
  }
  return "<no-name>";
}

static const char *type_to_string( int type )
{
  return (type == SENSOR_TYPE_ACCELEROMETER) ? SENSOR_STRING_TYPE_ACCELEROMETER :
    (type == SENSOR_TYPE_GEOMAGNETIC_FIELD) ? SENSOR_STRING_TYPE_MAGNETIC_FIELD :
    (type == SENSOR_TYPE_ORIENTATION) ? SENSOR_STRING_TYPE_ORIENTATION :
    (type == SENSOR_TYPE_GYROSCOPE) ? SENSOR_STRING_TYPE_GYROSCOPE :
    (type == SENSOR_TYPE_LIGHT) ? SENSOR_STRING_TYPE_LIGHT :
    (type == SENSOR_TYPE_PRESSURE) ? SENSOR_STRING_TYPE_PRESSURE :
    (type == SENSOR_TYPE_TEMPERATURE) ? SENSOR_STRING_TYPE_TEMPERATURE :
    (type == SENSOR_TYPE_PROXIMITY) ? SENSOR_STRING_TYPE_PROXIMITY :
    (type == SENSOR_TYPE_GRAVITY) ? SENSOR_STRING_TYPE_GRAVITY :
    (type == SENSOR_TYPE_LINEAR_ACCELERATION) ? SENSOR_STRING_TYPE_LINEAR_ACCELERATION :
    (type == SENSOR_TYPE_ROTATION_VECTOR) ? SENSOR_STRING_TYPE_ROTATION_VECTOR :
    (type == SENSOR_TYPE_RELATIVE_HUMIDITY) ? SENSOR_STRING_TYPE_RELATIVE_HUMIDITY :
    (type == SENSOR_TYPE_AMBIENT_TEMPERATURE) ? SENSOR_STRING_TYPE_AMBIENT_TEMPERATURE :
    (type == SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED) ? SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED :
    (type == SENSOR_TYPE_GAME_ROTATION_VECTOR) ? SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR :
    (type == SENSOR_TYPE_GYROSCOPE_UNCALIBRATED) ? SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED :
    (type == SENSOR_TYPE_SIGNIFICANT_MOTION) ? SENSOR_STRING_TYPE_SIGNIFICANT_MOTION :
    (type == SENSOR_TYPE_STEP_DETECTOR) ? SENSOR_STRING_TYPE_STEP_DETECTOR :
    (type == SENSOR_TYPE_STEP_COUNTER) ? SENSOR_STRING_TYPE_STEP_COUNTER :
    (type == SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR) ? SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR :
    (type == SENSOR_TYPE_HEART_RATE) ? SENSOR_STRING_TYPE_HEART_RATE :
    (type == SENSOR_TYPE_TILT_DETECTOR) ? SENSOR_STRING_TYPE_TILT_DETECTOR :
    (type == SENSOR_TYPE_WAKE_GESTURE) ? SENSOR_STRING_TYPE_WAKE_GESTURE :
    (type == SENSOR_TYPE_GLANCE_GESTURE) ? SENSOR_STRING_TYPE_GLANCE_GESTURE :
    (type == SENSOR_TYPE_PICK_UP_GESTURE) ? SENSOR_STRING_TYPE_PICK_UP_GESTURE :
    (type == SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED) ? SENSOR_STRING_TYPE_ACCELEROMETER_UNCALIBRATED :
#ifdef SENSORS_DEVICE_API_VERSION_1_1
    (type == SENSOR_TYPE_META_DATA) ? "Meta Data (FLUSH_CMPLT)" :
#endif
    (type == QTI_SENSOR_TYPE_BASE) ? "com.qti.sensor.basic_gestures" :
    (type == QTI_SENSOR_TYPE_BASE + 1) ? "com.qti.sensor.tap" :
    (type == QTI_SENSOR_TYPE_BASE + 2) ? "com.qti.sensor.facing" :
    (type == QTI_SENSOR_TYPE_BASE + 3) ? "com.qti.sensor.tilt" :
    (type == QTI_SENSOR_TYPE_BASE + 4) ? "com.qti.sensor.fns" :
    (type == QTI_SENSOR_TYPE_BASE + 5) ? "com.qti.sensor.bte" :
    (type == QTI_SENSOR_TYPE_BASE + 6) ? "com.qti.sensor.amd" :
    (type == QTI_SENSOR_TYPE_BASE + 7) ? "com.qti.sensor.rmd" :
    (type == QTI_SENSOR_TYPE_BASE + 8) ? "com.qti.sensor.vmd" :
    (type == QTI_SENSOR_TYPE_BASE + 9) ? "com.qti.sensor.pedometer" :
    (type == QTI_SENSOR_TYPE_BASE + 10) ? "com.qti.sensor.pam" :
    (type == QTI_SENSOR_TYPE_BASE + 11) ? "com.qti.sensor.motion_accel" :
    (type == QTI_SENSOR_TYPE_BASE + 12) ? "com.qti.sensor.cmc" :
    (type == QTI_SENSOR_TYPE_BASE + 13) ? "com.qti.sensor.rgb" :
    (type == QTI_SENSOR_TYPE_BASE + 14) ? "com.qti.sensor.ir_gesture" :
    (type == QTI_SENSOR_TYPE_BASE + 15) ? "com.qti.sensor.sar" :
    (type == QTI_SENSOR_TYPE_BASE + 16) ? "com.qti.sensor.hall_effect" :
    (type == QTI_SENSOR_TYPE_BASE + 17) ? "com.qti.sensor.fast_amd" :
    (type == QTI_SENSOR_TYPE_BASE + 18) ? "com.qti.sensor.uv" :
    (type == QTI_SENSOR_TYPE_BASE + 19) ? "com.qti.sensor.thermopile" :
    (type == QTI_SENSOR_TYPE_BASE + 20) ? "com.qti.sensor.cct" :
    "Unknown";
}

static const char *sensor_to_stringType (struct sensor_t const* sensor)
{
  return *(sensor->stringType) != 0 ? sensor->stringType : type_to_string(sensor->type);
}


// Print out the usage statement in interactive mode
void print_usage_msg( struct sensor_t const *list )
{
  int count = 0;
  struct sensor_t const *list_p = list;

  fprintf(stderr,"Sensors HAL TEST APP, version 1\n");
  fprintf(stderr,"Usage:\n");
  fprintf(stderr,"\tChoose a sensor to interact with by inputting the sensor's type and whether or not is the wakeup version as shown in the 'Sensors list'\n");
  fprintf(stderr,"\tNext, choose a command, by inputting one of the following characters:\n");
  fprintf(stderr,"\t\ta - Activate the sensor that was previously chosen.\n");
  fprintf(stderr,"\t\td - Deactivate the sensor.\n");
  fprintf(stderr,"\t\tf - Flush the sensor.\n");
  fprintf(stderr,"\t\tb - Batch. The program will prompt for additional information.\n");
  fprintf(stderr,"\t\te - Exit\n");
  fprintf(stderr,"Sensors list:\n");
  for (count = 0; count < sensor_count; count++ ) {
       fprintf(stderr,"\t[Type:%2d] (%s) Name:%s Vendor:%s Version:%d Handle:%d\n",
            list_p->type, type_to_string(list_p->type), list_p->name, list_p->vendor, list_p->version, list_p->handle);
    list_p++;
  }
  fprintf(stderr,"\n");
}

void print_sensor( struct sensor_t const* sensor)
{
  char wakeup_flag = (sensor->flags & SENSOR_FLAG_WAKE_UP) != 0 ? 'w' : 'n';
  fprintf(stderr, "[Type: %2d] %s (%s)\n", sensor->type, sensor_to_stringType(sensor), (sensor->flags & SENSOR_FLAG_WAKE_UP) != 0 ? "wakeup" : "non-wakeup");
  fprintf(stderr, "\tName:%s Vendor:%s Version:%d Handle:%d\n", sensor->name, sensor->vendor, sensor->version, sensor->handle);
  fprintf(stderr, "\tmaxRange: %f resolution: %f power: %f mA\n", sensor->maxRange, sensor->resolution, sensor->power);
  fprintf(stderr, "\tminDelay: %d us maxDelay: %d us\n", sensor->minDelay, sensor->maxDelay);
  fprintf(stderr, "\tfifoReservedEventCount: %d fifoMaxEventCount: %d\n", sensor->fifoReservedEventCount, sensor->fifoMaxEventCount);
  fprintf(stderr, "\trequiredPermission: %s\n", sensor->requiredPermission);
}

void print_sensor_list( struct sensor_t const* list )
{
  struct sensor_t const* list_p = list;

  fprintf(stderr, "Sensor list:\n");
  while ( list_p->name )
  {
    print_sensor(list_p);
    list_p++;
  }
}

void print_help_msg(int argc, char * const argv[]) {
  fprintf(stderr, "Usage: %s [OPTIONS]...\n\n", argc > 0 ? argv[0]: "");
  fprintf(stderr, "-h --help\t\t Print this message\n");
  fprintf(stderr, "-l --listsensors\t List all available sensors and their attributes\n");
  fprintf(stderr, "-o --output\t\tthe output file to write the sensor values to\n\t\t\tdefault: %s\n\n", DEFAULT_OUTPUT_FILE);
  fprintf(stderr, "Providing no parameter options runs the interactive command line interface\n");
  fprintf(stderr, "Providing 1 or more parameters to sns_hal_batch will run the following sequence:\n");
  fprintf(stderr, "\t- set batching parameters for the sensor: (sampling rate, report rate)\n");
  fprintf(stderr, "\t- activate the sensor\n");
  fprintf(stderr, "\t- wait for the specified duration\n");
  fprintf(stderr, "\t- deactivate the sensor\n\n");
  fprintf(stderr, "The parameters, as well as their default values, are as follows:\n");
  fprintf(stderr, "\t-s  --sensor\t\tthe android sensor type enumeration value\n\t\t\t\tdefault: %d for %s\n", DEFAULT_SENSOR_TYPE, type_to_string(DEFAULT_SENSOR_TYPE));
  fprintf(stderr, "\t-w  --wakeup\t\tflag for wakeup or non-wakeup sensor\n\t\t\t\t\tw for wakeup\n\t\t\t\t\tn for non-wakeup\n\t\t\t\t\td for don't care or default\n\t\t\t\tdefault: %c\n", DEFAULT_WAKEUP_FLAG);
  fprintf(stderr, "\t-sr --samplingrate\tthe sampling rate (in Hz)\n\t\t\t\tdefault: %d Hz\n", DEFAULT_SAMPLING_RATE_HZ);
  fprintf(stderr, "\t-rr --reportrate\tthe report rate (in Hz)\n\t\t\t\tuse 0 for no batching\n\t\t\t\t(report events as available)\n\t\t\t\tdefault: %d\n", DEFAULT_REPORT_RATE_HZ);
  fprintf(stderr, "\t-d  --duration\t\tthe duration (in seconds) to run the sensor for\n\t\t\t\tdefault: %d seconds\n", DEFAULT_DURATION_SECS);
}

// Constantly polls data from HAL
void *hal_poll( void *threadid )
{
#define POLL_NUM_EVENTS 100
  struct sensors_event_t data[POLL_NUM_EVENTS];
  int count;
  int i;
  struct timespec now_rt;
  uint64_t now_ns;

  while( 1 ) {
    count = dev->poll( dev_old, data, POLL_NUM_EVENTS );
    sensorEventsRecvd += count;
    clock_gettime( CLOCK_BOOTTIME, &now_rt );
    now_ns = (uint64_t)now_rt.tv_sec * 1000000000LL + (uint64_t)now_rt.tv_nsec;

    for( i = 0; i < count ; i++ ) {
#ifdef SENSORS_DEVICE_API_VERSION_1_1
      if( data[i].type == SENSOR_TYPE_META_DATA ) {
        printf("%lld.%06lld, %s/%s, %lld.%06lld %d latency: %lld.%06lld\n",
                 now_ns/NSEC_PER_MS, now_ns%NSEC_PER_MS, "META_DATA",
                 (data[i].meta_data.what == META_DATA_FLUSH_COMPLETE) ?"FLUSH_COMPLETE":"Unk",
                 data[i].timestamp/NSEC_PER_MS, data[i].timestamp%NSEC_PER_MS,
                 data[i].meta_data.sensor,
                 (now_ns - data[i].timestamp) / NSEC_PER_MS,
                 (now_ns - data[i].timestamp) % NSEC_PER_MS);
      } else
#endif /* SENSORS_DEVICE_API_VERSION_1_1 */
      {
        printf("%lld.%06lld, %s/%s, %lld.%06lld, %f, %f, %f, %f, %lld, latency(ms): %lld.%06lld\n",
                 now_ns/NSEC_PER_MS, now_ns%NSEC_PER_MS,
                 type_to_string( data[i].type ), handle_to_string(list, data[i].sensor),
                 data[i].timestamp/NSEC_PER_MS, data[i].timestamp%NSEC_PER_MS,
                 data[i].data[0], data[i].data[1], data[i].data[2], data[i].data[3], data[i].u64.step_counter,
                 (now_ns - data[i].timestamp) / NSEC_PER_MS,
                 (now_ns - data[i].timestamp) % NSEC_PER_MS);
      }
    }
    fflush(stdout);
  }
}

static int initGralloc1Dev(void)
{
  int ret = 0;

  const hw_module_t* hw_mod = NULL;
  ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hw_mod);
  if (ret != 0)
  {
    printf("%s: hw_get_module FAILED ret %d\n", __FUNCTION__, ret);
    return -1;
  }

  // Check the gralloc API version for gralloc1
  if (((hw_mod->module_api_version >> 8) & 0xFF) != 1)
  {
    printf("%s: unsupported gralloc version 0x%X\n", __FUNCTION__,
        hw_mod->module_api_version);
    return -2;
  }
  else
  {
    // Open the gralloc1 module
    ret = gralloc1_open(hw_mod, &gralloc1_device);
    if (ret != 0)
    {
      printf("%s: gralloc1_open FAILED ret %d\n", __FUNCTION__, ret);
      return -3;
    }
    else
    {
      // Get the necessary function pointers
      gralloc1_retain = (GRALLOC1_PFN_RETAIN)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_RETAIN);
      gralloc1_release = (GRALLOC1_PFN_RELEASE)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_RELEASE);
      gralloc1_lock = (GRALLOC1_PFN_LOCK)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_LOCK);
      gralloc1_unlock = (GRALLOC1_PFN_UNLOCK)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_UNLOCK);
      gralloc1_create_descriptor = (GRALLOC1_PFN_CREATE_DESCRIPTOR)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_CREATE_DESCRIPTOR);
      gralloc1_destroy_descriptor = (GRALLOC1_PFN_DESTROY_DESCRIPTOR)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_DESTROY_DESCRIPTOR);
      gralloc1_set_dimensions = (GRALLOC1_PFN_SET_DIMENSIONS)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_SET_DIMENSIONS);
      gralloc1_set_format = (GRALLOC1_PFN_SET_FORMAT)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_SET_FORMAT);;
      gralloc1_set_layer_count = (GRALLOC1_PFN_SET_LAYER_COUNT)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_SET_LAYER_COUNT);
      gralloc1_set_producer_usage = (GRALLOC1_PFN_SET_PRODUCER_USAGE)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_SET_PRODUCER_USAGE);
      gralloc1_allocate = (GRALLOC1_PFN_ALLOCATE)gralloc1_device->getFunction(gralloc1_device, GRALLOC1_FUNCTION_ALLOCATE);
    }
  }

  return 0;
}

static int allocate_gralloc1(gralloc1_buffer_descriptor_t* buffer_fd, buffer_handle_t* buffer_handle)
{
  int gralloc_result;
  int ret = 0;

  gralloc_result = gralloc1_create_descriptor(gralloc1_device, buffer_fd);
  if (gralloc_result != GRALLOC1_ERROR_NONE)
  {
    fprintf(stderr,"gralloc1_create_descriptor %d\n", gralloc_result);
    ret = gralloc_result;
  }
  else
  {
    // NOTE: If some of these calls (notably the gralloc1_set* ones) fail, it
    // may not be a catastrophic error so try to proceed as normal.
    gralloc_result = gralloc1_set_dimensions(gralloc1_device, *buffer_fd, DIRECT_MEM_SIZE, 1);
    if (gralloc_result != GRALLOC1_ERROR_NONE)
    {
      fprintf(stderr,"gralloc1_set_dimensions %d\n", gralloc_result);
    }
    gralloc_result = gralloc1_set_format(gralloc1_device, *buffer_fd, 0x21); // 0x21 is derived from HardwareBuffer.BLOB
    if (gralloc_result != GRALLOC1_ERROR_NONE)
    {
      fprintf(stderr,"gralloc1_set_format %d\n", gralloc_result);
    }
    gralloc_result = gralloc1_set_layer_count(gralloc1_device, *buffer_fd, 1);
    if (gralloc_result != GRALLOC1_ERROR_NONE)
    {
      fprintf(stderr,"gralloc1_set_layer_count %d\n", gralloc_result);
    }
    gralloc_result = gralloc1_set_producer_usage(gralloc1_device, *buffer_fd, GRALLOC1_PRODUCER_USAGE_SENSOR_DIRECT_DATA);
    if (gralloc_result != GRALLOC1_ERROR_NONE)
    {
      fprintf(stderr,"gralloc1_set_producer_usage %d\n", gralloc_result);
    }
    gralloc_result = gralloc1_allocate(gralloc1_device, 1, buffer_fd, buffer_handle);
    if (gralloc_result != GRALLOC1_ERROR_NONE)
    {
      fprintf(stderr,"gralloc1_allocate %d\n", gralloc_result);
      ret = gralloc_result;
      // Destroy the descriptor if the allocate failed
      gralloc_result = gralloc1_destroy_descriptor(gralloc1_device, *buffer_fd);
      if (gralloc_result != GRALLOC1_ERROR_NONE)
      {
        fprintf(stderr,"gralloc1_destroy_descriptor %d\n", gralloc_result);
      }
    }
  }

  return ret;
}

static int register_channel(struct sensors_poll_device_1 *dev, buffer_handle_t buffer_handle)
{
  struct sensors_direct_mem_t direct_mem;
  int channel_handle = -1;

  direct_mem.type = SENSOR_DIRECT_MEM_TYPE_GRALLOC;
  direct_mem.format = SENSOR_DIRECT_FMT_SENSORS_EVENT;
  direct_mem.size = DIRECT_MEM_SIZE;
  direct_mem.handle = buffer_handle;
  channel_handle = dev->register_direct_channel(dev, &direct_mem, 0);
  if (channel_handle < 0)
  {
    fprintf(stderr,"ERROR: Unable to register direct channel %d\n", channel_handle);
    return -1;
  }
  else
  {
    fprintf(stderr,"Channel handle %d assigned.\n", channel_handle);
  }

  return channel_handle;
}

static void configure_channel(struct sensors_poll_device_1 *dev, int sensor, int channel_handle, int rate_level)
{
    struct sensors_direct_cfg_t direct_cfg;
    int sensor_error;

    direct_cfg.rate_level = rate_level;
    sensor_error = dev->config_direct_report(dev, sensor, channel_handle, &direct_cfg);
    if (sensor_error < 0)
    {
      fprintf(stderr,"ERROR: Unable to configure and start the sensor stream %d\n", sensor_error);
      return;
    }
    else
    {
      fprintf(stderr,"Sensor %d successfully configured @ rate %d on channel %d.\n", sensor, rate_level, channel_handle);
    }
}

void* direct_report_channel_monitor(void* arg)
{
  buffer_handle_t buffer_handle = *((buffer_handle_t*) arg);
  void* buffer_ptr = NULL;
  int gralloc_result;
  int dummy;
  const gralloc1_rect_t accessRegion = {
      .left = 0,
      .top = 0,
      .width = DIRECT_MEM_SIZE,
      .height = 1
  };

  // Lock the buffer to get the pointer
  gralloc_result = gralloc1_lock(gralloc1_device, buffer_handle, GRALLOC1_PRODUCER_USAGE_SENSOR_DIRECT_DATA, GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN,
      &accessRegion, &buffer_ptr, -1);
  if (gralloc_result)
  {
    fprintf(stderr,"ERROR: Issue locking the buffer space %d\n", gralloc_result);
  }
  else
  {
    // Skim through the channel to find the latest sample

    // While the channel is running, verify the latest sample
    while(channel_running)
    {
      // Test 1: Make sure
      // This was originally reported as "Field changed after atomic counter is updated"
    }

    // Release the buffer handle
    dummy = -1;
    gralloc_result = gralloc1_unlock(gralloc1_device, buffer_handle, &dummy);
    if (gralloc_result)
    {
      fprintf(stderr,"ERROR: Issue releasing the buffer space %d\n", gralloc_result);
    }
  }

  return NULL;
}

int find_sensor_by_type(int sensor_type, char wakeup_flag, struct sensor_t const* list, const struct sensor_t **ret)
{
  const struct sensor_t *list_p = list;
  while( list_p->name ) {
    if( list_p->type == sensor_type ) {
      // now that we found a sensor of the correct type, find out if it is wakeup or not
      if (wakeup_flag == 'd') {
        break;
      } else if (wakeup_flag == 'w' && (list_p->flags & SENSOR_FLAG_WAKE_UP) != 0) {
        break;
      } else if (wakeup_flag == 'n' && (~(list_p->flags) & SENSOR_FLAG_WAKE_UP) != 0) {
        break;
      }
    }
    list_p++;
  }

  *ret = list_p;

  if ( list_p-> name ) {
    return 0;
  } else {
    return -1;
  }
}

int parseArgs(  int argc,
                char * const argv[],
                bool *is_interactive,
                bool *is_direct_report_test,
                bool *list_sensors,
                int *sensor_type,
                char *wakeup_flag,
                double *sampling_rate_hz,
                int64_t *sampling_rate_nsecs,
                double *report_rate_hz,
                int64_t *report_rate_nsecs,
                int *duration_secs,
                char *output_filename,
                size_t max_output_file_length)
{

  static struct option long_options[] =
        {
          /* These options set a flag. */
          {"sensor", required_argument, 0, 's'},
          {"wakeup", required_argument, 0, 'w'},
          {"samplingrate", required_argument, 0, 0},
          {"sr", required_argument, 0, 0},
          {"reportrate", required_argument, 0, 'r'},
          {"rr", required_argument, 0, 'r'},
          {"duration", required_argument, 0, 'd'},
          {"outputfile", optional_argument, 0, 'o'},
          {"help", no_argument, 0, 'h'},
          {"listsensors", no_argument, 0, 'l'},
          {"direct", no_argument, 0, 'i'},
          {0, 0, 0, 0}
        };

  *is_interactive = true;
  *list_sensors = false;
  *sensor_type = DEFAULT_SENSOR_TYPE;
  *wakeup_flag = DEFAULT_WAKEUP_FLAG;
  *sampling_rate_hz = DEFAULT_SAMPLING_RATE_HZ;
  *report_rate_hz = DEFAULT_REPORT_RATE_HZ;
  *duration_secs = DEFAULT_DURATION_SECS;
  *output_filename = 0;

  *is_interactive = true;
  *is_direct_report_test = false;

  int num_scanned;
  int option_index = 0;
  int opt;
  while( (opt = getopt_long_only (argc, argv, ":s:w:r:d:o:hli", long_options, &option_index)) != -1 ) {
    switch(opt)
    {
      // special handling for sampling rate (sr) because two characters
      case 0:
        if (strncmp("sr", long_options[option_index].name, 80) == 0
          || strncmp("samplingrate", long_options[option_index].name, 80) == 0)
        {
          num_scanned = sscanf(optarg, "%lf", sampling_rate_hz);
          if (num_scanned != 1) {
            fprintf(stderr, "ERROR: Invalid argument for %s option\n", long_options[option_index].name);
            exit(1);
          }
          *is_interactive = false;
        }
        break;

      // sensor type
      case 's':
        num_scanned = sscanf(optarg, "%d", sensor_type);
        if (num_scanned != 1) {
          fprintf(stderr, "ERROR: Invalid argument for %s option\n", long_options[option_index].name);
          exit(1);
        }
        *is_interactive = false;
        break;

      // wakeup flag
      case 'w':
        num_scanned = sscanf(optarg, "%c", wakeup_flag);
        if (num_scanned != 1) {
          fprintf(stderr, "ERROR: Invalid argument for %s option\n", long_options[option_index].name);
          exit(1);
        }
        *is_interactive = false;

        break;

      // report rate
      case 'r':
        num_scanned = sscanf(optarg, "%lf", report_rate_hz);
        if (num_scanned != 1) {
          fprintf(stderr, "ERROR: Invalid argument for %s option\n", long_options[option_index].name);
          exit(1);
        }
        *is_interactive = false;
        break;

      // duration
      case 'd':
        num_scanned = sscanf(optarg, "%d", duration_secs);
        if (num_scanned != 1) {
          fprintf(stderr, "ERROR: Invalid argument for %s option\n", long_options[option_index].name);
          exit(1);
         }
        *is_interactive = false;

        break;

      // output file
      case 'o':
          // this parameter is optional so make sure we are not using one of the other options
          // as our argument
          if(*optarg == '-') {
            optind -= 1;
            strlcpy(output_filename, DEFAULT_OUTPUT_FILE, max_output_file_length);
          } else {
            strlcpy(output_filename, optarg, max_output_file_length);
          }
        break;

      case 'l':
        fprintf(stderr, "List.\n");
        *list_sensors = true;
        return 0;

      case 'i':
        fprintf(stderr, "Direct Report.\n");
        *is_direct_report_test = true;
        return 0;

      // help
      case 'h':
        print_help_msg(argc, argv);
        exit(0);

      // unknown argument
      case '?':
      default:
        fprintf(stderr, "Unknown option \'%c\'.\n", optopt);
        print_help_msg(argc, argv);
        exit(1);
    }
  }


  *sampling_rate_nsecs = *sampling_rate_hz == 0 ? 0 : (uint64_t) HZ_TO_NSEC(*sampling_rate_hz);
  *report_rate_nsecs = *report_rate_hz == 0 ? 0 : (uint64_t) HZ_TO_NSEC(*report_rate_hz);

  return 0;
}

int validateParams(
                    int *sensor_type,
                    char *wakeup_flag,
                    double *sampling_rate_hz,
                    int64_t *sampling_rate_nsecs,
                    double *report_rate_hz,
                    int64_t *report_rate_nsecs,
                    int *duration_secs)
{
  // validate sensor type
  if (*sensor_type < 0) {
    fprintf(stderr, "Invalid sensor_type (sensor out of bounds): %d\n", *sensor_type);
    exit(1);
  }

  if (*wakeup_flag != 'w' && *wakeup_flag != 'n' && *wakeup_flag != 'd') {
    fprintf(stderr, "Invalid wakeup_flag (must be w, n, or d): %c\n", *wakeup_flag);
    exit(1);
  }

  if (*sampling_rate_hz <= 0) {
    fprintf(stderr, "Invalid sampling_rate_hz (must be non-negative): %f\n", *sampling_rate_hz);
    exit(1);
  }

  if (*sampling_rate_nsecs < 0) {
    fprintf(stderr, "Invalid sampling_rate_nsecs (must be non-negative): %" PRId64 "\n", *sampling_rate_nsecs);
    exit(1);
  }

  if (*report_rate_hz < 0) {
    fprintf(stderr, "Invalid report_rate_hz (must be non-negative): %f\n", *report_rate_hz);
    exit(1);
  }

  if (*report_rate_nsecs < 0) {
    fprintf(stderr, "Invalid report_rate_nsecs (must be non-negative): %" PRId64 "\n", *report_rate_nsecs);
    exit(1);
  }

  if (*duration_secs <= 0) {
    fprintf(stderr, "Invalid duration_secs (must be non-negative): %d\n", *duration_secs);
    exit(1);
  }

  return 0;
}

void print_direct_report_buffer(void* buffer_ptr, size_t buffer_size)
{
  sensors_event_t circ_buf_sample;
  sensors_event_t* circ_buf = (sensors_event_t*) buffer_ptr;
  unsigned int circ_buf_max_samples = buffer_size / sizeof(circ_buf_sample);

  printf("Dumping %u samples @ 0x%x.\n", circ_buf_max_samples, buffer_ptr);

  // Dump the set of circular buffer contents
  for (unsigned int j=0; j < circ_buf_max_samples; j++)
  {
    // Copy the sample over
    memcpy((void *) &circ_buf_sample,(void *) &(circ_buf[j]),sizeof(circ_buf_sample));
    printf("sensor %u type %u atomic %u ts %llu (%f, %f, %f) %d (%f, %f, %f)\n",
        circ_buf_sample.sensor, circ_buf_sample.type, circ_buf_sample.reserved0,
        circ_buf_sample.timestamp,
        circ_buf_sample.data[0], circ_buf_sample.data[1], circ_buf_sample.data[2],
        circ_buf_sample.gyro.status,
        circ_buf_sample.data[3], circ_buf_sample.data[4], circ_buf_sample.data[5]);
  }
}

int main( int argc, char * const argv[] )
{
  struct sensor_t const *list_p;
  pthread_t poll_thread;
  int num_scanned;
  int flags = 0;
  double sampling_rate_hz;
  int64_t sampling_rate_nsecs;
  double report_rate_hz;
  int64_t report_rate_nsecs;
  int error;
  char command;
  char wakeup_flag;
  bool is_interactive;
  bool is_direct_report_test;
  bool list_sensors;
  bool valid_input = false;
  char input_buffer[80];
  char output_filename[80];
  int sensor_type;
  int duration_secs;
  void* hal_lib;
  struct timespec before_rt;
  struct timespec after_rt;
  uint64_t before_ns;
  uint64_t after_ns;
  uint64_t time_taken_ns;

  error = parseArgs(argc, argv, &is_interactive, &is_direct_report_test, &list_sensors, &sensor_type, &wakeup_flag, &sampling_rate_hz, &sampling_rate_nsecs, &report_rate_hz, &report_rate_nsecs, &duration_secs, output_filename, 80);
  if (error)
    exit(1);

  error = validateParams(&sensor_type, &wakeup_flag, &sampling_rate_hz, &sampling_rate_nsecs, &report_rate_hz, &report_rate_nsecs, &duration_secs);
  if (error)
    exit(1);


  FILE* fp = NULL;
  // if the output parameter was set, then redirect stdout
  if (*output_filename != 0) {
    fprintf(stderr, "redirecting stdout to %s\n", output_filename);
    fp = freopen (output_filename, "w", stdout);
  }

  hal_lib = dlopen( HAL_LIB_NAME, RTLD_LAZY );

  if( NULL == hal_lib ) {
    fprintf(stderr,"dlopen of sensor HAL lib (%s) failed\n", HAL_LIB_NAME );
    fprintf(stderr,"exiting now. . .\n");
    exit(1);
  }

  dlerror();
  hal_sym = dlsym( hal_lib, HAL_MODULE_INFO_SYM_AS_STR );
  if( dlerror() != NULL ) {
    fprintf(stderr,"dlsym(%s) failed\n", HAL_MODULE_INFO_SYM_AS_STR );
    exit(1);
  }
  if(hal_sym == NULL) {
    fprintf(stderr,"dlsym(%s) failed to get symbol address\n", HAL_MODULE_INFO_SYM_AS_STR );
    exit(1);
  }

  if( 0 != hal_sym->common.methods->open( NULL, SENSORS_HARDWARE_POLL, (hw_device_t**)&dev ) )
  {
    fprintf(stderr,"Hal open failure\n");
    exit(1);
  }

  fprintf(stderr,"HAL open\n");
  fprintf(stderr,"HAL module_api_version: 0x%d\n", hal_sym->common.module_api_version );
  fprintf(stderr,"HAL hal_api_version   : 0x%d\n", hal_sym->common.hal_api_version );
  fprintf(stderr,"HAL hal_id            : %s\n", hal_sym->common.id );
  fprintf(stderr,"HAL hal_name          : %s\n", hal_sym->common.name );
  fprintf(stderr,"HAL hal_author        : %s\n", hal_sym->common.author );


  dev_old = (struct sensors_poll_device_t*) dev;
  usleep(50000);
  pthread_create( &poll_thread, NULL, hal_poll, NULL );
  usleep(50000);

  // get the sensors_list and time how long it took
  clock_gettime( CLOCK_BOOTTIME, &before_rt );
  sensor_count = hal_sym->get_sensors_list( NULL, ((struct sensor_t const**)&list) );
  clock_gettime( CLOCK_BOOTTIME, &after_rt );

  // get time in ms and print
  before_ns = (uint64_t)before_rt.tv_sec * 1000000000LL + (uint64_t)before_rt.tv_nsec;
  after_ns = (uint64_t)after_rt.tv_sec * 1000000000LL + (uint64_t)after_rt.tv_nsec;
  time_taken_ns = (after_ns-before_ns);
  fprintf(stderr, "get_sensors_list took %" PRId64 " nanoseconds\n", time_taken_ns);

  if ( sensor_count == 0 )
  {
    fprintf(stderr,"ERROR: No sensors in the list");
    dev->common.close( (hw_device_t*)dev );
    exit(1);
  }

  if (list_sensors)
  {
    print_sensor_list(list);
    exit(0);
  }

  if(is_direct_report_test)
  {
    fprintf(stderr,"Direct Report Test\n");
    int gralloc_result;
    int sensor_error;
    int channel_handle;
    gralloc1_buffer_descriptor_t buffer_fd;
    buffer_handle_t buffer_handle;
    void* buffer_ptr = NULL;
    int dummy;
    int channel_handle1;
    gralloc1_buffer_descriptor_t buffer_fd1;
    buffer_handle_t buffer_handle1;

    // Allocate some memory
    gralloc_result = initGralloc1Dev();
    if (gralloc_result)
    {
      fprintf(stderr,"ERROR: Issue with initialize gralloc device %d\n", gralloc_result);
      exit(1);
    }
    allocate_gralloc1(&buffer_fd, &buffer_handle);
    if (direct_report_multi_client_test)
    {
      allocate_gralloc1(&buffer_fd1, &buffer_handle1);
    }

    // Register the direct channel
    channel_handle = register_channel(dev, buffer_handle);
    if(direct_report_multi_client_test)
    {
      channel_handle1 = register_channel(dev, buffer_handle1);
    }

    // Configure the sensor stream
    configure_channel(dev, DIRECT_SENSOR, channel_handle, DIRECT_RATE_LEVEL);
    if(direct_report_multi_client_test)
    {
      configure_channel(dev, DIRECT_SENSOR1, channel_handle1, DIRECT_RATE_LEVEL1);
    }

    if(direct_report_reconfigure_test)
    {
      usleep(DIRECT_SLEEP_DURATION_US);
      configure_channel(dev, DIRECT_SENSOR, channel_handle, DIRECT_RATE_LEVEL2);
    }

    // Start the direct channel monitor thread
    if (direct_report_enable_monitor_thread)
    {
      channel_running = true;
      if (0 != pthread_create(&direct_report_monitor, NULL, direct_report_channel_monitor, &buffer_handle))
      {
        printf("- Failed to create the direct report channel monitor!");
      }
    }

    // Let it run for awhile
    usleep(DIRECT_SLEEP_DURATION_US);

    if(direct_report_double_register_test)
    {
      channel_handle1 = register_channel(dev, buffer_handle);
      if (channel_handle1 > 0)
      {
        configure_channel(dev, DIRECT_SENSOR1, channel_handle1, DIRECT_RATE_LEVEL1);
        usleep(DIRECT_SLEEP_DURATION_US);
      }
    }

    // Stop the stream
    // Stop all
    configure_channel(dev, -1, channel_handle, SENSOR_DIRECT_RATE_STOP);
    if(direct_report_multi_client_test)
    {
      configure_channel(dev, DIRECT_SENSOR1, channel_handle1, SENSOR_DIRECT_RATE_STOP);
    }

    if(direct_report_reenable_test)
    {
      // Pause in between re-enablement
      usleep(3*1000000);

      configure_channel(dev, DIRECT_SENSOR1, channel_handle, DIRECT_RATE_LEVEL1);

      // Let it run for awhile
      usleep(DIRECT_SLEEP_DURATION_US);

      // Stop the stream
      configure_channel(dev, DIRECT_SENSOR1, channel_handle, SENSOR_DIRECT_RATE_STOP);
    }

    // Lock the channel to get the pointer to the buffer space. Then print its
    // contents
    const gralloc1_rect_t accessRegion = {
        .left = 0,
        .top = 0,
        .width = DIRECT_MEM_SIZE,
        .height = 1
    };
    gralloc_result = gralloc1_lock(gralloc1_device, buffer_handle, GRALLOC1_PRODUCER_USAGE_SENSOR_DIRECT_DATA, GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN,
        &accessRegion, &buffer_ptr, -1);
    if (gralloc_result)
    {
      fprintf(stderr,"ERROR: Issue locking the buffer space %d\n", gralloc_result);
    }
    else
    {
      // Print the buffer contents
      print_direct_report_buffer(buffer_ptr, DIRECT_MEM_SIZE);

      // Release the buffer handle
      dummy = -1;
      gralloc_result = gralloc1_unlock(gralloc1_device, buffer_handle, &dummy);
      if (gralloc_result)
      {
        fprintf(stderr,"ERROR: Issue releasing the buffer space %d\n", gralloc_result);
      }
    }

    if(direct_report_multi_client_test)
    {
      // Lock the channel to get the pointer to the buffer space. Then print its
      // contents
      gralloc_result = gralloc1_lock(gralloc1_device, buffer_handle1, GRALLOC1_PRODUCER_USAGE_SENSOR_DIRECT_DATA, GRALLOC1_CONSUMER_USAGE_CPU_READ_OFTEN,
          &accessRegion, &buffer_ptr, -1);
      if (gralloc_result)
      {
        fprintf(stderr,"ERROR: Issue locking the buffer space %d\n", gralloc_result);
      }
      else
      {
        // Print the buffer contents
        print_direct_report_buffer(buffer_ptr, DIRECT_MEM_SIZE);

        // Release the buffer handle
        dummy = -1;
        gralloc_result = gralloc1_unlock(gralloc1_device, buffer_handle1, &dummy);
        if (gralloc_result)
        {
          fprintf(stderr,"ERROR: Issue releasing the buffer space %d\n", gralloc_result);
        }
      }
    }

    // Clean up the channel
    gralloc_result = gralloc1_release(gralloc1_device, buffer_handle);
    if (gralloc_result)
    {
      fprintf(stderr,"ERROR: Issue releasing the buffer space %d\n", gralloc_result);
    }
    gralloc_result = gralloc1_destroy_descriptor(gralloc1_device, buffer_fd);

    if(direct_report_multi_client_test)
    {
      // Clean up the channel
      gralloc_result = gralloc1_release(gralloc1_device, buffer_handle1);
      if (gralloc_result)
      {
        fprintf(stderr,"ERROR: Issue releasing the buffer space %d\n", gralloc_result);
      }
      gralloc_result = gralloc1_destroy_descriptor(gralloc1_device, buffer_fd1);
    }

    // Exit the test app
    exit(0);
  }

  if(!is_interactive)
  {
    const struct sensor_t *sensor_to_use;
    error = find_sensor_by_type(sensor_type, wakeup_flag, list, &sensor_to_use);
    if ( error ) {
      fprintf(stderr, "ERROR: sensor not found!");
      exit(1);
    }

    // BATCH
    error = dev->batch( dev, sensor_to_use->handle, flags, sampling_rate_nsecs, report_rate_nsecs );
    if( error ) {
      fprintf(stderr,"Error %d in batch\n", error );
      exit(1);
    } else {
      fprintf(stderr,"batch success\n");
    }

    // ACTIVATE
    error = dev->activate( dev_old, sensor_to_use->handle, true );
    if( error ) {
      fprintf(stderr,"Error %d activating sensor\n", error );
      exit(1);
    } else {
      fprintf(stderr, "activate success\n");
    }

    // print out a confirmation for what we just did
    fprintf(stderr, "Activated sensor [Type: %d] %s (%c) for %d seconds, sampling at %f Hz and reporting at %f Hz\n",
                sensor_to_use->type, sensor_to_use->name, wakeup_flag, duration_secs,
                sampling_rate_hz, report_rate_hz);

    // print out more information on the sensor we used
    print_sensor(sensor_to_use);

    // wait to deactivate and exit
    fprintf(stderr, "Sleeping for %d seconds before deactivating and exiting\n", duration_secs);
    sleep(duration_secs);
    fprintf(stderr, "Deactivating and exiting...\n");

    //DEACTIVATE;
    error = dev->activate( dev_old, sensor_to_use->handle, false );
    if( error ) {
      fprintf(stderr,"Error %d deactivating sensor\n", error );
      exit(1);
    }

    fprintf(stderr, "deactivate success\n");
    if (fp) fclose(fp);
    fprintf(stderr,"Exiting ESP+ test app...\n");
    dev->common.close( (hw_device_t*)dev_old );
    fprintf(stderr, "Received %d sensor events\n", sensorEventsRecvd);
    exit(0);
  }

  do
  {
    print_usage_msg(list);

    //fflush(NULL);

    // Get the sensor
    do
    {
      fprintf(stderr,"Please choose a listed sensor type to interact with> ");
      if( NULL == fgets(input_buffer,80,stdin) ) {
        while(1) {
          fprintf(stderr,"Will exit after 600 seconds of EOF\n");
          fprintf(stdout,"Will exit after 600 seconds of EOF\n");
          sleep(600);
          fprintf(stderr,"Exiting after 600 seconds of EOF\n");
          fprintf(stdout,"Exiting after 600 seconds of EOF\n");
          exit(0);
        }
      }
      fflush(stdin);
      num_scanned = sscanf(input_buffer, "%d", &sensor_type);
      valid_input = true;
      if ( num_scanned != 1 )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Unknown input.\n");
      }
      else
      {
        fprintf(stderr, "Do you want the wakeup sensor or the non-wakeup sensor? (w for wakeup, n for non-wakeup, d for don't care)> ");
        fgets(input_buffer,80,stdin);
        num_scanned = sscanf(input_buffer, "%c", &wakeup_flag);
        valid_input = true;
        if ( num_scanned != 1 || (wakeup_flag != 'w' && wakeup_flag != 'n' && wakeup_flag != 'd'))
        {
          valid_input = false;
          fprintf(stderr,"ERROR: Unknown input for wakeup_flag.\n");
        } else {
          list_p = list;
          while( list_p->name ) {
            if( list_p->type == sensor_type ) {
              // now that we found a sensor of the correct type, find out if it is wakeup or not
              if (wakeup_flag == 'd') {
                break;
              } else if (wakeup_flag == 'w' && (list_p->flags & SENSOR_FLAG_WAKE_UP) != 0) {
                break;
              } else if (wakeup_flag == 'n' && (~(list_p->flags) & SENSOR_FLAG_WAKE_UP) != 0) {
                break;
              }
            }
            list_p++;
          }
          if ( !list_p->name ) {
            valid_input = false;
            fprintf(stderr,"ERROR: No such sensor exists\n");
          }
        }
      }
    } while ( !valid_input );

    // Get the command
    do
    {
      fprintf(stderr,"Please choose a command (a,d,s,b,f,e)> ");
      fgets(input_buffer,80,stdin);
      fflush(stdin);
      num_scanned = sscanf(input_buffer, "%c", &command);
      valid_input = true;
      if ( num_scanned != 1 )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Unknown input.\n");
      }
      else if ( NULL == strchr(COMMAND_CHARS, command) )
      {
        valid_input = false;
        fprintf(stderr,"ERROR: Invalid command.\n");
      }
    } while ( !valid_input );

    switch(command)
    {
      case 'A':
      case 'a':
        //Activate;
        fprintf(stderr,"Activating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        printf("Activating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->activate( dev_old, list_p->handle, true );
        if( error ) {
          fprintf(stderr,"Error %d activating sensor\n", error );
        }
      break;

      case 'D':
      case 'd':
        //DEACTIVATE;
        fprintf(stderr,"Deactivating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        printf("Deactivating sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->activate( dev_old, list_p->handle, false );
        if( error ) {
          fprintf(stderr,"Error %d deactivating sensor\n", error );
        }
      break;

      case 'B':
      case 'b':
        //BATCH;
        do
        {
          fprintf(stderr,"Please specify a delay period (in Hz)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%lf", &sampling_rate_hz);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          } else if (sampling_rate_hz <= 0) {
            valid_input = false;
            fprintf(stderr,"ERROR: sampling_rate_hz must be positive\n");
          } else {
            sampling_rate_nsecs = (uint64_t) HZ_TO_NSEC(sampling_rate_hz);
          }
        } while ( !valid_input );
        do
        {
          fprintf(stderr,"Please specify a timeout (in Hz)> ");
          fgets(input_buffer,80,stdin);
          fflush(stdin);
          num_scanned = sscanf(input_buffer, "%lf", &report_rate_hz);
          valid_input = true;
          if ( num_scanned != 1 )
          {
            valid_input = false;
            fprintf(stderr,"ERROR: Unknown input.\n");
          } else if (report_rate_hz < 0) {
            valid_input = false;
            fprintf(stderr,"ERROR: report_rate_hz must be non-negative\n");
          } else {
            report_rate_nsecs = report_rate_hz == 0 ? 0 : (uint64_t) HZ_TO_NSEC(report_rate_hz);
          }
        } while ( !valid_input );

        fprintf(stderr,"Setting batch mode for sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->batch( dev, list_p->handle, flags, sampling_rate_nsecs, report_rate_nsecs );
        if( error ) {
          fprintf(stderr,"Error %d in batch\n", error );
        } else {
          fprintf(stderr,"batch success\n");
        }
        break;

      case 'F':
      case 'f':
        //Flush;
#ifdef SENSORS_DEVICE_API_VERSION_1_1
        fprintf(stderr,"flushing sensor %s @ handle %d ...\n", list_p->name, list_p->handle);
        error = dev->flush( dev, list_p->handle );
        if( error ) {
          fprintf(stderr,"Error %d in flush\n", error );
        } else {
          fprintf(stderr,"flush success\n");
        }
#else
        fprintf(stderr,"Flush not supported in this API version\n" );
#endif /* SENSORS_DEVICE_API_VERSION_1_1 */
        break;
      case 'E':
      case 'e':
        //EXIT;
        break;
    }
  } while( command != 'e' && command != 'E' );

  fprintf(stderr,"Exiting ESP+ test app...\n");
  dev->common.close( (hw_device_t*)dev_old );

  return 0;
}
