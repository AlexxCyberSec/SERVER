#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <cstring>

Logger::Logger(const std::string& filePath) : logFilePath_(filePath) {
    logFile_.open(filePath, std::ios::app);
    if (!logFile_.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл журнала: " 
                  << filePath << std::endl;
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentDateTime() const {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return std::string(buffer);
}

void Logger::log(LogLevel level, const std::string& message, 
                const std::string& details) {
    if (!logFile_.is_open()) {
        return;
    }
    
    std::string dateTime = getCurrentDateTime();
    std::string levelStr = levelToString(level);
    
    logFile_ << dateTime << " [" << levelStr << "] " << message;
    
    if (!details.empty()) {
        logFile_ << " (" << details << ")";
    }
    
    logFile_ << std::endl;
    
    // Для отладки выводим также в консоль
    std::cout << "[" << levelStr << "] " << message;
    if (!details.empty()) {
        std::cout << " (" << details << ")";
    }
    std::cout << std::endl;
    
    if (level == LogLevel::CRITICAL) {
        std::cerr << "КРИТИЧЕСКАЯ ОШИБКА: " << message;
        if (!details.empty()) {
            std::cerr << " (" << details << ")";
        }
        std::cerr << std::endl;
    }
}

void Logger::logSystemError(const std::string& context) {
    log(LogLevel::ERROR, context, strerror(errno));
}
