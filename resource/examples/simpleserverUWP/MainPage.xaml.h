//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "IAsyncFunction.h"

namespace simpleserverUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed : public IAsyncFunction
    {
    public:
        MainPage();

        virtual void ShowRequestText(Platform::String^ msg);
        virtual void ShowEventText(Platform::String^ msg);

    internal:
        static IAsyncFunction^ Current;

    private:
        void Start_Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}
