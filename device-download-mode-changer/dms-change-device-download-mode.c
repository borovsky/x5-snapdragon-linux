#include <gio/gio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libqmi-glib/libqmi-glib.h>

#include "dms-change-device-download-mode.h"
#include "qmi-message.h"

#define QMI_STATUS_SUCCESS 0x0000
#define QMI_STATUS_FAILURE 0x0001
#define QMI_MESSAGE_DMS_MESSAGE_RESULT_TLV_RESULT 0x02

typedef enum {
  QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE = 0x5556,
} QmiMessageDmsMy;

gchar *__qmi_utils_str_hex(gconstpointer mem, gsize size, gchar delimiter) {
  const guint8 *data = mem;
  gsize i;
  gsize j;
  gsize new_str_length;
  gchar *new_str;

  /* Get new string length. If input string has N bytes, we need:
   * - 1 byte for last NUL char
   * - 2N bytes for hexadecimal char representation of each byte...
   * - N-1 bytes for the separator ':'
   * So... a total of (1+2N+N-1) = 3N bytes are needed... */
  new_str_length = 3 * size;

  /* Allocate memory for new array and initialize contents to NUL */
  new_str = g_malloc0(new_str_length);

  /* Print hexadecimal representation of each byte... */
  for (i = 0, j = 0; i < size; i++, j += 3) {
    /* Print character in output string... */
    snprintf(&new_str[j], 3, "%02X", data[i]);
    /* And if needed, add separator */
    if (i != (size - 1)) new_str[j + 2] = delimiter;
  }

  /* Set output string */
  return new_str;
}

/*****************************************************************************/
/* REQUEST/RESPONSE: Qmi Message DMS Set Download Mode */

/* --- Input -- */

struct _QmiMessageDmsChangeDeviceDownloadModeInput {
  volatile gint ref_count;

  /* Mode */
  gboolean arg_mode_set;
  guint8 arg_mode;
};

#define QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_INPUT_TLV_MODE 0x01

/**
 * qmi_message_dms_change_device_download_mode_input_get_mode:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeInput.
 * @value_mode: a placeholder for the output #guint8, or %NULL if not required.
 * @error: Return location for error or %NULL.
 *
 * Get the 'Mode' field from @self.
 *
 * Returns: %TRUE if the field is found, %FALSE otherwise.
 */
gboolean qmi_message_dms_change_device_download_mode_input_get_mode(
    QmiMessageDmsChangeDeviceDownloadModeInput *self, guint8 *value_mode,
    GError **error) {
  g_return_val_if_fail(self != NULL, FALSE);

  if (!self->arg_mode_set) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_NOT_FOUND,
                "Field 'Mode' was not found in the message");
    return FALSE;
  }

  if (value_mode) *value_mode = self->arg_mode;

  return TRUE;
}

/**
 * qmi_message_dms_change_device_download_mode_input_set_mode:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeInput.
 * @value_mode: a #guint8.
 * @error: Return location for error or %NULL.
 *
 * Set the 'Mode' field in the message.
 *
 * Returns: %TRUE if @value was successfully set, %FALSE otherwise.
 */
gboolean qmi_message_dms_change_device_download_mode_input_set_mode(
    QmiMessageDmsChangeDeviceDownloadModeInput *self, guint8 value_mode,
    GError **error) {
  g_return_val_if_fail(self != NULL, FALSE);

  self->arg_mode = value_mode;
  self->arg_mode_set = TRUE;

  return TRUE;
}

GType qmi_message_dms_change_device_download_mode_input_get_type(void) {
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter(&g_define_type_id__volatile)) {
    GType g_define_type_id = g_boxed_type_register_static(
        g_intern_static_string("QmiMessageDmsChangeDeviceDownloadModeInput"),
        (GBoxedCopyFunc)qmi_message_dms_change_device_download_mode_input_ref,
        (GBoxedFreeFunc)qmi_message_dms_change_device_download_mode_input_unref);

    g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
  }

  return g_define_type_id__volatile;
}

/**
 * qmi_message_dms_change_device_download_mode_input_ref:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeInput.
 *
 * Atomically increments the reference count of @self by one.
 *
 * Returns: the new reference to @self.
 */
QmiMessageDmsChangeDeviceDownloadModeInput *qmi_message_dms_change_device_download_mode_input_ref(
    QmiMessageDmsChangeDeviceDownloadModeInput *self) {
  g_return_val_if_fail(self != NULL, NULL);

  g_atomic_int_inc(&self->ref_count);
  return self;
}

/**
 * qmi_message_dms_change_device_download_mode_input_unref:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeInput.
 *
 * Atomically decrements the reference count of @self by one.
 * If the reference count drops to 0, @self is completely disposed.
 */
void qmi_message_dms_change_device_download_mode_input_unref(
    QmiMessageDmsChangeDeviceDownloadModeInput *self) {
  g_return_if_fail(self != NULL);

  if (g_atomic_int_dec_and_test(&self->ref_count)) {
    g_slice_free(QmiMessageDmsChangeDeviceDownloadModeInput, self);
  }
}

/**
 * qmi_message_dms_change_device_download_mode_input_new:
 *
 * Allocates a new #QmiMessageDmsChangeDeviceDownloadModeInput.
 *
 * Returns: the newly created #QmiMessageDmsChangeDeviceDownloadModeInput. The returned
 * value should be freed with qmi_message_dms_change_device_download_mode_input_unref().
 */
QmiMessageDmsChangeDeviceDownloadModeInput *qmi_message_dms_change_device_download_mode_input_new(
    void) {
  QmiMessageDmsChangeDeviceDownloadModeInput *self;

  self = g_slice_new0(QmiMessageDmsChangeDeviceDownloadModeInput);
  self->ref_count = 1;
  return self;
}

static QmiMessage *__qmi_message_dms_change_device_download_mode_request_create(
    guint16 transaction_id, guint8 cid,
    QmiMessageDmsChangeDeviceDownloadModeInput *input, GError **error) {
  QmiMessage *self;

  self = qmi_message_new(QMI_SERVICE_DMS, cid, transaction_id,
                         QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE);

  /* All TLVs are optional, we allow NULL input */
  if (!input) return self;

  /* Try to add the 'Mode' TLV */
  if (input->arg_mode_set) {
    gsize tlv_offset;

    if (!(tlv_offset = qmi_message_tlv_write_init(
              self, (guint8)QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_INPUT_TLV_MODE,
              error))) {
      g_prefix_error(error, "Cannot initialize TLV 'Mode': ");
      goto error_out;
    }

    /* Write the guint8 variable to the buffer */
    if (!qmi_message_tlv_write_guint8(self, input->arg_mode, error)) {
      g_prefix_error(error, "Cannot write integer in TLV 'Mode': ");
      goto error_out;
    }

    if (!qmi_message_tlv_write_complete(self, tlv_offset, error)) {
      g_prefix_error(error, "Cannot complete TLV 'Mode': ");
      goto error_out;
    }
  }

  return self;

error_out:
  qmi_message_unref(self);
  return NULL;
}

/* --- Output -- */

/**
 * QmiMessageResult:
 * @error_status: a #guint16.
 * @error_code: a #guint16.
 *
 * A QmiMessageResult struct.
 */
typedef struct _QmiMessageResult {
  guint16 error_status;
  guint16 error_code;
} QmiMessageResult;

static gboolean qmi_message_result_validate(const guint8 *buffer,
                                            guint16 buffer_len) {
  static const guint expected_len = 4;

  if (buffer_len < expected_len) {
    g_warning(
        "Cannot read the 'Result' TLV: expected '%u' bytes, but only got '%u' "
        "bytes",
        expected_len, buffer_len);
    return FALSE;
  }

  return TRUE;
}

static gchar *qmi_message_result_get_printable(QmiMessage *self,
                                               const gchar *line_prefix) {
  const guint8 *buffer;
  guint16 buffer_len;

  buffer = qmi_message_get_raw_tlv(
      self, QMI_MESSAGE_DMS_MESSAGE_RESULT_TLV_RESULT, &buffer_len);
  if (buffer) {
    GString *printable;
    guint16 error_status;
    guint16 error_code;

    printable = g_string_new("");
    qmi_utils_read_guint16_from_buffer(&buffer, &buffer_len, QMI_ENDIAN_LITTLE,
                                       &error_status);
    qmi_utils_read_guint16_from_buffer(&buffer, &buffer_len, QMI_ENDIAN_LITTLE,
                                       &error_code);

    g_warn_if_fail(buffer_len == 0);

    if (error_status == QMI_STATUS_SUCCESS)
      g_string_append(printable, "SUCCESS");
    else
      g_string_append_printf(
          printable, "FAILURE: %s",
          qmi_protocol_error_get_string((QmiProtocolError)error_code));

    return g_string_free(printable, FALSE);
  }

  return NULL;
}

struct _QmiMessageDmsChangeDeviceDownloadModeOutput {
  volatile gint ref_count;

  /* Result */
  gboolean arg_result_set;
  QmiMessageResult arg_result;
};

#define QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_OUTPUT_TLV_RESULT 0x02

/**
 * qmi_message_dms_change_device_download_mode_output_get_result:
 * @self: a QmiMessageDmsChangeDeviceDownloadModeOutput.
 * @error: Return location for error or %NULL.
 *
 * Get the result of the QMI operation.
 *
 * Returns: %TRUE if the QMI operation succeeded, %FALSE if @error is set.
 */
gboolean qmi_message_dms_change_device_download_mode_output_get_result(
    QmiMessageDmsChangeDeviceDownloadModeOutput *self, GError **error) {
  g_return_val_if_fail(self != NULL, FALSE);

  /* We should always have a result set in the response message */
  if (!self->arg_result_set) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                "No 'Result' field given in the message");
    return FALSE;
  }

  if (self->arg_result.error_status == QMI_STATUS_SUCCESS) {
    /* Operation succeeded */
    return TRUE;
  }

  /* Report a QMI protocol error */
  g_set_error(error, QMI_PROTOCOL_ERROR,
              (QmiProtocolError)self->arg_result.error_code,
              "QMI protocol error (%u): '%s'", self->arg_result.error_code,
              qmi_protocol_error_get_string(
                  (QmiProtocolError)self->arg_result.error_code));
  return FALSE;
}

GType qmi_message_dms_change_device_download_mode_output_get_type(void) {
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter(&g_define_type_id__volatile)) {
    GType g_define_type_id = g_boxed_type_register_static(
        g_intern_static_string("QmiMessageDmsChangeDeviceDownloadModeOutput"),
        (GBoxedCopyFunc)qmi_message_dms_change_device_download_mode_output_ref,
        (GBoxedFreeFunc)qmi_message_dms_change_device_download_mode_output_unref);

    g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
  }

  return g_define_type_id__volatile;
}

/**
 * qmi_message_dms_change_device_download_mode_output_ref:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeOutput.
 *
 * Atomically increments the reference count of @self by one.
 *
 * Returns: the new reference to @self.
 */
QmiMessageDmsChangeDeviceDownloadModeOutput *
qmi_message_dms_change_device_download_mode_output_ref(
    QmiMessageDmsChangeDeviceDownloadModeOutput *self) {
  g_return_val_if_fail(self != NULL, NULL);

  g_atomic_int_inc(&self->ref_count);
  return self;
}

/**
 * qmi_message_dms_change_device_download_mode_output_unref:
 * @self: a #QmiMessageDmsChangeDeviceDownloadModeOutput.
 *
 * Atomically decrements the reference count of @self by one.
 * If the reference count drops to 0, @self is completely disposed.
 */
void qmi_message_dms_change_device_download_mode_output_unref(
    QmiMessageDmsChangeDeviceDownloadModeOutput *self) {
  g_return_if_fail(self != NULL);

  if (g_atomic_int_dec_and_test(&self->ref_count)) {
    g_slice_free(QmiMessageDmsChangeDeviceDownloadModeOutput, self);
  }
}

static gchar *qmi_message_dms_change_device_download_mode_input_mode_get_printable(
    QmiMessage *message, const gchar *line_prefix) {
  gsize offset = 0;
  gsize init_offset;
  GString *printable;
  GError *error = NULL;

  if ((init_offset = qmi_message_tlv_read_init(
           message, QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_INPUT_TLV_MODE, NULL,
           NULL)) == 0)
    return NULL;

  printable = g_string_new("");

  {
    guint8 tmp;

    if (!qmi_message_tlv_read_guint8(message, init_offset, &offset, &tmp,
                                     &error))
      goto out;
    g_string_append_printf(printable, "%u", (guint)tmp);
  }

  if ((offset = __qmi_message_tlv_read_remaining_size(message, init_offset,
                                                      offset)) > 0)
    g_string_append_printf(
        printable, "Additional unexpected '%" G_GSIZE_FORMAT "' bytes", offset);

out:
  if (error) g_string_append_printf(printable, " ERROR: %s", error->message);
  return g_string_free(printable, FALSE);
}

struct message_change_device_download_mode_context {
  QmiMessage *self;
  const gchar *line_prefix;
  GString *printable;
};

static void message_change_device_download_mode_get_tlv_printable(
    guint8 type, const guint8 *value, gsize length,
    struct message_change_device_download_mode_context *ctx) {
  const gchar *tlv_type_str = NULL;
  gchar *translated_value;

  if (!qmi_message_is_response(ctx->self)) {
    switch (type) {
      case QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_INPUT_TLV_MODE:
        tlv_type_str = "Mode";
        translated_value =
            qmi_message_dms_change_device_download_mode_input_mode_get_printable(
                ctx->self, ctx->line_prefix);
        break;
      default:
        break;
    }
  } else {
    switch (type) {
      case QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_OUTPUT_TLV_RESULT:
        tlv_type_str = "Result";
        translated_value =
            qmi_message_result_get_printable(ctx->self, ctx->line_prefix);
        break;
      default:
        break;
    }
  }

  if (!tlv_type_str) {
    gchar *value_str = NULL;

    value_str = qmi_message_get_tlv_printable(ctx->self, ctx->line_prefix, type,
                                              value, length);
    g_string_append(ctx->printable, value_str);
    g_free(value_str);
  } else {
    gchar *value_hex;

    value_hex = __qmi_utils_str_hex(value, length, ':');
    g_string_append_printf(ctx->printable,
                           "%sTLV:\n"
                           "%s  type       = \"%s\" (0x%02x)\n"
                           "%s  length     = %" G_GSIZE_FORMAT
                           "\n"
                           "%s  value      = %s\n"
                           "%s  translated = %s\n",
                           ctx->line_prefix, ctx->line_prefix, tlv_type_str,
                           type, ctx->line_prefix, length, ctx->line_prefix,
                           value_hex, ctx->line_prefix,
                           translated_value ? translated_value : "");
    g_free(value_hex);
    g_free(translated_value);
  }
}

static gchar *message_change_device_download_mode_get_printable(
    QmiMessage *self, const gchar *line_prefix) {
  GString *printable;

  printable = g_string_new("");
  g_string_append_printf(printable,
                         "%s  message     = \"Set Download Mode\" (0x5556)\n",
                         line_prefix);

  {
    struct message_change_device_download_mode_context ctx;
    ctx.self = self;
    ctx.line_prefix = line_prefix;
    ctx.printable = printable;
    qmi_message_foreach_raw_tlv(
        self,
        (QmiMessageForeachRawTlvFn)message_change_device_download_mode_get_tlv_printable,
        &ctx);
  }

  return g_string_free(printable, FALSE);
}

static QmiMessageDmsChangeDeviceDownloadModeOutput *
__qmi_message_dms_change_device_download_mode_response_parse(QmiMessage *message,
                                                   GError **error) {
  QmiMessageDmsChangeDeviceDownloadModeOutput *self;

  g_return_val_if_fail(
      qmi_message_get_message_id(message) == QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE,
      NULL);

  self = g_slice_new0(QmiMessageDmsChangeDeviceDownloadModeOutput);
  self->ref_count = 1;

  do {
    /* No Prerequisites for field */

    {
      gsize offset = 0;
      gsize init_offset;

      if ((init_offset = qmi_message_tlv_read_init(
               message, QMI_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_OUTPUT_TLV_RESULT,
               NULL, error)) == 0) {
        g_prefix_error(error, "Couldn't get the mandatory Result TLV: ");
        qmi_message_dms_change_device_download_mode_output_unref(self);
        return NULL;
      }
      if (!qmi_message_tlv_read_guint16(
              message, init_offset, &offset, QMI_ENDIAN_LITTLE,
              &(self->arg_result.error_status), error))
        goto qmi_message_result_out;
      if (!qmi_message_tlv_read_guint16(message, init_offset, &offset,
                                        QMI_ENDIAN_LITTLE,
                                        &(self->arg_result.error_code), error))
        goto qmi_message_result_out;

      /* The remaining size of the buffer needs to be 0 if we successfully read
       * the TLV */
      if ((offset = __qmi_message_tlv_read_remaining_size(message, init_offset,
                                                          offset)) > 0) {
        g_warning("Left '%" G_GSIZE_FORMAT
                  "' bytes unread when getting the 'Result' TLV",
                  offset);
      }

      self->arg_result_set = TRUE;

    qmi_message_result_out:
      if (!self->arg_result_set) {
        qmi_message_dms_change_device_download_mode_output_unref(self);
        return NULL;
      }
    }
  } while (0);

  return self;
}

static void change_device_download_mode_ready(QmiDevice *device, GAsyncResult *res,
                                    GSimpleAsyncResult *simple) {
  GError *error = NULL;
  QmiMessage *reply;
  QmiMessageDmsChangeDeviceDownloadModeOutput *output;

  reply = qmi_device_command_finish(device, res, &error);
  if (!reply) {
    g_simple_async_result_take_error(simple, error);
    g_simple_async_result_complete(simple);
    g_object_unref(simple);
    return;
  }

  /* Parse reply */
  output = __qmi_message_dms_change_device_download_mode_response_parse(reply, &error);
  if (!output)
    g_simple_async_result_take_error(simple, error);
  else
    g_simple_async_result_set_op_res_gpointer(
        simple, output,
        (GDestroyNotify)qmi_message_dms_change_device_download_mode_output_unref);
  g_simple_async_result_complete(simple);
  g_object_unref(simple);
  qmi_message_unref(reply);
}

/**
 * qmi_client_dms_change_device_download_mode:
 * @self: a #QmiClientDms.
 * @input: a #QmiMessageDmsChangeDeviceDownloadModeInput.
 * @timeout: maximum time to wait for the method to complete, in seconds.
 * @cancellable: a #GCancellable or %NULL.
 * @callback: a #GAsyncReadyCallback to call when the request is satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Asynchronously sends a Set Download Mode request to the device.
 *
 * When the operation is finished, @callback will be invoked in the
 * thread-default main loop of the thread you are calling this method from.
 *
 * You can then call qmi_client_dms_change_device_download_mode_finish() to get the result
 * of the operation.
 */
void qmi_client_dms_change_device_download_mode(QmiClientDms *self,
                                      QmiMessageDmsChangeDeviceDownloadModeInput *input,
                                      guint timeout, GCancellable *cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer user_data) {
  GSimpleAsyncResult *result;
  QmiMessage *request;
  GError *error = NULL;
  guint16 transaction_id;

  result = g_simple_async_result_new(G_OBJECT(self), callback, user_data,
                                     qmi_client_dms_change_device_download_mode);

  transaction_id = qmi_client_get_next_transaction_id(QMI_CLIENT(self));

  request = __qmi_message_dms_change_device_download_mode_request_create(
      transaction_id, qmi_client_get_cid(QMI_CLIENT(self)), input, &error);
  if (!request) {
    g_prefix_error(&error, "Couldn't create request message: ");
    g_simple_async_result_take_error(result, error);
    g_simple_async_result_complete_in_idle(result);
    g_object_unref(result);
    return;
  }

  qmi_device_command(QMI_DEVICE(qmi_client_peek_device(QMI_CLIENT(self))),
                     request, timeout, cancellable,
                     (GAsyncReadyCallback)change_device_download_mode_ready, result);
  qmi_message_unref(request);
}
