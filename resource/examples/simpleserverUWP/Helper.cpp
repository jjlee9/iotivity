#include "pch.h"
#include <memory>
#include "Helper.h"

Platform::String^ simpleserverUWP::Helper::CharPtrToPlatformString(const char* char_str)
{
    auto wchar_str_size = ::strlen(char_str) + 1;
    auto wchar_str_unique = std::make_unique<wchar_t[]>(wchar_str_size);
    auto wchar_str = wchar_str_unique.get();
    ::mbstowcs(wchar_str, char_str, wchar_str_size);

    return ref new Platform::String(wchar_str);
}
