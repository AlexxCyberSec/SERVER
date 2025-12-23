/**
 * @file TestDatabase.cpp
 * @brief Модульные тесты для класса Database
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include <UnitTest++/UnitTest++.h>
#include "../src/Database.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

// === 1. Тест загрузки из файла ===
TEST(Database_LoadFromFile_ValidFile) {
    const std::string testFile = "test_database_valid.db";
    
    // Создаём тестовый файл
    std::ofstream file(testFile);
    file << "user1:password1\n";
    file << "user2:password2\n";
    file << "user3:password3\n";
    file.close();
    
    Database db;
    bool result = db.loadFromFile(testFile);
    
    CHECK(result);
    CHECK_EQUAL(3, db.getClientCount());
    
    // Очистка
    std::remove(testFile.c_str());
}

// === 2. Тест загрузки с комментариями и пустыми строками ===
TEST(Database_LoadFromFile_WithComments) {
    const std::string testFile = "test_database_comments.db";
    
    std::ofstream file(testFile);
    file << "# Это комментарий\n";
    file << "\n"; // Пустая строка
    file << "alice:alice123\n";
    file << "# Ещё комментарий\n";
    file << "bob:bob456\n";
    file << "\n";
    file << "charlie:charlie789\n";
    file.close();
    
    Database db;
    bool result = db.loadFromFile(testFile);
    
    CHECK(result);
    CHECK_EQUAL(3, db.getClientCount());
    
    std::remove(testFile.c_str());
}

// === 3. Тест загрузки с некорректными строками ===
TEST(Database_LoadFromFile_InvalidLines) {
    const std::string testFile = "test_database_invalid.db";
    
    std::ofstream file(testFile);
    file << "user1:password1\n";
    file << "user2password2\n"; // Нет двоеточия
    file << "user3:password3\n";
    file << ":password4\n"; // Пустой логин
    file << "user5:\n"; // Пустой пароль
    file << "  user6  :  password6  \n"; // С пробелами
    file.close();
    
    Database db;
    bool result = db.loadFromFile(testFile);
    
    // Должны загрузиться только корректные строки
    CHECK(result);
    CHECK_EQUAL(3, db.getClientCount()); // user1, user3, user6 (после trim)
    
    std::remove(testFile.c_str());
}

// === 4. Тест проверки существования пользователя ===
TEST(Database_UserExists) {
    const std::string testFile = "test_database_exists.db";
    
    std::ofstream file(testFile);
    file << "testuser:testpass\n";
    file << "admin:admin123\n";
    file.close();
    
    Database db;
    db.loadFromFile(testFile);
    
    CHECK(db.userExists("testuser"));
    CHECK(db.userExists("admin"));
    CHECK(!db.userExists("nonexistent"));
    CHECK(!db.userExists("")); // Пустой логин
    
    std::remove(testFile.c_str());
}

// === 5. Тест получения пароля ===
TEST(Database_GetPassword) {
    const std::string testFile = "test_database_password.db";
    
    std::ofstream file(testFile);
    file << "user1:pass1\n";
    file << "user2:pass2\n";
    file.close();
    
    Database db;
    db.loadFromFile(testFile);
    
    CHECK_EQUAL("pass1", db.getPassword("user1"));
    CHECK_EQUAL("pass2", db.getPassword("user2"));
    CHECK_EQUAL("", db.getPassword("user3")); // Несуществующий
    CHECK_EQUAL("", db.getPassword("")); // Пустой логин
    
    std::remove(testFile.c_str());
}

// === 6. Тест загрузки пустого файла ===
TEST(Database_LoadFromFile_EmptyFile) {
    const std::string testFile = "test_database_empty.db";
    
    // Создаём пустой файл
    std::ofstream file(testFile);
    file.close();
    
    Database db;
    bool result = db.loadFromFile(testFile);
    
    CHECK(result); // Пустой файл - не ошибка
    CHECK_EQUAL(0, db.getClientCount());
    
    std::remove(testFile.c_str());
}

// === 7. Тест несуществующего файла ===
TEST(Database_LoadFromFile_NonexistentFile) {
    const std::string testFile = "nonexistent_file_12345.db";
    
    Database db;
    bool result = db.loadFromFile(testFile);
    
    CHECK(!result); // Должно вернуть false
    CHECK_EQUAL(0, db.getClientCount());
}

// === 8. Тест повторной загрузки (очистка старой базы) ===
TEST(Database_LoadFromFile_Reload) {
    const std::string testFile1 = "test_database_reload1.db";
    const std::string testFile2 = "test_database_reload2.db";
    
    // Первый файл
    std::ofstream file1(testFile1);
    file1 << "user1:pass1\n";
    file1.close();
    
    // Второй файл
    std::ofstream file2(testFile2);
    file2 << "user2:pass2\n";
    file2 << "user3:pass3\n";
    file2.close();
    
    Database db;
    
    // Загружаем первый файл
    db.loadFromFile(testFile1);
    CHECK_EQUAL(1, db.getClientCount());
    CHECK(db.userExists("user1"));
    
    // Загружаем второй файл
    db.loadFromFile(testFile2);
    CHECK_EQUAL(2, db.getClientCount());
    CHECK(!db.userExists("user1")); // Должен быть очищен
    CHECK(db.userExists("user2"));
    CHECK(db.userExists("user3"));
    
    // Очистка
    std::remove(testFile1.c_str());
    std::remove(testFile2.c_str());
}

// === 9. Тест тримминга пробелов ===
TEST(Database_LoadFromFile_TrimSpaces) {
    const std::string testFile = "test_database_trim.db";
    
    std::ofstream file(testFile);
    file << "  user1  :  password1  \n"; // Пробелы вокруг
    file << "user2 :password2\n"; // Пробел перед двоеточием
    file << "user3: password3\n"; // Пробел после двоеточия
    file << "  user4  :  password4  \n";
    file.close();
    
    Database db;
    db.loadFromFile(testFile);
    
    CHECK(db.userExists("user1"));
    CHECK(db.userExists("user2"));
    CHECK(db.userExists("user3"));
    CHECK(db.userExists("user4"));
    
    CHECK_EQUAL("password1", db.getPassword("user1"));
    CHECK_EQUAL("password2", db.getPassword("user2"));
    CHECK_EQUAL("password3", db.getPassword("user3"));
    CHECK_EQUAL("password4", db.getPassword("user4"));
    
    std::remove(testFile.c_str());
}

// === 10. Тест учёта регистра в логинах ===
TEST(Database_CaseSensitive) {
    const std::string testFile = "test_database_case.db";
    
    std::ofstream file(testFile);
    file << "User:pass1\n";
    file << "user:pass2\n";
    file << "USER:pass3\n";
    file.close();
    
    Database db;
    db.loadFromFile(testFile);
    
    // Логины должны быть регистрозависимыми
    CHECK_EQUAL(3, db.getClientCount());
    CHECK(db.userExists("User"));
    CHECK(db.userExists("user"));
    CHECK(db.userExists("USER"));
    CHECK_EQUAL("pass1", db.getPassword("User"));
    CHECK_EQUAL("pass2", db.getPassword("user"));
    CHECK_EQUAL("pass3", db.getPassword("USER"));
    
    std::remove(testFile.c_str());
}

/**
 * @brief Основная функция
 */
int main() {
    std::cout << "=== Testing Database ===" << std::endl;
    
    // Очищаем тестовые файлы перед запуском
    system("rm -f test_database_*.db 2>/dev/null");
    
    return UnitTest::RunAllTests();
}
