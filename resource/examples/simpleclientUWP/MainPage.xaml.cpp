﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace simpleclientUWP;

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

#if 0

using namespace OC;

static const char* SVR_DB_FILE_NAME = "./oic_svr_db_client.dat";
typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> curResource;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
static OCConnectivityType TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
std::mutex curResourceLock;

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

Light mylight;

int observe_count()
{
    static int oc = 0;
    return ++oc;
}

void onObserve(const HeaderOptions /*headerOptions*/, const OCRepresentation& rep,
    const int& eCode, const int& sequenceNumber)
{
    try
    {
        if (eCode == OC_STACK_OK && sequenceNumber <= MAX_SEQUENCE_NUMBER)
        {
            if (sequenceNumber == OC_OBSERVE_REGISTER)
            {
                std::cout << "Observe registration action is successful" << std::endl;
            }

            wchar_t msgbuf[256];
            swprintf_s(msgbuf,
                L"OBSERVE RESULT:\n"
                L"\tSequenceNumber: %d\n"
                L"state:%s\n"
                L"power:%d\n"
                L"name:%s\n\n",
                sequenceNumber,
                mylight.m_state ? L"true" : L"false",
                mylight.m_power,
                Helper::StdStringToStdWString(mylight.m_name).c_str());
            MainPage::Async->ShowResultText(ref new String(msgbuf));

          //std::cout << "OBSERVE RESULT:" << std::endl;
          //std::cout << "\tSequenceNumber: " << sequenceNumber << std::endl;
            rep.getValue("state", mylight.m_state);
            rep.getValue("power", mylight.m_power);
            rep.getValue("name", mylight.m_name);

          //std::cout << "\tstate: " << mylight.m_state << std::endl;
          //std::cout << "\tpower: " << mylight.m_power << std::endl;
          //std::cout << "\tname: " << mylight.m_name << std::endl;

            if (observe_count() == 11)
            {
                std::cout << "Cancelling Observe..." << std::endl;
                OCStackResult result = curResource->cancelObserve();

                std::cout << "Cancel result: " << result << std::endl;
                sleep(10);
                std::cout << "DONE" << std::endl;
                std::exit(0);
            }
        }
        else
        {
            if (eCode == OC_STACK_OK)
            {
                std::cout << "No observe option header is returned in the response." << std::endl;
                std::cout << "For a registration request, it means the registration failed"
                    << std::endl;
            }
            else
            {
                std::cout << "onObserve Response error: " << eCode << std::endl;
                std::exit(-1);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onObserve" << std::endl;
    }

}

void onPost2(const HeaderOptions& /*headerOptions*/,
    const OCRepresentation& rep, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED
            || eCode == OC_STACK_RESOURCE_CHANGED)
        {
            std::cout << "POST request was successful" << std::endl;

            if (rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                    << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
                rep.getValue("state", mylight.m_state);
                rep.getValue("power", mylight.m_power);
                rep.getValue("name", mylight.m_name);

                std::cout << "\tstate: " << mylight.m_state << std::endl;
                std::cout << "\tpower: " << mylight.m_power << std::endl;
                std::cout << "\tname: " << mylight.m_name << std::endl;
            }

            if (OBSERVE_TYPE_TO_USE == ObserveType::Observe)
                std::cout << std::endl << "Observe is used." << std::endl << std::endl;
            else if (OBSERVE_TYPE_TO_USE == ObserveType::ObserveAll)
                std::cout << std::endl << "ObserveAll is used." << std::endl << std::endl;

            curResource->observe(OBSERVE_TYPE_TO_USE, QueryParamsMap(), &onObserve);

        }
        else
        {
            std::cout << "onPost2 Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost2" << std::endl;
    }

}

void onPost(const HeaderOptions& /*headerOptions*/,
    const OCRepresentation& rep, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED
            || eCode == OC_STACK_RESOURCE_CHANGED)
        {
            std::cout << "POST request was successful" << std::endl;

            if (rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                    << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
                rep.getValue("state", mylight.m_state);
                rep.getValue("power", mylight.m_power);
                rep.getValue("name", mylight.m_name);

                std::cout << "\tstate: " << mylight.m_state << std::endl;
                std::cout << "\tpower: " << mylight.m_power << std::endl;
                std::cout << "\tname: " << mylight.m_name << std::endl;
            }

            OCRepresentation rep2;

            std::cout << "Posting light representation..." << std::endl;

            mylight.m_state = true;
            mylight.m_power = 55;

            rep2.setValue("state", mylight.m_state);
            rep2.setValue("power", mylight.m_power);

            curResource->post(rep2, QueryParamsMap(), &onPost2);
        }
        else
        {
            std::cout << "onPost Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost" << std::endl;
    }
}

// Local function to put a different state for this resource
void postLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        OCRepresentation rep;

        std::cout << "Posting light representation..." << std::endl;

        mylight.m_state = false;
        mylight.m_power = 105;

        rep.setValue("state", mylight.m_state);
        rep.setValue("power", mylight.m_power);

        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost);
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CHANGED)
        {
            std::cout << "PUT request was successful" << std::endl;

            rep.getValue("state", mylight.m_state);
            rep.getValue("power", mylight.m_power);
            rep.getValue("name", mylight.m_name);

            std::cout << "\tstate: " << mylight.m_state << std::endl;
            std::cout << "\tpower: " << mylight.m_power << std::endl;
            std::cout << "\tname: " << mylight.m_name << std::endl;

            postLightRepresentation(curResource);
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

// Local function to put a different state for this resource
void putLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        OCRepresentation rep;

        std::cout << "Putting light representation..." << std::endl;

        mylight.m_state = true;
        mylight.m_power = 15;

        rep.setValue("state", mylight.m_state);
        rep.setValue("power", mylight.m_power);

        // Invoke resource's put API with rep, query map and the callback parameter
        resource->put(rep, QueryParamsMap(), &onPut);
    }
}

// Callback handler on GET request
void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if (eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("state", mylight.m_state);
            rep.getValue("power", mylight.m_power);
            rep.getValue("name", mylight.m_name);

            std::cout << "\tstate: " << mylight.m_state << std::endl;
            std::cout << "\tpower: " << mylight.m_power << std::endl;
            std::cout << "\tname: " << mylight.m_name << std::endl;

            putLightRepresentation(curResource);
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }
}

// Local function to get representation of light resource
void getLightRepresentation(std::shared_ptr<OCResource> resource)
{
    if (resource)
    {
        std::cout << "Getting Light Representation..." << std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        resource->get(test, &onGet);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n";
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        {
            std::lock_guard<std::mutex> lock(curResourceLock);
            if (discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<
                    " for the first time on server with ID: " << resource->sid() << std::endl;
                discoveredResources[resource->uniqueIdentifier()] = resource;
            }
            else
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() << " again!" << std::endl;
            }

            if (curResource)
            {
                std::cout << "Found another resource, ignoring" << std::endl;
                return;
            }
        }

        // Do some operations with resource object.
        if (resource)
        {
            std::cout << "DISCOVERED Resource:" << std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for (auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for (auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            // Get Resource current host
            std::cout << "\tHost of resource: " << std::endl;
            std::cout << "\t\t" << resource->host() << std::endl;

            // Get Resource Endpoint Infomation
            std::cout << "\tList of resource endpoints: " << std::endl;
            for (auto &resourceEndpoints : resource->getAllHosts())
            {
                std::cout << "\t\t" << resourceEndpoints << std::endl;
            }

            // If resource is found from ip based adapter.
            if (std::string::npos != resource->host().find("coap://") ||
                std::string::npos != resource->host().find("coaps://") ||
                std::string::npos != resource->host().find("coap+tcp://") ||
                std::string::npos != resource->host().find("coaps+tcp://"))
            {
                for (auto &resourceEndpoints : resource->getAllHosts())
                {
                    if (resourceEndpoints.compare(resource->host()) != 0 &&
                        std::string::npos == resourceEndpoints.find("coap+rfcomm"))
                    {
                        std::string newHost = resourceEndpoints;

                        if (std::string::npos != newHost.find("tcp"))
                        {
                            TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_TCP;
                        }
                        else
                        {
                            TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
                        }
                        // Change Resource host if another host exists
                        std::cout << "\tChange host of resource endpoints" << std::endl;
                        std::cout << "\t\t" << "Current host is "
                            << resource->setHost(newHost) << std::endl;
                        break;
                    }
                }
            }

            if (resourceURI == "/a/light")
            {
                if (resource->connectivityType() & TRANSPORT_TYPE_TO_USE)
                {
                    curResource = resource;
                    // Get the resource host address
                    wchar_t msgbuf[256];

                    swprintf_s(msgbuf, L"\tAddress of selected resource: %s\n",
                        Helper::StdStringToStdWString(resource->host()).c_str());
                    MainPage::Async->ShowFoundText(ref new String(msgbuf));
                  //std::cout << "\tAddress of selected resource: " << resource->host() << std::endl;

                    // Call a local function which will internally invoke get API on the resource pointer
                    getLightRepresentation(resource);
                }
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in foundResource: " << e.what() << std::endl;
    }
}

void printUsage()
{
    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------\n";
    std::cout << "Usage : simpleclient <ObserveType> <TransportType>" << std::endl;
    std::cout << "   ObserveType : 1 - Observe" << std::endl;
    std::cout << "   ObserveType : 2 - ObserveAll" << std::endl;
    std::cout << "   TransportType : 1 - IP" << std::endl;
    std::cout << "   TransportType : 2 - TCP" << std::endl;
    std::cout << "---------------------------------------------------------------------\n\n";
}

void checkObserverValue(int value)
{
    if (value == 1)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::Observe;
        std::cout << "<===Setting ObserveType to Observe===>\n\n";
    }
    else if (value == 2)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::ObserveAll;
        std::cout << "<===Setting ObserveType to ObserveAll===>\n\n";
    }
    else
    {
        std::cout << "<===Invalid ObserveType selected."
            << " Setting ObserveType to Observe===>\n\n";
    }
}

void checkTransportValue(int value)
{
    if (1 == value)
    {
        TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
        std::cout << "<===Setting TransportType to IP===>\n\n";
    }
    else if (2 == value)
    {
        TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_TCP;
        std::cout << "<===Setting TransportType to TCP===>\n\n";
    }
    else
    {
        std::cout << "<===Invalid TransportType selected."
            << " Setting TransportType to IP===>\n\n";
    }
}

static FILE* client_open(const char* path, const char* mode)
{
    if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME))
    {
        return fopen(SVR_DB_FILE_NAME, mode);
    }
    else
    {
        return fopen(path, mode);
    }
}

void clientThread() {

    std::ostringstream requestURI;
    OCPersistentStorage ps{ client_open, fread, fwrite, fclose, unlink };
    try
    {
      //printUsage();
        int argc = 1;

        if (argc == 1)
        {
            std::cout << "<===Setting ObserveType to Observe and ConnectivityType to IP===>\n\n";
        }
        else if (argc == 2)
        {
          //checkObserverValue(std::stoi(argv[1]));
            checkObserverValue(1);
        }
        else if (argc == 3)
        {
          //checkObserverValue(std::stoi(argv[1]));
            checkObserverValue(1);
          //checkTransportValue(std::stoi(argv[2]));
            checkTransportValue(1);
        }
        else
        {
            std::cout << "<===Invalid number of command line arguments===>\n\n";
            return;
        }
    }
    catch (std::exception&)
    {
        std::cout << "<===Invalid input arguments===>\n\n";
        return;
    }

    // Create PlatformConfig object
    PlatformConfig cfg{
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        &ps
    };

    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP |
        OCTransportAdapter::OC_ADAPTER_TCP);
    cfg.QoS = OC::QualityOfService::HighQos;

    OCPlatform::Configure(cfg);
    try
    {
        OC_VERIFY(OCPlatform::start() == OC_STACK_OK);

        // makes it so that all boolean values are printed as 'true/false' in this stream
        std::cout.setf(std::ios::boolalpha);
        // Find all resources
        requestURI << OC_RSRVD_WELL_KNOWN_URI;// << "?rt=core.light";

        OCPlatform::findResource("", requestURI.str(),
            CT_DEFAULT, &foundResource);
        std::cout << "Finding Resource... " << std::endl;

        // Find resource is done twice so that we discover the original resources a second time.
        // These resources will have the same uniqueidentifier (yet be different objects), so that
        // we can verify/show the duplicate-checking code in foundResource(above);
        OCPlatform::findResource("", requestURI.str(),
            CT_DEFAULT, &foundResource);
        std::cout << "Finding Resource for second time..." << std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

        // Perform platform clean up.
        OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);

    }
    catch (OCException& e)
    {
        oclog() << "Exception in main: " << e.what();
    }

    return;
}

#endif

IAsyncFunctions^ MainPage::Async = nullptr;

MainPage::MainPage()
{
    InitializeComponent();
    MainPage::Async = this;
}

void MainPage::ShowNotify(
    Platform::String^ msg,
    NotifyType        type)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg, type]()
    {
        Notify(msg, type);
    }));
}

void MainPage::ShowFoundText(Platform::String^ msg)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg]()
    {
        FoundText->Text += msg;
    }));
}

void MainPage::ClearFoundText()
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this]()
    {
        FoundText->Text = "";
    }));
}

void MainPage::ShowResultText(Platform::String^ msg)
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, msg]()
    {
        ResultText->Text += msg;
    }));
}

void MainPage::ClearResultText()
{
    Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this]()
    {
        ResultText->Text = "";
    }));
}

void MainPage::Notify(
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

void MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^ e)
{
    Loaded += ref new RoutedEventHandler(this, &MainPage::OnLoaded);
    Unloaded += ref new RoutedEventHandler(this, &MainPage::OnUnloaded);
}

void MainPage::OnLoaded(
    Platform::Object^                  sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    iotivity_client = std::make_unique<IoTivityClient>();
}

void MainPage::OnUnloaded(
    Platform::Object^                  sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    iotivity_client.reset();
}

void MainPage::Start_Button_Click(
    Platform::Object^                   sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    StartBtn->IsEnabled = false;

    std::thread asyncThread(&IoTivityClient::clientThread, iotivity_client.get());
    asyncThread.detach();
}
