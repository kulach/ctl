#pragma once

#include <vec.h>

template <typename T>
class Matrix : public Vec<T> {
    enum MatrixOrdering {
        RowMaj, // Default
        ColMaj 
    };
    public:
    Matrix (size_t m, size_t n) {
        
    }
}
