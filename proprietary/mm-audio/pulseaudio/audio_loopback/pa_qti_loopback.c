/* pa_qti_loopback.c
*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <gio/gio.h>

#include "pa_qti_loopback.h"

#define PA_QTI_LOOPBACK_DBUS_MODULE_OBJECT_PATH_SIZE 256
#define PA_QTI_LOOPBACK_DBUS_OBJECT_PATH_PREFIX "/org/pulseaudio/ext/qahw"
#define PA_QTI_LOOPBACK_DBUS_MODULE_IFACE "org.PulseAudio.Ext.Loopback"
#define PA_QTI_LOOPBACK_DBUS_SESSION_IFACE "org.PulseAudio.Ext.Loopback.Session"

/******* Global variables ********/
static struct pa_qti_loopback_module_data {
    GDBusConnection *conn;
    char obj_path[PA_QTI_LOOPBACK_DBUS_MODULE_OBJECT_PATH_SIZE];
    GHashTable *ses_hash_table;
} *g_mod_data;

struct pa_qti_loopback_session_data {
    char *obj_path;
    GThread *thread_loop;
    GMainLoop *loop;
    guint sub_id;
    pa_qti_loopback_event_callback_t callback;
};

/******* Helper functions ********/
int pa_qti_loopback_init(const char *module_name) {
    char module_string[128];
    const gchar *s_address = NULL;
    GError *error = NULL;
    int ret = 0;

    g_debug("%s enter\n", __func__);

    if (!g_strcmp0(module_name, PA_QTI_LOOPBACK_MODULE_ID_PRIMARY)) {
        g_strlcpy(module_string, "loopback", sizeof(module_string));
    } else {
        g_printerr("Unsupported module %s", module_name);
        ret = -EINVAL;
        goto exit;
    }

    g_mod_data = g_malloc0(sizeof(struct pa_qti_loopback_module_data));
    if (g_mod_data == NULL) {
        g_printerr("Could not allocate memory for module data\n");
        ret = -EINVAL;
        goto exit;
    }

    s_address = getenv("PULSE_DBUS_SERVER");
    if (!s_address) {
        g_info("Unable to obtain server address, using default address\n");
        g_mod_data->conn = g_dbus_connection_new_for_address_sync("unix:path=/var/run/pulse/dbus-socket",
                  G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &error);
    } else {
        g_info("server address %s\n", s_address);
        g_mod_data->conn = g_dbus_connection_new_for_address_sync(s_address,
                  G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &error);
    }

    if (g_mod_data->conn == NULL) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", s_address, error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_snprintf(g_mod_data->obj_path, PA_QTI_LOOPBACK_DBUS_MODULE_OBJECT_PATH_SIZE,
                  "%s/%s", PA_QTI_LOOPBACK_DBUS_OBJECT_PATH_PREFIX, module_string);

    /* hash table to retrieve session information */
    g_mod_data->ses_hash_table = g_hash_table_new(g_direct_hash, g_direct_equal);

exit:
    g_debug("%s exit\n", __func__);
    return ret;

}

void pa_qti_loopback_deinit(void) {
    GError *error = NULL;

    g_debug("%s enter\n", __func__);

    if (g_mod_data) {
        if (g_mod_data->ses_hash_table)
            g_hash_table_destroy(g_mod_data->ses_hash_table);

        if (g_mod_data->conn) {
            if (!g_dbus_connection_close_sync(g_mod_data->conn, NULL, &error)) {
                g_printerr("Error in connection close(): %s\n", error->message);
                g_error_free(error);
            }
        }

        g_free(g_mod_data);
    }

    g_debug("%s exit\n", __func__);
}

static pa_qti_loopback_handle_t pa_qti_loopback_parse_ses_handle(char *obj_path) {
    char **handle_string;
    pa_qti_loopback_handle_t handle;

    handle_string = g_strsplit(obj_path, "_", -1);
    handle = g_ascii_strtoll(handle_string[1], NULL, 0);
    g_strfreev(handle_string);

    return handle;
}

static void pa_qti_loopback_marshal_port_config(GVariantBuilder *builder, struct pa_qti_loopback_port_config *cfg) {
    g_variant_builder_add(builder, "i", (gint32)cfg->id);
    g_variant_builder_add(builder, "i", (gint32)cfg->role);
    g_variant_builder_add(builder, "u", (guint32)cfg->config_mask);
    g_variant_builder_add(builder, "u", (guint32)cfg->sample_rate);
    g_variant_builder_add(builder, "u", (guint32)cfg->num_channels);
    g_variant_builder_add(builder, "i", (gint32)cfg->format);
    g_variant_builder_add(builder, "i", (gint32)cfg->bitwidth);
    g_variant_builder_add(builder, "d", (gdouble)cfg->gain);
    g_variant_builder_add(builder, "s", cfg->port_name);
}

static pa_qti_loopback_event_t pa_qti_loopback_get_event_type(const char *event_string) {
    if (!g_strcmp0(event_string, PA_QTI_LOOPBACK_ERROR_EVENT_STRING))
        return PA_QTI_LOOPBACK_ERROR_EVENT;
    else if (!g_strcmp0(event_string, PA_QTI_LOOPBACK_FORMAT_UPDATE_EVENT_STRING))
        return PA_QTI_LOOPBACK_FORMAT_UPDATE_EVENT;
    else if (!g_strcmp0(event_string, PA_QTI_LOOPBACK_RECREATED_EVENT_STRING))
        return PA_QTI_LOOPBACK_RECREATED_EVENT;

    return PA_QTI_LOOPBACK_ERROR_EVENT;
}

static void on_det_event_callback(GDBusConnection *conn,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer data) {
    struct pa_qti_loopback_session_data *ses_data = (struct pa_qti_loopback_session_data *)data;
    GError *error = NULL;
    GVariant *array_v;
    GVariantIter arg_i, array_i;
    const char *event_string = NULL;
    pa_qti_loopback_event_t event;
    struct pa_qti_loopback_event_data event_data;
    event_data.encoding_string = NULL;
    event_data.channel_maps = NULL;
    int i = 0;

    event_data.channel_maps = g_malloc(sizeof(char *) * NUM_CHANNEL_POSITIONS);

    g_debug("signal handler: Ondetection event signal received\n");
    g_variant_iter_init(&arg_i, parameters);
    g_variant_iter_next(&arg_i, "s", &event_string);
    g_variant_iter_next(&arg_i, "s", &(event_data.port_name));
    g_variant_iter_next(&arg_i, "u", &(event_data.sample_rate));
    g_variant_iter_next(&arg_i, "u", &(event_data.channels));
    g_variant_iter_next(&arg_i, "s", &(event_data.encoding_string));

    array_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&array_i, array_v);
    for (i = 0; i < NUM_CHANNEL_POSITIONS; i++)
        g_variant_iter_next(&array_i, "s", &(event_data.channel_maps[i]));

    /* Retrieve event type from event string */
    event = pa_qti_loopback_get_event_type(event_string);

    ses_data->callback(event, event_data);

    g_free(event_data.channel_maps);

    g_debug("return Ondetection event signal\n");
}

static int subscribe_detection_event(struct pa_qti_loopback_session_data *ses_data,
                                     bool subscribe) {
    GVariant *result;
    GVariant *argument_sig_listener = NULL;
    GError *error = NULL;
    guint id;
    char signal_name[128];
    gint ret = 0;

    g_snprintf(signal_name, sizeof(signal_name),
               "%s.%s", PA_QTI_LOOPBACK_DBUS_SESSION_IFACE, "DetectionEvent");

    if (subscribe) {
       /* Add listener for signal to PulseAudio core.
        * this is done during load of first session i.e. empty hash table
        * Empty obj path array is sent to listen for signals from all objects on
        * this connection
        */
        if (g_hash_table_size(g_mod_data->ses_hash_table) == 0) {
            const gchar *obj_str[] = {};
            argument_sig_listener = g_variant_new("(@s@ao)",
                            g_variant_new_string(signal_name),
                            g_variant_new_objv(obj_str, 0));

            result = g_dbus_connection_call_sync(g_mod_data->conn,
                                    NULL,
                                    "/org/pulseaudio/core1",
                                    "org.PulseAudio.Core1",
                                    "ListenForSignal",
                                    argument_sig_listener,
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);

            if (result == NULL) {
                g_printerr ("Error invoking ListenForSignal(): %s\n", error->message);
                g_error_free(error);
                ret = -EINVAL;
                goto exit;
            }

            g_variant_unref(result);
        }

        /* subscribe for detection event signal */
        ses_data->sub_id = g_dbus_connection_signal_subscribe(g_mod_data->conn,
                           NULL,
                           PA_QTI_LOOPBACK_DBUS_SESSION_IFACE,
                           "DetectionEvent",
                           ses_data->obj_path,
                           NULL,
                           G_DBUS_SIGNAL_FLAGS_NONE,
                           on_det_event_callback,
                           ses_data,
                           NULL);
    } else {
        /* Remove signal listener to PulseAudio core.
        * this is done during unload of last session i.e. hash table size == 1.
        */
        if (g_hash_table_size(g_mod_data->ses_hash_table) == 1) {
            argument_sig_listener = g_variant_new("(@s)",
                            g_variant_new_string(signal_name));

            result = g_dbus_connection_call_sync(g_mod_data->conn,
                                    NULL,
                                    "/org/pulseaudio/core1",
                                    "org.PulseAudio.Core1",
                                    "StopListeningForSignal",
                                    argument_sig_listener,
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);

            if (result == NULL) {
                g_printerr ("Error invoking ListenForSignal(): %s\n", error->message);
                g_error_free(error);
                ret = -EINVAL;
                goto exit;
            }

            g_variant_unref(result);
        }

        if (ses_data->sub_id)
            g_dbus_connection_signal_unsubscribe(g_mod_data->conn, ses_data->sub_id);
    }

exit:
    return ret;
}

static void *signal_threadloop(void *cookie) {
    struct pa_qti_loopback_session_data *ses_data = (struct pa_qti_loopback_session_data *)cookie;

    if (!ses_data) {
        g_printerr("Invalid thread params");
        goto exit;
    }

    ses_data->loop = g_main_loop_new(NULL, FALSE);
    g_debug("initiate main loop run for detections %d\n", ses_data->sub_id);
    g_main_loop_run(ses_data->loop);

    g_main_loop_unref(ses_data->loop);

exit:
    return NULL;
}

/******* Module specific functions ********/
int pa_qti_loopback_get_port_config(struct pa_qti_loopback_port_config *cfg, pa_qti_loopback_port_handle_t id,
                                                                             pa_qti_loopback_port_role_t role) {
    gint ret = 0;
    GError *error = NULL;
    GVariant *result, *struct_v;
    GVariantIter arg_i, struct_i;

    if (!cfg) {
        ret = -EINVAL;
        goto exit;
    }

    g_debug("Calling GetPortConfig\n");
    result = g_dbus_connection_call_sync(g_mod_data->conn,
                            NULL,
                            g_mod_data->obj_path,
                            PA_QTI_LOOPBACK_DBUS_MODULE_IFACE,
                            "GetPortConfig",
                            g_variant_new("(ii)", id, role),
                            G_VARIANT_TYPE("((iiuuuiids))"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

    if (result == NULL) {
        g_printerr ("Error invoking GetPortConfig(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_iter_init(&arg_i, result);
    struct_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&struct_i, struct_v);
    g_variant_iter_next(&struct_i, "i", &cfg->id);
    g_variant_iter_next(&struct_i, "i", &cfg->role);
    g_variant_iter_next(&struct_i, "u", &cfg->config_mask);
    g_variant_iter_next(&struct_i, "u", &cfg->sample_rate);
    g_variant_iter_next(&struct_i, "u", &cfg->num_channels);
    g_variant_iter_next(&struct_i, "i", &cfg->format);
    g_variant_iter_next(&struct_i, "i", &cfg->bitwidth);
    g_variant_iter_next(&struct_i, "d", &cfg->gain);
    g_variant_iter_next(&struct_i, "s", &cfg->port_name);
    g_variant_unref(result);

exit:
    return ret;
}

int pa_qti_loopback_set_port_config(struct pa_qti_loopback_port_config *cfg)
{
    gint ret = 0;
    GVariant *result;
    GError *error = NULL;
    GVariant *value_1 = NULL;
    GVariantBuilder builder_1;
    GVariant *argument;

    if (!cfg) {
        ret = -EINVAL;
        goto exit;
    }

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(iiuuuiids)"));
    pa_qti_loopback_marshal_port_config(&builder_1, cfg);
    value_1 = g_variant_builder_end(&builder_1);

    argument = g_variant_new("(@(iiuuuiids))", value_1);

    g_debug("Calling SetPortConfig\n");
    result = g_dbus_connection_call_sync(g_mod_data->conn,
                            NULL,
                            g_mod_data->obj_path,
                            PA_QTI_LOOPBACK_DBUS_MODULE_IFACE,
                            "SetPortConfig",
                            argument,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

    if (result == NULL) {
        g_printerr ("Error invoking SetPortConfig(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);

exit:
    return ret;
}

pa_qti_loopback_handle_t pa_qti_loopback_create(unsigned int num_srcs,
                                                struct pa_qti_loopback_port_config *src_cfg,
                                                unsigned int num_sinks,
                                                struct pa_qti_loopback_port_config *sink_cfg,
                                                pa_qti_loopback_event_callback_t callback) {
    gint i;
    GVariant *result;
    GError *error = NULL;
    GVariant *argument;
    GVariant *value_1 = NULL;
    GVariant *value_2 = NULL;
    GVariantBuilder builder_1, builder_2;
    struct pa_qti_loopback_session_data *ses_data;
    pa_qti_loopback_handle_t handle = PA_QTI_LOOPBACK_PATCH_HANDLE_NONE;
    gchar thread_name[32];

    if (!src_cfg || !sink_cfg)
        goto exit;


    ses_data = g_malloc0(sizeof(struct pa_qti_loopback_session_data));
    if (ses_data == NULL) {
        g_printerr("Memory allocation failed for pa_qti_loopback_session_data\n");
        goto exit;
    }

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(iiuuuiids)"));
    pa_qti_loopback_marshal_port_config(&builder_1, src_cfg);
    value_1 = g_variant_builder_end(&builder_1);

    g_variant_builder_init(&builder_2, G_VARIANT_TYPE("a(iiuuuiids)"));
    for (i = 0; i < num_sinks; i++) {
        g_variant_builder_open(&builder_2, G_VARIANT_TYPE("(iiuuuiids)"));
        pa_qti_loopback_marshal_port_config(&builder_2, &sink_cfg[i]);
        g_variant_builder_close(&builder_2);
    }
    value_2 = g_variant_builder_end(&builder_2);

    argument = g_variant_new("(@(iiuuuiids)@a(iiuuuiids))",
                   value_1,
                   value_2);

    g_debug("Calling Create\n");
    result = g_dbus_connection_call_sync(g_mod_data->conn,
                            NULL,
                            g_mod_data->obj_path,
                            PA_QTI_LOOPBACK_DBUS_MODULE_IFACE,
                            "Create",
                            argument,
                            G_VARIANT_TYPE("(o)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

    if (result == NULL) {
        g_printerr ("Error invoking Create(): %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(o)", &ses_data->obj_path);
    g_info("The server answered: obj path: '%s'\n", ses_data->obj_path);
    g_variant_unref(result);

    handle = pa_qti_loopback_parse_ses_handle(ses_data->obj_path);
    if (handle < PA_QTI_LOOPBACK_PATCH_HANDLE_NONE)
        handle = PA_QTI_LOOPBACK_PATCH_HANDLE_NONE;

    ses_data->callback = callback;

    /* Start threadloop to listen to signals from server */
    g_snprintf(thread_name, sizeof(thread_name), "pa_loop_%d", handle);
    g_debug("create thread %s\n", thread_name);
    ses_data->thread_loop = g_thread_try_new(thread_name, signal_threadloop,
                                                           ses_data, &error);

    if (!ses_data->thread_loop) {
        g_error("Could not create thread %s, error %s\n", thread_name, error->message);
        g_error_free(error);
        goto exit_1;
    }

    if (subscribe_detection_event(ses_data, true /* subscribe */)) {
        g_printerr("Failed to subscribe for detection event");
        goto exit_1;
    }

    g_hash_table_insert(g_mod_data->ses_hash_table, GINT_TO_POINTER(handle), ses_data);

    return handle;

exit_1:
    pa_qti_loopback_stop(handle);

exit:
    if (ses_data)
        g_free(ses_data);

    return handle;
}

/******* Session specific function ********/
int pa_qti_loopback_stop(pa_qti_loopback_handle_t id) {
    gint ret = 0;
    GVariant *result;
    GError *error = NULL;
    struct pa_qti_loopback_session_data *ses_data;

    if ((ses_data = (struct pa_qti_loopback_session_data *)g_hash_table_lookup(
                                            g_mod_data->ses_hash_table,GINT_TO_POINTER(id))) == NULL) {
        g_printerr("No session exists for given handle %d\n", id);
        ret = -EINVAL;
        goto exit;
    }

    g_info("patch handle %d obj_path %s\n", id, ses_data->obj_path);

    g_debug("Calling Stop\n");
    result = g_dbus_connection_call_sync(g_mod_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QTI_LOOPBACK_DBUS_SESSION_IFACE,
                            "Stop",
                            NULL,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

    if (result == NULL) {
        g_printerr ("Error invoking Stop(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);

    if (subscribe_detection_event(ses_data, false /*unsubscribe */))
        g_printerr("Failed to unsubscribe for detection event");

    /* Quit mainloop started to listen for detection signals */
    if (!ses_data->thread_loop) {
        g_main_loop_quit(ses_data->loop);
        g_thread_join(ses_data->thread_loop);
        ses_data->thread_loop = NULL;
    }

exit:
    g_hash_table_remove(g_mod_data->ses_hash_table, GINT_TO_POINTER(id));

    if (ses_data)
        g_free(ses_data);

    return ret;
}
