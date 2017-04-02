#pragma once

#include "iotivity_config.h"

#include <functional>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "ocpayload.h"

using namespace OC;

namespace simpleclientUWP
{
class Light
{
public:
    bool m_state;
    int m_power;
    std::string m_name;

    Light() : m_state(false), m_power(0), m_name("")
    {
    }
};

class IoTivityClient
{
public:
    using OCResource = OC::OCResource;

    static constexpr char* SVR_DB_FILE_NAME = "./oic_svr_db_client.dat";
    typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

    DiscoveredResourceMap discoveredResources;
    std::shared_ptr<OCResource> curResource;
    ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
    OCConnectivityType TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
    std::mutex curResourceLock;

    Light mylight;

    IoTivityClient();

    void onObserve(const HeaderOptions, const OCRepresentation& rep, const int& eCode, const int& sequenceNumber);
    void onPost2(const HeaderOptions&, const OCRepresentation& rep, const int eCode);
    void onPost(const HeaderOptions&, const OCRepresentation& rep, const int eCode);
    void postLightRepresentation(std::shared_ptr<OCResource> resource);
    void onPut(const HeaderOptions&, const OCRepresentation& rep, const int eCode);
    void putLightRepresentation(std::shared_ptr<OCResource> resource);
    void onGet(const HeaderOptions&, const OCRepresentation& rep, const int eCode);
    void getLightRepresentation(std::shared_ptr<OCResource> resource);
    void foundResource(std::shared_ptr<OCResource> resource);
    void checkObserverValue(int value);
    void checkTransportValue(int value);
    static FILE* client_open(const char* path, const char* mode);
    void clientThread();
};
}