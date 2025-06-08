#include <windows.h>

#include "ExceptionManager/IException.h"
#include "RenderManager/RenderSystem.h"
#include "Utils/Logger/Logger.h"

#include "WindowsManager/WindowsSystem.h"
#include "SystemManager/DependencyHandler/DependencyHandler.h"
#include "SystemManager/EventQueue/EventQueue.h"


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow) {
#ifdef _DEBUG
    LOGGER_INITIALIZE_DESC desc{};
    desc.FilePrefix = "SweetLog";
    desc.EnableTerminal = true;
    desc.FolderPath = "Logs";
    INIT_GLOBAL_LOGGER(&desc);
#endif

    try
    {
        DependencyHandler handler{};
        auto winSystem = std::make_unique<WindowsSystem>();
        auto renderer = std::make_unique<RenderSystem>(winSystem.get());

        handler.Register(winSystem.get());
        handler.Register(renderer.get());

        handler.AddDependency(renderer.get(), winSystem.get());

        SweetLoader sweetLoader{};
        bool flag = true;

        handler.InitAll(sweetLoader);

        while (true)
        {
            if (WindowsSystem::ProcessAndExit() || winSystem->Keyboard.WasKeyPressed(VK_ESCAPE))
            {
                handler.ShutdownAll(sweetLoader);
                return S_OK;
            }
            if (flag)
            {
                winSystem->SetFullScreen(true);
                flag = false;
            }
            winSystem->OnTick(0.0f);
            renderer->OnTick(0.0f);
            EventBus::DispatchAll();
        }
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
        logDesc.FilePrefix = "BasicException";
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
        logDesc.FilePrefix = "BasicException";
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

    return E_FAIL;
}
