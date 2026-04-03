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


/* =================================================================
 * [참고용] 정사각행렬 연산 포함 확장 버전
 * - initializer_list 생성자
 * - operator[], isSquare(), identity(), ==, !=
 * - friend operator*(scalar, matrix) — 좌측 스칼라 곱
 * - trace, subMatrix, det (여인수 전개), cofactor, adjugate, inverse, pow
 * =================================================================
 *
 * #pragma once
 * #include <vector>
 * #include <stdexcept>
 * #include <string>
 * #include <initializer_list>
 *
 * template<typename T>
 * class Matrix {
 * public:
 *     Matrix(size_t row, size_t col, T init = T{})
 *         : row(row), col(col), data(row, std::vector<T>(col, init)) {}
 *
 *     Matrix() : Matrix(1, 1) {}
 *
 *     Matrix(std::initializer_list<std::initializer_list<T>> init) {
 *         row = init.size();
 *         col = init.begin()->size();
 *         data.reserve(row);
 *         for (const auto& rowData : init) {
 *             if (rowData.size() != col)
 *                 throw std::invalid_argument("All rows must have the same number of columns");
 *             data.emplace_back(rowData);
 *         }
 *     }
 *
 *     ~Matrix() = default;
 *
 *     // --- Access ---
 *     T& at(size_t r, size_t c) {
 *         if (r >= row || c >= col)
 *             throw std::out_of_range("Index Over");
 *         return data[r][c];
 *     }
 *
 *     const T& at(size_t r, size_t c) const {
 *         if (r >= row || c >= col)
 *             throw std::out_of_range("Index Over");
 *         return data[r][c];
 *     }
 *
 *     std::vector<T>& operator[](size_t r) { return data[r]; }
 *     const std::vector<T>& operator[](size_t r) const { return data[r]; }
 *
 *     size_t getRow() const { return row; }
 *     size_t getCol() const { return col; }
 *     bool isSquare() const { return row == col; }
 *
 *     // --- Static Factory ---
 *     static Matrix<T> identity(size_t n) {
 *         Matrix<T> I(n, n);
 *         for (size_t i = 0; i < n; i++)
 *             I.data[i][i] = T{1};
 *         return I;
 *     }
 *
 *     // --- Arithmetic ---
 *     Matrix<T> operator+(const Matrix<T>& other) const {
 *         if (row != other.row || col != other.col)
 *             throw std::invalid_argument("Matrix sizes do not match for addition");
 *         Matrix<T> result(row, col);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 result.data[i][j] = data[i][j] + other.data[i][j];
 *         return result;
 *     }
 *
 *     Matrix<T> operator-(const Matrix<T>& other) const {
 *         if (row != other.row || col != other.col)
 *             throw std::invalid_argument("Matrix sizes do not match for subtraction");
 *         Matrix<T> result(row, col);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 result.data[i][j] = data[i][j] - other.data[i][j];
 *         return result;
 *     }
 *
 *     Matrix<T> operator*(T scalar) const {
 *         Matrix<T> result(row, col);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 result.data[i][j] = data[i][j] * scalar;
 *         return result;
 *     }
 *
 *     friend Matrix<T> operator*(T scalar, const Matrix<T>& m) {
 *         return m * scalar;
 *     }
 *
 *     Matrix<T> operator*(const Matrix<T>& other) const {
 *         if (col != other.row)
 *             throw std::invalid_argument("Matrix sizes do not match for product");
 *         Matrix<T> result(row, other.col);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < other.col; j++) {
 *                 T sum{};
 *                 for (size_t k = 0; k < col; k++)
 *                     sum += data[i][k] * other.data[k][j];
 *                 result.data[i][j] = sum;
 *             }
 *         return result;
 *     }
 *
 *     bool operator==(const Matrix<T>& other) const {
 *         if (row != other.row || col != other.col) return false;
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 if (data[i][j] != other.data[i][j]) return false;
 *         return true;
 *     }
 *
 *     bool operator!=(const Matrix<T>& other) const { return !(*this == other); }
 *
 *     // --- General ---
 *     Matrix<T> transpose() const {
 *         Matrix<T> result(col, row);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 result.data[j][i] = data[i][j];
 *         return result;
 *     }
 *
 *     // --- Square Matrix ---
 *     T trace() const {
 *         assertSquare("trace");
 *         T sum{};
 *         for (size_t i = 0; i < row; i++)
 *             sum += data[i][i];
 *         return sum;
 *     }
 *
 *     Matrix<T> subMatrix(size_t excludeRow, size_t excludeCol) const {
 *         if (row <= 1 || col <= 1)
 *             throw std::invalid_argument("Cannot create submatrix of 1x1 or smaller");
 *         Matrix<T> result(row - 1, col - 1);
 *         size_t ri = 0;
 *         for (size_t i = 0; i < row; i++) {
 *             if (i == excludeRow) continue;
 *             size_t ci = 0;
 *             for (size_t j = 0; j < col; j++) {
 *                 if (j == excludeCol) continue;
 *                 result.data[ri][ci] = data[i][j];
 *                 ci++;
 *             }
 *             ri++;
 *         }
 *         return result;
 *     }
 *
 *     T det() const {
 *         assertSquare("det");
 *         if (row == 1) return data[0][0];
 *         if (row == 2) return data[0][0] * data[1][1] - data[0][1] * data[1][0];
 *
 *         T result{};
 *         T sign{1};
 *         for (size_t j = 0; j < col; j++) {
 *             result += sign * data[0][j] * subMatrix(0, j).det();
 *             sign = sign * T{-1};
 *         }
 *         return result;
 *     }
 *
 *     T cofactor(size_t r, size_t c) const {
 *         assertSquare("cofactor");
 *         T sign = ((r + c) % 2 == 0) ? T{1} : T{-1};
 *         return sign * subMatrix(r, c).det();
 *     }
 *
 *     Matrix<T> adjugate() const {
 *         assertSquare("adjugate");
 *         Matrix<T> result(row, col);
 *         for (size_t i = 0; i < row; i++)
 *             for (size_t j = 0; j < col; j++)
 *                 result.data[j][i] = cofactor(i, j);
 *         return result;
 *     }
 *
 *     Matrix<T> inverse() const {
 *         assertSquare("inverse");
 *         T d = det();
 *         if (d == T{})
 *             throw std::runtime_error("det = 0 → inverse does not exist");
 *         return adjugate() * (T{1} / d);
 *     }
 *
 *     Matrix<T> pow(int n) const {
 *         assertSquare("pow");
 *         if (n == 0) return identity(row);
 *         if (n < 0) return inverse().pow(-n);
 *         if (n == 1) return *this;
 *
 *         Matrix<T> half = pow(n / 2);
 *         Matrix<T> result = half * half;
 *         if (n % 2 == 1) result = result * (*this);
 *         return result;
 *     }
 *
 * private:
 *     std::vector<std::vector<T>> data;
 *     size_t row;
 *     size_t col;
 *
 *     void assertSquare(const std::string& op) const {
 *         if (row != col)
 *             throw std::invalid_argument(op + " requires a square matrix");
 *     }
 * };
 */
