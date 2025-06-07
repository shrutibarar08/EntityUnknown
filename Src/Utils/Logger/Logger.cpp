#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>
#include <fstream>
#include <iostream>

Logger* gLogger = nullptr;

Logger::Logger(const LOGGER_INITIALIZE_DESC* desc)
{
    if (desc->EnableTerminal) EnableTerminal();

    mLoggerDesc.FilePrefix = desc->FilePrefix;
    mLoggerDesc.EnableTerminal = desc->EnableTerminal;
    mLoggerDesc.FolderPath = desc->FolderPath;

    mMutexHandle = CreateMutex(nullptr,
        FALSE,
        nullptr);

    mFileSystem.OpenForWrite(GetTimestampForLogPath());
}

Logger::~Logger()
{
    Close();
}

void Logger::EnableTerminal()
{
    // Create a new console window if one doesn't exist
    if (!AllocConsole()) return;

    // Redirect standard output to the console
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONIN$", "r", stdin);

    mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool Logger::Log(const std::string& prefix, const std::string& message, WORD color,
    const char* file, int line, const char* func)
{
    std::ostringstream oss;

    // Optional: Add timestamp
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);
    oss << "[" << std::put_time(&localTime, "%H:%M:%S") << "] ";

    // Prefix and location
    oss << "[" << prefix << "] ";
    if (file && func && line >= 0)
    {
        oss << "(" << file << ":" << line << " | " << func << ") ";
    }

    // Actual message
    oss << "- " << message << "\n";

    std::string refinedMessage = oss.str();
    bool saved = false;

    DWORD res = WaitForSingleObject(mMutexHandle, 2000);

    if (res == WAIT_OBJECT_0)
    {
        // Got the lock
        SetConsoleTextAttribute(mConsoleHandle, color);
        std::cout << refinedMessage;
        SetConsoleTextAttribute(mConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        saved = mFileSystem.WritePlainText(refinedMessage);

        ReleaseMutex(mMutexHandle);
    }
    else if (res == WAIT_TIMEOUT)
    {
        saved = mFileSystem.WritePlainText("[FAILURE] Logger timeout.\n");
        return false;
    }
    else
    {
        return false;
    }
    return saved;
}

bool Logger::Info(const std::string& message)
{
    return Log("INFO", message, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

bool Logger::Print(const std::string& message)
{
    return Log("", message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

bool Logger::Warning(const std::string& message)
{
    return Log("WARNING", message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

bool Logger::Error(const std::string& message, const char* file, int line, const char* func)
{
    return Log("ERROR", message, FOREGROUND_RED | FOREGROUND_INTENSITY, file, line, func);
}

bool Logger::Success(const std::string& message)
{
    return Log("SUCCESS", message, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

bool Logger::Fail(const std::string& message, const char* file, int line, const char* func)
{
    return Log("FAIL", message, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, file, line, func);
}

std::string Logger::GetTimestampForLogPath()
{
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

    localtime_s(&localTime, &timeNow);


    std::string folder = mLoggerDesc.FolderPath;
    if (*folder.rbegin() != '/')
    {
        folder += "/";
    }

    std::string file = mLoggerDesc.FilePrefix;
    if (*file.rbegin() != '_')
    {
        file += "_";
    }

    std::ostringstream oss;
    oss << folder
        << file
        << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S")
        << ".txt";

    return oss.str();
}

void Logger::Close()
{
    mFileSystem.Close();
}