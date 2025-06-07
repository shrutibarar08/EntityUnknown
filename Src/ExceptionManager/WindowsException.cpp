#include "WindowsException.h"
#include <sstream>
#include <string>

WindowsException::WindowsException(const char* file, int line, const char* function)
    : IException(file, line, function), mLastError(::GetLastError())
{
    SetErrorMessage();
}

const char* WindowsException::what() const noexcept
{
    if (mWhatBuffer.empty())
    {
        std::ostringstream oss;
        oss << "[WindowsException] " << GetExceptionMessage()
            << "\nAt: " << GetFile() << " (Line: " << GetLine() << ")"
            << "\nFunction: " << GetFunction()
            << "\nWin32 Error Code: 0x" << std::hex << mLastError;

        mWhatBuffer = oss.str();
    }
    return mWhatBuffer.c_str();
}

long WindowsException::GetLastErrorCode() const noexcept
{
    return static_cast<long>(mLastError);
}

bool WindowsException::HasLastError() const noexcept
{
    return true;
}

void WindowsException::SetErrorMessage()
{
    LPVOID msgBuffer = nullptr;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        mLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&msgBuffer),
        0,
        nullptr
    );

    if (size && msgBuffer)
    {
        mErrorMessage = static_cast<char*>(msgBuffer);
        LocalFree(msgBuffer);
    }
    else
    {
        mErrorMessage = "Unknown Win32 error.";
    }
}
