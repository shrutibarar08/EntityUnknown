#include "IException.h"

IException::IException(const char* file, int line, const char* function)
    : mFile(file), mLine(line), mFunction(function)
{
}

const char* IException::what() const noexcept
{
    if (mWhatBuffer.empty())
    {
        mWhatBuffer = "[IException] " + GetExceptionMessage() +
            "\nAt: " + mFile +
            " (Line: " + std::to_string(mLine) + ")" +
            "\nFunction: " + mFunction;
    }
    return mWhatBuffer.c_str();
}

void IException::SaveCrashReport()
{
    if (!mLogger)
    {
        LOGGER_INITIALIZE_DESC desc{};
        desc.FilePrefix = "report";
        desc.FolderPath = Draco::Exception::DEFAULT_CRASH_FOLDER;
        mLogger = std::make_unique<Logger>(&desc);
    }
    if (mLogger)
    {
        mLogger->Error(GetExceptionMessage(), mFile.c_str(), mLine, mFunction.c_str());
        mLogger->Close();
    }
}
