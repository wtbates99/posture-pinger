#pragma once

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#else
    // Linux/Unix placeholder definitions
    typedef void* HWND;
    typedef void* HINSTANCE;
    typedef unsigned long DWORD;
    typedef long LONG;
    typedef long LPARAM;
    typedef unsigned int UINT;
    typedef unsigned int WPARAM;
    typedef void* HMENU;
    typedef void* HICON;
    typedef void* HANDLE;
    typedef LONG LRESULT;
    typedef void* LPVOID;
    typedef const void* LPCVOID;
    typedef char* LPSTR;
    typedef const wchar_t* LPCWSTR;
    typedef wchar_t* LPWSTR;
    typedef int BOOL;
    typedef void* HMODULE;
    typedef int (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);
    
    #define CALLBACK
    #define WINAPI
    #define WM_DESTROY 0x0002
    #define WM_COMMAND 0x0111
    #define WM_APP 0x8000
    #define TRUE 1
    #define FALSE 0
    #define NULL nullptr
    #define MB_OK 0x00000000L
    #define MB_ICONERROR 0x00000010L
    #define TPM_RIGHTALIGN 0x0008L
    #define TPM_BOTTOMALIGN 0x0020L
    #define NIM_ADD 0x00000000
    #define NIM_MODIFY 0x00000001
    #define NIM_DELETE 0x00000002
    #define NIF_ICON 0x00000002
    #define NIF_MESSAGE 0x00000001
    #define NIF_TIP 0x00000004
    #define NIF_INFO 0x00000010
    #define NIIF_INFO 0x00000001
    
    struct POINT {
        long x;
        long y;
    };
    
    struct NOTIFYICONDATA {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        wchar_t szTip[128];
        wchar_t szInfo[256];
        wchar_t szInfoTitle[64];
        DWORD dwInfoFlags;
    };

    // Stub functions for Linux implementation
    inline BOOL Shell_NotifyIcon(DWORD dwMessage, NOTIFYICONDATA* lpData) { return TRUE; }
    inline BOOL GetCursorPos(POINT* lpPoint) { return TRUE; }
    inline BOOL SetForegroundWindow(HWND hWnd) { return TRUE; }
    inline BOOL TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y,
                              int nReserved, HWND hWnd, const RECT* prcRect) { return TRUE; }
#endif 