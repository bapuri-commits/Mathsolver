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
    Matrix<T> operator*(T scaler) const {
        Matrix<T> result(row, col);
        for (size_t i = 0; i < row; i++)
        {
            for (size_t j = 0; j < col; j++)
            {
                result.data[i][j] = this->data[i][j] * scaler;
            }
        }
        return result;
    }
    Matrix<T> operator+(const Matrix<T>& other) const {
        if (row != other.row || col != other.col)
            throw std::invalid_argument("Matrix sizes do not match for addition");
        Matrix<T> result(row, col);
        for (size_t i = 0; i < row; i++)
        {
            for (size_t j = 0; j < col; j++)
            {
                result.data[i][j] = this->data[i][j] + other.data[i][j];
            }
        }
        return result;
    }
    Matrix<T> operator-(const Matrix<T>& other) const {
        if (row != other.row || col != other.col)
            throw std::invalid_argument("Matrix sizes do not match for subtraction");
        Matrix<T> result(row, col);
        for (size_t i = 0; i < row; i++)
        {
            for (size_t j = 0; j < col; j++)
            {
                result.data[i][j] = this->data[i][j] - other.data[i][j];
            }
        }
        return result;
    }
    Matrix<T> operator*(const Matrix<T>& other) const {
        if (col != other.row) {
            throw std::invalid_argument("Matrix sizes do not match for product");
        }
        Matrix<T> result(row,other.col);
        for (size_t i = 0; i < row; i++){
            for (size_t j = 0; j < other.col; j++){
                T sum{};
                for (size_t k = 0; k < col; k++){
                    sum += this->data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }
    Matrix<T> transpose() const {
        Matrix<T> result(col, row);
        for (size_t i = 0; i < row; i++){
            for (size_t j = 0; j < col; j++){
                result.data[j][i] = this->data[i][j];
            }
        }
        return result;
    }


private:
    std::vector<std::vector<T>> data;
    size_t row;
    size_t col;
};
