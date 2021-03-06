//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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
#include <string.h>
#include <assert.h>

#include "utlist.h"
#include "oic_malloc.h"
#include "policyengine.h"
#include "resourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "srmresourcestrings.h"
#include "logger.h"
#include "aclresource.h"
#include "srmutility.h"
#include "doxmresource.h"
#include "iotvticalendar.h"
#include "pstatresource.h"
#include "dpairingresource.h"
#include "pconfresource.h"
#include "amaclresource.h"
#include "credresource.h"
#include "rolesresource.h"

#define TAG "OIC_SRM_PE"

uint16_t GetPermissionFromCAMethod_t(const CAMethod_t method)
{
    uint16_t perm = 0;
    switch (method)
    {
        case CA_GET:
            perm = (uint16_t)PERMISSION_READ;
            break;
        case CA_POST: // Treat all POST as Write (Update) because
                      // we don't know if resource exists yet.
                      // This will be addressed in IoTivity impl of OCF 1.0
            perm = (uint16_t)PERMISSION_WRITE;
            break;
        case CA_PUT: // Per convention, OIC/OCF uses PUT only for Create,
                     // never for Update.
            perm = (uint16_t)PERMISSION_CREATE;
            break;
        case CA_DELETE:
            perm = (uint16_t)PERMISSION_DELETE;
            break;
        default: // if not recognized, must assume requesting full control
            perm = (uint16_t)PERMISSION_FULL_CONTROL;
            break;
    }

    OIC_LOG_V(INFO, TAG, "%s: CA method %d requires permission %#x",
        __func__, method, (uint32_t)perm);
    return perm;
}

/**
 * Compares two OicUuid_t structs.
 *
 * @return true if the two OicUuid_t structs are equal, else false.
 */
static bool UuidCmp(OicUuid_t *firstId, OicUuid_t *secondId)
{
    // TODO use VERIFY macros to check for null when they are merged.
    if(NULL == firstId || NULL == secondId)
    {
        return false;
    }
    // Check empty uuid string
    if('\0' == firstId->id[0] || '\0' == secondId->id[0])
    {
        return false;
    }
    for(int i = 0; i < UUID_LENGTH; i++)
    {
        if(firstId->id[i] != secondId->id[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * Compare the request's subject to DevOwner.
 *
 * @return true if context->subjectId == GetDoxmDevOwner(), else false.
 */
static bool IsRequestFromDevOwner(SRMRequestContext_t *context)
{
    bool retVal = false;

    if(NULL == context)
    {
        return retVal;
    }

    OicSecDoxm_t* doxm = (OicSecDoxm_t*) GetDoxmResourceData();
    if (doxm)
    {
        retVal = UuidCmp(&doxm->owner, &context->subjectUuid);
        OIC_LOG_V(DEBUG, TAG, "%s: request was %s received from device owner",
            __func__, retVal ? "" : "NOT ");

        if (!retVal)
        {
            OIC_LOG(DEBUG, TAG, "Owner UUID  :");
            OIC_LOG_BUFFER(DEBUG, TAG, (const uint8_t *)&doxm->owner.id, sizeof(&doxm->owner.id));
            OIC_LOG(DEBUG, TAG, "Request UUID:");
            OIC_LOG_BUFFER(DEBUG, TAG, (const uint8_t *)&context->subjectUuid.id, sizeof(&context->subjectUuid.id));
        }
    }

    return retVal;
}

/**
 * Check if the request has been received from a session used for Ownership Transfer.
 *
 * @return true if the CA_SECURE_ENDPOINT_ATTRIBUTE_ADMINISTRATOR bit is set, else false.
 */
static bool IsRequestFromOwnershipTransferSession(SRMRequestContext_t *context)
{
    bool retVal = false;

#if defined(__WITH_DTLS__) || defined(__WITH_TLS__)
    //Ownership Transfer sessions are allowed to bypass SVR ACEs, while this
    //Device is not ready for normal operation yet.
    if ((NULL != context) && (NULL != context->endPoint))
    {
        uint32_t allAttributes;
        if (CAGetSecureEndpointAttributes(context->endPoint, &allAttributes) &&
            (allAttributes & CA_SECURE_ENDPOINT_ATTRIBUTE_ADMINISTRATOR))
        {
            retVal = true;
        }

        OIC_LOG_V(DEBUG, TAG, "%s: request was %sreceived from Ownership Transfer session",
            __func__, retVal ? "" : "NOT ");
    }
#else
    OC_UNUSED(context);
#endif

    return retVal;
}

#ifdef MULTIPLE_OWNER
/**
 * Compare the request's subject to SubOwner.
 *
 * @return true if context->subjectId exist subowner list, else false.
 */
static bool IsRequestFromSubOwner(SRMRequestContext_t *context)
{
    bool retVal = false;

    if (NULL != context)
    {
        retVal = IsSubOwner(&context->subjectUuid);
    }

    OIC_LOG_V(INFO, TAG, "%s: returning %s", __func__, retVal ? "true" : "false");
    return retVal;
}

/**
 * Verify the SubOwner's request.
 *
 * @return true if request is valid, else false.
 */
static bool IsValidRequestFromSubOwner(SRMRequestContext_t *context)
{
    bool isValidRequest = false;

    if(NULL == context)
    {
        return isValidRequest;
    }

    switch(context->resourceType)
    {
        case OIC_R_DOXM_TYPE:
            //SubOwner has READ permission only for DOXM
            if(PERMISSION_READ == context->requestedPermission)
            {
                isValidRequest = true;
            }
            break;
        case OIC_R_PSTAT_TYPE:
            //SubOwner has full permsion for PSTAT
            isValidRequest = true;
            break;
        case OIC_R_CRED_TYPE:
            //SubOwner can only access the credential which is registered as the eowner.
            isValidRequest = IsValidCredentialAccessForSubOwner(&context->subjectUuid,
             context->payload, context->payloadSize);
            break;
        case OIC_R_ACL_TYPE:
            //SubOwner can only access the ACL which is registered as the eowner.
            isValidRequest = IsValidAclAccessForSubOwner(&context->subjectUuid,
                context->payload, context->payloadSize);
            break;
        default:
            //SubOwner has full permission for all resource except the security resource
            isValidRequest = true;
            break;
    }

    OIC_LOG_V(INFO, TAG, "%s: returning %s", __func__, isValidRequest ? "true" : "false");
    return isValidRequest;
}
#endif //MULTIPLE_OWNER

static GetSvrRownerId_t GetSvrRownerId[OIC_SEC_SVR_TYPE_COUNT + 1] = {
    NULL,
    GetAclRownerId,
    NULL,               // amacl's rowneruuid has been removed from the OCF 1.0 Security spec.
    GetCredRownerId,
    NULL,               // crl doesn't have rowneruuid.
    GetDoxmRownerId,
    GetDpairingRownerId,
    GetPconfRownerId,
    GetPstatRownerId,
    NULL,               // sacl is not implemented yet.
    NULL                // svc has been removed from the OCF 1.0 Security spec.
};

/**
 * Compare the request's subject to resource.ROwner.
 *
 * @return true if context->subjectId equals SVR rowner id, else return false
 */
bool IsRequestFromResourceOwner(SRMRequestContext_t *context)
{
    bool retVal = false;
    OicUuid_t resourceOwner;

    if(NULL == context)
    {
        return false;
    }

    if((OIC_R_ACL_TYPE <= context->resourceType) && \
        (OIC_SEC_SVR_TYPE_COUNT > context->resourceType))
    {
        GetSvrRownerId_t getRownerId = GetSvrRownerId[(int)context->resourceType];

        if((NULL != getRownerId) && (OC_STACK_OK == getRownerId(&resourceOwner)))
        {
            retVal = UuidCmp(&context->subjectUuid, &resourceOwner);
        }
    }

    OIC_LOG_V(INFO, TAG, "%s: returning %s", __func__, retVal ? "true" : "false");
    return retVal;
}

/**
 * Bitwise check to see if 'permission' contains 'request'.
 *
 * @param permission is the allowed CRUDN permission.
 * @param request is the CRUDN permission being requested.
 *
 * @return true if 'permission' bits include all 'request' bits.
 */
INLINE_API bool IsPermissionAllowingRequest(const uint16_t permission,
    const uint16_t request)
{
    bool allowed = (request == (request & permission));

    OIC_LOG_V(INFO, TAG, "%s: ACE allows permission %#x, "
        "requested permission %#x -> allowed = %u", __func__,
        (uint32_t)permission, (uint32_t)request, (uint32_t)allowed);

    return allowed;
}

/**
 * Compare the passed subject to the wildcard (aka anonymous) subjectId.
 *
 * @return true if 'subject' is the wildcard, false if it is not.
 */
INLINE_API bool IsWildCardSubject(OicUuid_t *subject)
{
    if(NULL == subject)
    {
        return false;
    }

    // Because always comparing to string literal, use strcmp()
    if(0 == memcmp(subject, &WILDCARD_SUBJECT_ID, sizeof(OicUuid_t)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Check whether 'resource' is getting accessed within the valid time period.
 *
 * @param acl is the ACL to check.
 *
 * @return true if access is within valid time period or if the period or recurrence is not present.
 * false if period and recurrence present and the access is not within valid time period.
 */
static bool IsAccessWithinValidTime(const OicSecAce_t *ace)
{
#ifndef WITH_ARDUINO //Period & Recurrence not supported on Arduino due
    //lack of absolute time
    if (NULL== ace || NULL == ace->validities)
    {
        return true;
    }

    //periods & recurrences rules are paired.
    if (NULL == ace->validities->recurrences)
    {
        return false;
    }

    OicSecValidity_t* validity =  NULL;
    LL_FOREACH(ace->validities, validity)
    {
        for(size_t i = 0; i < validity->recurrenceLen; i++)
        {
            if (IOTVTICAL_VALID_ACCESS == IsRequestWithinValidTime(validity->period,
                validity->recurrences[i]))
            {
                OIC_LOG(INFO, TAG, "Access request is in allowed time period");
                return true;
            }
        }
    }
    OIC_LOG(ERROR, TAG, "Access request is in invalid time period");
    return false;

#else
    return true;
#endif
}

/**
 * Check whether 'resource' is in the passed ACE.
 *
 * @param resource is the resource being searched.
 * @param ace is the ACE to check.
 *
 * @return true if 'resource' found, otherwise false.
 */
static bool IsResourceInAce(const char *resource, const OicSecAce_t *ace)
{
    if (NULL== ace || NULL == resource)
    {
        return false;
    }

    OicSecRsrc_t* rsrc = NULL;
    LL_FOREACH(ace->resources, rsrc)
    {
         if (0 == strcmp(resource, rsrc->href) || // TODO null terms?
             0 == strcmp(WILDCARD_RESOURCE_URI, rsrc->href))
         {
             return true;
         }
    }
    return false;
}

static void ProcessMatchingACE(SRMRequestContext_t *context, const OicSecAce_t *currentAce)
{
    // Found the subject, so how about resource?
    OIC_LOG_V(DEBUG, TAG, "%s:found ACE matching subject", __func__);

    // Subject was found, so err changes to Rsrc not found for now.
    context->responseVal = ACCESS_DENIED_RESOURCE_NOT_FOUND;
    OIC_LOG_V(DEBUG, TAG, "%s:Searching for resource...", __func__);
    if (IsResourceInAce(context->resourceUri, currentAce))
    {
        OIC_LOG_V(INFO, TAG, "%s:found matching resource in ACE", __func__);

        // Found the resource, so it's down to valid period & permission.
        context->responseVal = ACCESS_DENIED_INVALID_PERIOD;
        if (IsAccessWithinValidTime(currentAce))
        {
            context->responseVal = ACCESS_DENIED_INSUFFICIENT_PERMISSION;
            if (IsPermissionAllowingRequest(currentAce->permission,
                context->requestedPermission))
            {
                context->responseVal = ACCESS_GRANTED;
            }
        }
    }
}

/**
 * Find ACLs containing context->subject.
 * Search each ACL for requested resource.
 * If resource found, check for context->permission and period validity.
 * If the ACL is not found locally and AMACL for the resource is found
 * then sends the request to AMS service for the ACL.
 * Set context->retVal to result from first ACL found which contains
 * correct subject AND resource.
 */
static void ProcessAccessRequest(SRMRequestContext_t *context)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "ProcessAccessRequest(): context is NULL, returning.");
        return;
    }

    OIC_LOG_V(DEBUG, TAG, "Entering %s(%s)", __func__, context->resourceUri);

    const OicSecAce_t *currentAce = NULL;
    OicSecAce_t *aceSavePtr = NULL;

    OIC_LOG_V(DEBUG, TAG, "Entering ProcessAccessRequest(%s)",
        context->resourceUri);

    // Start out assuming subject not found.
    context->responseVal = ACCESS_DENIED_SUBJECT_NOT_FOUND;

    // Loop through all ACLs with a matching Subject searching for the right
    // ACL for this request.
    do
    {
        currentAce = GetACLResourceData(&context->subjectUuid, &aceSavePtr);

        if (NULL != currentAce)
        {
            ProcessMatchingACE(context, currentAce);
        }
        else
        {
            OIC_LOG_V(INFO, TAG, "%s:no ACL found matching subject for resource %s",
                __func__, context->resourceUri);
        }
    } while ((NULL != currentAce) && !IsAccessGranted(context->responseVal));

#if defined(__WITH_DTLS__) || defined(__WITH_TLS__)
        // If no subject ACE granted access, try role ACEs.
        if (!IsAccessGranted(context->responseVal))
        {
            currentAce = NULL;
            aceSavePtr = NULL;
            OicSecRole_t *roles = NULL;
            size_t roleCount = 0;
            OCStackResult res = GetEndpointRoles(context->endPoint, &roles, &roleCount);
            if (OC_STACK_OK != res)
            {
                OIC_LOG_V(ERROR, TAG, "Error getting asserted roles for endpoint: %d", res);
            }
            else
            {
                OIC_LOG_V(DEBUG, TAG, "Found %u asserted roles for endpoint", (unsigned int) roleCount);
                do
                {
                    currentAce = GetACLResourceDataByRoles(roles, roleCount, &aceSavePtr);
                    if (NULL != currentAce)
                    {
                        ProcessMatchingACE(context, currentAce);
                    }
                    else
                    {
                        OIC_LOG_V(INFO, TAG, "%s:no ACL found matching roles for resource %s",
                            __func__, context->resourceUri);
                    }
                } while ((NULL != currentAce) && !IsAccessGranted(context->responseVal));

                OICFree(roles);
            }
        }
#endif /* defined(__WITH_DTLS__) || defined(__WITH_TLS__) */

    OIC_LOG_V(INFO, TAG, "%s:Leaving with responseVal = %s", __func__,
        IsAccessGranted(context->responseVal) ? "ACCESS_GRANTED" : "ACCESS_DENIED");
    return;
}

void CheckPermission(SRMRequestContext_t *context)
{
    assert(NULL != context);
    assert(0 != context->requestedPermission);
    assert(0 == (context->requestedPermission & ~PERMISSION_FULL_CONTROL));

    // Before doing any ACL processing, check if request is a) coming
    // from DevOwner AND b) the device is in Ready for OTM or Reset state
    // (which in IoTivity is equivalent to isOp == false && owned == false)
    // AND c) the request is for a SVR resource.
    // If all 3 conditions are met, grant request.
    // TODO_IoTivity_1.3: use pstat.dos instead of these two checks.
    bool isDeviceOwned = true; // default to value that will NOT grant access
    if (OC_STACK_OK != GetDoxmIsOwned(&isDeviceOwned)) // if runtime error, don't grant
    {
        OIC_LOG(ERROR, TAG, "GetDoxmIsOwned() call failed.");
        context->responseVal = ACCESS_DENIED_POLICY_ENGINE_ERROR;
    }
    // If we were able to get the value of doxm->isOwned, proceed with
    // test for implicit access.
    else if (IsRequestFromDevOwner(context) &&  // if from DevOwner
            !GetPstatIsop() &&                  // AND if pstat->isOp == false
            !isDeviceOwned &&                   // AND if doxm->isOwned == false
            (NOT_A_SVR_RESOURCE != context->resourceType)) // AND if is SVR type
    {
        OIC_LOG(INFO, TAG, "CheckPermission: granting access to device owner");
        context->responseVal = ACCESS_GRANTED;
    }
    // If not granted via DevOwner status and not a subowner,
    // then check if request is for a SVR and coming from rowner
    else if (IsRequestFromResourceOwner(context))
    {
        OIC_LOG(INFO, TAG, "CheckPermission: granting access to resource owner");
        context->responseVal = ACCESS_GRANTED;
    }
#ifdef MULTIPLE_OWNER // TODO Samsung reviewer: per above comment, should this
                      // go above IsRequestFromResourceOwner() call, or here?
    // Then check if request from SubOwner.
    else if (IsRequestFromSubOwner(context))
    {
        if (IsValidRequestFromSubOwner(context))
        {
            OIC_LOG(INFO, TAG, "CheckPermission: granting access to device sub-owner");
            context->responseVal = ACCESS_GRANTED;
        }
    }
#endif //MULTIPLE_OWNER
    else if (!GetPstatIsop() &&
             (NOT_A_SVR_RESOURCE != context->resourceType) &&
             IsRequestFromOwnershipTransferSession(context))
    {
        OIC_LOG(INFO, TAG, "CheckPermission: granting access to OT session request");
        context->responseVal = ACCESS_GRANTED;
    }
    // Else request is a "normal" request that must be tested against ACL.
    else
    {
        OicUuid_t saveSubjectUuid = {.id={0}};
        // bool isSubEmpty = IsRequestSubjectEmpty(context);

        ProcessAccessRequest(context);

        // TODO_IoTivity_1.3: ensure check order results in Union permissions
        // If access not already granted, and requested subject != wildcard,
        // try looking for a wildcard ACE that grants access.
        // See JIRA ticket 1795 (https://jira.iotivity.org/browse/IOT-1795)
        if (ACCESS_GRANTED != context->responseVal &&
            !IsWildCardSubject(&context->subjectUuid))
        {
            // Save subject to restore context after wildcard subject check.
            memcpy(&saveSubjectUuid, &context->subjectUuid,
                sizeof(context->subjectUuid));

            // Set context->subjectUuid to WILDCARD_SUBJECT_ID.
            memset(&context->subjectUuid, 0, sizeof(context->subjectUuid));
            memcpy(&context->subjectUuid,
                &WILDCARD_SUBJECT_ID, sizeof(context->subjectUuid));
            ProcessAccessRequest(context);

            // Restore subjectUuid.
            memcpy(&context->subjectUuid,
                &saveSubjectUuid, sizeof(context->subjectUuid));
        }
    }

    return;
}
