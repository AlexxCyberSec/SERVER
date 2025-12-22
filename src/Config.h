/**
 * @file Config.h
 * @brief Конфигурация сервера
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    INFO,       ///< Информационное сообщение
    WARNING,    ///< Предупреждение
    ERROR,      ///< Ошибка
    CRITICAL    ///< Критическая ошибка
};

/**
 * @brief Класс конфигурации сервера
 */
class Config {
private:
    std::string clientDbPath_;
    std::string logFilePath_;
    uint16_t port_;
    
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Config();
    
    /**
     * @brief Парсинг параметров командной строки
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @return true - успешный парсинг
     */
    bool parseCommandLine(int argc, char** argv);
    
    /**
     * @brief Установка значений по умолчанию
     */
    void setDefaults();
    
    // Геттеры
    std::string getClientDbPath() const { return clientDbPath_; }
    std::string getLogFilePath() const { return logFilePath_; }
    uint16_t getPort() const { return port_; }
    
    /**
     * @brief Показать справку
     * @param programName Имя программы
     */
    static void showHelp(const char* programName);
};

#endif // CONFIG_H
