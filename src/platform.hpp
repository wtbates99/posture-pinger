#pragma once

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#else
    // Linux/Unix headers
    #include <X11/Xlib.h>
    // Add other required Linux headers
#endif 