/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "pa_qti_effect"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#define PA_QTI_MODULE_ID "primary"
#define PA_QTI_EFFECT_OBJ_PATH "/org/pulseaudio/core1/effect"
#define PA_QTI_EFFECT_MODULE_IFACE "org.PulseAudio.Core1.Effect"
#define PA_QTI_EFFECT_SESSION_IFACE "org.PulseAudio.Core1.Effect.Session"
#define PA_QTI_EFFECT_MAX_G_OBJ_PATH 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pa_qti_effect.h"

#include <gio/gio.h>

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) \
        (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

struct pa_qti_effect_module_data {
    GDBusConnection *conn;
    char g_obj_path[PA_QTI_EFFECT_MAX_G_OBJ_PATH];
};

struct pa_qti_effect_session_data {
    char *obj_path;
};

static struct pa_qti_effect_module_data *effect_mdata = NULL;

static void get_uuid(GVariantIter *struct_u,
                            pa_qti_effect_uuid_t *uuid) {
        int n_elements = 0;
        gsize element_size = sizeof(guchar);
        GVariant *array_v;
        gconstpointer value;
        g_variant_iter_next(struct_u, "u", &uuid->timeLow);
        g_variant_iter_next(struct_u, "q", &uuid->timeMid);
        g_variant_iter_next(struct_u, "q", &uuid->timeHiAndVersion);
        g_variant_iter_next(struct_u, "q", &uuid->clockSeq);
        array_v = g_variant_iter_next_value(struct_u);
        value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
        memscpy(&uuid->node[0], 6, value, n_elements);
}

int pa_qti_effect_get_version(void) {
    gint version;
    GVariant *result;
    GError *error = NULL;

    g_debug("%s\n", __func__);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "GetVersion",
                                         NULL,
                                         G_VARIANT_TYPE("(i)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking EffectGetVersion: %s\n", __func__, error->message);
        g_error_free(error);
        version = -EINVAL;
        goto exit;
    }

    g_variant_get(result, "(i)", &version);
    g_info("%s: The server answered: version: '%d'\n", __func__, version);
    g_variant_unref(result);
exit:
    return version;
}

int pa_qti_effect_get_descriptor(pa_qti_effect_handle_t effect_handle,
                                 const pa_qti_effect_uuid_t uuid,
                                 pa_qti_effect_descriptor_t *desc) {
    struct pa_qti_effect_session_data *ses_data =
                                 (struct pa_qti_effect_session_data *)effect_handle;
    GVariant *result, *struct_v, *struct_v2, *struct_v3;
    GVariant *array_v3, *array_v4;
    GVariant *value_1, *value_arr;
    GVariant *argument;
    GVariantBuilder builder_1;
    GVariantIter arg_i, struct_i, struct_ii, struct_iii;
    GError *error = NULL;
    gconstpointer value;
    gsize n_elements = 0;
    gsize element_size = sizeof(guchar);
    int rc = 0;

    if (!ses_data) {
        g_printerr("Invalid effect handle\n");
        rc = -EINVAL;
        goto exit;
    }

    g_debug("%s\n", __func__);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_1 = g_variant_builder_end(&builder_1);

    argument = g_variant_new("(@(uqqqay))", value_1);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         ses_data->obj_path,
                                         PA_QTI_EFFECT_SESSION_IFACE,
                                         "GetDescriptor",
                                         argument,
                                         G_VARIANT_TYPE("(((uqqqay)(uqqqay)uuqqayay))"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking EffectGetDescriptor: %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_variant_iter_init(&arg_i, result);
    struct_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&struct_i, struct_v);
    struct_v2 = g_variant_iter_next_value(&struct_i);
    g_variant_iter_init(&struct_ii, struct_v2);

    get_uuid(&struct_ii, &desc->type);

    struct_v3 = g_variant_iter_next_value(&struct_i);
    g_variant_iter_init(&struct_iii, struct_v3);

    get_uuid(&struct_iii, &desc->uuid);

    g_variant_iter_next(&struct_i, "u", &desc->api_version);
    g_variant_iter_next(&struct_i, "u", &desc->flags);
    g_variant_iter_next(&struct_i, "q", &desc->cpuLoad);
    g_variant_iter_next(&struct_i, "q", &desc->memoryUsage);
    array_v3 = g_variant_iter_next_value(&struct_i);
    value = g_variant_get_fixed_array(array_v3, &n_elements, element_size);
    memscpy(desc->name, n_elements, value, n_elements);
    array_v4 = g_variant_iter_next_value(&struct_i);
    value = g_variant_get_fixed_array(array_v4, &n_elements, element_size);
    memscpy(desc->implementor, n_elements, value, n_elements);
    g_variant_unref(result);

exit:
    return rc;
}

int pa_qti_effect_command(pa_qti_effect_handle_t effect_handle,
                          uint32_t cmd_code,
                          uint32_t cmd_size,
                          void *cmd_data,
                          uint32_t reply_size,
                          void *reply_data) {
    struct pa_qti_effect_session_data *ses_data =
                                 (struct pa_qti_effect_session_data *)effect_handle;
    GError *error = NULL;
    GVariant *argument;
    GVariant *value_1, *value_2, *value_3;
    GVariant *result, *array_v;
    gconstpointer value;
    int rc = 0;
    gsize n_elements = 0;
    gsize element_size = sizeof(guchar);

    if (!ses_data) {
        g_printerr("%s: Invalid effect handle\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    if (!reply_data) {
        g_printerr("%s: Reply data is NULL\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    g_debug("%s\n", __func__);

    value_1 = g_variant_new_uint32(cmd_code);
    value_2 = g_variant_new_uint32(cmd_size);
    value_3 = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
              (gconstpointer)(cmd_data),
              cmd_size, sizeof(gchar));
    argument =  g_variant_new("(@u@u@ay)", value_1, value_2, value_3);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         ses_data->obj_path,
                                         PA_QTI_EFFECT_SESSION_IFACE,
                                         "Command",
                                         argument,
                                         G_VARIANT_TYPE("(ay)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking EffectCommand: %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    array_v = g_variant_get_child_value(result, 0);
    value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
    memscpy(reply_data, reply_size, value, n_elements);
    g_variant_unref(result);

exit:
    return rc;
}

int pa_qti_loopback_effect_release(pa_qti_effect_handle_t effect_handle) {
    struct pa_qti_effect_session_data *ses_data =
                                 (struct pa_qti_effect_session_data *)effect_handle;
    gint rc = 0;
    GVariant *result;
    GError *error = NULL;

    if (!ses_data) {
        g_printerr("%s: Invalid effect handle\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    g_debug("%s\n", __func__);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         ses_data->obj_path,
                                         PA_QTI_EFFECT_SESSION_IFACE,
                                         "LoopbackEffectRelease",
                                         NULL,
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking LoopbackEffectRelease: %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);
    g_free(ses_data);

exit:
    return rc;
}

int pa_qti_port_effect_release(pa_qti_effect_handle_t effect_handle) {
    struct pa_qti_effect_session_data *ses_data =
                                 (struct pa_qti_effect_session_data *)effect_handle;
    gint rc = 0;
    GVariant *result;
    GError *error = NULL;

    if (!ses_data) {
        g_printerr("%s: Invalid effect handle\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    g_debug("%s\n", __func__);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         ses_data->obj_path,
                                         PA_QTI_EFFECT_SESSION_IFACE,
                                         "PortEffectRelease",
                                         NULL,
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking PortEffectRelease: %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);
    g_free(ses_data);

exit:
    return rc;
}

int pa_qti_sink_effect_release(pa_qti_effect_handle_t effect_handle) {
    struct pa_qti_effect_session_data *ses_data =
                                 (struct pa_qti_effect_session_data *)effect_handle;
    gint rc = 0;
    GVariant *result;
    GError *error = NULL;

    if (!ses_data) {
        g_printerr("%s: Invalid effect handle\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    g_debug("%s\n", __func__);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         ses_data->obj_path,
                                         PA_QTI_EFFECT_SESSION_IFACE,
                                         "SinkEffectRelease",
                                         NULL,
                                         NULL,
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking SinkEffectRelease: %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);
    g_free(ses_data);

exit:
    return rc;
}

pa_qti_effect_handle_t pa_qti_port_effect_create(const pa_qti_effect_uuid_t uuid,
                                                 const char *port_name) {
    struct pa_qti_effect_session_data *ses_data;
    GVariant *result;
    GVariant *value_1, *value_arr, *argument;
    GVariantBuilder builder_1;
    GError *error = NULL;
    pa_qti_effect_handle_t effect_handle = NULL;

    ses_data = g_malloc0(sizeof(struct pa_qti_effect_session_data));
    g_debug("%s\n", __func__);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_1 = g_variant_builder_end(&builder_1);

    argument = g_variant_new("(@(uqqqay)@s)", value_1, g_variant_new_string(port_name));

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "PortEffectCreate",
                                         argument,
                                         G_VARIANT_TYPE("(o)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking PortEffectCreate: %s\n", __func__, error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(o)", &ses_data->obj_path);
    g_info("%s: The server answered : obj path: %s \n", __func__, ses_data->obj_path);
    g_variant_unref(result);
    effect_handle = (pa_qti_effect_handle_t) ses_data;

    return effect_handle;

exit:
    if (ses_data)
        g_free(ses_data);
    return NULL;
}

pa_qti_effect_handle_t pa_qti_loopback_effect_create(const pa_qti_effect_uuid_t uuid,
                                                     uint64_t patch_id) {
    struct pa_qti_effect_session_data *ses_data;
    GVariant *result;
    GVariant *value_1, *value_arr, *argument;
    GVariantBuilder builder_1;
    GError *error = NULL;
    pa_qti_effect_handle_t effect_handle = NULL;

    ses_data = g_malloc0(sizeof(struct pa_qti_effect_session_data));
    g_debug("%s\n", __func__);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_1 = g_variant_builder_end(&builder_1);

    argument = g_variant_new("(@(uqqqay)@t)", value_1, g_variant_new_uint64(patch_id));

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "LoopbackEffectCreate",
                                         argument,
                                         G_VARIANT_TYPE("(o)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking LoopbackEffectCreate: %s\n", __func__, error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(o)", &ses_data->obj_path);
    g_info("%s: The server answered : obj path: %s \n", __func__, ses_data->obj_path);
    g_variant_unref(result);
    effect_handle = (pa_qti_effect_handle_t) ses_data;

    return effect_handle;

exit:
    if (ses_data)
        g_free(ses_data);
    return NULL;
}

pa_qti_effect_handle_t pa_qti_sink_effect_create(const pa_qti_effect_uuid_t uuid,
                                                 uint32_t sink_index) {
    struct pa_qti_effect_session_data *ses_data;
    GVariant *result;
    GVariant *value_1, *value_arr, *argument;
    GVariantBuilder builder_1;
    GError *error = NULL;
    pa_qti_effect_handle_t effect_handle = NULL;

    ses_data = g_malloc0(sizeof(struct pa_qti_effect_session_data));
    g_debug("%s\n", __func__);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_1 = g_variant_builder_end(&builder_1);

    argument = g_variant_new("(@(uqqqay)@u)", value_1, g_variant_new_uint32(sink_index));

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "SinkEffectCreate",
                                         argument,
                                         G_VARIANT_TYPE("(o)"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking SinkEffectCreate: %s\n", __func__, error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(o)", &ses_data->obj_path);
    g_info("%s: The server answered : obj path: %s \n", __func__, ses_data->obj_path);
    g_variant_unref(result);
    effect_handle = (pa_qti_effect_handle_t) ses_data;

    return effect_handle;

exit:
    if (ses_data)
        g_free(ses_data);
    return NULL;
}

int pa_qti_port_get_supported_effects(const char *port_name,
                                      uint32_t *port_effects,
                                      uint32_t *device_index,
                                      pa_qti_effect_uuid_t *uuid_list) {
    GVariant *argument = NULL;
    GVariant *result, *array_v, *struct_v;
    GVariantIter arg_i, struct_i, array_i;
    GError *error = NULL;
    int i = 0;
    int rc = 0;

    g_debug("%s\n", __func__);

    argument = g_variant_new("(@s)", g_variant_new_string(port_name));

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "GetPortSupportedEffects",
                                         argument,
                                         G_VARIANT_TYPE("(uua(uqqqay))"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking GetPortSupportedEffects: %s\n", __func__, error->message);
        g_error_free(error);
        return -EINVAL;
    }

    g_variant_iter_init(&arg_i, result);
    g_variant_iter_next(&arg_i, "u", port_effects);
    g_variant_iter_next(&arg_i, "u", device_index);
    array_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&array_i, array_v);
    for (i = 0; i < *port_effects; i++) {
        struct_v = g_variant_iter_next_value(&array_i);
        g_variant_iter_init(&struct_i, struct_v);
        get_uuid(&struct_i, &uuid_list[i]);
    }
    return rc;
}

int pa_qti_sink_get_supported_effects(uint32_t sink_index,
                                      uint32_t *sink_effects,
                                      pa_qti_effect_uuid_t *uuid_list) {
    GVariant *argument = NULL;
    GVariant *result, *array_v, *struct_v;
    GVariantIter arg_i, struct_i, array_i;
    GError *error = NULL;
    int i = 0;
    int rc = 0;

    g_debug("%s\n", __func__);

    argument = g_variant_new("(@u)", g_variant_new_uint32(sink_index));

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "GetSinkSupportedEffects",
                                         argument,
                                         G_VARIANT_TYPE("(ua(uqqqay))"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking GetSinkSupportedEffects: %s\n", __func__, error->message);
        g_error_free(error);
        return -EINVAL;
    }

    g_variant_iter_init(&arg_i, result);
    g_variant_iter_next(&arg_i, "u", sink_effects);
    array_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&array_i, array_v);
    for (i = 0; i < *sink_effects; i++) {
        struct_v = g_variant_iter_next_value(&array_i);
        g_variant_iter_init(&struct_i, struct_v);
        get_uuid(&struct_i, &uuid_list[i]);
    }

    return rc;
}

int pa_qti_module_get_supported_effects(uint32_t *num_effects,
                                        pa_qti_effect_descriptor_t *desc) {
    GError *error = NULL;
    GVariant *result, *struct_r, *struct_v, *struct_d, *struct_e;
    GVariant *array_v3, *array_v4;
    GVariantIter arg_i, struct_i, array_d, struct_u, struct_c;
    gconstpointer value;
    gsize element_size = sizeof(guchar);
    gsize n_elements = 0;
    int i = 0;
    int rc = 0;

    g_debug("%s\n", __func__);

    result = g_dbus_connection_call_sync(effect_mdata->conn,
                                         NULL,
                                         effect_mdata->g_obj_path,
                                         PA_QTI_EFFECT_MODULE_IFACE,
                                         "GetModuleSupportedEffects",
                                         NULL,
                                         G_VARIANT_TYPE("(ua((uqqqay)(uqqqay)uuqqayay))"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         NULL,
                                         &error);

    if (result == NULL) {
        g_printerr("%s: Error invoking GetModuleSupportedEffects(): %s\n", __func__, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_variant_iter_init(&arg_i, result);
    g_variant_iter_next(&arg_i, "u", num_effects);
    struct_r = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&array_d, struct_r);

    for (i = 0; i < *num_effects; i++) {
        struct_d = g_variant_iter_next_value(&array_d);
        g_variant_iter_init(&struct_c, struct_d);
        struct_e = g_variant_iter_next_value(&struct_c);
        g_variant_iter_init(&struct_u, struct_e);

        get_uuid(&struct_u, &desc[i].type);

        struct_v = g_variant_iter_next_value(&struct_c);
        g_variant_iter_init(&struct_i, struct_v);

        get_uuid(&struct_i, &desc[i].uuid);

        g_variant_iter_next(&struct_c, "u", &desc[i].api_version);
        g_variant_iter_next(&struct_c, "u", &desc[i].flags);
        g_variant_iter_next(&struct_c, "q", &desc[i].cpuLoad);
        g_variant_iter_next(&struct_c, "q", &desc[i].memoryUsage);
        array_v3 = g_variant_iter_next_value(&struct_c);
        value = g_variant_get_fixed_array(array_v3, &n_elements, element_size);
        memscpy(desc[i].name, n_elements, value, n_elements);
        array_v4 = g_variant_iter_next_value(&struct_c);
        value = g_variant_get_fixed_array(array_v4, &n_elements, element_size);
        memscpy(desc[i].implementor, n_elements, value, n_elements);
    }
    g_variant_unref(result);

exit:
    return rc;
}

void pa_qti_effect_deinit(void)
{
    g_debug("%s\n", __func__);
    if (!effect_mdata)
        g_free(effect_mdata);

    effect_mdata = NULL;
}

int pa_qti_effect_init(void)
{
    const gchar *s_address = NULL;
    GError *error = NULL;
    int rc = 0;

    g_debug("%s\n", __func__);

    if (!effect_mdata) {
        effect_mdata = g_malloc0(sizeof(struct pa_qti_effect_module_data));
    } else {
        g_info("%s: Effects already initialized\n", __func__);
        return EBUSY;
    }

    s_address = getenv("PULSE_DBUS_SERVER");
    if (!s_address) {
        g_info("Pulse DBus server address not set, use default address\n");
        effect_mdata->conn = g_dbus_connection_new_for_address_sync(
                             "unix:path=/var/run/pulse/dbus-socket",
                             G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
                             NULL, NULL, &error);
    } else {
        g_info("server address %s\n", s_address);
        effect_mdata->conn = g_dbus_connection_new_for_address_sync(s_address,
                             G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
                             NULL, NULL, &error);
    }

    if (effect_mdata->conn == NULL) {
        g_printerr("%s: Error connecting to D-Bus address %s: %s\n", __func__, s_address, error->message);
        g_error_free(error);
        rc = -EINVAL;
        goto exit;
    }

    g_snprintf(effect_mdata->g_obj_path, PA_QTI_EFFECT_MAX_G_OBJ_PATH,
               "%s/%s", PA_QTI_EFFECT_OBJ_PATH, PA_QTI_MODULE_ID);
    return rc;

exit:
    if (!effect_mdata)
        free(effect_mdata);
    return rc;
}
