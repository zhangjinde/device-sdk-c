/*
 * Copyright (c) 2019
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "metrics.h"
#include "parson.h"

#include <sys/time.h>
#include <sys/resource.h>

#ifdef __GNU_LIBRARY__
#include <malloc.h>
#endif

#include <microhttpd.h>

int edgex_device_handler_metrics
(
  void *ctx,
  char *url,
  edgex_http_method method,
  const char *upload_data,
  size_t upload_data_size,
  char **reply,
  const char **reply_type
)
{
  struct rusage rstats;
  JSON_Value *val = json_value_init_object ();
  JSON_Object *obj = json_value_get_object (val);

#ifdef __GNU_LIBRARY__
  struct mallinfo mi = mallinfo ();
  json_object_set_number (obj, "Alloc", mi.uordblks);
  json_object_set_number (obj, "Heap", mi.arena + mi.hblkhd);
#endif

  if (getrusage (RUSAGE_SELF, &rstats) == 0)
  {
    double cputime = rstats.ru_utime.tv_sec + rstats.ru_stime.tv_sec;
    cputime += (rstats.ru_utime.tv_usec + rstats.ru_stime.tv_usec) / 1000000.0;
    json_object_set_number (obj, "CPU", cputime);
  }
  *reply = json_serialize_to_string (val);
  *reply_type = "application/json";
  json_value_free (val);
  return MHD_HTTP_OK;
}
