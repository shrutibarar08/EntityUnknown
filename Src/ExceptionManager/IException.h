#pragma once

#include <stdexcept>
#include <string>
#include <memory>

#include "Utils/Logger/Logger.h"

namespace Draco
{
    namespace Exception
    {
        constexpr const char* DEFAULT_CRASH_FOLDER = "CrashReport";
    }
}

// Base Exception Interface
class IException : public std::exception
{
public:
    IException(const char* file, int line, const char* function);

    virtual const char* what() const noexcept override;
    void SaveCrashReport();

    const std::string& GetFile() const { return mFile; }
    int GetLine() const { return mLine; }
    const std::string& GetFunction() const { return mFunction; }
    const std::string& GetExceptionMessage() const { return mErrorMessage; }

    // Specialized extension points
    virtual void SetErrorMessage() {}

protected:
    std::string mFile;
    int mLine;
    std::string mFunction;
    std::string mErrorMessage;
    mutable std::string mWhatBuffer;
    std::unique_ptr<Logger> mLogger;
};

#define THROW_EXCEPTION() throw IException(__FILE__, __LINE__, __FUNCTION__)