#pragma once

namespace simpleserverUWP
{
public enum class NotifyType
{
    Status,
    Error,
};

public interface struct IAsyncFunctions
{
    void ShowNotify(Platform::String^ msg, NotifyType type);
    void ShowRequestText(Platform::String^ msg);
    void ShowEventText(Platform::String^ msg);
};
}