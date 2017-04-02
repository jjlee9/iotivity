#pragma once

namespace simpleclientUWP
{
public enum class NotifyType
{
    Status,
    Error,
};

public interface struct IAsyncFunctions
{
    void ShowNotify(Platform::String^ msg, NotifyType type);
    void ShowFoundText(Platform::String^ msg);
    void ClearFoundText();
    void ShowResultText(Platform::String^ msg);
    void ClearResultText();
};
}