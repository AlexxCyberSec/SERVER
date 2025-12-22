#include "Config.h"
#include <iostream>
#include <cstring>
#include <getopt.h>

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
        {"client-db", required_argument, 0, 'c'},
        {"log-file", required_argument, 0, 'l'},
        {"port", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option;
    int optionIndex = 0;
    
    while ((option = getopt_long(argc, argv, "c:l:p:h", longOptions, &optionIndex)) != -1) {
        switch (option) {
            case 'c':
                clientDbPath_ = optarg;
                break;
            case 'l':
                logFilePath_ = optarg;
                break;
            case 'p':
                try {
                    port_ = static_cast<uint16_t>(std::stoi(optarg));
                    if (port_ == 0) {
                        std::cerr << "Ошибка: некорректный номер порта" << std::endl;
                        return false;
                    }
                } catch (...) {
                    std::cerr << "Ошибка: некорректный номер порта" << std::endl;
                    return false;
                }
                break;
            case 'h':
                showHelp(argv[0]);
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
    std::cout << "Параметры:\n";
    std::cout << "  -c, --client-db FILE    Файл базы клиентов (по умолчанию: /etc/vealc.conf)\n";
    std::cout << "  -l, --log-file FILE     Файл журнала (по умолчанию: /var/log/vealc.log)\n";
    std::cout << "  -p, --port PORT         Порт сервера (по умолчанию: 33333)\n";
    std::cout << "  -h, --help              Показать эту справку\n\n";
    std::cout << "Примеры:\n";
    std::cout << "  " << programName << " -c clients.txt -l server.log -p 44444\n";
    std::cout << "  " << programName << " --help\n";
}
