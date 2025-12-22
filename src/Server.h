/**
 * @file Server.h
 * @brief Главный класс сервера
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef SERVER_H
#define SERVER_H

#include "Config.h"
#include "Database.h"
#include "Logger.h"
#include "Authenticator.h"
#include "VectorProcessor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>

/**
 * @brief Главный класс сервера
 */
class Server {
private:
    Config config_;
    Database database_;
    Logger logger_;
    int serverSocket_;
    bool running_;
    
public:
    /**
     * @brief Конструктор
     * @param config Конфигурация
     */
    Server(const Config& config);
    
    /**
     * @brief Деструктор
     */
    ~Server();
    
    /**
     * @brief Запустить сервер
     * @return true - успешный запуск
     */
    bool start();
    
    /**
     * @brief Остановить сервер
     */
    void stop();
    
    /**
     * @brief Проверить работает ли сервер
     * @return true - сервер работает
     */
    bool isRunning() const { return running_; }
    
private:
    /**
     * @brief Инициализировать сетевое соединение
     * @return true - успешно
     */
    bool initializeNetwork();
    
    /**
     * @brief Главный цикл сервера
     */
    void mainLoop();
    
    /**
     * @brief Обработать клиента
     * @param clientSocket Сокет клиента
     */
    void handleClient(int clientSocket);
    
    /**
     * @brief Аутентифицировать клиента
     * @param clientSocket Сокет клиента
     * @param clientLogin Логин клиента (выходной параметр)
     * @return true - аутентификация успешна
     */
    bool authenticateClient(int clientSocket, std::string& clientLogin);
    
    /**
     * @brief Обработать векторные данные
     * @param clientSocket Сокет клиента
     */
    void processVectorData(int clientSocket);
    
    /**
     * @brief Отправить строку клиенту
     * @param socket Сокет
     * @param str Строка
     * @return true - успешно
     */
    bool sendString(int socket, const std::string& str);
    
    /**
     * @brief Получить строку от клиента
     * @param socket Сокет
     * @param str Строка (выходной параметр)
     * @param maxLength Максимальная длина
     * @return true - успешно
     */
    bool recvString(int socket, std::string& str, size_t maxLength = 1024);
    
    /**
     * @brief Отправить двоичные данные
     * @param socket Сокет
     * @param data Данные
     * @param size Размер данных
     * @return true - успешно
     */
    bool sendAll(int socket, const void* data, size_t size);
    
    /**
     * @brief Получить двоичные данные
     * @param socket Сокет
     * @param data Данные (выходной параметр)
     * @param size Размер данных
     * @return true - успешно
     */
    bool recvAll(int socket, void* data, size_t size);
    
    /**
     * @brief Закрыть соединение
     * @param socket Сокет
     */
    void closeConnection(int socket);
};

#endif // SERVER_H
