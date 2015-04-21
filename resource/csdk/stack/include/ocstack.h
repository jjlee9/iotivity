//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef OCSTACK_H_
#define OCSTACK_H_

#include <stdint.h>
#include "ocstackconfig.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#define WITH_PRESENCE
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//TODO: May want to refactor this in upcoming sprints.
//Don't want to expose to application layer that lower level stack is using CoAP.

#define OC_WELL_KNOWN_QUERY                  "224.0.1.187:5683/oc/core"
#define OC_EXPLICIT_DEVICE_DISCOVERY_URI     "224.0.1.187:5683/oc/core/d?rt=core.led"
#define OC_MULTICAST_PREFIX                  "224.0.1.187:5683"
#define OC_MULTICAST_IP                      "224.0.1.187"
#define OC_MULTICAST_PORT                    5683

#ifdef WITH_PRESENCE
#define OC_DEFAULT_PRESENCE_TTL (60)
#define OC_PRESENCE_URI                      "/oc/presence"
#endif

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/**
 * Data structure to encapsulate IPv4/IPv6/Contiki/lwIP device addresses.
 */
typedef struct OCDevAddr
{
    uint32_t     size;                    // length of the address stored in addr field.
    uint8_t      addr[DEV_ADDR_SIZE_MAX]; // device address.
} OCDevAddr;

/**
 * OC Virtual resources supported by every OC device.
 */
typedef enum
{
    OC_WELL_KNOWN_URI= 0,       // "/oc/core"
    OC_DEVICE_URI,              // "/oc/core/d"
    OC_RESOURCE_TYPES_URI,      // "/oc/core/d/type"
    #ifdef WITH_PRESENCE
    OC_PRESENCE,                // "/oc/presence"
    #endif
    OC_MAX_VIRTUAL_RESOURCES    // Max items in the list
} OCVirtualResources;

/**
 * Standard RESTful HTTP Methods.
 */
typedef enum
{
    OC_REST_NOMETHOD    = 0,
    OC_REST_GET         = (1 << 0),     // Read
    OC_REST_PUT         = (1 << 1),     // Write
    OC_REST_POST        = (1 << 2),     // Update
    OC_REST_DELETE      = (1 << 3),     // Delete
    // Register observe request for most up date notifications ONLY.
    OC_REST_OBSERVE     = (1 << 4),
    // Register observe request for all notifications, including stale notifications.
    OC_REST_OBSERVE_ALL = (1 << 5),
    // Deregister observation, intended for internal use
    OC_REST_CANCEL_OBSERVE = (1 << 6),
    #ifdef WITH_PRESENCE
    // Subscribe for all presence notifications of a particular resource.
    OC_REST_PRESENCE    = (1 << 7)
    #endif
} OCMethod;

/**
 * Host Mode of Operation.
 */
typedef enum
{
    OC_CLIENT = 0,
    OC_SERVER,
    OC_CLIENT_SERVER
} OCMode;

/**
 * Quality of Service.
 */
typedef enum
{
    OC_LOW_QOS = 0,
    OC_MEDIUM_QOS,
    OC_HIGH_QOS,
    OC_NA_QOS // No Quality is defined, let the stack decide
} OCQualityOfService;

/**
 * Resource Properties.
 *
 * ::OC_ACTIVE       When this bit is set, the resource is initialized, otherwise the resource
 *                   is 'inactive'. 'inactive' signifies that the resource has been marked for
 *                   deletion or is already deleted.
 * ::OC_DISCOVERABLE When this bit is set, the resource is allowed to be discovered by clients.
 * ::OC_OBSERVABLE   When this bit is set, the resource is allowed to be observed by clients.
 * ::OC_SLOW         When this bit is set, the resource has been marked as 'slow'. 'slow' signifies
 *                   that responses from this resource can expect delays in processing its
 *                   requests from clients.
 * ::OC_SECURE       When this bit is set, the resource is a secure resource.
 */
typedef enum
{
    OC_ACTIVE       = (1 << 0),
    OC_DISCOVERABLE = (1 << 1),
    OC_OBSERVABLE   = (1 << 2),
    OC_SLOW         = (1 << 3),
    OC_SECURE       = (1 << 4)
} OCResourceProperty;

/**
 * Transport Protocol IDs.
 */
typedef enum
{
    OC_INVALID_ID   = (1 << 0),
    OC_COAP_ID      = (1 << 1)
} OCTransportProtocolID;

/**
 * Adaptor types.
 */
typedef enum
{
    OC_ETHERNET = 0,
    OC_WIFI,
    OC_EDR,
    OC_LE,
    OC_ALL // Multicast message: send over all the interfaces.
} OCConnectivityType;

/**
 * Declares Stack Results & Errors.
 */
typedef enum
{
    /* Success status code - START HERE */
    OC_STACK_OK = 0,
    OC_STACK_RESOURCE_CREATED,
    OC_STACK_RESOURCE_DELETED,
    OC_STACK_CONTINUE,
    /* Success status code - END HERE */
    /* Error status code - START HERE */
    OC_STACK_INVALID_URI = 20,
    OC_STACK_INVALID_QUERY,
    OC_STACK_INVALID_IP,
    OC_STACK_INVALID_PORT,
    OC_STACK_INVALID_CALLBACK,
    OC_STACK_INVALID_METHOD,
    OC_STACK_INVALID_PARAM,
    OC_STACK_INVALID_OBSERVE_PARAM,
    OC_STACK_NO_MEMORY,
    OC_STACK_COMM_ERROR,
    OC_STACK_TIMEOUT,
    OC_STACK_ADAPTER_NOT_ENABLED,
    OC_STACK_NOTIMPL,
    OC_STACK_NO_RESOURCE,               /* resource not found */
    OC_STACK_RESOURCE_ERROR,            /* ex: not supported method or interface */
    OC_STACK_SLOW_RESOURCE,
    OC_STACK_DUPLICATE_REQUEST,
    OC_STACK_NO_OBSERVERS,              /* resource has no registered observers */
    OC_STACK_OBSERVER_NOT_FOUND,
    OC_STACK_VIRTUAL_DO_NOT_HANDLE,
    OC_STACK_INVALID_OPTION,
    OC_STACK_MALFORMED_RESPONSE,        /* the remote reply contained malformed data */
    OC_STACK_PERSISTENT_BUFFER_REQUIRED,
    OC_STACK_INVALID_REQUEST_HANDLE,
    OC_STACK_INVALID_DEVICE_INFO,
    OC_STACK_INVALID_JSON,
    /* NOTE: Insert all new error codes here!*/
    #ifdef WITH_PRESENCE
    OC_STACK_PRESENCE_STOPPED = 128,
    OC_STACK_PRESENCE_TIMEOUT,
    OC_STACK_PRESENCE_DO_NOT_HANDLE,
    #endif
    OC_STACK_ERROR = 255
    /* Error status code - END HERE */
} OCStackResult;

/**
 * Handle to an @ref OCDoResource invocation.
 */
typedef void * OCDoHandle;

/**
 * Handle to an OCResource object owned by the OCStack.
 */
typedef void * OCResourceHandle;

typedef void * OCRequestHandle;
typedef void * OCResponseHandle;

/**
 * Unique identifier for each observation request. Used when observations are
 * registered or deregistering. Used by entity handler to signal specific
 * observers to be notified of resource changes.
 * There can be maximum of 256 observations per server.
 */
typedef uint8_t OCObservationId;

/**
 * Action associated with observation.
 */
typedef enum
{
    OC_OBSERVE_REGISTER = 0,
    OC_OBSERVE_DEREGISTER = 1,
    OC_OBSERVE_NO_OPTION = 2
} OCObserveAction;

typedef struct
{
    // Action associated with observation request
    OCObserveAction action;
    // Identifier for observation being registered/deregistered
    OCObservationId obsId;
} OCObservationInfo;

/**
 * Possible returned values from entity handler.
 */
typedef enum
{
    OC_EH_OK = 0,
    OC_EH_ERROR,
    OC_EH_RESOURCE_CREATED,
    OC_EH_RESOURCE_DELETED,
    OC_EH_SLOW,
    OC_EH_FORBIDDEN,
    OC_EH_RESOURCE_NOT_FOUND
} OCEntityHandlerResult;

/**
 * This structure will be used to define the vendor specific header options to be included
 * in communication packets.
 */
typedef struct OCHeaderOption
{
    // The protocol ID this option applies to
    OCTransportProtocolID protocolID;
    // The header option ID which will be added to communication packets
    uint16_t optionID;
    // its length   191
    uint16_t optionLength;
    // pointer to its data
    uint8_t optionData[MAX_HEADER_OPTION_DATA_LENGTH];
} OCHeaderOption;

/**
 * Incoming requests handled by the server. Requests are passed in as a parameter to the
 * @ref OCEntityHandler callback API.
 * @brief The @ref OCEntityHandler callback API must be implemented in the application in order
 * to receive these requests.
 */
typedef struct
{
    // Associated resource
    OCResourceHandle resource;
    OCRequestHandle requestHandle;
    // the REST method retrieved from received request PDU
    OCMethod method;
    // resource query send by client
    char * query;
    // Information associated with observation - valid only when OCEntityHandler
    // flag includes OC_OBSERVE_FLAG
    OCObservationInfo obsInfo;
    // An array of the received vendor specific header options
    uint8_t numRcvdVendorSpecificHeaderOptions;
    OCHeaderOption * rcvdVendorSpecificHeaderOptions;
    // reqJSON is retrieved from the payload of the received request PDU
    char * reqJSONPayload;
} OCEntityHandlerRequest;

/**
 * Response from queries to remote servers. Queries are made by calling the @ref OCDoResource API.
 */
typedef struct
{
    // Address of remote server
    OCDevAddr * addr;
    // Indicates adaptor type on which the response was received
    OCConnectivityType connType;
    // the is the result of our stack, OCStackResult should contain coap/other error codes;
    OCStackResult result;
    // If associated with observe, this will represent the sequence of notifications from server.
    uint32_t sequenceNumber;
    // resJSONPayload is retrieved from the payload of the received request PDU
    const char * resJSONPayload;
    // An array of the received vendor specific header options
    uint8_t numRcvdVendorSpecificHeaderOptions;
    OCHeaderOption rcvdVendorSpecificHeaderOptions[MAX_HEADER_OPTIONS];
} OCClientResponse;

/**
 * This structure describes the device properties. All non-Null properties will be included
 * in a device discovery request.
 */
typedef struct
{
    char *deviceName;
    char *hostName;
    char *deviceUUID;
    char *contentType;
    char *version;
    char *manufacturerName;
    char *manufacturerUrl;
    char *modelNumber;
    char *dateOfManufacture;
    char *platformVersion;
    char *firmwareVersion;
    char *supportUrl;
} OCDeviceInfo;

typedef struct
{
    // Request handle is passed to server via the entity handler for each incoming request.
    // Stack assigns when request is received, server sets to indicate what request response is for
    OCRequestHandle requestHandle;
    // New handle for tracking block (or slow) response.  Stack assigns, server uses for subsequent calls
    OCResponseHandle  *responseHandle;
    // Resource handle
    OCResourceHandle resourceHandle;
    // Allow the entity handler to pass a result with the response
    OCEntityHandlerResult  ehResult;
    // this is the pointer to server payload data to be transferred
    char *payload;
    // size of server payload data.  I don't think we should rely on null terminated data for size
    uint16_t payloadSize;
    // An array of the vendor specific header options the entity handler wishes to use in response
    uint8_t numSendVendorSpecificHeaderOptions;
    OCHeaderOption sendVendorSpecificHeaderOptions[MAX_HEADER_OPTIONS];
    // URI of new resource that entity handler might create
    char resourceUri[MAX_URI_LENGTH];
    // Server sets to true for persistent response buffer, false for non-persistent response buffer
    uint8_t persistentBufferFlag;
} OCEntityHandlerResponse;

typedef enum
{
    OC_INIT_FLAG    = (1 << 0),
    OC_REQUEST_FLAG = (1 << 1),
    OC_OBSERVE_FLAG = (1 << 2)
} OCEntityHandlerFlag; //entity_handler_flag_t ;

/**
 * Possible returned values from client application.
 */
typedef enum
{
    OC_STACK_DELETE_TRANSACTION = 0,//!< OC_STACK_DELETE_TRANSACTION
    OC_STACK_KEEP_TRANSACTION       //!< OC_STACK_KEEP_TRANSACTION
} OCStackApplicationResult;

//-----------------------------------------------------------------------------
// Callback function definitions
//-----------------------------------------------------------------------------

/**
 * Client applications implement this callback to consume responses received from Servers.
 */
typedef OCStackApplicationResult (* OCClientResponseHandler)(void *context, OCDoHandle handle,
    OCClientResponse * clientResponse);

/**
 * Client applications using a context pointer implement this callback to delete the
 * context upon removal of the callback/context pointer from the internal callback-list.
 */
typedef void (* OCClientContextDeleter)(void *context);

/*
 * This info is passed from application to OC Stack when initiating a request to Server.
 */
typedef struct
{
    void *context;
    OCClientResponseHandler cb;
    OCClientContextDeleter cd;
} OCCallbackData;

/**
 * Application server implementations must implement this callback to consume requests OTA.
 * Entity handler callback needs to fill the resPayload of the entityHandlerRequest.
 */
typedef OCEntityHandlerResult (*OCEntityHandler)
(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest);

/**
 * Device Entity handler need to use this call back instead of OCEntityHandler.
 */
typedef OCEntityHandlerResult (*OCDeviceEntityHandler)
(OCEntityHandlerFlag flag, OCEntityHandlerRequest * entityHandlerRequest, char* uri);

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

/**
 * Initialize the OC Stack.  Must be called prior to starting the stack.
 *
 * @param ipAddr
 *     IP Address of host device. Deprecated parameter.
 * @param port
 *     Port of host device. Deprecated parameter.
 * @param mode
 *     Host device is client, server, or client-server.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCInit(const char *ipAddr, uint16_t port, OCMode mode);

/**
 * Stop the OC stack.  Use for a controlled shutdown.
 *
 * Note: OCStop() performs operations similar to OCStopPresence(), as well as OCDeleteResource() on
 * all resources this server is hosting. OCDeleteResource() performs operations similar to
 * OCNotifyAllObservers() to notify all client observers that the respective resource is being
 * deleted.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStop();

/**
 * Called in main loop of OC client or server.  Allows low-level processing of
 * stack services.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCProcess();

/**
 * Discover or Perform requests on a specified resource (specified by that Resource's respective
 * URI).
 *
 * @param handle             @ref OCDoHandle to refer to the request sent out on behalf of
 *                           calling this API. This handle can be used to cancel this operation
 *                           via the OCCancel API.
 *                           Note: This reference is handled internally, and
 *                           should not be free'd by the consumer.  A NULL handle is permitted
 *                           in the event where the caller has no use for the return value.
 * @param method             @ref OCMethod to perform on the resource.
 * @param requiredUri        URI of the resource to interact with.
 * @param referenceUri       URI of the reference resource.
 * @param request            JSON encoded request.
 * @param conType            @ref OCConnectivityType type of connectivity indicating the
 *                           interface. Example: ::OC_WIFI, ::OC_ETHERNET, ::OC_ALL.
 * @param qos                Quality of service. Note that if this API is called on a uri with
 *                           the well-known multicast IP address, the qos will be forced to
 *                           ::OC_LOW_QOS
 *                           since it is impractical to send other QOS levels on such addresses.
 * @param cbData             Asynchronous callback function that is invoked
 *                           by the stack when discovery or resource interaction is complete.
 * @param options            The address of an array containing the vendor specific
 *                           header options to be sent with the request.
 * @param numOptions         Number of header options to be included.
 *
 * Note: Presence subscription amendments (ie. adding additional resource type filters by calling
 * this API again) require the use of the same base URI as the original request to successfully
 * amend the presence filters.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDoResource(OCDoHandle *handle, OCMethod method, const char *requiredUri,
            const char *referenceUri, const char *request, OCConnectivityType conType,
            OCQualityOfService qos, OCCallbackData *cbData,
            OCHeaderOption * options, uint8_t numOptions);

/**
 * Cancel a request associated with a specific @ref OCDoResource invocation.
 *
 * @param handle - Used to identify a specific OCDoResource invocation.
 * @param qos    - used to specify Quality of Service (read below for more info)
 * @param options- used to specify vendor specific header options when sending
 *                 explicit observe cancellation
 * @param numOptions- Number of header options to be included
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCancel(OCDoHandle handle, OCQualityOfService qos, OCHeaderOption * options,
        uint8_t numOptions);

#ifdef WITH_PRESENCE
/**
 * When operating in @ref OCServer or @ref OCClientServer mode, this API will start sending out
 * presence notifications to clients via multicast. Once this API has been called with a success,
 * clients may query for this server's presence and this server's stack will respond via multicast.
 *
 * Server can call this function when it comes online for the first time, or when it comes back
 * online from offline mode, or when it re enters network.
 *
 * @param ttl Time To Live in seconds.
 * Note: If ttl is '0', then the default stack value will be used (60 Seconds).
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCStartPresence(const uint32_t ttl);

/**
 * When operating in @ref OCServer or @ref OCClientServer mode, this API will stop sending out
 * presence notifications to clients via multicast. Once this API has been called with a success,
 * this server's stack will not respond to clients querying for this server's presence.
 *
 * Server can call this function when it is terminating, going offline, or when going
 * away from network.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */

OCStackResult OCStopPresence();
#endif


/**
 * Set default device entity handler.
 *
 * @param entityHandler Entity handler function that is called by ocstack to handle requests for
 *                      any undefined resources or default actions.
 *                      If NULL is passed it removes the device default entity handler.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandler entityHandler);

/**
 * Set device information.
 *
 * @param deviceInfo - Structure passed by the server application containing
 *                     the device information.
 *
 *
 * @return
 *     OC_STACK_OK              - no errors
 *     OC_STACK_INVALID_PARAM   - invalid paramerter
 *     OC_STACK_ERROR           - stack process error
 */
OCStackResult OCSetDeviceInfo(OCDeviceInfo deviceInfo);

/**
 * Create a resource.
 *
 * @param handle Pointer to handle to newly created resource.  Set by ocstack and
 *               used to refer to resource.
 * @param resourceTypeName Name of resource type.  Example: "core.led".
 * @param resourceInterfaceName Name of resource interface.  Example: "core.rw".
 * @param uri URI of the resource.  Example:  "/a/led".
 * @param entityHandler Entity handler function that is called by ocstack to handle requests, etc.
 *                      NULL for default entity handler.
 * @param resourceProperties Properties supported by resource.
 *                           Example: ::OC_DISCOVERABLE|::OC_OBSERVABLE.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCreateResource(OCResourceHandle *handle,
                               const char *resourceTypeName,
                               const char *resourceInterfaceName,
                               const char *uri,
                               OCEntityHandler entityHandler,
                               uint8_t resourceProperties);

/**
 * Create a resource. with host ip address for remote resource.
 *
 * @param handle Pointer to handle to newly created resource.  Set by ocstack.
 *               Used to refer to resource.
 * @param resourceTypeName Name of resource type.  Example: "core.led".
 * @param resourceInterfaceName Name of resource interface.  Example: "core.rw".
 * @param host HOST address of the remote resource.  Example:  "coap://xxx.xxx.xxx.xxx:xxxxx".
 * @param uri URI of the resource.  Example:  "/a/led".
 * @param entityHandler Entity handler function that is called by ocstack to handle requests, etc.
 *                      NULL for default entity handler.
 * @param resourceProperties Properties supported by resource.
 *                           Example: ::OC_DISCOVERABLE|::OC_OBSERVABLE
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCreateResourceWithHost(OCResourceHandle *handle,
                               const char *resourceTypeName,
                               const char *resourceInterfaceName,
                               const char *host,
                               const char *uri,
                               OCEntityHandler entityHandler,
                               uint8_t resourceProperties);

/**
 * Add a resource to a collection resource.
 *
 * @param collectionHandle Handle to the collection resource.
 * @param resourceHandle Handle to resource to be added to the collection resource.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResource(OCResourceHandle collectionHandle, OCResourceHandle resourceHandle);

/**
 * Remove a resource from a collection resource.
 *
 * @param collectionHandle Handle to the collection resource.
 * @param resourceHandle Handle to resource to be removed from the collection resource.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCUnBindResource(OCResourceHandle collectionHandle, OCResourceHandle resourceHandle);

/**
 * Bind a resourcetype to a resource.
 *
 * @param handle Handle to the resource.
 * @param resourceTypeName Name of resource type.  Example: "core.led".
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceTypeToResource(OCResourceHandle handle,
                                           const char *resourceTypeName);
/**
 * Bind a resource interface to a resource.
 *
 * @param handle Handle to the resource.
 * @param resourceInterfaceName Name of resource interface.  Example: "core.rw".
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceInterfaceToResource(OCResourceHandle handle,
                                                const char *resourceInterfaceName);

/**
 * Bind an entity handler to the resource.
 *
 * @param handle Handle to the resource that the contained resource is to be bound.
 * @param entityHandler Entity handler function that is called by ocstack to handle requests, etc.
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCBindResourceHandler(OCResourceHandle handle, OCEntityHandler entityHandler);

/**
 * Get the number of resources that have been created in the stack.
 *
 * @param numResources Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResources(uint8_t *numResources);

/**
 * Get a resource handle by index.
 *
 * @param index Index of resource, 0 to Count - 1.
 *
 * @return Found resource handle or NULL if not found.
 */
OCResourceHandle OCGetResourceHandle(uint8_t index);

/**
 * Delete resource specified by handle.  Deletes resource and all resourcetype and resourceinterface
 * linked lists.
 *
 * Note: OCDeleteResource() performs operations similar to OCNotifyAllObservers() to notify all
 * client observers that "this" resource is being deleted.
 *
 * @param handle Handle of resource to be deleted.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDeleteResource(OCResourceHandle handle);

/**
 * Get the URI of the resource specified by handle.
 *
 * @param handle Handle of resource.
 * @return URI string if resource found or NULL if not found.
 */
const char *OCGetResourceUri(OCResourceHandle handle);

/**
 * Get the properties of the resource specified by handle.
 * NOTE: that after a resource is created, the OC_ACTIVE property is set
 * for the resource by the stack.
 *
 * @param handle Handle of resource.
 * @return OCResourceProperty Bitmask or -1 if resource is not found.
 */
OCResourceProperty OCGetResourceProperties(OCResourceHandle handle);

/**
 * Get the number of resource types of the resource.
 *
 * @param handle Handle of resource.
 * @param numResourceTypes Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResourceTypes(OCResourceHandle handle, uint8_t *numResourceTypes);

/**
 * Get name of resource type of the resource.
 *
 * @param handle Handle of resource.
 * @param index Index of resource, 0 to Count - 1.
 *
 * @return Resource type name if resource found or NULL if resource not found.
 */
const char *OCGetResourceTypeName(OCResourceHandle handle, uint8_t index);

/**
 * Get the number of resource interfaces of the resource.
 *
 * @param handle Handle of resource.
 * @param numResourceInterfaces Pointer to count variable.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCGetNumberOfResourceInterfaces(OCResourceHandle handle,
        uint8_t *numResourceInterfaces);

/**
 * Get name of resource interface of the resource.
 *
 * @param handle Handle of resource.
 * @param index Index of resource, 0 to Count - 1.
 *
 * @return Resource interface name if resource found or NULL if resource not found.
 */
const char *OCGetResourceInterfaceName(OCResourceHandle handle, uint8_t index);

/**
 * Get methods of resource interface of the resource.
 *
 * @param handle Handle of resource.
 * @param index Index of resource, 0 to Count - 1.
 *
 * @return Allowed methods if resource found or NULL if resource not found.
 */
uint8_t OCGetResourceInterfaceAllowedMethods(OCResourceHandle handle, uint8_t index);

/**
 * Get resource handle from the collection resource by index.
 *
 * @param collectionHandle Handle of collection resource.
 * @param index Index of contained resource, 0 to Count - 1.
 *
 * @return Handle to contained resource if resource found or NULL if resource not found.
 */
OCResourceHandle OCGetResourceHandleFromCollection(OCResourceHandle collectionHandle,
        uint8_t index);

/**
 * Get the entity handler for a resource.
 *
 * @param handle Handle of resource.
 *
 * @return Entity handler if resource found or NULL resource not found.
 */
OCEntityHandler OCGetResourceHandler(OCResourceHandle handle);

/**
 * Notify all registered observers that the resource representation has
 * changed. If observation includes a query the client is notified only
 * if the query is valid after the resource representation has changed.
 *
 * @param handle Handle of resource.
 * @param qos    Desired quality of service for the observation notifications.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCNotifyAllObservers(OCResourceHandle handle, OCQualityOfService qos);

/**
 * Notify specific observers with updated value of representation.
 * Before this API is invoked by entity handler it has finished processing
 * queries for the associated observers.
 *
 * @param handle Handle of resource.
 * @param obsIdList List of observation ids that need to be notified.
 * @param numberOfIds Number of observation ids included in obsIdList.
 * @param notificationJSONPayload JSON encoded payload to send in notification.
 * @param qos Desired quality of service of the observation notifications.
 * NOTE: The memory for obsIdList and notificationJSONPayload is managed by the
 * entity invoking the API. The maximum size of the notification is 1015 bytes
 * for non-Arduino platforms. For Arduino the maximum size is 247 bytes.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult
OCNotifyListOfObservers (OCResourceHandle handle,
                            OCObservationId  *obsIdList,
                            uint8_t          numberOfIds,
                            const char    *notificationJSONPayload,
                            OCQualityOfService qos);


/**
 * Send a response to a request.
 * The response can be a normal, slow, or block (i.e. a response that
 * is too large to be sent in a single PDU and must span multiple transmissions).
 *
 * @param response Pointer to structure that contains response parameters.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCDoResponse(OCEntityHandlerResponse *response);

/**
 * Cancel a response.  Applies to a block response.
 *
 * @param responseHandle Response handle set by stack in OCServerResponse after
 *                       OCDoResponse is called.
 *
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
OCStackResult OCCancelResponse(OCResponseHandle responseHandle);

//Utility methods

/**
 * This method is used to retrieved the IPv4 address from OCDev address
 * data structure.
 *
 * @param ipAddr OCDevAddr address.
 * @param a first byte of IPv4 address.
 * @param b second byte of IPv4 address.
 * @param c third byte of IPv4 address.
 * @param d fourth byte of IPv4 address.
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
int32_t OCDevAddrToIPv4Addr(OCDevAddr *ipAddr, uint8_t *a, uint8_t *b,
            uint8_t *c, uint8_t *d );

/**
 * This method is used to retrieve the port number from OCDev address
 * data structure.
 *
 * @param ipAddr OCDevAddr address.
 * @param port Port number.
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
int32_t OCDevAddrToPort(OCDevAddr *ipAddr, uint16_t *port);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* OCSTACK_H_ */


