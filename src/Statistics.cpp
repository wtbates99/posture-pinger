#include "Statistics.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

Statistics::Statistics() : m_db(nullptr) {
    m_lastCheck = std::chrono::system_clock::now();
    Initialize();
}

Statistics::~Statistics() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

bool Statistics::Initialize() {
    m_dbPath = std::filesystem::path(GetDatabasePath());
    
    // Create directory if it doesn't exist
    std::filesystem::create_directories(m_dbPath.parent_path());
    
    return InitializeDatabase();
}

std::string Statistics::GetDatabasePath() {
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
            appData = std::string(home) + "/.local/share";
        }
    #endif
    
    return (appData / "PostureChecker" / "statistics.db").string();
}

bool Statistics::InitializeDatabase() {
    int rc = sqlite3_open(m_dbPath.string().c_str(), &m_db);
    if (rc) {
        sqlite3_close(m_db);
        return false;
    }
    
    return CreateTables();
}

bool Statistics::CreateTables() {
    const char* query = 
        "CREATE TABLE IF NOT EXISTS posture_records ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    is_good_posture BOOLEAN,"
        "    duration INTEGER"
        ");"
        
        "CREATE INDEX IF NOT EXISTS idx_timestamp "
        "ON posture_records(timestamp);";
    
    return ExecuteQuery(query);
}

bool Statistics::ExecuteQuery(const char* query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, query, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        if (errMsg) {
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}

void Statistics::LogGoodPosture() {
    CalculateDuration();
    
    std::stringstream ss;
    ss << "INSERT INTO posture_records (is_good_posture, duration) VALUES (1, "
       << std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now() - m_lastCheck).count()
       << ");";
    
    ExecuteQuery(ss.str().c_str());
    m_lastCheck = std::chrono::system_clock::now();
}

void Statistics::LogBadPosture() {
    CalculateDuration();
    
    std::stringstream ss;
    ss << "INSERT INTO posture_records (is_good_posture, duration) VALUES (0, "
       << std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::system_clock::now() - m_lastCheck).count()
       << ");";
    
    ExecuteQuery(ss.str().c_str());
    m_lastCheck = std::chrono::system_clock::now();
}

void Statistics::CalculateDuration() {
    auto now = std::chrono::system_clock::now();
    if (now - m_lastCheck > std::chrono::minutes(5)) {
        // If more than 5 minutes passed, assume user was away
        m_lastCheck = now;
    }
}

DailyStats Statistics::GetDailyStats() {
    DailyStats stats = {0, 0, 0.0, 0};
    
    const char* query = 
        "SELECT COUNT(*) as total,"
        "       SUM(CASE WHEN is_good_posture THEN 1 ELSE 0 END) as good,"
        "       SUM(duration) as total_duration "
        "FROM posture_records "
        "WHERE date(timestamp) = date('now', 'localtime');";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalChecks = sqlite3_column_int(stmt, 0);
            stats.goodPostureCount = sqlite3_column_int(stmt, 1);
            stats.totalDuration = sqlite3_column_int(stmt, 2);
            
            if (stats.totalChecks > 0) {
                stats.averagePostureScore = 
                    (double)stats.goodPostureCount / stats.totalChecks * 100.0;
            }
        }
        sqlite3_finalize(stmt);
    }
    
    return stats;
}

std::vector<DailyStats> Statistics::GetWeeklyStats() {
    std::vector<DailyStats> weeklyStats;
    
    const char* query = 
        "SELECT COUNT(*) as total,"
        "       SUM(CASE WHEN is_good_posture THEN 1 ELSE 0 END) as good,"
        "       SUM(duration) as total_duration "
        "FROM posture_records "
        "WHERE timestamp >= date('now', '-6 days') "
        "GROUP BY date(timestamp) "
        "ORDER BY date(timestamp);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            DailyStats stats;
            stats.totalChecks = sqlite3_column_int(stmt, 0);
            stats.goodPostureCount = sqlite3_column_int(stmt, 1);
            stats.totalDuration = sqlite3_column_int(stmt, 2);
            
            if (stats.totalChecks > 0) {
                stats.averagePostureScore = 
                    (double)stats.goodPostureCount / stats.totalChecks * 100.0;
            }
            
            weeklyStats.push_back(stats);
        }
        sqlite3_finalize(stmt);
    }
    
    return weeklyStats;
} 