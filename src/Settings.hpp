#pragma once

#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

class Settings {
public:
    Settings();
    ~Settings();

    bool Load();
    bool Save();

    // Getters
    bool AreNotificationsEnabled() const { return m_notificationsEnabled; }
    int GetCheckInterval() const { return m_checkIntervalSeconds; }
    float GetPostureSensitivity() const { return m_postureSensitivity; }
    bool IsAutoStartEnabled() const { return m_autoStartEnabled; }

    // Setters
    void SetNotificationsEnabled(bool enabled);
    void SetCheckInterval(int seconds);
    void SetPostureSensitivity(float sensitivity);
    void SetAutoStartEnabled(bool enabled);

private:
    std::filesystem::path m_settingsPath;
    
    // Settings variables
    bool m_notificationsEnabled;
    int m_checkIntervalSeconds;
    float m_postureSensitivity;
    bool m_autoStartEnabled;

    // Default values
    static constexpr bool DEFAULT_NOTIFICATIONS = true;
    static constexpr int DEFAULT_CHECK_INTERVAL = 30;
    static constexpr float DEFAULT_SENSITIVITY = 0.5f;
    static constexpr bool DEFAULT_AUTO_START = true;

    bool CreateSettingsFile();
    std::filesystem::path GetSettingsPath();
    void SetDefaults();
    bool UpdateAutoStartRegistry(bool enable);
}; 