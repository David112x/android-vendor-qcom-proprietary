/* sensor_util_xml.c
 *
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <libxml/parser.h>
#include "sensor_util_xml.h"
#include "sensor_common.h"

#define MAX_KEY_VALUE_PAIRS 20

static char special_mode_string[SENSOR_SPECIAL_MODE_MAX][50] =
{
  [SENSOR_SPECIAL_MODE_FLASH] = "FLASH",
  [SENSOR_SPECIAL_MODE_ZOOM_UPSCALE] = "ZOOM_UPSCALE",
  [SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE] = "ZOOM_DOWNSCALE",
  [SENSOR_SPECIAL_MODE_IR] = "IR MODE",
  [SENSOR_SPECIAL_MODE_OIS_CAPTURE] = "OIS_CAPTURE",
  [SENSOR_SPECIAL_MODE_ISO_100] = "ISO_100",
  [SENSOR_SPECIAL_MODE_ISO_200] = "ISO_200",
  [SENSOR_SPECIAL_MODE_ISO_400] = "ISO_400",
  [SENSOR_SPECIAL_MODE_ISO_800] = "ISO_800",
  [SENSOR_SPECIAL_MODE_ISO_1600] = "ISO_1600",
  [SENSOR_SPECIAL_MODE_DIM_720P] = "DIM_720P",
  [SENSOR_SPECIAL_MODE_DIM_1080P] = "DIM_1080P",
  [SENSOR_SPECIAL_MODE_DIM_UHD] = "DIM_UHD",
  [SENSOR_SPECIAL_MODE_SCENE_LANDSCAPE] = "SCENE_LANDSCAPE",
  [SENSOR_SPECIAL_MODE_SCENE_SNOW] = "SCENE_SNOW",
  [SENSOR_SPECIAL_MODE_SCENE_BEACH] = "SCENE_BEACH",
  [SENSOR_SPECIAL_MODE_SCENE_SUNSET] = "SCENE_SUNSET",
  [SENSOR_SPECIAL_MODE_SCENE_NIGHT] = "SCENE_NIGHT",
  [SENSOR_SPECIAL_MODE_SCENE_PORTRAIT] = "SCENE_PORTRAIT",
  [SENSOR_SPECIAL_MODE_SCENE_BACKLIGHT] = "SCENE_BACKLIGHT",
  [SENSOR_SPECIAL_MODE_SCENE_SPORTS] = "SCENE_SPORTS",
  [SENSOR_SPECIAL_MODE_SCENE_ANTISHAKE] = "SCENE_ANTISHAKE",
  [SENSOR_SPECIAL_MODE_SCENE_FLOWERS] = "SCENE_FLOWERS",
  [SENSOR_SPECIAL_MODE_SCENE_CANDLELIGHT] = "SCENE_CANDLELIGHT",
  [SENSOR_SPECIAL_MODE_SCENE_FIREWORKS] = "SCENE_FIREWORKS",
  [SENSOR_SPECIAL_MODE_SCENE_PARTY] = "SCENE_PARTY",
  [SENSOR_SPECIAL_MODE_SCENE_NIGHT_PORTRAIT] = "SCENE_NIGHT_PORTRAIT",
  [SENSOR_SPECIAL_MODE_SCENE_THEATRE] = "SCENE_THEATRE",
  [SENSOR_SPECIAL_MODE_SCENE_ACTION] = "SCENE_ACTION",
  [SENSOR_SPECIAL_MODE_SCENE_AR] = "SCENE_AR",
  [SENSOR_SPECIAL_MODE_SCENE_FACE_PRIORITY] = "SCENE_FACE_PRIORITY",
  [SENSOR_SPECIAL_MODE_SCENE_BARCODE] = "SCENE_BARCODE",
};

/**
 * sensor_xml_util_unload_file: Unload the xml document
 *
 * @ docPtr: pointer to the loaded xml document.
 */
void sensor_xml_util_unload_file(xmlDocPtr docPtr)
{
  xmlFreeDoc(docPtr);
}

/**
 * sensor_xml_util_load_file: Load the xml document
 *
 * @file_name: xml file to be loaded
 * @pDocPtr: address of Document pointer
 * @nodeName: Root node in document
 * @pNodePtr: Address of root node to be returned
 */
boolean sensor_xml_util_load_file(char * file_name, xmlDocPtr *pDocPtr,
  xmlNodePtr *pNodePtr, const char *nodeName)
{
  xmlDocPtr docPtr = NULL;
  xmlNodePtr rootPtr = NULL;
  xmlNodePtr cur = NULL;

  docPtr = xmlParseFile(file_name);

  if (!docPtr) {
    SERR("xmlParseFile failed. please validate the xml: %s",
      file_name);
    return FALSE;
  }

  rootPtr = xmlDocGetRootElement(docPtr);
  if (!rootPtr) {
    SERR("xmlDocGetRootElement failed rootPtr NULL");
    goto ERROR;
  }

  SLOW("node_name = %s root_name = %s", nodeName, rootPtr->name);

  if (xmlStrncmp(rootPtr->name, (const xmlChar *)nodeName,
    xmlStrlen(rootPtr->name))) {
    SERR("nodeName :%s not found Invalid node: %s", nodeName, rootPtr->name);
    goto ERROR;
  }

  *pDocPtr = docPtr;
  *pNodePtr = rootPtr;

  return TRUE;

ERROR:
  xmlFreeDoc(docPtr);
  return FALSE;
}

/**
 * sensor_xml_util_get_num_nodes: returns number of nodes with the name
 *
 * @pNodePtr: pointer of parent node
 * @nodeName: node name in the document
 */
uint32_t sensor_xml_util_get_num_nodes(
  xmlNodePtr pNodePtr, const char *nodeName)
{
  uint32_t   i = 0;
  uint32_t   child_count = 0;
  uint32_t   num_nodes = 0;
  xmlNodePtr curPtr = NULL;

  if (!pNodePtr || !nodeName)
    return 0;
  if (!xmlStrncmp(pNodePtr->name, (const xmlChar *)nodeName,
    xmlStrlen(pNodePtr->name)))
    num_nodes++;
  for (curPtr = xmlFirstElementChild(pNodePtr);
    curPtr; curPtr = xmlNextElementSibling(curPtr)) {
      num_nodes += sensor_xml_util_get_num_nodes(curPtr, nodeName);
  }
  return num_nodes;
}

/**
 * sensor_xml_util_get_num_nodes: returns number of nodes with the name
 *
 * @pNodePtr: pointer of parent node
 * @nodeName: node name in the document
 * @node_index: index of the node for which pointer has to be returned.
 */
xmlNodePtr sensor_xml_util_get_node(xmlNodePtr pNodePtr,
  const char *nodeName, uint32_t node_index)
{
  uint32_t   node_count = 0;
  xmlNodePtr curPtr = NULL;

  if (!xmlStrncmp(pNodePtr->name, (const xmlChar *)nodeName,
    xmlStrlen(pNodePtr->name))) {
    if (node_index == 0)
      return pNodePtr;
    else
      node_index--;
  }

  for (curPtr = xmlFirstElementChild(pNodePtr);
    curPtr; curPtr = xmlNextElementSibling(curPtr)) {
      node_count = sensor_xml_util_get_num_nodes(curPtr, nodeName);
      if (node_count > node_index)
        return sensor_xml_util_get_node(curPtr, nodeName,
          node_index);
      else
        node_index -= node_count;
  }
  return NULL;
}

/**
 * sensor_xml_util_get_node_data: fills in the data from docPtr
 *
 * @pNodePtr: pointer of parent node
 * @parentName: parent name of the data nodes.
 * @key_value_pair: key value pair in which data is updated.
 * @num_pairs: number of items to be updated.
 * @index: index of the tree from which data must be retrieved.
 */
static boolean sensor_xml_util_get_node_data(xmlDocPtr docPtr,
  xmlNodePtr pNodePtr, const char *parentName,
  struct xmlKeyValuePair *key_value_pair, uint16_t num_pairs, uint8_t index)
{
  uint16_t i = 0;
  char *str = NULL;
  uint32_t size = 0;
  xmlNodePtr curPtr = NULL;

  RETURN_ON_NULL(key_value_pair);
  RETURN_ON_NULL(parentName);
  RETURN_ON_NULL(pNodePtr);
  RETURN_ON_NULL(docPtr);

  RETURN_FALSE_IF(!num_pairs);
  RETURN_FALSE_IF(num_pairs > MAX_KEY_VALUE_PAIRS, "pairs = %d", num_pairs);

  SLOW("num_pairs = %d", num_pairs);

  pNodePtr = sensor_xml_util_get_node(pNodePtr, parentName, index);
  RETURN_ON_NULL(pNodePtr);

  for(i = 0; i < num_pairs; i++) {

    /* Validate the destination pointer */
    RETURN_ON_NULL(key_value_pair[i].value);

    if (key_value_pair[i].nodeType == XML_ELEMENT_NODE) {

      /* Match the key from xml with the requested key list */
      for (curPtr = xmlFirstElementChild(pNodePtr); curPtr && xmlStrncmp(
        curPtr->name, (const xmlChar *)key_value_pair[i].key,
          xmlStrlen(curPtr->name)); curPtr = xmlNextElementSibling(curPtr));

      /* Check if we found a match */
      if (!curPtr) {
        SLOW("Tag %s not present in XML file", key_value_pair[i].key);
        continue;
      }

      /* Get the data pointer  */
      str = (char *)xmlNodeListGetString(docPtr, curPtr->xmlChildrenNode, 1);
    } else if (key_value_pair[i].nodeType == XML_ATTRIBUTE_NODE) {
      str = (char *)xmlGetProp(
        pNodePtr, (const xmlChar *)key_value_pair[i].key);
    } else {
      SERR("Invalid nodeType = %d", key_value_pair[i].nodeType);
      return FALSE;
    }

    /* Check if data pointer is NULL */
    if(str == NULL) {
      SLOW("Empty node %s in XML file", key_value_pair[i].key);
      continue;
    }

    SLOW(" i = %d str = %s", i, str);

    /* Copy the string */
    switch (key_value_pair[i].value_type) {

    case XML_VALUE_STRING:
      strlcpy(key_value_pair[i].value, str,
        xmlStrlen((const xmlChar *)str) + 1);
      break;
    case XML_VALUE_INT8:
      *((int8_t *)key_value_pair[i].value) = (int8_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_UINT8:
      *((uint8_t *)key_value_pair[i].value) = (uint8_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_INT16:
      *((int16_t *)key_value_pair[i].value) = (int16_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_UINT16:
      *((uint16_t *)key_value_pair[i].value) = (uint16_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_UINT64:
      *((uint64_t *)key_value_pair[i].value) = (uint64_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_INT32:
      *((int32_t *)key_value_pair[i].value) = (int32_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_UINT32:
      *((uint32_t *)key_value_pair[i].value) = (uint32_t)strtoll(str, NULL, 0);
      break;
    case XML_VALUE_FLOAT:
      *((float *)key_value_pair[i].value) = atof(str);
      break;
    default:
      SERR(" Invalid value_type = %d", key_value_pair[i].value_type);
      return FALSE;
    }

    xmlFree(str);
  }

  return TRUE;
}


/**
 * sensor_xml_util_get_lens_type: Get the lens_type from string
 *
 * @lens_type_str: lens_type in string
 * @lens_type: lens_type in hal format
 *
 * Returns TRUE is lens_type is valid.
 */

static boolean sensor_xml_util_get_lens_type(
  char* lens_type_str, cam_lens_type_t *lens_type)
{
  if (!strncmp(lens_type_str, "NORMAL", strlen(lens_type_str) + 1) ||
    !strlen(lens_type_str))
    *lens_type = CAM_LENS_NORMAL;
  else if (!strncmp(lens_type_str, "WIDE", strlen(lens_type_str) + 1))
    *lens_type = CAM_LENS_WIDE;
  else if (!strncmp(lens_type_str, "TELE", strlen(lens_type_str) + 1))
    *lens_type = CAM_LENS_TELE;
  else
    return FALSE;

  return TRUE;
}

static boolean sensor_xml_util_get_lens_info(xmlDocPtr docPtr,
  xmlNodePtr nodePtr, camera_module_config_t *configPtr)
{
  uint32_t num_pairs = 0;
  struct xmlKeyValuePair key_value_pair[20];
  char                   lens_type_str[25];

  memset(lens_type_str, 0, sizeof(lens_type_str));
  lens_type_str[24] = '\0';

  strlcpy(key_value_pair[num_pairs].key, "FocalLength", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.focal_length;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "FNumber", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.f_number;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "TotalFocusDistance", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.total_f_dist;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "HorizontalViewAngle", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.hor_view_angle;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "VerticalViewAngle", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.ver_view_angle;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "MinFocusDistance", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.min_focus_distance;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "LensType", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = lens_type_str;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  strlcpy(key_value_pair[num_pairs].key, "MaxRollDegree", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.max_roll_degree;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;
  strlcpy(key_value_pair[num_pairs].key, "MaxPitchDegree", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.max_pitch_degree;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;
  strlcpy(key_value_pair[num_pairs].key, "MaxYawDegree", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->lens_info.max_yaw_degree;
  key_value_pair[num_pairs].value_type = XML_VALUE_FLOAT;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  RETURN_ON_FALSE(sensor_xml_util_get_node_data(
    docPtr, nodePtr, "LensInfo", key_value_pair, num_pairs, 0));

  RETURN_ON_FALSE(sensor_xml_util_get_lens_type(lens_type_str,
    &configPtr->lens_info.lens_type));

  SLOW("lens_type = %s", configPtr->lens_info.lens_type);
  SLOW("focal_length = %f", configPtr->lens_info.focal_length);
  SLOW("f_number = %f", configPtr->lens_info.f_number);
  SLOW("total_f_dist = %f", configPtr->lens_info.total_f_dist);
  SLOW("hor_view_angle = %f", configPtr->lens_info.hor_view_angle);
  SLOW("ver_view_angle = %f", configPtr->lens_info.ver_view_angle);
  SLOW("min_focus_distance = %f", configPtr->lens_info.min_focus_distance);
  SLOW("max_roll_degree = %f", configPtr->lens_info.max_roll_degree);
  SLOW("max_pitch_degree = %f", configPtr->lens_info.max_pitch_degree);
  SLOW("max_yaw_degree = %f", configPtr->lens_info.max_yaw_degree);

  return TRUE;
}

/**
 * sensor_xml_util_get_csi_info: updated csi info from xml to configPtr
 *
 * @pNodePtr: pointer of parent node
 * @configPtr: buffer in which csi info has tobe updated.
 */
static boolean sensor_xml_util_get_csi_info(xmlDocPtr docPtr,
  xmlNodePtr nodePtr, camera_module_config_t *configPtr)
{
  uint32_t num_pairs = 0;
  struct xmlKeyValuePair key_value_pair[5];

  configPtr->camera_csi_params.csid_core = 0xFF;

  strlcpy(key_value_pair[num_pairs].key, "CSIDCore", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->camera_csi_params.csid_core;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT8;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "LaneMask", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->camera_csi_params.lane_mask;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT16;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "LaneAssign", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->camera_csi_params.lane_assign;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT16;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "ComboMode", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->camera_csi_params.combo_mode;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT8;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  RETURN_ON_FALSE(sensor_xml_util_get_node_data(
    docPtr, nodePtr, "CSIInfo", key_value_pair, num_pairs, 0));

  RETURN_FALSE_IF(configPtr->camera_csi_params.csid_core > MAX_CSID_CORE_VAL,
    "csid_core = %d", configPtr->camera_csi_params.csid_core);

  SLOW("csid_core = %d", configPtr->camera_csi_params.csid_core);
  SLOW("lane_mask = 0x%X", configPtr->camera_csi_params.lane_mask);
  SLOW("lane_assign = 0x%X", configPtr->camera_csi_params.lane_assign);
  SLOW("combo_mode = 0x%d", configPtr->camera_csi_params.combo_mode);

  return TRUE;
}

static boolean sensor_xml_util_parse_special_mode(
  char *special_mode, module_chromatix_name_t *chromatix_name)
{
  char     delimiter[2] = "";
  char    *pch = NULL;
  char    *save = NULL;
  uint32_t i = 0;

  if (strstr(special_mode, "|") && strstr(special_mode, ",")) {
    SERR("special_mode check fail special_mode = %s", special_mode);
    return FALSE;
  }

  if (strstr(special_mode, "|")) {
    delimiter[0] = '|';
    chromatix_name->special_mode_type = 1;
  }
  else if (strstr(special_mode, ","))
    delimiter[0] = ',';

  pch = strtok_r(special_mode, delimiter, &save);
  while(pch) {
    for (i = 0; i < SENSOR_SPECIAL_MODE_MAX; i++) {
      if (!strncmp(pch, special_mode_string[i], strlen(pch) + 1)) {
        chromatix_name->special_mode_mask |= (1LL << i);
        break;
      }
    }
    pch = strtok_r(NULL, delimiter, &save);
  }
  return TRUE;
}

static boolean sensor_xml_util_parse_chromatix_name(
  camera_module_config_t *configPtr, xmlNodePtr nodePtr,
  xmlDocPtr chromatixDocPtr, uint8_t is_common_chroamtix)
{
  uint8_t                count = 0;
  uint8_t                chroamtix_arr_size = 0;
  uint8_t                num_pairs = 0;
  struct xmlKeyValuePair key_value_pair[20];
  uint8_t                i = configPtr->chromatix_info.size;
  char                   special_mode_str[255];

  chroamtix_arr_size = sensor_xml_util_get_num_nodes(nodePtr, "ChromatixName");
  if (chroamtix_arr_size == 0) {
    SERR("No nodes in ChromatixName");
    return FALSE;
  }

  SLOW("num_modes = %d", chroamtix_arr_size);

  for (count = 0; count < chroamtix_arr_size; count++, i++) {
    num_pairs = 0;

    if (is_common_chroamtix) {
      configPtr->chromatix_info.chromatix_name[i].sensor_resolution_index =
        0xFF;
    } else {
      strlcpy(key_value_pair[num_pairs].key, "sensor_resolution_index",
        MAX_KEY_SIZE);
      key_value_pair[num_pairs].value =
        &(configPtr->chromatix_info.chromatix_name[i].sensor_resolution_index);
      key_value_pair[num_pairs].value_type = XML_VALUE_UINT8;
      key_value_pair[num_pairs].nodeType = XML_ATTRIBUTE_NODE;
      num_pairs++;
    }

    memset(special_mode_str, 0, sizeof (special_mode_str));
    strlcpy(key_value_pair[num_pairs].key, "special_mode_mask", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value = special_mode_str;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ATTRIBUTE_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "ISPCommon", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].isp_common;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "ISPPreview", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].isp_preview;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "ISPSnapshot", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].isp_snapshot;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "ISPVideo", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].isp_video;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "CPPPreview", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].cpp_preview;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "CPPSnapshot", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].cpp_snapshot;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "CPPVideo", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].cpp_video;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "CPPLiveshot", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].cpp_liveshot;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "PostProc", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].postproc;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "A3Preview", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].a3_preview;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "A3Video", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].a3_video;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "External", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].external;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    strlcpy(key_value_pair[num_pairs].key, "Iot", MAX_KEY_SIZE);
    key_value_pair[num_pairs].value =
      configPtr->chromatix_info.chromatix_name[i].iot;
    key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
    key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
    num_pairs++;

    RETURN_ON_FALSE(sensor_xml_util_get_node_data(chromatixDocPtr, nodePtr,
      "ChromatixName", key_value_pair, num_pairs, count));

    RETURN_ON_FALSE(sensor_xml_util_parse_special_mode(special_mode_str,
        &(configPtr->chromatix_info.chromatix_name[i])));

    SLOW("special_mode_mask[%d] = %lld", i,
      configPtr->chromatix_info.chromatix_name[i].special_mode_mask);
    SLOW("sensor_resolution_index[%d] = %d", i,
      configPtr->chromatix_info.chromatix_name[i].sensor_resolution_index);
    SLOW("isp_common[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].isp_common);
    SLOW("isp_preview[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].isp_preview);
    SLOW("isp_snapshot[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].isp_snapshot);
    SLOW("isp_video[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].isp_video);

    SLOW("cpp_preview[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].cpp_preview);
    SLOW("cpp_snapshot[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].cpp_snapshot);
    SLOW("cpp_video[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].cpp_video);
    SLOW("cpp_liveshot[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].cpp_liveshot);

    SLOW("postproc[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].postproc);
    SLOW("a3_preview[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].a3_preview);
    SLOW("a3_video[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].a3_video);
    SLOW("iot[%d] = %s", i,
      configPtr->chromatix_info.chromatix_name[i].iot);
  }

  configPtr->chromatix_info.size+= chroamtix_arr_size;
  return TRUE;
}
/**
 * sensor_xml_util_parse_chromatix_config: updates chromatix from xml to configPtr
 *
 * @configPtr: buffer in which chromatix has tobe updated.
 */
static boolean sensor_xml_util_parse_chromatix_config(
  camera_module_config_t *configPtr)
{
  boolean                ret = FALSE;
  xmlNodePtr             nodePtr = NULL;
  xmlDocPtr              chromatixDocPtr = NULL;
  xmlNodePtr             chromatixRootPtr = NULL;
  char chromatix_xml_name[NAME_SIZE_MAX];

  /* Create the xml path */
  snprintf(chromatix_xml_name, NAME_SIZE_MAX, "%s%s.xml",
    CONFIG_XML_PATH, configPtr->chromatix_name);

  if (access(chromatix_xml_name, R_OK)) {
    SHIGH("cannot read file %s. Trying from system partition",
      chromatix_xml_name);
    /* Create the xml path from system partition */
    snprintf(chromatix_xml_name, NAME_SIZE_MAX, "%s%s.xml",
      CONFIG_XML_SYSTEM_PATH, configPtr->chromatix_name);
    if (access(chromatix_xml_name, R_OK)) {
      SERR("Cannot read file from %s. read failed",
        chromatix_xml_name);
      return FALSE;
    }
  }

  SLOW("reading from file %s", chromatix_xml_name);

  /* Get the Root pointer and Document pointer of XMl file */
  RETURN_ON_FALSE(sensor_xml_util_load_file(chromatix_xml_name,
    &chromatixDocPtr, &chromatixRootPtr, "ChromatixConfigurationRoot"));

  configPtr->chromatix_info.size = 0;

  nodePtr = sensor_xml_util_get_node(chromatixRootPtr,
    "ResolutionChromatixInfo", 0);
  if (nodePtr ==  NULL) {
    ret = FALSE;
    SERR("Could not get nodePtr for ResolutionChromatixInfo");
    goto CHROMATIX_CONFIG_EXIT;
  }

  ret = sensor_xml_util_parse_chromatix_name(
    configPtr, nodePtr, chromatixDocPtr, FALSE);
  if (ret == FALSE) {
    SERR("sensor_xml_util_parse_chromatix_name failed");
    goto CHROMATIX_CONFIG_EXIT;
  }

  nodePtr = sensor_xml_util_get_node(chromatixRootPtr,
    "CommonChromatixInfo", 0);
  if (nodePtr ==  NULL) {
    ret = FALSE;
    SERR("Could not get nodePtr for CommonChromatixInfo");
    goto CHROMATIX_CONFIG_EXIT;
  }

  ret = sensor_xml_util_parse_chromatix_name(
    configPtr, nodePtr, chromatixDocPtr, TRUE);
  if (ret == FALSE) {
    SERR("sensor_xml_util_parse_chromatix_name failed");
    goto CHROMATIX_CONFIG_EXIT;
  }

CHROMATIX_CONFIG_EXIT:
  sensor_xml_util_unload_file(chromatixDocPtr);
  return ret;
}

boolean sensor_xml_util_get_camera_full_config(
  struct xmlCameraConfigInfo *pXmlCameraConfig)
{
  xmlDocPtr               docPtr = pXmlCameraConfig->docPtr;
  xmlNodePtr              nodePtr = pXmlCameraConfig->nodePtr;
  camera_module_config_t *configPtr = pXmlCameraConfig->configPtr;

  /* Read the Lens info */
  RETURN_ON_FALSE(sensor_xml_util_get_lens_info(docPtr, nodePtr, configPtr));

  /* Read the CSI info */
  RETURN_ON_FALSE(sensor_xml_util_get_csi_info(docPtr, nodePtr, configPtr));

  /* Read the chromatix info */
  if (xmlStrlen((const xmlChar *)(configPtr->chromatix_name))) {
    RETURN_ON_FALSE(sensor_xml_util_parse_chromatix_config(configPtr));
  } else {
    SHIGH(" No chromatix xml mentioned for %s", configPtr->sensor_name);
  }

  return TRUE;
}

/**
 * sensor_xml_util_get_position_num: Get the camera position number from string
 *
 * @position: position information in string
 *
 * Returns the camera position enumeration value
 */

static boolean sensor_xml_util_get_position_num(
  char* position_str, cam_position_t *position)
{
  if (!strncmp(position_str, "BACK", strlen(position_str) + 1))
    *position = CAM_POSITION_BACK;
  else if (!strncmp(position_str, "FRONT", strlen(position_str) + 1))
    *position = CAM_POSITION_FRONT;
  else if (!strncmp(position_str, "BACK_AUX", strlen(position_str) + 1))
    *position = CAM_POSITION_BACK_AUX;
  else if (!strncmp(position_str, "FRONT_AUX", strlen(position_str) + 1))
    *position = CAM_POSITION_FRONT_AUX;
  else
    return FALSE;

  return TRUE;
}

/**
 * sensor_xml_util_get_i2cfrequency_mode: Get the i2c frequency of sensor
 *
 * @i2cfrequencymode: i2c frequency information in string
 *
 * Returns the i2c frequency enumeration value
 */
static void sensor_xml_util_get_i2cfrequency_mode(
  char* i2cfrequency_str,  enum camera_i2c_freq_mode *i2cfreqmode)
{
  if (!strncmp(i2cfrequency_str, "STANDARD", strlen(i2cfrequency_str) + 1))
    *i2cfreqmode = SENSOR_I2C_MODE_STANDARD;
  else if (!strncmp(i2cfrequency_str, "FAST", strlen(i2cfrequency_str) + 1))
    *i2cfreqmode = SENSOR_I2C_MODE_FAST;
  else if (!strncmp(i2cfrequency_str, "CUSTOM", strlen(i2cfrequency_str) + 1))
    *i2cfreqmode = SENSOR_I2C_MODE_CUSTOM;
  else if (!strncmp(i2cfrequency_str, "FAST_PLUS", strlen(i2cfrequency_str) + 1))
    *i2cfreqmode = SENSOR_I2C_MODE_FAST_PLUS;
  else
    *i2cfreqmode = SENSOR_I2C_MODE_MAX;
}

/**
 * sensor_xml_util_get_camera_probe_config: Updates buffer with probe info from xml
 *
 * @configPtr: buffer in which info has to be updated.
 * @docPtr: pointer of xml document.
 * @nodePtr: pointer of node from which data should be copied.
 */
boolean sensor_xml_util_get_camera_probe_config(
  struct xmlCameraConfigInfo *pXmlCameraConfig, const char* parentName)
{
  uint32_t                num_pairs = 0;
  xmlDocPtr               docPtr = pXmlCameraConfig->docPtr;
  xmlNodePtr              nodePtr = pXmlCameraConfig->nodePtr;
  camera_module_config_t *configPtr = pXmlCameraConfig->configPtr;
  struct xmlKeyValuePair  key_value_pair[20];
  char                    positionStr[25], i2cfrequencyStr[25];

  RETURN_ON_NULL(docPtr);
  RETURN_ON_NULL(nodePtr);
  RETURN_ON_NULL(configPtr);
  RETURN_ON_NULL(parentName);

  /* Reset the camera config structure */
  memset(configPtr, 0, sizeof(camera_module_config_t));
  configPtr->camera_id = 0xFF;

  strlcpy(key_value_pair[num_pairs].key, "VendorName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->vendor_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "SensorName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->sensor_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "ActuatorName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->actuator_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "EepromName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->eeprom_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "OisName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->ois_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "FlashName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->flash_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "ChromatixName", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = configPtr->chromatix_name;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "I2CFrequencyMode", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = i2cfrequencyStr;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "CameraId", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->camera_id;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT8;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "ModesSupported", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->modes_supported;
  key_value_pair[num_pairs].value_type = XML_VALUE_INT32;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "Position", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = positionStr;
  key_value_pair[num_pairs].value_type = XML_VALUE_STRING;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "MountAngle", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->sensor_mount_angle;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT32;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  strlcpy(key_value_pair[num_pairs].key, "SensorSlaveAddress", MAX_KEY_SIZE);
  key_value_pair[num_pairs].value = &configPtr->sensor_slave_addr;
  key_value_pair[num_pairs].value_type = XML_VALUE_UINT16;
  key_value_pair[num_pairs].nodeType = XML_ELEMENT_NODE;
  num_pairs++;

  RETURN_ON_FALSE(sensor_xml_util_get_node_data(
    docPtr, nodePtr, parentName, key_value_pair, num_pairs, 0));

  positionStr[24] = '\0';
  RETURN_ON_FALSE(
    sensor_xml_util_get_position_num(positionStr, &configPtr->position));

  /* Not treated as failure cause this is an optional node */
  i2cfrequencyStr[24] = '\0';
  sensor_xml_util_get_i2cfrequency_mode(i2cfrequencyStr, &configPtr->i2c_freq_mode);

  SLOW("sensor_name = %s", configPtr->sensor_name);
  SLOW("actuator_name = %s", configPtr->actuator_name);
  SLOW("flash_name = %s", configPtr->flash_name);
  SLOW("eeprom_name = %s", configPtr->eeprom_name);
  SLOW("ois_name = %s", configPtr->ois_name);
  SLOW("camera_id = %d", configPtr->camera_id);
  SLOW("i2c_freq_mode = %d", configPtr->i2c_freq_mode);

  RETURN_FALSE_IF(configPtr->camera_id > MAX_CAMERA_ID_VAL,
    "camera_id = %d", configPtr->camera_id);

  return TRUE;
}

