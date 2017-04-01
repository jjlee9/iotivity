#pragma once

namespace simpleserverUWP
{
public interface struct IAsyncFunction
{
    void ShowRequestText(Platform::String^ msg);
    void ShowEventText(Platform::String^ msg);
};
}