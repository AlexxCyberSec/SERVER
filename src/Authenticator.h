/**
 * @file Authenticator.h
 * @brief Аутентификация клиентов
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>

/**
 * @brief Класс аутентификации
 */
class Authenticator {
public:
    /**
     * @brief Сгенерировать соль
     * @return Соль в hex формате (16 символов)
     */
    static std::string generateSalt();
    
    /**
     * @brief Вычислить SHA256 хеш от соли и пароля
     * @param salt Соль
     * @param password Пароль в открытом виде
     * @return Хеш в hex формате (64 символа)
     */
    static std::string calculateSHA256(const std::string& salt, 
                                       const std::string& password);
    
    /**
     * @brief Проверить хеш
     * @param receivedHash Хеш от клиента
     * @param salt Соль
     * @param storedPassword Пароль в открытом виде из базы
     * @return true - хеши совпадают
     */
    static bool verifyHash(const std::string& receivedHash,
                          const std::string& salt,
                          const std::string& storedPassword);
    
    /**
     * @brief Проверить hex строку
     * @param hexString Строка
     * @param expectedLength Ожидаемая длина
     * @return true - строка корректна
     */
    static bool isValidHexString(const std::string& hexString, 
                                size_t expectedLength);
};

#endif // AUTHENTICATOR_H
