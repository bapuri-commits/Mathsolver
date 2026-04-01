#pragma once
#include <vector>
#include <stdexcept>
#include <string>

template<typename T>
class Matrix {
public:
    Matrix(size_t row, size_t col, T init = T{})
        : row(row), col(col), data(row, std::vector<T>(col, init)) {}

    Matrix() : Matrix(1, 1) {}

    ~Matrix() = default;

    T& at(size_t r, size_t c) {
        if (r >= row || c >= col)
            throw std::out_of_range("Index Over");
        return data[r][c];
    }

    const T& at(size_t r, size_t c) const {
        if (r >= row || c >= col)
            throw std::out_of_range("Index Over");
        return data[r][c];
    }

    size_t getRow() const { return row; }
    size_t getCol() const { return col; }

    /*Operation*/
    Matrix<T> operator+(const Matrix<T>& other) const {

    }
    Matrix<T> operator*(const [atrix<T>& other) const {}
    Matrix<T> transpose() const {}


private:
    std::vector<std::vector<T>> data;
    size_t row;
    size_t col;
};
