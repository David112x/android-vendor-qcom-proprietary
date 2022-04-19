/* sensor_util_xml.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <libxml/parser.h>
#include "sensor_common.h"

#define MAX_KEY_SIZE 64

enum xmlValueType {
  XML_VALUE_STRING,
  XML_VALUE_INT8,
  XML_VALUE_UINT8,
  XML_VALUE_INT16,
  XML_VALUE_UINT16,
  XML_VALUE_UINT64,
  XML_VALUE_INT32,
  XML_VALUE_UINT32,
  XML_VALUE_FLOAT,
  XML_VALUE_MAX
};

struct xmlKeyValuePair {
  void *value;
  char key[MAX_KEY_SIZE];
  xmlElementType nodeType;
  enum xmlValueType value_type;
};

struct xmlCameraConfigInfo {
  xmlDocPtr docPtr;
  xmlNodePtr nodePtr;
  camera_module_config_t *configPtr;
};

xmlNodePtr sensor_xml_util_get_node(xmlNodePtr pNodePtr,
  const char *nodeName, uint32_t node_index);
boolean sensor_xml_util_load_file(char * file_name, xmlDocPtr *pDocPtr,
  xmlNodePtr *pNodePtr, const char *nodeName);
void sensor_xml_util_unload_file(xmlDocPtr docPtr);
uint32_t sensor_xml_util_get_num_nodes(
  xmlNodePtr pNodePtr, const char *nodeName);
boolean sensor_xml_util_get_camera_probe_config(
  struct xmlCameraConfigInfo *pXmlCameraConfig, const char* parentName);
boolean sensor_xml_util_get_camera_full_config(
  struct xmlCameraConfigInfo *pXmlCameraConfig);
