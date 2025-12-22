/**
 * @file main.cpp
 * @brief Главная функция сервера
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include "Config.h"
#include "Server.h"
#include <iostream>
#include <csignal>
#include <cstdlib>

Server* serverInstance = nullptr;

/**
 * @brief Обработчик сигнала Ctrl+C
 * @param signal Номер сигнала
 */
void signalHandler(int signal) {
    if (signal == SIGINT && serverInstance != nullptr) {
        std::cout << "\nПолучен сигнал Ctrl+C, остановка сервера..." << std::endl;
        serverInstance->stop();
    }
}

int main(int argc, char** argv) {
    // Настройка обработчика сигналов
    signal(SIGINT, signalHandler);
    
    // Парсинг конфигурации
    Config config;
    if (!config.parseCommandLine(argc, argv)) {
        return EXIT_FAILURE;
    }
    
    try {
        // Создание и запуск сервера
        Server server(config);
        serverInstance = &server;
        
        std::cout << "=========================================" << std::endl;
        std::cout << "Сервер векторных вычислений" << std::endl;
        std::cout << "Автор: Судариков А.В." << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << "Конфигурация:" << std::endl;
        std::cout << "  База клиентов: " << config.getClientDbPath() << std::endl;
        std::cout << "  Файл журнала:  " << config.getLogFilePath() << std::endl;
        std::cout << "  Порт:          " << config.getPort() << std::endl;
        std::cout << "=========================================" << std::endl;
        std::cout << std::endl;
        
        if (!server.start()) {
            std::cerr << "Ошибка запуска сервера" << std::endl;
            return EXIT_FAILURE;
        }
        
        std::cout << "Сервер остановлен" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Неизвестное исключение" << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
