#pragma once

#include "platform.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include "PoseDetector.hpp"
#include "Statistics.hpp"
#include "Settings.hpp"

#define WM_APP_TRAYICON (WM_APP + 1)
#define IDM_EXIT       101
#define IDM_SETTINGS   102
#define IDM_STATS      103
#define IDM_TOGGLE     104

class PostureChecker {
public:
    PostureChecker();
    ~PostureChecker();

    bool Initialize();
    int Run();
    void Shutdown();

private:
    // Windows specific members
    HWND m_hwnd;
    NOTIFYICONDATA m_nid;
    HMENU m_hMenu;
    HINSTANCE m_hInstance;
    
    // Application state
    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_isMonitoring;
    std::unique_ptr<std::thread> m_monitorThread;
    
    // Components
    std::unique_ptr<PoseDetector> m_poseDetector;
    std::unique_ptr<Statistics> m_statistics;
    std::unique_ptr<Settings> m_settings;

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, 
                                     WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Private methods
    bool InitializeWindow();
    bool InitializeSystemTray();
    void CreateTrayMenu();
    void UpdateTrayIcon(bool isMonitoring);
    void StartMonitoring();
    void StopMonitoring();
    void MonitoringThread();
    void ShowStatistics();
    void ShowSettings();
    void ToggleMonitoring();

    // Utility methods
    void ShowNotification(const wchar_t* title, const wchar_t* message);
    bool RegisterWindowClass();
    void ProcessTrayIcon(LPARAM lParam);
}; 