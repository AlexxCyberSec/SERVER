/**
 * @file Logger.h
 * @brief Логирование сервера
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include "Config.h"

/**
 * @brief Класс логирования
 */
class Logger {
private:
    std::string logFilePath_;
    std::ofstream logFile_;
    
    std::string levelToString(LogLevel level) const;
    std::string getCurrentDateTime() const;
    
public:
    /**
     * @brief Конструктор
     * @param filePath Путь к файлу журнала
     */
    Logger(const std::string& filePath);
    
    /**
     * @brief Деструктор
     */
    ~Logger();
    
    /**
     * @brief Записать сообщение в журнал
     * @param level Уровень
     * @param message Сообщение
     * @param details Детали
     */
    void log(LogLevel level, const std::string& message, 
             const std::string& details = "");
    
    /**
     * @brief Записать системную ошибку
     * @param context Контекст
     */
    void logSystemError(const std::string& context);
};

#endif // LOGGER_H
