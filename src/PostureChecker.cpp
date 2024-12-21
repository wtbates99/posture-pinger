#include "PostureChecker.hpp"
#include <format>
#include "resource.h"

PostureChecker::PostureChecker()
    : m_hwnd(nullptr)
    , m_hMenu(nullptr)
    , m_hInstance(GetModuleHandle(nullptr))
    , m_isRunning(false)
    , m_isMonitoring(false)
{
    ZeroMemory(&m_nid, sizeof(m_nid));
}

PostureChecker::~PostureChecker() {
    Shutdown();
}

bool PostureChecker::Initialize() {
    // Initialize components
    m_settings = std::make_unique<Settings>();
    m_statistics = std::make_unique<Statistics>();
    m_poseDetector = std::make_unique<PoseDetector>();

    if (!m_settings->Load()) {
        return false;
    }

    if (!InitializeWindow()) {
        return false;
    }

    if (!InitializeSystemTray()) {
        return false;
    }

    m_isRunning = true;
    return true;
}

bool PostureChecker::InitializeWindow() {
    if (!RegisterWindowClass()) {
        return false;
    }

    m_hwnd = CreateWindowEx(
        0,                              // Optional window styles
        L"PostureCheckerClass",         // Window class
        L"Posture Checker",             // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,   // Position
        400, 300,                       // Size
        nullptr,                        // Parent window    
        nullptr,                        // Menu
        m_hInstance,                    // Instance handle
        this                            // Additional application data
    );

    if (!m_hwnd) {
        return false;
    }

    // Store the this pointer
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    return true;
}

bool PostureChecker::RegisterWindowClass() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"PostureCheckerClass";
    wc.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wc.hIconSm = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_SMALL));
    
    return RegisterClassEx(&wc);
}

bool PostureChecker::InitializeSystemTray() {
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_APP_TRAYICON;
    m_nid.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcscpy_s(m_nid.szTip, L"Posture Checker");

    if (!Shell_NotifyIcon(NIM_ADD, &m_nid)) {
        return false;
    }

    CreateTrayMenu();
    return true;
}

void PostureChecker::CreateTrayMenu() {
    m_hMenu = CreatePopupMenu();
    AppendMenu(m_hMenu, MF_STRING, IDM_TOGGLE, L"Start Monitoring");
    AppendMenu(m_hMenu, MF_STRING, IDM_STATS, L"Statistics");
    AppendMenu(m_hMenu, MF_STRING, IDM_SETTINGS, L"Settings");
    AppendMenu(m_hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(m_hMenu, MF_STRING, IDM_EXIT, L"Exit");
}

LRESULT CALLBACK PostureChecker::WindowProc(HWND hwnd, UINT uMsg, 
                                          WPARAM wParam, LPARAM lParam) {
    PostureChecker* checker = reinterpret_cast<PostureChecker*>(
        GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (checker) {
        return checker->HandleMessage(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT PostureChecker::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_APP_TRAYICON:
            ProcessTrayIcon(lParam);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_EXIT:
                    PostQuitMessage(0);
                    return 0;
                case IDM_SETTINGS:
                    ShowSettings();
                    return 0;
                case IDM_STATS:
                    ShowStatistics();
                    return 0;
                case IDM_TOGGLE:
                    ToggleMonitoring();
                    return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void PostureChecker::ProcessTrayIcon(LPARAM lParam) {
    if (lParam == WM_RBUTTONUP) {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(m_hwnd);
        TrackPopupMenu(m_hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                      pt.x, pt.y, 0, m_hwnd, nullptr);
    }
}

void PostureChecker::ToggleMonitoring() {
    if (m_isMonitoring) {
        StopMonitoring();
    } else {
        StartMonitoring();
    }
}

void PostureChecker::StartMonitoring() {
    if (!m_isMonitoring) {
        m_isMonitoring = true;
        m_monitorThread = std::make_unique<std::thread>(
            &PostureChecker::MonitoringThread, this);
        UpdateTrayIcon(true);
        ModifyMenu(m_hMenu, IDM_TOGGLE, MF_STRING, IDM_TOGGLE, L"Stop Monitoring");
    }
}

void PostureChecker::StopMonitoring() {
    if (m_isMonitoring) {
        m_isMonitoring = false;
        if (m_monitorThread && m_monitorThread->joinable()) {
            m_monitorThread->join();
        }
        UpdateTrayIcon(false);
        ModifyMenu(m_hMenu, IDM_TOGGLE, MF_STRING, IDM_TOGGLE, L"Start Monitoring");
    }
}

void PostureChecker::MonitoringThread() {
    while (m_isMonitoring) {
        if (m_poseDetector->CheckPosture()) {
            m_statistics->LogGoodPosture();
        } else {
            m_statistics->LogBadPosture();
            if (m_settings->AreNotificationsEnabled()) {
                ShowNotification(L"Posture Check", 
                               L"Please correct your posture!");
            }
        }
        std::this_thread::sleep_for(
            std::chrono::seconds(m_settings->GetCheckInterval()));
    }
}

void PostureChecker::ShowNotification(const wchar_t* title, const wchar_t* message) {
    m_nid.uFlags = NIF_INFO;
    wcscpy_s(m_nid.szInfoTitle, title);
    wcscpy_s(m_nid.szInfo, message);
    m_nid.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

int PostureChecker::Run() {
    ShowWindow(m_hwnd, SW_HIDE);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void PostureChecker::Shutdown() {
    StopMonitoring();
    if (m_nid.hWnd) {
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
    }
    if (m_hMenu) {
        DestroyMenu(m_hMenu);
    }
    m_isRunning = false;
} 