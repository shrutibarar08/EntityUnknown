#pragma once

#include "ExceptionManager/IException.h"
#include <Windows.h>

class WindowsException : public IException
{
public:
    WindowsException(const char* file, int line, const char* function);

    virtual const char* what() const noexcept override;
    long GetLastErrorCode() const noexcept;
    bool HasLastError() const noexcept;
    void SetErrorMessage() override;

private:
    DWORD mLastError{};
};

#define THROW_WINDOWS_EXCEPTION() \
    throw WindowsException(__FILE__, __LINE__, __FUNCTION__)

#define THROW_WINDOWS_EXCEPTION_IF_FAILED(expr) \
    if (!(expr)) { throw WindowsException(__FILE__, __LINE__, __FUNCTION__); }
