//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "IAsyncFunctions.h"

namespace simpleserverUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed : public IAsyncFunctions
    {
    public:
        MainPage();

        virtual void ShowNotify(Platform::String^ msg, NotifyType type);
        virtual void ShowRequestText(Platform::String^ msg);
        virtual void ShowEventText(Platform::String^ msg);

        void Notify(Platform::String^ msg, NotifyType type);

    internal:
        static IAsyncFunctions^ Async;

    private:
        void Start_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
