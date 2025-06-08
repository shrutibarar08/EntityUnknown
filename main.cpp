#include <windows.h>
#include "Utils/Logger/Logger.h"

#include "WindowsManager/WindowsSystem.h"
#include "SystemManager/DependencyHandler/DependencyHandler.h"


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

    DependencyHandler handler{};
    WindowsSystem winSystem{};

    handler.Register(&winSystem);
    SweetLoader sweetLoader{};
    
    handler.InitAll(sweetLoader);

    while (true)
    {
        if (WindowsSystem::ProcessAndExit() || winSystem.Keyboard.WasKeyPressed(VK_ESCAPE))
        {
            handler.ShutdownAll(sweetLoader);
            return S_OK;
        }
        handler.RunAll(0.0f);
    }
}
