	#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <comdef.h>
#include <Windows.h>
#include "IException.h"

class RenderException : public IException
{
public:
    RenderException(const char* file, int line, const char* function, HRESULT hr);
    ~RenderException() override = default;

    RenderException(const RenderException&) = default;
    RenderException& operator=(const RenderException&) = default;

    void SetErrorMessage() override;

    // Accessors for HRESULT and DirectX function name
    HRESULT GetErrorCode() const { return mHr; }

private:
    HRESULT mHr;
};

// Macros to throw RenderException with file, line, function, HRESULT, and DX function info
#define THROW_RENDER_EXCEPTION_HR(hr) \
    throw RenderException(__FILE__, __LINE__, __FUNCTION__, (hr))

#define THROW_RENDER_EXCEPTION_IF_FAILED(expr)              \
    do {                                                               \
        HRESULT hr_ = (expr);                                          \
        if (FAILED(hr_)) {                                             \
            throw RenderException(__FILE__, __LINE__, __FUNCTION__, hr_); \
        }                                                              \
    } while(false)
