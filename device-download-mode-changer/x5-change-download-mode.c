
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libqmi-glib/libqmi-glib.h>

#include "dms-change-device-download-mode.h"

static GMainLoop *loop;
static GCancellable *cancellable;
static QmiDevice *device;
static QmiClient *client;
static gboolean operation_status;

typedef struct {
  QmiDevice *device;
  QmiClientDms *client;
  GCancellable *cancellable;
} Context;
static Context *ctx;

/* Main options */
static gchar *device_str;
static gchar *download_mode_str;

static GOptionEntry main_entries[] = {
    {"device", 'd', 0, G_OPTION_ARG_STRING, &device_str, "Specify device path",
     "[PATH]"},
    {"download-mode", 'm', 0, G_OPTION_ARG_STRING, &download_mode_str,
     "Specify download mode", "[PATH]"},
    {NULL}};

/**
 * Abort signal
 */
static void signals_handler(int signum) {
  if (cancellable) {
    /* Ignore consecutive requests of cancellation */
    if (!g_cancellable_is_cancelled(cancellable)) {
      g_printerr("%s\n", "cancelling the operation...\n");
      g_cancellable_cancel(cancellable);
    }
    return;
  }

  if (loop && g_main_loop_is_running(loop)) {
    g_printerr("%s\n", "cancelling the main loop...\n");
    g_main_loop_quit(loop);
  }
}

/**
 * qmi_client_dms_change_device_download_mode_finish:
 * @self: a #QmiClientDms.
 * @res: the #GAsyncResult obtained from the #GAsyncReadyCallback passed to
 * qmi_client_dms_change_device_download_mode().
 * @error: Return location for error or %NULL.
 *
 * Finishes an async operation started with qmi_client_dms_change_device_download_mode().
 *
 * Returns: a #QmiMessageDmsChangeDeviceDownloadModeOutput, or %NULL if @error is set.
 * The returned value should be freed with
 * qmi_message_dms_change_device_download_mode_output_unref().
 */
QmiMessageDmsChangeDeviceDownloadModeOutput *qmi_client_dms_change_device_download_mode_finish(
    QmiClientDms *self, GAsyncResult *res, GError **error) {
  if (g_simple_async_result_propagate_error(G_SIMPLE_ASYNC_RESULT(res), error))
    return NULL;

  return qmi_message_dms_change_device_download_mode_output_ref(
      g_simple_async_result_get_op_res_gpointer(G_SIMPLE_ASYNC_RESULT(res)));
}

/**
 * Step 7: Releasing client
 */
static void release_client_ready(QmiDevice *dev, GAsyncResult *res) {
  GError *error = NULL;

  if (!qmi_device_release_client_finish(dev, res, &error)) {
    g_printerr("error: couldn't release client: %s", error->message);
    g_error_free(error);
  } else
    g_debug("Client released");

  g_main_loop_quit(loop);
}

/**
 * Step 6: Shuting down operations
 */
void operation_shutdown(gboolean reported_operation_status) {
  QmiDeviceReleaseClientFlags flags = QMI_DEVICE_RELEASE_CLIENT_FLAGS_NONE;

  /* Keep the result of the operation */
  operation_status = reported_operation_status;

  if (cancellable) {
    g_object_unref(cancellable);
    cancellable = NULL;
  }

  /* If no client was allocated (e.g. generic action), just quit */
  if (!client) {
    g_main_loop_quit(loop);
    return;
  }

  flags |= QMI_DEVICE_RELEASE_CLIENT_FLAGS_RELEASE_CID;
  qmi_device_release_client(device, client, flags, 10, NULL,
                            (GAsyncReadyCallback)release_client_ready, NULL);
}

/**
 * Step 5: Set download mode command processed
 */
static void qmi_client_change_device_download_mode_ready(QmiClientDms *client,
                                               GAsyncResult *res) {
  QmiMessageDmsChangeDeviceDownloadModeOutput *output;
  GError *error = NULL;

  output = qmi_client_dms_change_device_download_mode_finish(client, res, &error);
  if (!output) {
    g_printerr("error: operation failed: %s\n", error->message);
    g_error_free(error);
    operation_shutdown(FALSE);
    return;
  }

  if (!qmi_message_dms_change_device_download_mode_output_get_result(output, &error)) {
    g_printerr("error: couldn't set operating mode: %s\n", error->message);
    g_error_free(error);
    qmi_message_dms_change_device_download_mode_output_unref(output);
    operation_shutdown(FALSE);
    return;
  }

  g_print("[%s] Operating mode set successfully\n",
          qmi_device_get_path_display(ctx->device));

  qmi_message_dms_change_device_download_mode_output_unref(output);
  operation_shutdown(TRUE);
}

gboolean qmicli_read_uint8_from_string(const gchar *str, guint8 *out) {
  gulong num;

  if (!str || !str[0]) return FALSE;

  for (num = 0; str[num]; num++) {
    if (!g_ascii_isdigit(str[num])) return FALSE;
  }

  errno = 0;
  num = strtoul(str, NULL, 10);
  if (!errno && num <= G_MAXUINT8) {
    *out = (guint)num;
    return TRUE;
  }
  return FALSE;
}

static QmiMessageDmsChangeDeviceDownloadModeInput *change_device_download_mode_input_create(
    const gchar *str) {
  QmiMessageDmsChangeDeviceDownloadModeInput *input = NULL;
  guint8 mode;

  if (qmicli_read_uint8_from_string(str, &mode)) {
    GError *error = NULL;

    input = qmi_message_dms_change_device_download_mode_input_new();
    if (!qmi_message_dms_change_device_download_mode_input_set_mode(input, mode,
                                                          &error)) {
      g_printerr("error: couldn't create input data bundle: '%s'\n",
                 error->message);
      g_error_free(error);
      qmi_message_dms_change_device_download_mode_input_unref(input);
      input = NULL;
    }
  }

  return input;
}

/**
 * Step 4: Setting download mode
 */
static void change_device_download_mode() {
  QmiMessageDmsChangeDeviceDownloadModeInput *input;

  g_debug("Asynchronously setting download mode...");
  input = change_device_download_mode_input_create(download_mode_str);
  if (!input) {
    operation_shutdown(FALSE);
    return;
  }
  qmi_client_dms_change_device_download_mode(
      (QmiClientDms *)client, input, 10, cancellable,
      (GAsyncReadyCallback)qmi_client_change_device_download_mode_ready, NULL);
  qmi_message_dms_change_device_download_mode_input_unref(input);
}

/**
 * Step 3: Client allocated. Sending command
 */
static void allocate_client_ready(QmiDevice *dev, GAsyncResult *res) {
  GError *error = NULL;

  client = qmi_device_allocate_client_finish(dev, res, &error);
  if (!client) {
    g_printerr("error: couldn't create client for the '%s' service: %s\n",
               qmi_service_get_string(QMI_SERVICE_DMS), error->message);
    exit(EXIT_FAILURE);
  }

  ctx = g_slice_new(Context);
  ctx->device = g_object_ref(dev);
  change_device_download_mode();
}

static void device_allocate_client(QmiDevice *dev) {
  guint8 cid = QMI_CID_NONE;

  /* As soon as we get the QmiDevice, create a client for the requested
   * service */
  qmi_device_allocate_client(dev, QMI_SERVICE_DMS, cid, 10, cancellable,
                             (GAsyncReadyCallback)allocate_client_ready, NULL);
}

/**
 * Step 2: Device opened. Alocating client.
 */
static void device_open_ready(QmiDevice *dev, GAsyncResult *res) {
  GError *error = NULL;

  if (!qmi_device_open_finish(dev, res, &error)) {
    g_printerr("error: couldn't open the QmiDevice: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  g_debug("QMI Device at '%s' ready", qmi_device_get_path_display(dev));

  device_allocate_client(dev);
}

/**
 * Step 1: Device ready. Opening it.
 */
static void device_new_ready(GObject *unused, GAsyncResult *res) {
  QmiDeviceOpenFlags open_flags = QMI_DEVICE_OPEN_FLAGS_NONE;
  GError *error = NULL;

  device = qmi_device_new_finish(res, &error);
  if (!device) {
    g_printerr("error: couldn't create QmiDevice: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  open_flags |= QMI_DEVICE_OPEN_FLAGS_PROXY;

  /* Open the device */
  qmi_device_open(device, open_flags, 15, cancellable,
                  (GAsyncReadyCallback)device_open_ready, NULL);
}

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                        const gchar *message, gpointer user_data) {
  const gchar *log_level_str;
  time_t now;
  gchar time_str[64];
  struct tm *local_time;
  gboolean err;

  now = time((time_t *)NULL);
  local_time = localtime(&now);
  strftime(time_str, 64, "%d %b %Y, %H:%M:%S", local_time);
  err = FALSE;

  switch (log_level) {
    case G_LOG_LEVEL_WARNING:
      log_level_str = "-Warning **";
      err = TRUE;
      break;

    case G_LOG_LEVEL_CRITICAL:
    case G_LOG_FLAG_FATAL:
    case G_LOG_LEVEL_ERROR:
      log_level_str = "-Error **";
      err = TRUE;
      break;

    case G_LOG_LEVEL_DEBUG:
      log_level_str = "[Debug]";
      break;

    default:
      log_level_str = "";
      break;
  }

  g_fprintf(err ? stderr : stdout, "[%s] %s %s\n", time_str, log_level_str,
            message);
}

int main(int argc, char **argv) {
  GError *error = NULL;
  GFile *file;
  GOptionContext *context;

  setlocale(LC_ALL, "");

  context = g_option_context_new("- Control QMI devices");
  g_option_context_add_main_entries(context, main_entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_printerr("error: %s\n", error->message);
    exit(EXIT_FAILURE);
  }
  g_option_context_free(context);

  /* No device path given? */
  if (!device_str) {
    g_printerr("error: no device path specified\n");
    exit(EXIT_FAILURE);
  }

  /* No download mode given? */
  if (!download_mode_str) {
    g_printerr("error: no download mode specified\n");
    exit(EXIT_FAILURE);
  }

  /* Setup signals */
  signal(SIGINT, signals_handler);
  signal(SIGHUP, signals_handler);
  signal(SIGTERM, signals_handler);

  g_log_set_handler(NULL, G_LOG_LEVEL_MASK, log_handler, NULL);
  g_log_set_handler("Qmi", G_LOG_LEVEL_MASK, log_handler, NULL);
  qmi_utils_set_traces_enabled(TRUE);

  file = g_file_new_for_path(device_str);
  loop = g_main_loop_new(NULL, FALSE);
  qmi_device_new(file, NULL, (GAsyncReadyCallback)device_new_ready, NULL);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
  g_object_unref(file);

  return 0;
}
