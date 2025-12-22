#include "Database.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

bool Database::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл базы данных: " 
                  << filename << std::endl;
        return false;
    }
    
    clients_.clear();
    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        lineNum++;
        
        // Пропуск пустых строк и комментариев
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            std::cerr << "Предупреждение: некорректный формат в строке " 
                      << lineNum << ": " << line << std::endl;
            continue;
        }
        
        std::string login = line.substr(0, colonPos);
        std::string password = line.substr(colonPos + 1);
        
        // Удаление пробелов
        login.erase(std::remove_if(login.begin(), login.end(), ::isspace), login.end());
        password.erase(std::remove_if(password.begin(), password.end(), ::isspace), 
                      password.end());
        
        if (login.empty() || password.empty()) {
            std::cerr << "Предупреждение: пустой логин или пароль в строке " 
                      << lineNum << std::endl;
            continue;
        }
        
        clients_[login] = password;
    }
    
    std::cout << "Загружено клиентов: " << clients_.size() << std::endl;
    
    // Вывод загруженных пользователей для отладки
    std::cout << "Загруженные пользователи:" << std::endl;
    for (const auto& client : clients_) {
        std::cout << "  " << client.first << " : " << client.second << std::endl;
    }
    
    return true;
}

bool Database::userExists(const std::string& login) const {
    return clients_.find(login) != clients_.end();
}

std::string Database::getPassword(const std::string& login) const {
    auto it = clients_.find(login);
    if (it != clients_.end()) {
        return it->second;
    }
    return "";
}
