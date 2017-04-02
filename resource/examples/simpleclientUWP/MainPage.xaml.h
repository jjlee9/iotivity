//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "IAsyncFunctions.h"
#include "IoTivityClient.h"

namespace simpleclientUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed : public IAsyncFunctions
    {
    public:
        MainPage();

        virtual void ShowNotify(Platform::String^ msg, NotifyType type);
        virtual void ShowFoundText(Platform::String^ msg);
        virtual void ClearFoundText();
        virtual void ShowResultText(Platform::String^ msg);
        virtual void ClearResultText();

        void Notify(Platform::String^ msg, NotifyType type);

    internal:
        static IAsyncFunctions^ Async;

    protected:
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnUnloaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Start_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        std::unique_ptr<IoTivityClient> iotivity_client;
    };
}
