/**
 * @file Database.h
 * @brief База данных клиентов
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unordered_map>

/**
 * @brief Класс базы данных клиентов
 */
class Database {
private:
    std::unordered_map<std::string, std::string> clients_; // login -> password (open text)
    
public:
    /**
     * @brief Загрузить базу из файла
     * @param filename Имя файла
     * @return true - успешно
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief Проверить существование пользователя
     * @param login Логин
     * @return true - существует
     */
    bool userExists(const std::string& login) const;
    
    /**
     * @brief Получить пароль пользователя
     * @param login Логин
     * @return Пароль в открытом виде
     */
    std::string getPassword(const std::string& login) const;
    
    /**
     * @brief Получить количество клиентов
     * @return Количество клиентов
     */
    size_t getClientCount() const { return clients_.size(); }
};

#endif // DATABASE_H
