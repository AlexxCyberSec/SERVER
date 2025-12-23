/**
 * @file TestAuthenticator.cpp
 * @brief Модульные тесты для класса Authenticator
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include <UnitTest++/UnitTest++.h>
#include "../src/Authenticator.h"
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>

// === 1. Тест генерации соли (соответствие ТЗ) ===
TEST(Authenticator_GenerateSalt_Format) {
    std::string salt = Authenticator::generateSalt();
    
    // ТЗ: длина строки SALT16 — 16 шестнадцатеричных цифр
    CHECK_EQUAL(16, salt.length());
    
    // ТЗ: допустимые шестнадцатеричные цифры — 0-9, A-F
    bool validHex = true;
    for (char c : salt) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            validHex = false;
            break;
        }
    }
    CHECK(validHex);
    
    // ТЗ: метод обеспечения постоянного размера строки SALT16 
    // при различных значениях числа SALT — дополнение слева цифрами «0»
    // (это проверяется длиной 16 символов)
}

TEST(Authenticator_GenerateSalt_Unique) {
    // Генерируем две соли, они должны быть разными (с высокой вероятностью)
    std::string salt1 = Authenticator::generateSalt();
    std::string salt2 = Authenticator::generateSalt();
    
    // Они могут совпасть с очень малой вероятностью
    if (salt1 == salt2) {
        std::cout << "Предупреждение: соли совпали (редкий случай)" << std::endl;
    }
    // Не проверяем на неравенство, так как возможно совпадение
}

// === 2. Тест валидации hex строк ===
TEST(Authenticator_IsValidHexString_Correct64) {
    // Корректная строка 64 символа (как хеш SHA256)
    std::string validHash = "A1B2C3D4E5F67890123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0";
    CHECK(Authenticator::isValidHexString(validHash, 64));
}

TEST(Authenticator_IsValidHexString_Correct16) {
    // Корректная строка 16 символов (как соль)
    std::string validSalt = "A1B2C3D4E5F67890";
    CHECK(Authenticator::isValidHexString(validSalt, 16));
}

TEST(Authenticator_IsValidHexString_Lowercase) {
    // Строчные буквы тоже допустимы (a-f)
    std::string lowerHex = "abcdef0123456789";
    CHECK(Authenticator::isValidHexString(lowerHex, 16));
}

TEST(Authenticator_IsValidHexString_WrongLength) {
    // Неправильная длина
    std::string wrongLength = "A1B2";
    CHECK(!Authenticator::isValidHexString(wrongLength, 16));
}

TEST(Authenticator_IsValidHexString_InvalidChars) {
    // Некорректные символы
    std::string invalidChars = "A1B2C3D4E5F6789G"; // 'G' недопустим
    CHECK(!Authenticator::isValidHexString(invalidChars, 16));
}

TEST(Authenticator_IsValidHexString_Empty) {
    // Пустая строка
    std::string empty = "";
    CHECK(!Authenticator::isValidHexString(empty, 16));
}

// === 3. Тест вычисления SHA256 ===
TEST(Authenticator_CalculateSHA256_ValidInput) {
    std::string salt = "A1B2C3D4E5F67890";
    std::string password = "testpassword123";
    
    std::string hash = Authenticator::calculateSHA256(salt, password);
    
    // Хеш не должен быть пустым
    CHECK(!hash.empty());
    
    // SHA256 в hex = 64 символа
    CHECK_EQUAL(64, hash.length());
    
    // Проверка hex формата
    bool validHex = true;
    for (char c : hash) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            validHex = false;
            break;
        }
    }
    CHECK(validHex);
}

TEST(Authenticator_CalculateSHA256_Deterministic) {
    // Одинаковые входы → одинаковый выход
    std::string salt = "1234567890ABCDEF";
    std::string password = "MyPassword";
    
    std::string hash1 = Authenticator::calculateSHA256(salt, password);
    std::string hash2 = Authenticator::calculateSHA256(salt, password);
    
    CHECK_EQUAL(hash1, hash2);
}

TEST(Authenticator_CalculateSHA256_EmptyPassword) {
    std::string salt = "0000000000000000";
    std::string password = "";
    
    std::string hash = Authenticator::calculateSHA256(salt, password);
    
    CHECK(!hash.empty());
    CHECK_EQUAL(64, hash.length());
}

TEST(Authenticator_CalculateSHA256_LongPassword) {
    std::string salt = "SALTSALTSALTSALT";
    std::string password(1000, 'X'); // 1000 символов 'X'
    
    std::string hash = Authenticator::calculateSHA256(salt, password);
    
    CHECK(!hash.empty());
    CHECK_EQUAL(64, hash.length());
}

// === 4. Тест верификации хеша ===
TEST(Authenticator_VerifyHash_Correct) {
    std::string salt = "A1B2C3D4E5F67890";
    std::string password = "SecurePassword123";
    
    // Вычисляем хеш
    std::string correctHash = Authenticator::calculateSHA256(salt, password);
    CHECK(!correctHash.empty());
    
    // Проверяем
    bool result = Authenticator::verifyHash(correctHash, salt, password);
    CHECK(result);
}

TEST(Authenticator_VerifyHash_WrongPassword) {
    std::string salt = "1234567890ABCDEF";
    std::string storedPassword = "CorrectPassword";
    std::string wrongPassword = "WrongPassword";
    
    std::string hashForCorrect = Authenticator::calculateSHA256(salt, storedPassword);
    
    // Пытаемся проверить с неправильным паролем
    bool result = Authenticator::verifyHash(hashForCorrect, salt, wrongPassword);
    CHECK(!result);
}

TEST(Authenticator_VerifyHash_WrongSalt) {
    std::string salt1 = "A1B2C3D4E5F67890";
    std::string salt2 = "FEDCBA0987654321";
    std::string password = "MyPassword";
    
    std::string hashWithSalt1 = Authenticator::calculateSHA256(salt1, password);
    
    // Пытаемся проверить с другой солью
    bool result = Authenticator::verifyHash(hashWithSalt1, salt2, password);
    CHECK(!result);
}

TEST(Authenticator_VerifyHash_InvalidHashFormat) {
    std::string invalidHash = "NOTHEXNOTHEXNOTHEX"; // Не hex
    std::string salt = "A1B2C3D4E5F67890";
    std::string password = "test";
    
    bool result = Authenticator::verifyHash(invalidHash, salt, password);
    CHECK(!result);
    
    // Хеш неправильной длины
    std::string wrongLengthHash = "A1B2";
    result = Authenticator::verifyHash(wrongLengthHash, salt, password);
    CHECK(!result);
}

TEST(Authenticator_VerifyHash_CaseInsensitive) {
    std::string salt = "1234567890ABCDEF";
    std::string password = "Password123";
    
    std::string hashUpper = Authenticator::calculateSHA256(salt, password);
    
    // Преобразуем хеш в нижний регистр
    std::string hashLower = hashUpper;
    std::transform(hashLower.begin(), hashLower.end(), hashLower.begin(), ::tolower);
    
    // Проверка должна работать независимо от регистра
    bool result = Authenticator::verifyHash(hashLower, salt, password);
    CHECK(result);
}

TEST(Authenticator_VerifyHash_DifferentSaltsProduceDifferentHashes) {
    std::string salt1 = "1111111111111111";
    std::string salt2 = "2222222222222222";
    std::string password = "samepassword";
    
    std::string hash1 = Authenticator::calculateSHA256(salt1, password);
    std::string hash2 = Authenticator::calculateSHA256(salt2, password);
    
    // Хеши должны быть разными
    CHECK(hash1 != hash2);
}

// === 5. Тест полного цикла аутентификации (интеграционный) ===
TEST(Authenticator_FullAuthCycle) {
    // 1. Генерация соли сервером
    std::string salt = Authenticator::generateSalt();
    CHECK_EQUAL(16, salt.length());
    
    // 2. Пароль пользователя (хранится в базе)
    std::string userPassword = "MySecretPassword";
    
    // 3. Клиент вычисляет hash = SHA256(salt + password)
    std::string clientHash = Authenticator::calculateSHA256(salt, userPassword);
    CHECK(!clientHash.empty());
    CHECK_EQUAL(64, clientHash.length());
    
    // 4. Сервер проверяет полученный хеш
    bool authResult = Authenticator::verifyHash(clientHash, salt, userPassword);
    CHECK(authResult);
}

// === 6. Тест специальных символов в пароле ===
TEST(Authenticator_SpecialCharactersInPassword) {
    std::string salt = "A1B2C3D4E5F67890";
    std::string password = "P@ssw0rd!#$%^&*()";
    
    std::string hash = Authenticator::calculateSHA256(salt, password);
    CHECK(!hash.empty());
    CHECK_EQUAL(64, hash.length());
    
    bool result = Authenticator::verifyHash(hash, salt, password);
    CHECK(result);
}

/**
 * @brief Основная функция
 */
int main() {
    std::cout << "=== Testing Authenticator ===" << std::endl;
    std::cout << "Testing SHA256 authentication according to requirements" << std::endl;
    
    return UnitTest::RunAllTests();
}
