#pragma once

namespace simpleserverUWP
{
class Helper
{
public:
    static Platform::String^ CharPtrToPlatformString(const char* char_str);
};
}