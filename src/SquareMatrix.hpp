#pragma once
#include "Matrix.hpp"

template<typename T>
class SquareMatrix {
public:
    SquareMatrix(size_t n,T init):matrix(n,n,init),N(n) {}
    ~SquareMatrix() = default;
    T& at(size_t r, size_t c) { return matrix.at(r, c); }
    const T& at(size_t r, size_t c) const { return matrix.at(r, c); }
    size_t size() const { return N; }
private:
    Matrix<T> matrix;
    size_t N;
};
