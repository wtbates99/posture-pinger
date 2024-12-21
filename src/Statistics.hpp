#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

struct PostureRecord {
    std::chrono::system_clock::time_point timestamp;
    bool wasGoodPosture;
    int duration;
};

struct DailyStats {
    int totalChecks;
    int goodPostureCount;
    double averagePostureScore;
    int totalDuration;
};

class Statistics {
public:
    Statistics();
    ~Statistics();

    bool Initialize();
    void LogGoodPosture();
    void LogBadPosture();
    DailyStats GetDailyStats();
    std::vector<DailyStats> GetWeeklyStats();
    
private:
    sqlite3* m_db;
    std::filesystem::path m_dbPath;
    std::chrono::system_clock::time_point m_lastCheck;
    
    bool InitializeDatabase();
    bool CreateTables();
    bool ExecuteQuery(const char* query);
    void CalculateDuration();
    std::string GetDatabasePath();
}; 