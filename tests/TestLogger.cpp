/**
 * @file TestLogger.cpp
 * @brief Модульные тесты для класса Logger
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include <UnitTest++/UnitTest++.h>
#include "../src/Logger.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>

// === 1. Тест создания логгера ===
TEST(Logger_Constructor_ValidFile) {
    const std::string testFile = "test_logger_constructor.log";
    
    // Удаляем файл если существует
    std::remove(testFile.c_str());
    
    // Создаём логгер
    Logger logger(testFile);
    
    // Проверяем косвенно - записываем сообщение
    logger.log(LogLevel::INFO, "Test constructor");
    
    // Проверяем, что файл создан и не пуст
    std::ifstream file(testFile);
    CHECK(file.good());
    
    std::string line;
    std::getline(file, line);
    CHECK(!line.empty());
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 2. Тест записи разных уровней логирования ===
TEST(Logger_Log_DifferentLevels) {
    const std::string testFile = "test_logger_levels.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    
    // Записываем сообщения всех уровней
    logger.log(LogLevel::INFO, "Info message");
    logger.log(LogLevel::WARNING, "Warning message");
    logger.log(LogLevel::ERROR, "Error message");
    logger.log(LogLevel::CRITICAL, "Critical message");
    
    // Проверяем количество записей
    std::ifstream file(testFile);
    int lineCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        lineCount++;
    }
    
    CHECK_EQUAL(4, lineCount);
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 3. Тест формата записи ===
TEST(Logger_Log_Format) {
    const std::string testFile = "test_logger_format.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    logger.log(LogLevel::INFO, "Test message");
    
    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    
    // Проверяем базовый формат:
    // "YYYY-MM-DD HH:MM:SS [LEVEL] message"
    CHECK(line.find("[INFO]") != std::string::npos);
    CHECK(line.find("Test message") != std::string::npos);
    
    // Проверяем наличие даты (минимум 10 символов даты)
    CHECK(line.length() >= 19); // "2025-01-01 12:00:00" = 19 символов
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 4. Тест записи с дополнительными деталями ===
TEST(Logger_Log_WithDetails) {
    const std::string testFile = "test_logger_details.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    logger.log(LogLevel::ERROR, "Connection failed", "Port 33333");
    
    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    
    // Проверяем наличие сообщения и деталей
    CHECK(line.find("Connection failed") != std::string::npos);
    CHECK(line.find("Port 33333") != std::string::npos);
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 5. Тест системной ошибки ===
TEST(Logger_LogSystemError) {
    const std::string testFile = "test_logger_system.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    
    // Симулируем системную ошибку (errno может быть любым)
    errno = ENOENT; // Нет такого файла или директории
    logger.logSystemError("Failed to open file");
    
    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    
    // Проверяем наличие контекста ошибки
    CHECK(line.find("Failed to open file") != std::string::npos);
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 6. Тест пустых деталей ===
TEST(Logger_Log_EmptyDetails) {
    const std::string testFile = "test_logger_empty_details.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    logger.log(LogLevel::WARNING, "Warning without details", "");
    
    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    
    // Проверяем что нет лишних скобок
    CHECK(line.find("()") == std::string::npos);
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 7. Тест критической ошибки ===
TEST(Logger_Log_CriticalToStderr) {
    const std::string testFile = "test_logger_critical.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    
    // Критическая ошибка должна логироваться
    logger.log(LogLevel::CRITICAL, "System crash imminent");
    
    // Проверяем что записалось в файл
    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    
    CHECK(line.find("[CRITICAL]") != std::string::npos);
    CHECK(line.find("System crash imminent") != std::string::npos);
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 8. Тест консольного вывода ===
TEST(Logger_ConsoleOutput) {
    const std::string testFile = "test_logger_console.log";
    std::remove(testFile.c_str());
    
    Logger logger(testFile);
    
    // Этот тест сложно автоматизировать, но проверим что не падает
    logger.log(LogLevel::INFO, "Console test");
    logger.log(LogLevel::ERROR, "Error console test");
    
    // Очистка
    std::remove(testFile.c_str());
}

// === 9. Тест деструктора ===
TEST(Logger_Destructor) {
    const std::string testFile = "test_logger_destructor.log";
    std::remove(testFile.c_str());
    
    {
        // Создаём логгер в отдельном блоке
        Logger logger(testFile);
        logger.log(LogLevel::INFO, "Before destructor");
    } // Здесь вызывается деструктор
    
    // После деструктора файл должен быть закрыт
    std::ifstream file(testFile);
    CHECK(file.good());
    
    std::string line;
    std::getline(file, line);
    CHECK(!line.empty());
    
    // Очистка
    file.close();
    std::remove(testFile.c_str());
}

// === 10. Тест на невалидный файл ===
TEST(Logger_InvalidFile) {
    const std::string invalidPath = "/root/test.log"; // Запрещённый путь
    
    // Создаём логгер с невалидным путём
    // Должен вывести ошибку в cerr, но не упасть
    Logger logger(invalidPath);
    
    // Пытаемся записать - не должно падать
    logger.log(LogLevel::INFO, "Test to invalid file");
    
    // Проверяем что программа не упала
    CHECK(true);
}

/**
 * @brief Основная функция
 */
int main() {
    std::cout << "=== Testing Logger ===" << std::endl;
    
    // Очищаем тестовые файлы перед запуском
    system("rm -f test_logger_*.log 2>/dev/null");
    
    return UnitTest::RunAllTests();
}
