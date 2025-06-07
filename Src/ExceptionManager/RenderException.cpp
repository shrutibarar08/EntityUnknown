#include "RenderException.h"

RenderException::RenderException(
    const char* file,
    int line,
    const char* function,
    HRESULT hr)
    : IException(file, line, function), mHr(hr)
{
    SetErrorMessage();
}

void RenderException::SetErrorMessage()
{
    // Retrieve the system error message for the HRESULT code
    LPSTR messageBuffer = nullptr;
    DWORD messageLength = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        mHr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        nullptr
    );

    std::string errorMessage;
    if (messageLength == 0) {
        errorMessage = "Unknown error";
    }
    else {
        errorMessage = messageBuffer;
        // Trim any trailing newline characters from the message
        while (!errorMessage.empty() && (errorMessage.back() == '\n' || errorMessage.back() == '\r')) {
            errorMessage.pop_back();
        }
    }
    if (messageBuffer) {
        LocalFree(messageBuffer);
    }

    // Format the detailed error message including the HRESULT and DirectX function name
    std::ostringstream oss;
    oss << "RenderException: DirectX call '"
        << "' failed with HRESULT 0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
        << static_cast<unsigned int>(mHr) << std::dec
        << ": " << errorMessage;

    mErrorMessage = oss.str();
}
