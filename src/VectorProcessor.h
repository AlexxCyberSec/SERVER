/**
 * @file VectorProcessor.h
 * @brief Обработка векторных данных
 * @author Судариков А.В.
 * @version 1.0
 * @date 2025
 */

#ifndef VECTORPROCESSOR_H
#define VECTORPROCESSOR_H

#include <cstdint>
#include <vector>
#include <climits>

/**
 * @brief Класс обработки векторов
 */
class VectorProcessor {
public:
    /**
     * @brief Вычислить сумму вектора
     * @param vector Вектор для обработки
     * @return Сумма элементов
     */
    static int32_t calculateSum(const std::vector<int32_t>& vector);
    
    /**
     * @brief Обработать массив векторов
     * @param vectors Массив векторов
     * @return Массив сумм
     */
    static std::vector<int32_t> processVectors(
        const std::vector<std::vector<int32_t>>& vectors);
    
private:
    static const int32_t MAX_INT32 = INT_MAX;      // 2^31-1
    static const int32_t MIN_INT32 = INT_MIN;      // -2^31
    
    /**
     * @brief Проверить переполнение
     * @param a Первое слагаемое
     * @param b Второе слагаемое
     * @return true - будет переполнение
     */
    static bool willOverflowAdd(int32_t a, int32_t b);
};

#endif // VECTORPROCESSOR_H
