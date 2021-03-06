/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _EDGEX_DEVSDK_H_
#define _EDGEX_DEVSDK_H_ 1

/**
 * @file
 * @brief This file defines the functions and callbacks relating to the SDK.
 */

#include "edgex/edgex.h"
#include "edgex/error.h"
#include "edgex/edgex_logging.h"
#include "edgex/registry.h"

typedef struct edgex_blob
{
  size_t size;
  uint8_t *bytes;
} edgex_blob;

typedef union edgex_device_resultvalue
{
  bool bool_result;
  char *string_result;
  uint8_t ui8_result;
  uint16_t ui16_result;
  uint32_t ui32_result;
  uint64_t ui64_result;
  int8_t i8_result;
  int16_t i16_result;
  int32_t i32_result;
  int64_t i64_result;
  float f32_result;
  double f64_result;
  edgex_blob binary_result;
} edgex_device_resultvalue;

/**
 * @brief Structure containing information about a get or set request.
 */

typedef struct edgex_device_commandrequest
{
  /** Corresponds to a get or set line in a resource of the device profile. */
  const edgex_resourceoperation *ro;
  /** Corresponds to a deviceResource in the device profile. */
  const edgex_deviceresource *devobj;
} edgex_device_commandrequest;

/**
 * @brief Structure containing a parameter (for set) or a result (for get).
 */

typedef struct edgex_device_commandresult
{
  /**
   * The timestamp of the result. Should only be set if the device itself
   * supplies one.
   */
  uint64_t origin;
  /** The type of the parameter or result. */
  edgex_propertytype type;
  /** The value of the parameter or result. */
  edgex_device_resultvalue value;
} edgex_device_commandresult;

/* Callback functions */

/**
 * @brief Function called during service start operation.
 * @param impl The context data passed in when the service was created.
 * @param lc A logging client for the device service.
 * @param config Name-Value pairs in the <service-name>.driver hierarchy. NB
 *               these are represented in .toml configuration fies as a
 *               "Driver" table.
 * @return true if the operation was successful, false otherwise.
 */

typedef bool (*edgex_device_device_initialize)
(
  void *impl,
  struct iot_logging_client *lc,
  const edgex_nvpairs *config
);

/**
 * @brief Request to dynamically discover devices. If the implementation is
 *        capable of doing so, it should detect devices and register them
 *        using the edgex_device_add_device API call.
 * @param impl The context data passed in when the service was created.
 */

typedef void (*edgex_device_discover)
(
  void *impl
);

/**
 * @brief Callback issued to handle GET requests for device readings.
 * @param impl The context data passed in when the service was created.
 * @param devaddr The address of the device to be queried.
 * @param nreadings The number of readings requested.
 * @param requests An array specifying the readings that have been requested.
 * @param readings An array in which to return the requested readings.
 * @return true if the operation was successful, false otherwise.
 */

typedef bool (*edgex_device_handle_get)
(
  void *impl,
  const edgex_addressable *devaddr,
  uint32_t nreadings,
  const edgex_device_commandrequest *requests,
  edgex_device_commandresult *readings
);

/**
 * @brief Callback issued to handle PUT requests for setting device values.
 * @param impl The context data passed in when the service was created.
 * @param devaddr The address of the device to be written to.
 * @param nvalues The number of set operations requested.
 * @param requests An array specifying the resources to which to write.
 * @param values An array specifying the values to be written.
 * @return true if the operation was successful, false otherwise.
 */

typedef bool (*edgex_device_handle_put)
(
  void *impl,
  const edgex_addressable *devaddr,
  uint32_t nvalues,
  const edgex_device_commandrequest *requests,
  const edgex_device_commandresult *values
);

/**
 * @brief Unused in the current implementation. In future this may be used to
 *        notify a device service implementation that a device has been removed
 *        and any resources relating to it may be released.
 */

typedef bool (*edgex_device_disconnect_device)
(
  void *impl,
  edgex_addressable *device
);

/**
 * @brief Callback issued during device service shutdown. The implementation
 * should stop processing and release any resources that were being used.
 * @param impl The context data passed in when the service was created.
 * @param force A 'force' stop has been requested. An unclean shutdown may be
 *              performed if necessary.
 */

typedef void (*edgex_device_stop) (void *impl, bool force);

typedef struct edgex_device_callbacks
{
  edgex_device_device_initialize init;
  edgex_device_discover discover;
  edgex_device_handle_get gethandler;
  edgex_device_handle_put puthandler;
  edgex_device_disconnect_device disconnect;
  edgex_device_stop stop;
} edgex_device_callbacks;

/* Device service */

struct edgex_device_service;
typedef struct edgex_device_service edgex_device_service;

/**
 * @brief Create a new device service.
 * @param name The device service name, used in logging, metadata lookups and
 *             and to scope configuration.
 * @param version The version string for this service. For information only.
 * @param impldata An object pointer which will be passed back whenever one of
 *                 the callback functions is invoked.
 * @param implfns Structure containing the device implementation functions. The
 *                SDK will call these functions in order to carry out its
 *                various actions.
 * @param err Nonzero reason codes will be set here in the event of errors.
 * @return The newly instantiated service
 */

edgex_device_service *edgex_device_service_new
(
  const char *name,
  const char *version,
  void *impldata,
  edgex_device_callbacks implfns,
  edgex_error *err
);

/**
 * @brief Start a device service.
 * @param svc The service to start.
 * @param registryURL If set, this identifies a registry implementation for the
 *                    service to use. The service will register itself and
                      obtain configuration from this registry. If no
 *                    configuration is available, it will be read from file and
 *                    uploaded to the registry ready for subsequent runs.
 * @param profile Configuration profile to use (may be null).
 * @param confDir Directory containing configuration files.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void edgex_device_service_start
(
  edgex_device_service *svc,
  const char *registryURL,
  const char *profile,
  const char *confDir,
  edgex_error *err
);

/**
 * @brief Post readings to the core-data service. This method allows readings
 *        to be generated other than in response to a device GET invocation.
 * @param svc The device service.
 * @param device_name The name of the device that the readings have come from.
 * @param nreadings Number of readings being submitted.
 * @param sources An array specifying the resources from which the readings
 *        have been taken.
 * @param values An array of readings. These will be combined into an Event
 *        and submitted to core-data. For readings of String or Binary type,
 *        the SDK takes ownership of the memory containing the string or
 *        byte array.
 */

void edgex_device_post_readings
(
  edgex_device_service *svc,
  const char *device_name,
  uint32_t nreadings,
  const edgex_device_commandrequest *sources,
  const edgex_device_commandresult *values
);

/**
 * @brief Stop the event service. Any locally-scheduled events will be
 *        cancelled, the rest api for the device will be shutdown, and
 *        resources will be freed.
 * @param svc The device service.
 * @param force Force stop.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void edgex_device_service_stop
(
  edgex_device_service *svc,
  bool force,
  edgex_error *err
);

/**
 * @brief Add a device to the EdgeX system. This will generally be called in
 *        response to a request for device discovery.
 * @param svc The device service.
 * @param name The name of the new device.
 * @param description Optional description of the new device.
 * @param labels Optional labels for the new device.
 * @param profile_name Name of the device profile to be used with this device.
 * @param address Addressable for this device. The addressable will be created
 *        in metadata. The address' name and origin timestamp will be generated
 *        if not set.
 * @param err Nonzero reason codes will be set here in the event of errors.
 * @returns The id of the newly created or existing device, or NULL if an error
 *          occurred.
 */

char * edgex_device_add_device
(
  edgex_device_service *svc,
  const char *name,
  const char *description,
  const edgex_strings *labels,
  const char *profile_name,
  edgex_addressable *address,
  edgex_error *err
);

/**
 * @brief Remove a device from EdgeX. The device will be deleted from the
 *        device service and from core-metadata.
 * @param svc The device service.
 * @param id The id of the device to be removed.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void edgex_device_remove_device
  (edgex_device_service *svc, const char *id, edgex_error *err);

/**
 * @brief Remove a device from EdgeX. The device will be deleted from the
 *        device service and from core-metadata.
 * @param svc The device service.
 * @param name The name of the device to be removed.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void edgex_device_remove_device_byname
  (edgex_device_service *svc, const char *name, edgex_error *err);

/**
 * @brief Update a device's details.
 * @param svc The device service.
 * @param id The id of the device to update. If this is unset, the device will
 *           be located by name.
 * @param name If id is unset, this parameter must be set to the name of the
 *             devce to be updated. Otherwise, it is optional and if set,
 *             specifies a new name for the device.
 * @param description If set, a new description for the device.
 * @param labels If set, a new set of labels for the device.
 * @param profilename If set, a new device profile for the device.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

void edgex_device_update_device
(
  edgex_device_service *svc,
  const char *id,
  const char *name,
  const char *description,
  const edgex_strings *labels,
  const char *profilename,
  edgex_error *err
);

/**
 * @brief Obtain a list of devices known to the system. This function calls
 *        core-metadata to retrieve all devices associated with this device
 *        service.
 * @param svc The device service.
 * @param err Nonzero reason codes will be set here in the event of errors.
 */

edgex_device * edgex_device_devices
(
  edgex_device_service *svc,
  edgex_error *err
);

/**
 * @brief Retrieve device information.
 * @param svc The device service.
 * @param id The device id.
 * @returns The requested device metadata or null if the device was not found.
 */

edgex_device * edgex_device_get_device
  (edgex_device_service *svc, const char *id);

/**
 * @brief Retrieve device information.
 * @param svc The device service.
 * @param name The device name.
 * @returns The requested device metadata or null if the device was not found.
 */

edgex_device * edgex_device_get_device_byname
  (edgex_device_service *svc, const char *name);

/**
 * @brief Free a device structure or list of device structures.
 * @param The device or the first device in the list.
 */

void edgex_device_free_device (edgex_device *e);

/**
 * @brief Retrieve the device profiles currently known in the SDK.
 * @param svc The device service.
 * @param count Is set to the number of profiles returned.
 * @param profiles Is set to an array of device profiles. This should be
 *        free()d after use.
 */

void edgex_device_service_getprofiles
(
  edgex_device_service *svc,
  uint32_t *count,
  edgex_deviceprofile **profiles
);

#endif
