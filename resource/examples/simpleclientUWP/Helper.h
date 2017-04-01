#pragma once

#include <string>

namespace simpleclientUWP
{
class Helper
{
public:
    static Platform::String^ StdStringToPlatformString(const std::string& char_str)
    {
        return CharPtrToPlatformString(char_str.c_str());
    }

    static Platform::String^ CharPtrToPlatformString(const char* char_ptr);
    static std::wstring StdStringToStdWString(const std::string& char_str)
    {
        return CharPtrToStdWString(char_str.c_str());
    }

    static std::wstring CharPtrToStdWString(const char* char_ptr);
};
}