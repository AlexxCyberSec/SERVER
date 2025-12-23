/**
 * @file TestVectorProcessor.cpp
 * @brief Модульные тесты для класса VectorProcessor
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#include "/usr/include/UnitTest++/UnitTest++.h"
#include "../src/VectorProcessor.h"
#include <iostream>
#include <vector>
#include <climits>
#include <cstdint>

// === 1. Базовые тесты суммы вектора ===
TEST(CalculateSum_EmptyVector) {
    std::vector<int32_t> empty;
    CHECK_EQUAL(0, VectorProcessor::calculateSum(empty));
}

TEST(CalculateSum_SingleElement) {
    std::vector<int32_t> vec = {5};
    CHECK_EQUAL(5, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_MultipleElements) {
    std::vector<int32_t> vec = {1, 2, 3, 4, 5};
    CHECK_EQUAL(15, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_NegativeNumbers) {
    std::vector<int32_t> vec = {-5, -10, -3};
    CHECK_EQUAL(-18, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_MixedSigns) {
    std::vector<int32_t> vec = {10, -3, 7, -2, 5};
    CHECK_EQUAL(17, VectorProcessor::calculateSum(vec));
}

// === 2. Граничные значения ===
TEST(CalculateSum_MaxInt32) {
    std::vector<int32_t> vec = {INT_MAX};
    CHECK_EQUAL(INT_MAX, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_MinInt32) {
    std::vector<int32_t> vec = {INT_MIN};
    CHECK_EQUAL(INT_MIN, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_MaxPlusZero) {
    std::vector<int32_t> vec = {INT_MAX, 0};
    CHECK_EQUAL(INT_MAX, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_NearMaxNoOverflow) {
    std::vector<int32_t> vec = {INT_MAX - 5, 4};
    CHECK_EQUAL(INT_MAX - 1, VectorProcessor::calculateSum(vec));
}

// === 3. Переполнение ===
TEST(CalculateSum_OverflowPositive) {
    std::vector<int32_t> vec = {INT_MAX, 1};
    CHECK_EQUAL(INT_MAX, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_OverflowNegative) {
    std::vector<int32_t> vec = {INT_MIN, -1};
    CHECK_EQUAL(INT_MIN, VectorProcessor::calculateSum(vec));
}

// === 4. Пример из курсовой ===
TEST(CalculateSum_CourseExample) {
    std::vector<int32_t> vec = {8000, 10000, 12000, 12000};
    CHECK_EQUAL(42000, VectorProcessor::calculateSum(vec));
}

TEST(CalculateSum_ZeroVector) {
    std::vector<int32_t> vec = {0, 0, 0, 0};
    CHECK_EQUAL(0, VectorProcessor::calculateSum(vec));
}

int main() {
    std::cout << "=== Тестирование VectorProcessor ===" << std::endl;
    return UnitTest::RunAllTests();
}
