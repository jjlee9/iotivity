﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Helper.h"

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

using namespace simpleserverUWP;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

using namespace OC;
using namespace std;
namespace PH = std::placeholders;
static const char* SVR_DB_FILE_NAME = "./oic_svr_db_server.dat";
int gObservation = 0;
void * ChangeLightRepresentation(void *param);
void * handleSlowResponse(void *param, std::shared_ptr<OCResourceRequest> pRequest);

// Set of strings for each of platform Info fields
std::string  platformId = "0A3E0D6F-DBF5-404E-8719-D6880042463A";
std::string  manufacturerName = "OCF";
std::string  manufacturerLink = "https://www.iotivity.org";
std::string  modelNumber = "myModelNumber";
std::string  dateOfManufacture = "2016-01-15";
std::string  platformVersion = "myPlatformVersion";
std::string  operatingSystemVersion = "myOS";
std::string  hardwareVersion = "myHardwareVersion";
std::string  firmwareVersion = "1.0";
std::string  supportLink = "https://www.iotivity.org";
std::string  systemTime = "2016-01-15T11.01";

// Set of strings for each of device info fields
std::string  deviceName = "IoTivity Simple Server";
std::string  deviceType = "oic.wk.tv";
std::string  specVersion = "ocf.1.1.0";
std::vector<std::string> dataModelVersions = { "ocf.res.1.1.0", "ocf.sh.1.1.0" };
std::string  protocolIndependentID = "fa008167-3bbf-4c9d-8604-c9bcb96cb712";

// OCPlatformInfo Contains all the platform info to be stored
OCPlatformInfo platformInfo;

// Specifies where to notify all observers or list of observers
// false: notifies all observers
// true: notifies list of observers
bool isListOfObservers = false;

// Specifies secure or non-secure
// false: non-secure resource
// true: secure resource
bool isSecure = false;

/// Specifies whether Entity handler is going to do slow response or not
bool isSlowResponse = false;

// Forward declaring the entityHandler

/// This class represents a single resource named 'lightResource'. This resource has
/// two simple properties named 'state' and 'power'

class LightResource
{

public:
    /// Access this property from a TB client
    std::string m_name;
    bool m_state;
    int m_power;
    std::string m_lightUri;
    OCResourceHandle m_resourceHandle;
    OCRepresentation m_lightRep;
    ObservationIds m_interestedObservers;

public:
    /// Constructor
    LightResource()
        :m_name("John's light"), m_state(false), m_power(0), m_lightUri("/a/light"),
        m_resourceHandle(nullptr) {
        // Initialize representation
        m_lightRep.setUri(m_lightUri);

        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("power", m_power);
        m_lightRep.setValue("name", m_name);
    }

    /* Note that this does not need to be a member function: for classes you do not have
    access to, you can accomplish this with a free function: */

    /// This function internally calls registerResource API.
    void createResource()
    {
        //URI of the resource
        std::string resourceURI = m_lightUri;
        //resource type name. In this case, it is light
        std::string resourceTypeName = "core.light";
        // resource interface.
        std::string resourceInterface = DEFAULT_INTERFACE;

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty;
        if (isSecure)
        {
            resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE;
        }
        else
        {
            resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        }
        EntityHandler cb = std::bind(&LightResource::entityHandler, this, PH::_1);

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
            m_resourceHandle, resourceURI, resourceTypeName,
            resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            MainPage::Current->ShowNotify(
                Helper::CharPtrToPlatformString("Resource creation was unsuccessful\n"),
                NotifyType::Error);
          //cout << "Resource creation was unsuccessful\n";
        }
    }

    OCStackResult createResource1()
    {
        // URI of the resource
        std::string resourceURI = "/a/light1";
        // resource type name. In this case, it is light
        std::string resourceTypeName = "core.light";
        // resource interface.
        std::string resourceInterface = DEFAULT_INTERFACE;

        // OCResourceProperty is defined ocstack.h
        uint8_t resourceProperty;
        if (isSecure)
        {
            resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE;
        }
        else
        {
            resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
        }
        EntityHandler cb = std::bind(&LightResource::entityHandler, this, PH::_1);

        OCResourceHandle resHandle;

        // This will internally create and register the resource.
        OCStackResult result = OCPlatform::registerResource(
            resHandle, resourceURI, resourceTypeName,
            resourceInterface, cb, resourceProperty);

        if (OC_STACK_OK != result)
        {
            MainPage::Current->ShowNotify(
                Helper::CharPtrToPlatformString("Resource creation was unsuccessful\n"),
                NotifyType::Error);
          //cout << "Resource creation was unsuccessful\n";
        }

        return result;
    }

    OCResourceHandle getHandle()
    {
        return m_resourceHandle;
    }

    // Puts representation.
    // Gets values from the representation and
    // updates the internal state
    void put(OCRepresentation& rep)
    {
        try {
            if (rep.getValue("state", m_state))
            {
                cout << "\t\t\t\t" << "state: " << m_state << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "state not found in the representation" << endl;
            }

            if (rep.getValue("power", m_power))
            {
                cout << "\t\t\t\t" << "power: " << m_power << endl;
            }
            else
            {
                cout << "\t\t\t\t" << "power not found in the representation" << endl;
            }
        }
        catch (exception& e)
        {
            cout << e.what() << endl;
        }

    }

    // Post representation.
    // Post can create new resource or simply act like put.
    // Gets values from the representation and
    // updates the internal state
    OCRepresentation post(OCRepresentation& rep)
    {
        static int first = 1;

        // for the first time it tries to create a resource
        if (first)
        {
            first = 0;

            if (OC_STACK_OK == createResource1())
            {
                OCRepresentation rep1;
                rep1.setValue("createduri", std::string("/a/light1"));

                return rep1;
            }
        }

        // from second time onwards it just puts
        put(rep);
        return get();
    }


    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {
        m_lightRep.setValue("state", m_state);
        m_lightRep.setValue("power", m_power);

        return m_lightRep;
    }

    void addType(const std::string& type) const
    {
        OCStackResult result = OCPlatform::bindTypeToResource(m_resourceHandle, type);
        if (OC_STACK_OK != result)
        {
            MainPage::Current->ShowNotify(
                Helper::CharPtrToPlatformString("Binding TypeName to Resource was unsuccessful\n"),
                NotifyType::Error);
          //cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

    void addInterface(const std::string& iface) const
    {
        OCStackResult result = OCPlatform::bindInterfaceToResource(m_resourceHandle, iface);
        if (OC_STACK_OK != result)
        {
            MainPage::Current->ShowNotify(
                Helper::CharPtrToPlatformString("Binding TypeName to Resource was unsuccessful\n"),
                NotifyType::Error);
          //cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

private:
    // This is just a sample implementation of entity handler.
    // Entity handler can be implemented in several ways by the manufacturer
    OCEntityHandlerResult entityHandler(std::shared_ptr<OCResourceRequest> request)
    {
        MainPage::Current->ShowRequestText(
            Helper::CharPtrToPlatformString("\tIn Server CPP entity handler:\n"));
      //cout << "\tIn Server CPP entity handler:\n";
        OCEntityHandlerResult ehResult = OC_EH_ERROR;
        if (request)
        {
            // Get the request type and request flag
            std::string requestType = request->getRequestType();
            int requestFlag = request->getRequestHandlerFlag();

            if (requestFlag & RequestHandlerFlag::RequestFlag)
            {
                MainPage::Current->ShowRequestText(
                    Helper::CharPtrToPlatformString("\t\trequestFlag : Request\n"));
              //cout << "\t\trequestFlag : Request\n";
                auto pResponse = std::make_shared<OC::OCResourceResponse>();
                pResponse->setRequestHandle(request->getRequestHandle());
                pResponse->setResourceHandle(request->getResourceHandle());

                // Check for query params (if any)
                QueryParamsMap queries = request->getQueryParameters();

                if (!queries.empty())
                {
                    std::cout << "\nQuery processing upto entityHandler" << std::endl;
                }
                for (auto it : queries)
                {
                    std::cout << "Query key: " << it.first << " value : " << it.second
                        << std::endl;
                }

                // If the request type is GET
                if (requestType == "GET")
                {
                    MainPage::Current->ShowRequestText(
                        Helper::CharPtrToPlatformString("\t\t\trequestType : GET\n"));
                  //cout << "\t\t\trequestType : GET\n";
                    if (isSlowResponse) // Slow response case
                    {
                        static int startedThread = 0;
                        if (!startedThread)
                        {
                            std::thread t(handleSlowResponse, (void *)this, request);
                            startedThread = 1;
                            t.detach();
                        }
                        ehResult = OC_EH_SLOW;
                    }
                    else // normal response case.
                    {

                        pResponse->setResponseResult(OC_EH_OK);
                        pResponse->setResourceRepresentation(get());
                        if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                        {
                            ehResult = OC_EH_OK;
                        }
                    }
                }
                else if (requestType == "PUT")
                {
                    MainPage::Current->ShowRequestText(
                        Helper::CharPtrToPlatformString("\t\t\trequestType : PUT\n"));
                  //cout << "\t\t\trequestType : PUT\n";
                    OCRepresentation rep = request->getResourceRepresentation();

                    // Do related operations related to PUT request
                    // Update the lightResource
                    put(rep);

                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(get());
                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "POST")
                {
                    MainPage::Current->ShowRequestText(
                        Helper::CharPtrToPlatformString("\t\t\trequestType : POST\n"));
                  //cout << "\t\t\trequestType : POST\n";

                    OCRepresentation rep = request->getResourceRepresentation();

                    // Do related operations related to POST request
                    OCRepresentation rep_post = post(rep);
                    pResponse->setResourceRepresentation(rep_post);

                    if (rep_post.hasAttribute("createduri"))
                    {
                        pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                        pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
                    }
                    else
                    {
                        pResponse->setResponseResult(OC_EH_OK);
                    }

                    if (OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
                else if (requestType == "DELETE")
                {
                    MainPage::Current->ShowRequestText(
                        Helper::CharPtrToPlatformString("Delete request received\n"));
                  //cout << "Delete request received" << endl;
                }
            }

            if (requestFlag & RequestHandlerFlag::ObserverFlag)
            {
                ObservationInfo observationInfo = request->getObservationInfo();
                if (ObserveAction::ObserveRegister == observationInfo.action)
                {
                    m_interestedObservers.push_back(observationInfo.obsId);
                }
                else if (ObserveAction::ObserveUnregister == observationInfo.action)
                {
                    m_interestedObservers.erase(std::remove(
                        m_interestedObservers.begin(),
                        m_interestedObservers.end(),
                        observationInfo.obsId),
                        m_interestedObservers.end());
                }

#if defined(_WIN32)
                DWORD threadId = 0;
                HANDLE threadHandle = INVALID_HANDLE_VALUE;
#else
                pthread_t threadId;
#endif

                cout << "\t\trequestFlag : Observer\n";
                gObservation = 1;
                static int startedThread = 0;

                // Observation happens on a different thread in ChangeLightRepresentation function.
                // If we have not created the thread already, we will create one here.
                if (!startedThread)
                {
#if defined(_WIN32)
                    threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ChangeLightRepresentation, (void*)this, 0, &threadId);
#else
                    pthread_create(&threadId, NULL, ChangeLightRepresentation, (void *)this);
#endif
                    startedThread = 1;
                }
                ehResult = OC_EH_OK;
            }
        }
        else
        {
            MainPage::Current->ShowNotify(
                Helper::CharPtrToPlatformString("Request invalid\n"),
                NotifyType::Error);
          //std::cout << "Request invalid" << std::endl;
        }

        return ehResult;
    }

};
LightResource myLight;
// ChangeLightRepresentaion is an observation function,
// which notifies any changes to the resource to stack
// via notifyObservers
void * ChangeLightRepresentation(void *param)
{
    LightResource* lightPtr = (LightResource*)param;

    // This function continuously monitors for the changes
    while (1)
    {
        sleep(3);

        if (gObservation)
        {
            // If under observation if there are any changes to the light resource
            // we call notifyObservors
            //
            // For demostration we are changing the power value and notifying.
            lightPtr->m_power += 10;

            cout << "\nPower updated to : " << lightPtr->m_power << endl;
            cout << "Notifying observers with resource handle: " << lightPtr->getHandle() << endl;

            wchar_t msgbuf[256];
            swprintf_s(msgbuf, L"\nPower updated to : %d\n", lightPtr->m_power);
            MainPage::Current->ShowEventText(ref new String(msgbuf));

            OCStackResult result = OC_STACK_OK;

            if (isListOfObservers)
            {
                std::shared_ptr<OCResourceResponse> resourceResponse =
                { std::make_shared<OCResourceResponse>() };

                resourceResponse->setResourceRepresentation(lightPtr->get(), DEFAULT_INTERFACE);

                result = OCPlatform::notifyListOfObservers(lightPtr->getHandle(),
                    lightPtr->m_interestedObservers,
                    resourceResponse);
            }
            else
            {
                result = OCPlatform::notifyAllObservers(lightPtr->getHandle());
            }

            if (OC_STACK_NO_OBSERVERS == result)
            {
                cout << "No More observers, stopping notifications" << endl;
                MainPage::Current->ShowEventText(ref new String(L"No More observers, stopping notifications\n"));
                gObservation = 0;
            }
        }
    }

    return NULL;
}

void DeletePlatformInfo()
{
    delete[] platformInfo.platformID;
    delete[] platformInfo.manufacturerName;
    delete[] platformInfo.manufacturerUrl;
    delete[] platformInfo.modelNumber;
    delete[] platformInfo.dateOfManufacture;
    delete[] platformInfo.platformVersion;
    delete[] platformInfo.operatingSystemVersion;
    delete[] platformInfo.hardwareVersion;
    delete[] platformInfo.firmwareVersion;
    delete[] platformInfo.supportUrl;
    delete[] platformInfo.systemTime;
}

void DuplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

OCStackResult SetPlatformInfo(std::string platformID, std::string manufacturerName,
    std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
    std::string platformVersion, std::string operatingSystemVersion,
    std::string hardwareVersion, std::string firmwareVersion, std::string supportUrl,
    std::string systemTime)
{
    DuplicateString(&platformInfo.platformID, platformID);
    DuplicateString(&platformInfo.manufacturerName, manufacturerName);
    DuplicateString(&platformInfo.manufacturerUrl, manufacturerUrl);
    DuplicateString(&platformInfo.modelNumber, modelNumber);
    DuplicateString(&platformInfo.dateOfManufacture, dateOfManufacture);
    DuplicateString(&platformInfo.platformVersion, platformVersion);
    DuplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion);
    DuplicateString(&platformInfo.hardwareVersion, hardwareVersion);
    DuplicateString(&platformInfo.firmwareVersion, firmwareVersion);
    DuplicateString(&platformInfo.supportUrl, supportUrl);
    DuplicateString(&platformInfo.systemTime, systemTime);

    return OC_STACK_OK;
}

OCStackResult SetDeviceInfo()
{
    OCStackResult result = OC_STACK_ERROR;

    OCResourceHandle handle = OCGetResourceHandleAtUri(OC_RSRVD_DEVICE_URI);
    if (handle == NULL)
    {
        cout << "Failed to find resource " << OC_RSRVD_DEVICE_URI << endl;
        return result;
    }

    result = OCBindResourceTypeToResource(handle, deviceType.c_str());
    if (result != OC_STACK_OK)
    {
        cout << "Failed to add device type" << endl;
        return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DEVICE_NAME, deviceName);
    if (result != OC_STACK_OK)
    {
        cout << "Failed to set device name" << endl;
        return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DATA_MODEL_VERSION,
        dataModelVersions);
    if (result != OC_STACK_OK)
    {
        cout << "Failed to set data model versions" << endl;
        return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_SPEC_VERSION, specVersion);
    if (result != OC_STACK_OK)
    {
        cout << "Failed to set spec version" << endl;
        return result;
    }

    result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_PROTOCOL_INDEPENDENT_ID,
        protocolIndependentID);
    if (result != OC_STACK_OK)
    {
        cout << "Failed to set piid" << endl;
        return result;
    }

    return OC_STACK_OK;
}

void * handleSlowResponse(void *param, std::shared_ptr<OCResourceRequest> pRequest)
{
    // This function handles slow response case
    LightResource* lightPtr = (LightResource*)param;
    // Induce a case for slow response by using sleep
    std::cout << "SLOW response" << std::endl;
    sleep(10);

    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(lightPtr->get());

    pResponse->setResponseResult(OC_EH_OK);

    // Set the slow response flag back to false
    isSlowResponse = false;
    OCPlatform::sendResponse(pResponse);
    return NULL;
}

void PrintUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : simpleserver <value>\n";
    std::cout << "    Default - Non-secure resource and notify all observers\n";
    std::cout << "    1 - Non-secure resource and notify list of observers\n\n";
    std::cout << "    2 - Secure resource and notify all observers\n";
    std::cout << "    3 - Secure resource and notify list of observers\n\n";
    std::cout << "    4 - Non-secure resource, GET slow response, notify all observers\n";
}

static FILE* client_open(const char* path, const char* mode)
{
    if (strcmp(path, OC_INTROSPECTION_FILE_NAME) == 0)
    {
        return fopen("light_introspection.json", mode);
    }
    else
    {
        return fopen(SVR_DB_FILE_NAME, mode);
    }
}

void serverThread()
{
  //PrintUsage();
    OCPersistentStorage ps{ client_open, fread, fwrite, fclose, unlink };

    int argc = 1;

    if (argc == 1)
    {
        isListOfObservers = false;
        isSecure = false;
    }
    else if (argc == 2)
    {
      //int value = atoi(argv[1]);
        int value = 1;
        switch (value)
        {
        case 1:
            isListOfObservers = true;
            isSecure = false;
            break;
        case 2:
            isListOfObservers = false;
            isSecure = true;
            break;
        case 3:
            isListOfObservers = true;
            isSecure = true;
            break;
        case 4:
            isSlowResponse = true;
            break;
        default:
            break;
        }
    }
    else
    {
        return;
    }

    // Create PlatformConfig object
    PlatformConfig cfg{
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        &ps
    };

    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP |
        OCTransportAdapter::OC_ADAPTER_TCP);
    cfg.QoS = OC::QualityOfService::LowQos;

    OCPlatform::Configure(cfg);
    OC_VERIFY(OCPlatform::start() == OC_STACK_OK);
    std::cout << "Starting server & setting platform info\n";

    OCStackResult result = SetPlatformInfo(platformId, manufacturerName, manufacturerLink,
        modelNumber, dateOfManufacture, platformVersion, operatingSystemVersion,
        hardwareVersion, firmwareVersion, supportLink, systemTime);

    result = OCPlatform::registerPlatformInfo(platformInfo);

    if (result != OC_STACK_OK)
    {
        MainPage::Current->ShowNotify(
            Helper::CharPtrToPlatformString("Platform Registration failed\n"),
            NotifyType::Error);
      //std::cout << "Platform Registration failed\n";
        return;
    }

    result = SetDeviceInfo();

    if (result != OC_STACK_OK)
    {
        MainPage::Current->ShowNotify(
            Helper::CharPtrToPlatformString("Device Registration failed\n"),
            NotifyType::Error);
      //std::cout << "Device Registration failed\n";
        return;
    }

    try
    {
        // Create the instance of the resource class
        // (in this case instance of class 'LightResource').
      //LightResource myLight;

        // Invoke createResource function of class light.
        myLight.createResource();
        std::cout << "Created resource." << std::endl;

        myLight.addType(std::string("core.brightlight"));
        myLight.addInterface(std::string(LINK_INTERFACE));
        std::cout << "Added Interface and Type" << std::endl;

        DeletePlatformInfo();

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        std::cout << "Waiting" << std::endl;
        cv.wait(lock, [] {return false; });
    }
    catch (OCException &e)
    {
        std::cout << "OCException in main : " << e.what() << endl;
    }

    OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);

    return;
}

IAsyncFunctions^ MainPage::Current = nullptr;

MainPage::MainPage()
{
    InitializeComponent();
    MainPage::Current = this;
}

void simpleserverUWP::MainPage::ShowNotify(
    Platform::String^ msg,
    NotifyType        type)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg, type]()
    {
        Notify(msg, type);
    }));
}

void MainPage::ShowRequestText(Platform::String^ msg)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg]()
    {
        RequestText->Text += msg;
    }));
}

void MainPage::ShowEventText(Platform::String^ msg)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg]()
    {
        EventText->Text += msg;
    }));
}

void simpleserverUWP::MainPage::Notify(
    Platform::String^ msg,
    NotifyType        type)
{
    switch (type)
    {
    case NotifyType::Status:
        NotifyBorder->Background = ref new SolidColorBrush(Colors::Green);
        break;
    case NotifyType::Error:
        NotifyBorder->Background = ref new SolidColorBrush(Colors::Red);
        break;
    default:
        break;
    }

    NotifyBlock->Text = msg;

    // Collapse the StatusBlock if it has no text to conserve real estate.
    if (NotifyBlock->Text != "")
    {
        NotifyBorder->Visibility = Xaml::Visibility::Visible;
    }
    else
    {
        NotifyBorder->Visibility = Xaml::Visibility::Collapsed;
    }
}

void simpleserverUWP::MainPage::Start_Button_Click(
    Platform::Object^                   sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    StartBtn->IsEnabled = false;

    std::thread asyncThread(serverThread);
    asyncThread.detach();
}
