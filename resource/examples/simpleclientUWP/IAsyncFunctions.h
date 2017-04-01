#pragma once

namespace simpleclientUWP
{
public interface struct IAsyncFunctions
{
    void ShowFoundText(Platform::String^ msg);
    void ClearFoundText();
    void ShowResultText(Platform::String^ msg);
    void ClearResultText();
};
}