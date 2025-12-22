#include "Authenticator.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <openssl/evp.h>

std::string Authenticator::generateSalt() {
    // Инициализация генератора случайных чисел
    static bool initialized = false;
    if (!initialized) {
        srand(time(nullptr));
        initialized = true;
    }
    
    uint64_t salt = 0;
    
    // Генерация 64-битной соли (8 байт)
    for (int i = 0; i < 8; i++) {
        salt = (salt << 8) | (rand() & 0xFF);
    }
    
    // Преобразование в hex строку с ведущими нулями
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << salt;
    
    std::string result = ss.str();
    // Приведение к верхнему регистру
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    
    return result;
}

std::string Authenticator::calculateSHA256(const std::string& salt, 
                                           const std::string& password) {
    std::string input = salt + password;
    
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr) {
        std::cerr << "Ошибка создания контекста SHA256" << std::endl;
        return "";
    }
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    
    // Инициализация контекста для SHA-256
    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        std::cerr << "Ошибка инициализации SHA256" << std::endl;
        EVP_MD_CTX_free(context);
        return "";
    }
    
    // Добавление данных для хеширования
    if (EVP_DigestUpdate(context, input.c_str(), input.length()) != 1) {
        std::cerr << "Ошибка обновления данных SHA256" << std::endl;
        EVP_MD_CTX_free(context);
        return "";
    }
    
    // Получение финального хеша
    if (EVP_DigestFinal_ex(context, hash, &hashLength) != 1) {
        std::cerr << "Ошибка получения финального хеша SHA256" << std::endl;
        EVP_MD_CTX_free(context);
        return "";
    }
    
    EVP_MD_CTX_free(context);
    
    // Преобразование в hex строку
    std::stringstream ss;
    for (unsigned int i = 0; i < hashLength; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    std::string result = ss.str();
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    
    return result;
}

bool Authenticator::verifyHash(const std::string& receivedHash,
                              const std::string& salt,
                              const std::string& storedPassword) {
    // Проверка формата полученного хеша
    if (!isValidHexString(receivedHash, 64)) {
        std::cerr << "Некорректный формат полученного хеша" << std::endl;
        return false;
    }
    
    // Вычисляем хеш от соли и пароля
    std::string calculatedHash = calculateSHA256(salt, storedPassword);
    
    if (calculatedHash.empty()) {
        std::cerr << "Ошибка вычисления хеша для проверки" << std::endl;
        return false;
    }
    
    // Сравниваем хеши
    std::string receivedUpper = receivedHash;
    std::string calculatedUpper = calculatedHash;
    std::transform(receivedUpper.begin(), receivedUpper.end(), receivedUpper.begin(), ::toupper);
    
    // calculatedHash уже в верхнем регистру
    
    if (receivedUpper != calculatedUpper) {
        std::cerr << "Хеши не совпадают:" << std::endl;
        std::cerr << "  Полученный: " << receivedUpper.substr(0, 16) << "..." << std::endl;
        std::cerr << "  Вычисленный: " << calculatedUpper.substr(0, 16) << "..." << std::endl;
        return false;
    }
    
    return true;
}

bool Authenticator::isValidHexString(const std::string& hexString, 
                                    size_t expectedLength) {
    if (hexString.length() != expectedLength) {
        std::cerr << "Некорректная длина hex строки: ожидалось " 
                  << expectedLength << ", получено " << hexString.length() << std::endl;
        return false;
    }
    
    for (char c : hexString) {
        if (!((c >= '0' && c <= '9') || 
              (c >= 'A' && c <= 'F') || 
              (c >= 'a' && c <= 'f'))) {
            std::cerr << "Некорректный символ в hex строке: " << c << std::endl;
            return false;
        }
    }
    
    return true;
}
