#include "pch.h"
#include <memory>
#include "Helper.h"

using namespace simpleserverUWP;

Platform::String^ Helper::CharPtrToPlatformString(const char* char_ptr)
{
    auto wchar_str_size = ::strlen(char_ptr) + 1;
    auto wchar_str_unique = std::make_unique<wchar_t[]>(wchar_str_size);
    auto wchar_str = wchar_str_unique.get();
    ::mbstowcs(wchar_str, char_ptr, wchar_str_size);

    return ref new Platform::String(wchar_str);
}
