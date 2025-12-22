#include "VectorProcessor.h"
#include <iostream>

int32_t VectorProcessor::calculateSum(const std::vector<int32_t>& vector) {
    int64_t sum = 0;
    
    for (int32_t value : vector) {
        // Проверка на переполнение при сложении
        if (willOverflowAdd(static_cast<int32_t>(sum), value)) {
            if (value > 0) {
                return MAX_INT32;  // Переполнение вверх
            } else {
                return MIN_INT32;  // Переполнение вниз
            }
        }
        
        sum += value;
        
        // Дополнительная проверка границ
        if (sum > MAX_INT32) {
            return MAX_INT32;
        }
        if (sum < MIN_INT32) {
            return MIN_INT32;
        }
    }
    
    return static_cast<int32_t>(sum);
}

std::vector<int32_t> VectorProcessor::processVectors(
    const std::vector<std::vector<int32_t>>& vectors) {
    
    std::vector<int32_t> results;
    results.reserve(vectors.size());
    
    for (const auto& vector : vectors) {
        results.push_back(calculateSum(vector));
    }
    
    return results;
}

bool VectorProcessor::willOverflowAdd(int32_t a, int32_t b) {
    if (b > 0) {
        return a > MAX_INT32 - b;  // a + b > MAX_INT32
    } else {
        return a < MIN_INT32 - b;  // a + b < MIN_INT32
    }
}
