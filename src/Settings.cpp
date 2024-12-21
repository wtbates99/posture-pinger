#include "Settings.hpp"
#include <fstream>
#include <windows.h>

Settings::Settings()
    : m_notificationsEnabled(DEFAULT_NOTIFICATIONS)
    , m_checkIntervalSeconds(DEFAULT_CHECK_INTERVAL)
    , m_postureSensitivity(DEFAULT_SENSITIVITY)
    , m_autoStartEnabled(DEFAULT_AUTO_START)
{
    m_settingsPath = GetSettingsPath();
}

Settings::~Settings() {
    Save();
}

std::filesystem::path Settings::GetSettingsPath() {
    std::filesystem::path appData;
    
    #ifdef _WIN32
        char* localAppData = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&localAppData, &sz, "LOCALAPPDATA") == 0 && localAppData) {
            appData = localAppData;
            free(localAppData);
        }
    #else
        const char* home = getenv("HOME");
        if (home) {
            appData = std::string(home) + "/.config";
        }
    #endif
    
    return appData / "PostureChecker" / "settings.json";
}

bool Settings::Load() {
    try {
        if (!std::filesystem::exists(m_settingsPath)) {
            return CreateSettingsFile();
        }

        std::ifstream file(m_settingsPath);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json j;
        file >> j;

        m_notificationsEnabled = j.value("notifications_enabled", DEFAULT_NOTIFICATIONS);
        m_checkIntervalSeconds = j.value("check_interval", DEFAULT_CHECK_INTERVAL);
        m_postureSensitivity = j.value("posture_sensitivity", DEFAULT_SENSITIVITY);
        m_autoStartEnabled = j.value("auto_start", DEFAULT_AUTO_START);

        return true;
    }
    catch (const std::exception&) {
        SetDefaults();
        return false;
    }
}

bool Settings::Save() {
    try {
        std::filesystem::create_directories(m_settingsPath.parent_path());

        nlohmann::json j;
        j["notifications_enabled"] = m_notificationsEnabled;
        j["check_interval"] = m_checkIntervalSeconds;
        j["posture_sensitivity"] = m_postureSensitivity;
        j["auto_start"] = m_autoStartEnabled;

        std::ofstream file(m_settingsPath);
        if (!file.is_open()) {
            return false;
        }

        file << j.dump(4);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool Settings::CreateSettingsFile() {
    SetDefaults();
    return Save();
}

void Settings::SetDefaults() {
    m_notificationsEnabled = DEFAULT_NOTIFICATIONS;
    m_checkIntervalSeconds = DEFAULT_CHECK_INTERVAL;
    m_postureSensitivity = DEFAULT_SENSITIVITY;
    m_autoStartEnabled = DEFAULT_AUTO_START;
}

void Settings::SetNotificationsEnabled(bool enabled) {
    m_notificationsEnabled = enabled;
    Save();
}

void Settings::SetCheckInterval(int seconds) {
    m_checkIntervalSeconds = std::max(10, std::min(300, seconds));
    Save();
}

void Settings::SetPostureSensitivity(float sensitivity) {
    m_postureSensitivity = std::max(0.1f, std::min(1.0f, sensitivity));
    Save();
}

void Settings::SetAutoStartEnabled(bool enabled) {
    if (m_autoStartEnabled != enabled) {
        if (UpdateAutoStartRegistry(enabled)) {
            m_autoStartEnabled = enabled;
            Save();
        }
    }
}

bool Settings::UpdateAutoStartRegistry(bool enable) {
    HKEY hKey;
    const wchar_t* keyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t* valueName = L"PostureChecker";
    
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    bool success = false;
    if (enable) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        result = RegSetValueExW(hKey, valueName, 0, REG_SZ,
                              (BYTE*)exePath, 
                              (wcslen(exePath) + 1) * sizeof(wchar_t));
        success = (result == ERROR_SUCCESS);
    } else {
        result = RegDeleteValueW(hKey, valueName);
        success = (result == ERROR_SUCCESS);
    }

    RegCloseKey(hKey);
    return success;
} 