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

#include "iotivity_config.h"
#include "ocstack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include <logger.h>

#define TAG ("occlient")

int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

// This is a function called back when a device is discovered
OCStackApplicationResult applicationDiscoverCB(
        OCClientResponse * clientResponse) {
    (void)clientResponse;
    OIC_LOG(INFO, TAG, "Entering applicationDiscoverCB (Application Layer CB)");
    OIC_LOG_V(INFO, TAG, "Device =============> Discovered %s @ %s:%d",
                                    clientResponse->resourceUri,
                                    clientResponse->devAddr.addr,
                                    clientResponse->devAddr.port);
    //return OC_STACK_DELETE_TRANSACTION;
    return OC_STACK_KEEP_TRANSACTION;
}

int main() {
    OIC_LOG_V(INFO, TAG, "Starting occlient");

    /* Initialize OCStack*/
    if (OCInit(NULL, 0, OC_CLIENT) != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /* Start a discovery query*/
    char szQueryUri[MAX_QUERY_LENGTH] = { 0 };
    strcpy(szQueryUri, OC_MULTICAST_DISCOVERY_URI);
    if (OCDoResource(NULL, OC_REST_GET, szQueryUri, 0, 0, 
            CT_DEFAULT, OC_LOW_QOS, 0, 0, 0) != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack resource error");
        return 0;
    }

    // Break from loop with Ctrl+C
    OIC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag) {

        if (OCProcess() != OC_STACK_OK) {
            OIC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(1);
    }

    OIC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK) {
        OIC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

