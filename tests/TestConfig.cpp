/**
 * @file TestConfig.cpp
 * @brief Модульные тесты для класса Config
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include <UnitTest++/UnitTest++.h>
#include "../src/Config.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// Вспомогательная функция для сброса getopt
void resetGetopt() {
    #ifdef _GNU_SOURCE
    optind = 0;
    #else
    optind = 1;
    #endif
    opterr = 0; // Отключаем автоматический вывод ошибок getopt
}

// === 1. Тест конструктора и значений по умолчанию ===
TEST(Config_Constructor_Defaults) {
    Config config;
    
    CHECK_EQUAL("/etc/vealc.conf", config.getClientDbPath());
    CHECK_EQUAL("/var/log/vealc.log", config.getLogFilePath());
    CHECK_EQUAL(33333, config.getPort());
}

// === 2. Тест парсинга коротких опций ===
TEST(Config_ParseCommandLine_ShortOptions) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-c", "custom.conf",
        "-l", "custom.log",
        "-p", "44444"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    if (result) {
        CHECK_EQUAL("custom.conf", config.getClientDbPath());
        CHECK_EQUAL("custom.log", config.getLogFilePath());
        CHECK_EQUAL(44444, config.getPort());
    }
}

// === 3. Тест парсинга длинных опций ===
TEST(Config_ParseCommandLine_LongOptions) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "--client-db", "long.conf",
        "--log-file", "long.log",
        "--port", "55555"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    if (result) {
        CHECK_EQUAL("long.conf", config.getClientDbPath());
        CHECK_EQUAL("long.log", config.getLogFilePath());
        CHECK_EQUAL(55555, config.getPort());
    }
}

// === 4. Тест смешанных опций ===
TEST(Config_ParseCommandLine_MixedOptions) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-c", "mixed.conf",
        "--log-file", "mixed.log",
        "-p", "12345"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    if (result) {
        CHECK_EQUAL("mixed.conf", config.getClientDbPath());
        CHECK_EQUAL("mixed.log", config.getLogFilePath());
        CHECK_EQUAL(12345, config.getPort());
    }
}

// === 5. Тест справки (help) ===
TEST(Config_ParseCommandLine_Help) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-h"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    
    // Перенаправляем вывод, чтобы не засорять тесты
    std::streambuf* oldCout = std::cout.rdbuf();
    std::stringstream buffer;
    std::cout.rdbuf(buffer.rdbuf());
    
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    // Восстанавливаем вывод
    std::cout.rdbuf(oldCout);
    
    // При -h должен вернуть false
    CHECK(!result);
}

// === 6. Тест некорректного порта ===
TEST(Config_ParseCommandLine_InvalidPortZero) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-p", "0"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    
    // Перенаправляем stderr чтобы не видеть сообщение об ошибке
    std::streambuf* oldCerr = std::cerr.rdbuf();
    std::stringstream errorBuffer;
    std::cerr.rdbuf(errorBuffer.rdbuf());
    
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    // Восстанавливаем stderr
    std::cerr.rdbuf(oldCerr);
    
    CHECK(!result); // Должен вернуть false
}

TEST(Config_ParseCommandLine_InvalidPortText) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-p", "abc"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    
    std::streambuf* oldCerr = std::cerr.rdbuf();
    std::stringstream errorBuffer;
    std::cerr.rdbuf(errorBuffer.rdbuf());
    
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    std::cerr.rdbuf(oldCerr);
    
    CHECK(!result);
}

// === 7. Тест без параметров (только программа) ===
TEST(Config_ParseCommandLine_NoOptions) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    // Должны остаться значения по умолчанию
    CHECK_EQUAL("/etc/vealc.conf", config.getClientDbPath());
    CHECK_EQUAL("/var/log/vealc.log", config.getLogFilePath());
    CHECK_EQUAL(33333, config.getPort());
}

// === 8. Тест неизвестного параметра ===
TEST(Config_ParseCommandLine_UnknownOption) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-x", "value" // Неизвестная опция
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    
    std::streambuf* oldCerr = std::cerr.rdbuf();
    std::stringstream errorBuffer;
    std::cerr.rdbuf(errorBuffer.rdbuf());
    
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    std::cerr.rdbuf(oldCerr);
    
    CHECK(!result);
}

// === 9. Тест частичных параметров ===
TEST(Config_ParseCommandLine_Partial) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-c", "partial.conf"
        // Остальные параметры по умолчанию
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    CHECK_EQUAL("partial.conf", config.getClientDbPath());
    CHECK_EQUAL("/var/log/vealc.log", config.getLogFilePath()); // По умолчанию
    CHECK_EQUAL(33333, config.getPort()); // По умолчанию
}

// === 10. Тест метода setDefaults ===
TEST(Config_SetDefaults) {
    Config config;
    
    // Проверяем значения по умолчанию
    CHECK_EQUAL("/etc/vealc.conf", config.getClientDbPath());
    CHECK_EQUAL("/var/log/vealc.log", config.getLogFilePath());
    CHECK_EQUAL(33333, config.getPort());
    
    // Метод setDefaults вызывается в конструкторе,
    // так что просто проверяем что он существует
    config.setDefaults();
    CHECK(true); // Просто проверяем что не падает
}

// === 11. Тест граничного значения порта ===
TEST(Config_ParseCommandLine_BoundaryPort) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-p", "1" // Минимальный валидный порт
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    CHECK_EQUAL(1, config.getPort());
}

TEST(Config_ParseCommandLine_BoundaryPortMax) {
    resetGetopt();
    
    const char* argv[] = {
        "testprogram",
        "-p", "65535" // Максимальный порт
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    Config config;
    bool result = config.parseCommandLine(argc, const_cast<char**>(argv));
    
    CHECK(result);
    CHECK_EQUAL(65535, config.getPort());
}

// === 12. Тест метода showHelp (не падает) ===
TEST(Config_ShowHelp) {
    // Перенаправляем вывод
    std::streambuf* oldCout = std::cout.rdbuf();
    std::stringstream buffer;
    std::cout.rdbuf(buffer.rdbuf());
    
    // Просто проверяем что метод не падает
    Config::showHelp("testprogram");
    
    // Восстанавливаем вывод
    std::cout.rdbuf(oldCout);
    
    CHECK(true);
}

/**
 * @brief Основная функция
 */
int main() {
    std::cout << "=== Testing Config ===" << std::endl;
    
    // Отключаем вывод ошибок getopt
    opterr = 0;
    
    return UnitTest::RunAllTests();
}
