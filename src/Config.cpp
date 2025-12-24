#include "Config.h"
#include <iostream>
#include <cstring>
#include <getopt.h>
#include <limits>

Config::Config() : port_(33333) {
    setDefaults();
}

void Config::setDefaults() {
    clientDbPath_ = "/etc/vealc.conf";
    logFilePath_ = "/var/log/vealc.log";
    port_ = 33333;
}

bool Config::parseCommandLine(int argc, char** argv) {
    struct option longOptions[] = {
        {"config", required_argument, 0, 'c'},
        {"log", required_argument, 0, 'l'},
        {"port", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    int option;
    int optionIndex = 0;
    
    while ((option = getopt_long(argc, argv, "c:l:p:hv", longOptions, &optionIndex)) != -1) {
        switch (option) {
            case 'c':
                clientDbPath_ = optarg;
                break;
            case 'l':
                logFilePath_ = optarg;
                break;
            case 'p':
                try {
                    int port = std::stoi(optarg);
                    
                    // Проверка диапазона портов (1024-65535)
                    if (port < 1024 || port > 65535) {
                        std::cerr << "Ошибка: порт должен быть в диапазоне 1024-65535" << std::endl;
                        return false;
                    }
                    
                    port_ = static_cast<uint16_t>(port);
                    
                } catch (const std::invalid_argument&) {
                    std::cerr << "Ошибка: некорректный номер порта (не число)" << std::endl;
                    return false;
                } catch (const std::out_of_range&) {
                    std::cerr << "Ошибка: некорректный номер порта (выход за диапазон)" << std::endl;
                    return false;
                }
                break;
            case 'h':
                showHelp(argv[0]);
                return false;
            case 'v':
                showVersion();
                return false;
            default:
                std::cerr << "Неизвестный параметр. Используйте -h для справки." << std::endl;
                return false;
        }
    }
    
    if (optind < argc) {
        std::cerr << "Неизвестные аргументы: ";
        for (int i = optind; i < argc; i++) {
            std::cerr << argv[i] << " ";
        }
        std::cerr << std::endl;
        showHelp(argv[0]);
        return false;
    }
    
    return true;
}

void Config::showHelp(const char* programName) {
    std::cout << "Использование: " << programName << " [ПАРАМЕТРЫ]\n\n";
    std::cout << "Сервер для обработки векторных данных\n\n";
    std::cout << "Обязательные опции:\n";
    std::cout << "  -c, --config FILE    Файл базы данных пользователей\n";
    std::cout << "  -l, --log FILE       Файл журнала для записи ошибок\n";
    std::cout << "  -p, --port PORT      Порт сервера (1024-65535)\n\n";
    std::cout << "Дополнительные опции:\n";
    std::cout << "  -h, --help           Показать эту справку\n";
    std::cout << "  -v, --version        Показать информацию о версии\n\n";
    std::cout << "Значения по умолчанию:\n";
    std::cout << "  --config " << clientDbPath_ << "\n";
    std::cout << "  --log   " << logFilePath_ << "\n";
    std::cout << "  --port  " << port_ << "\n\n";
    std::cout << "Примеры:\n";
    std::cout << "  " << programName << " -c /etc/my.conf -l /var/log/my.log -p 30000\n";
    std::cout << "  " << programName << " --config ~/server.conf --log /tmp/server.log --port 33333\n";
    std::cout << "  " << programName << " --help\n";
    std::cout << "  " << programName << " --version\n";
}

void Config::showVersion() {
    std::cout << "vealc_server версия 1.0.0\n";
    std::cout << "Сервер для векторных вычислений\n";
    std::cout << "Автор: Судариков А.В., группа 24ПИ1\n";
    std::cout << "Пензенский государственный университет, 2025\n";
}

// Реализации геттеров
const std::string& Config::getClientDbPath() const {
    return clientDbPath_;
}

const std::string& Config::getLogFilePath() const {
    return logFilePath_;
}

uint16_t Config::getPort() const {
    return port_;
}
