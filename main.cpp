#include <windows.h>
#include "Utils/Logger/Logger.h"


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow) {
#ifdef _DEBUG
    LOGGER_INITIALIZE_DESC desc{};
    desc.FilePath = "SweetLog";
    desc.EnableTerminal = true;
    desc.FolderPath = "Logs";
    INIT_GLOBAL_LOGGER(&desc);
#endif

    HRESULT result = E_FAIL;
    try
    {

    }
    catch (IException& e)
    {
        e.SaveCrashReport();
        MessageBoxA(nullptr, e.what(),
            "Application Error",
            MB_ICONERROR | MB_OK);
    }
    catch (const std::exception& e)
    {
        // Catch any standard C++ exceptions
        LOGGER_INITIALIZE_DESC logDesc{};
        logDesc.FilePath = "BasicException";
        logDesc.FolderPath = Draco::Exception::DEFAULT_CRASH_FOLDER;

        Logger logger{ &logDesc };
        logger.Error(e.what(),
            "UnknownFile",
            0,
            "UnknownFunction"
        );
        logger.Close();

        MessageBoxA(nullptr,
            e.what(),
            "Standard Exception",
            MB_ICONERROR | MB_OK);
    }
    catch (...)
    {
        // Catch absolutely everything else
        LOGGER_INITIALIZE_DESC logDesc{};
        logDesc.FilePath = "BasicException";
        logDesc.FolderPath = Draco::Exception::DEFAULT_CRASH_FOLDER;

        Logger logger{ &logDesc };
        logger.Error("Unknown fatal error occurred.",
            "UnknownFile",
            0,
            "UnknownFunction");
        logger.Close();

        MessageBoxA(nullptr,
            "Unknown fatal error occurred.",
            "Fatal Error",
            MB_ICONERROR | MB_OK);
    }
    return result;
}
