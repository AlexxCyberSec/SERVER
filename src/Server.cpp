#include "Server.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <arpa/inet.h>

// ========== ФУНКЦИИ ДЛЯ РАБОТЫ С LITTLE-ENDIAN ==========

/**
 * @brief Конвертировать из little-endian в хостовый порядок
 */
static uint32_t le32_to_host(uint32_t value) {
    // Для x86/x64 систем (little-endian) ничего не делаем
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return value;
    #else
        // Для big-endian систем конвертируем
        return ((value & 0xFF) << 24) |
               ((value & 0xFF00) << 8) |
               ((value & 0xFF0000) >> 8) |
               ((value >> 24) & 0xFF);
    #endif
}

/**
 * @brief Конвертировать хостовый порядок в little-endian
 */
static uint32_t host_to_le32(uint32_t value) {
    // Для x86/x64 систем (little-endian) ничего не делаем
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return value;
    #else
        // Для big-endian систем конвертируем
        return ((value & 0xFF) << 24) |
               ((value & 0xFF00) << 8) |
               ((value & 0xFF0000) >> 8) |
               ((value >> 24) & 0xFF);
    #endif
}

// Версии для int32_t
static int32_t le32_to_host_int(int32_t value) {
    return static_cast<int32_t>(le32_to_host(static_cast<uint32_t>(value)));
}

static int32_t host_to_le32_int(int32_t value) {
    return static_cast<int32_t>(host_to_le32(static_cast<uint32_t>(value)));
}

Server::Server(const Config& config) 
    : config_(config), 
      logger_(config.getLogFilePath()), 
      serverSocket_(-1), 
      running_(false) {
    
    std::cout << "Сервер инициализирован" << std::endl;
    std::cout << "  База клиентов: " << config_.getClientDbPath() << std::endl;
    std::cout << "  Файл журнала:  " << config_.getLogFilePath() << std::endl;
    std::cout << "  Порт:          " << config_.getPort() << std::endl;
}

Server::~Server() {
    stop();
}

bool Server::start() {
    if (running_) {
        logger_.log(LogLevel::WARNING, "Сервер уже запущен");
        return false;
    }
    
    // Загрузка базы клиентов
    if (!database_.loadFromFile(config_.getClientDbPath())) {
        logger_.log(LogLevel::ERROR, "Не удалось загрузить базу клиентов", 
                   config_.getClientDbPath());
        return false;
    }
    
    logger_.log(LogLevel::INFO, "База клиентов загружена", 
               "клиентов: " + std::to_string(database_.getClientCount()));
    
    // Инициализация сети
    if (!initializeNetwork()) {
        logger_.log(LogLevel::CRITICAL, "Не удалось инициализировать сеть");
        return false;
    }
    
    running_ = true;
    logger_.log(LogLevel::INFO, "Сервер запущен", 
               "порт: " + std::to_string(config_.getPort()));
    
    mainLoop();
    
    return true;
}

void Server::stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
    
    logger_.log(LogLevel::INFO, "Сервер остановлен");
}

bool Server::initializeNetwork() {
    // Создание сокета
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        logger_.logSystemError("Ошибка создания сокета");
        return false;
    }
    
    // Настройка опций сокета
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logger_.logSystemError("Ошибка настройки сокета");
        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }
    
    // Установка таймаута (3 минуты)
    struct timeval timeout;
    timeout.tv_sec = 180;
    timeout.tv_usec = 0;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        logger_.logSystemError("Ошибка установки таймаута");
    }
    
    // Привязка сокета
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(config_.getPort());
    
    if (bind(serverSocket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        logger_.logSystemError("Ошибка привязки сокета");
        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }
    
    // Начало прослушивания
    if (listen(serverSocket_, 10) < 0) {
        logger_.logSystemError("Ошибка начала прослушивания");
        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }
    
    return true;
}

void Server::mainLoop() {
    std::cout << "Сервер запущен. Ожидание подключений..." << std::endl;
    
    while (running_) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket_, 
                                 (struct sockaddr*)&clientAddr, 
                                 &clientLen);
        
        if (clientSocket < 0) {
            if (errno == EINTR) continue;
            if (running_) { // Только если сервер еще работает
                logger_.logSystemError("Ошибка принятия соединения");
            }
            continue;
        }
        
        // Получение IP клиента
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        
        logger_.log(LogLevel::INFO, "Новое подключение", clientIP);
        
        // Обработка клиента
        handleClient(clientSocket);
    }
}

bool Server::sendString(int socket, const std::string& str) {
    return sendAll(socket, str.c_str(), str.length() + 1); // +1 для нуль-терминатора
}

bool Server::recvString(int socket, std::string& str, size_t maxLength) {
    char buffer[1024];
    str.clear();
    
    // Вариант 1: сначала пробуем прочитать всю строку с MSG_PEEK
    ssize_t peeked = recv(socket, buffer, std::min(sizeof(buffer), maxLength), MSG_PEEK);
    if (peeked <= 0) {
        return false;
    }
    
    // Ищем нуль-терминатор в заглянутых данных
    for (ssize_t i = 0; i < peeked; i++) {
        if (buffer[i] == '\0') {
            // Нашли нуль-терминатор - читаем всю строку включая его
            ssize_t toRead = i + 1; // включая \0
            ssize_t received = recv(socket, buffer, toRead, 0);
            if (received != toRead) {
                return false;
            }
            str.assign(buffer, i); // без нуль-терминатора
            return true;
        }
    }
    
    // Если не нашли нуль-терминатор, может клиент не отправляет его
    // Попробуем прочитать строку до пробела или перевода строки
    for (ssize_t i = 0; i < peeked; i++) {
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r') {
            // Читаем до разделителя
            ssize_t received = recv(socket, buffer, i, 0);
            if (received != i) {
                return false;
            }
            str.assign(buffer, i);
            return true;
        }
    }
    
    // Если не нашли разделитель, читаем все что есть
    ssize_t received = recv(socket, buffer, peeked, 0);
    if (received != peeked) {
        return false;
    }
    str.assign(buffer, received);
    
    // Обрезаем пробелы и управляющие символы
    size_t last = str.find_last_not_of(" \t\n\r");
    if (last != std::string::npos) {
        str = str.substr(0, last + 1);
    }
    
    return true;
}

bool Server::sendAll(int socket, const void* data, size_t size) {
    const char* ptr = static_cast<const char*>(data);
    size_t totalSent = 0;
    
    while (totalSent < size) {
        ssize_t sent = send(socket, ptr + totalSent, size - totalSent, 0);
        if (sent <= 0) {
            if (errno == EINTR) continue;
            return false;
        }
        totalSent += sent;
    }
    return true;
}

bool Server::recvAll(int socket, void* data, size_t size) {
    char* ptr = static_cast<char*>(data);
    size_t totalReceived = 0;
    
    while (totalReceived < size) {
        ssize_t received = recv(socket, ptr + totalReceived, size - totalReceived, 0);
        if (received <= 0) {
            return false;
        }
        totalReceived += received;
    }
    
    return true;
}

bool Server::authenticateClient(int clientSocket, std::string& clientLogin) {
    // Шаг 2: Получение логина
    std::string login;
    if (!recvString(clientSocket, login, 32)) {
        logger_.log(LogLevel::ERROR, "Ошибка получения логина");
        return false;
    }
    
    // Отладочный вывод
    std::cout << "DEBUG: Получен логин: '" << login << "' (длина: " << login.length() << ")" << std::endl;
    logger_.log(LogLevel::INFO, "Получен логин", login);
    
    // Шаг 3: Проверка идентификации
    if (!database_.userExists(login)) {
        // Отправляем "ERR" с нуль-терминатором
        std::string err_msg = "ERR";
        if (!sendString(clientSocket, err_msg)) {
            logger_.log(LogLevel::ERROR, "Ошибка отправки ERR", login);
        }
        logger_.log(LogLevel::WARNING, "Неизвестный пользователь", login);
        return false;
    }
    
    // Шаг 3a: Отправка соли (16 hex символов без нуль-терминатора)
    std::string salt = Authenticator::generateSalt();
    logger_.log(LogLevel::INFO, "Сгенерирована соль", salt);
    
    // Отправляем соль без нуль-терминатора (как ожидает клиент)
    if (!sendAll(clientSocket, salt.c_str(), salt.length())) {
        logger_.log(LogLevel::ERROR, "Ошибка отправки соли", login);
        return false;
    }
    
    // Шаг 4: Получение хеша пароля
    std::string passwordHash;
    if (!recvString(clientSocket, passwordHash, 64)) {
        logger_.log(LogLevel::ERROR, "Ошибка получения хеша пароля", login);
        return false;
    }
    
    logger_.log(LogLevel::INFO, "Получен хеш пароля", passwordHash.substr(0, 16) + "...");
    
    // Шаг 5: Проверка аутентификации
    std::string storedPassword = database_.getPassword(login);
    if (!Authenticator::verifyHash(passwordHash, salt, storedPassword)) {
        // Отправляем "ERR" с нуль-терминатором
        std::string err_msg = "ERR";
        if (!sendString(clientSocket, err_msg)) {
            logger_.log(LogLevel::ERROR, "Ошибка отправки ERR при аутентификации", login);
        }
        logger_.log(LogLevel::WARNING, "Ошибка аутентификации", login);
        return false;
    }
    
    // Шаг 5a: Успешная аутентификация
    // Отправляем "OK" с нуль-терминатором
    std::string ok_msg = "OK";
    if (!sendString(clientSocket, ok_msg)) {
        logger_.log(LogLevel::ERROR, "Ошибка отправки OK", login);
        return false;
    }
    
    clientLogin = login;
    logger_.log(LogLevel::INFO, "Клиент аутентифицирован", login);
    return true;
}

void Server::processVectorData(int clientSocket) {
    // Шаг 6: Получение количества векторов (4 байта, uint32_t)
    uint32_t numVectors;
    if (!recvAll(clientSocket, &numVectors, sizeof(numVectors))) {
        logger_.log(LogLevel::ERROR, "Ошибка получения количества векторов");
        return;
    }
    
    // КОНВЕРТИРУЕМ ИЗ LITTLE-ENDIAN (клиент отправляет в little-endian!)
    numVectors = le32_to_host(numVectors);
    
    std::cout << "DEBUG: Получено количество векторов (после конвертации): " << numVectors << std::endl;
    
    logger_.log(LogLevel::INFO, "Получено количество векторов", 
               std::to_string(numVectors));
    
    if (numVectors == 0 || numVectors > 100) {
        logger_.log(LogLevel::ERROR, "Некорректное количество векторов", 
                   std::to_string(numVectors));
        return;
    }
    
    // Шаги 7-10: Обработка каждого вектора
    for (uint32_t i = 0; i < numVectors; i++) {
        // Шаг 7: Получение размера вектора (4 байта, uint32_t)
        uint32_t vectorSize;
        if (!recvAll(clientSocket, &vectorSize, sizeof(vectorSize))) {
            logger_.log(LogLevel::ERROR, "Ошибка получения размера вектора " + std::to_string(i+1));
            return;
        }
        
        // КОНВЕРТИРУЕМ ИЗ LITTLE-ENDIAN
        vectorSize = le32_to_host(vectorSize);
        
        logger_.log(LogLevel::INFO, "Размер вектора " + std::to_string(i+1), 
                   std::to_string(vectorSize));
        
        if (vectorSize == 0 || vectorSize > 1000) {
            logger_.log(LogLevel::ERROR, "Некорректный размер вектора", 
                       std::to_string(vectorSize));
            return;
        }
        
        // Шаг 8: Получение всех значений вектора одним блоком
        std::vector<int32_t> vector(vectorSize);
        if (!recvAll(clientSocket, vector.data(), vectorSize * sizeof(int32_t))) {
            logger_.log(LogLevel::ERROR, "Ошибка получения данных вектора " + std::to_string(i+1));
            return;
        }
        
        // КОНВЕРТИРУЕМ КАЖДОЕ ЗНАЧЕНИЕ ИЗ LITTLE-ENDIAN
        for (auto& value : vector) {
            value = le32_to_host_int(value);
        }
        
        // Логирование для отладки
        if (vectorSize > 0) {
            std::cout << "DEBUG: Вектор " << (i+1) << " значения[0]=" << vector[0];
            if (vectorSize > 1) std::cout << " [1]=" << vector[1];
            if (vectorSize > 2) std::cout << " [last]=" << vector.back();
            std::cout << std::endl;
        }
        
        // Шаг 9: Вычисление и возврат результата по вектору
        int32_t result = VectorProcessor::calculateSum(vector);
        std::cout << "DEBUG: Сумма вектора " << (i+1) << " = " << result << std::endl;
        
        // КОНВЕРТИРУЕМ В LITTLE-ENDIAN ДЛЯ ОТПРАВКИ
        int32_t resultLE = host_to_le32_int(result);
        
        // Отправка результата (4 байта, int32_t)
        if (!sendAll(clientSocket, &resultLE, sizeof(resultLE))) {
            logger_.log(LogLevel::ERROR, "Ошибка отправки результата вектора " + std::to_string(i+1));
            return;
        }
        
        logger_.log(LogLevel::INFO, "Отправлен результат вектора " + std::to_string(i+1), 
                   std::to_string(result));
    }
    
    logger_.log(LogLevel::INFO, "Все векторы обработаны", 
               "количество: " + std::to_string(numVectors));
}


void Server::handleClient(int clientSocket) {
    // Установка таймаута на чтение (5 секунд)
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        logger_.logSystemError("Ошибка установки таймаута на клиентский сокет");
    }
    
    std::string clientLogin;
    
    // Аутентификация клиента
    if (!authenticateClient(clientSocket, clientLogin)) {
        closeConnection(clientSocket);
        return;
    }
    
    // Обработка векторных данных
    processVectorData(clientSocket);
    
    // Закрытие соединения
    closeConnection(clientSocket);
    logger_.log(LogLevel::INFO, "Сеанс завершен", clientLogin);
}
void Server::closeConnection(int socket) {
    if (socket >= 0) {
        // Завершаем передачу данных в обоих направлениях
        shutdown(socket, SHUT_RDWR);
        
        // Закрываем сокет
        close(socket);
        
        // Отладочный вывод
        std::cout << "DEBUG: Соединение закрыто (socket: " << socket << ")" << std::endl;
    }
}
