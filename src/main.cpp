#include "PostureChecker.hpp"
#include <windows.h>
#include <memory>
#include <stdexcept>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    try {
        // Ensure only one instance is running
        HANDLE hMutex = CreateMutex(NULL, TRUE, L"PostureCheckerMutex");
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            return 1;
        }

        // Initialize COM for system tray
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) {
            return 1;
        }

        // Create and run the application
        auto app = std::make_unique<PostureChecker>();
        if (!app->Initialize()) {
            return 1;
        }

        int result = app->Run();

        // Cleanup
        CoUninitialize();
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);

        return result;
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
} 