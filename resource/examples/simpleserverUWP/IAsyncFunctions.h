#pragma once

namespace simpleserverUWP
{
public interface struct IAsyncFunctions
{
    void ShowRequestText(Platform::String^ msg);
    void ShowEventText(Platform::String^ msg);
};
}