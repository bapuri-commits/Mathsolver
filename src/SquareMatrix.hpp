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


/* =================================================================
 * [참고용] SquareMatrix 완성 버전
 * - Matrix<T>를 내부에 들고 정사각행렬 전용 연산 제공
 * - identity, initializer_list 생성자
 * - 사칙연산, ==, !=, operator[]
 * - trace, subMatrix, det (여인수 전개), cofactor, adjugate, inverse, pow
 * - toMatrix()로 내부 Matrix<T> 꺼내기
 * =================================================================
 *
 * #pragma once
 * #include "Matrix.hpp"
 * #include <initializer_list>
 *
 * template<typename T>
 * class SquareMatrix {
 * public:
 *     SquareMatrix(size_t n, T init = T{}) : matrix(n, n, init), N(n) {}
 *
 *     SquareMatrix(std::initializer_list<std::initializer_list<T>> init)
 *         : matrix(init)
 *     {
 *         if (matrix.getRow() != matrix.getCol())
 *             throw std::invalid_argument("SquareMatrix requires row == col");
 *         N = matrix.getRow();
 *     }
 *
 *     explicit SquareMatrix(const Matrix<T>& m) : matrix(m) {
 *         if (m.getRow() != m.getCol())
 *             throw std::invalid_argument("SquareMatrix requires row == col");
 *         N = m.getRow();
 *     }
 *
 *     ~SquareMatrix() = default;
 *
 *     // --- Access ---
 *     T& at(size_t r, size_t c) { return matrix.at(r, c); }
 *     const T& at(size_t r, size_t c) const { return matrix.at(r, c); }
 *
 *     std::vector<T>& operator[](size_t r) { return matrix[r]; }
 *     const std::vector<T>& operator[](size_t r) const { return matrix[r]; }
 *
 *     size_t size() const { return N; }
 *     const Matrix<T>& toMatrix() const { return matrix; }
 *
 *     // --- Static Factory ---
 *     static SquareMatrix<T> identity(size_t n) {
 *         SquareMatrix<T> I(n);
 *         for (size_t i = 0; i < n; i++)
 *             I.matrix.at(i, i) = T{1};
 *         return I;
 *     }
 *
 *     // --- Arithmetic ---
 *     SquareMatrix<T> operator+(const SquareMatrix<T>& other) const {
 *         return SquareMatrix<T>(matrix + other.matrix);
 *     }
 *
 *     SquareMatrix<T> operator-(const SquareMatrix<T>& other) const {
 *         return SquareMatrix<T>(matrix - other.matrix);
 *     }
 *
 *     SquareMatrix<T> operator*(T scalar) const {
 *         return SquareMatrix<T>(matrix * scalar);
 *     }
 *
 *     friend SquareMatrix<T> operator*(T scalar, const SquareMatrix<T>& m) {
 *         return m * scalar;
 *     }
 *
 *     SquareMatrix<T> operator*(const SquareMatrix<T>& other) const {
 *         return SquareMatrix<T>(matrix * other.matrix);
 *     }
 *
 *     bool operator==(const SquareMatrix<T>& other) const {
 *         return matrix == other.matrix;
 *     }
 *
 *     bool operator!=(const SquareMatrix<T>& other) const {
 *         return !(matrix == other.matrix);
 *     }
 *
 *     // --- General ---
 *     SquareMatrix<T> transpose() const {
 *         return SquareMatrix<T>(matrix.transpose());
 *     }
 *
 *     // --- Square Matrix Operations ---
 *     T trace() const {
 *         T sum{};
 *         for (size_t i = 0; i < N; i++)
 *             sum += matrix.at(i, i);
 *         return sum;
 *     }
 *
 *     SquareMatrix<T> subMatrix(size_t excludeRow, size_t excludeCol) const {
 *         if (N <= 1)
 *             throw std::invalid_argument("Cannot create submatrix of 1x1 or smaller");
 *         SquareMatrix<T> result(N - 1);
 *         size_t ri = 0;
 *         for (size_t i = 0; i < N; i++) {
 *             if (i == excludeRow) continue;
 *             size_t ci = 0;
 *             for (size_t j = 0; j < N; j++) {
 *                 if (j == excludeCol) continue;
 *                 result.at(ri, ci) = matrix.at(i, j);
 *                 ci++;
 *             }
 *             ri++;
 *         }
 *         return result;
 *     }
 *
 *     T det() const {
 *         if (N == 1) return matrix.at(0, 0);
 *         if (N == 2) return matrix.at(0,0) * matrix.at(1,1)
 *                          - matrix.at(0,1) * matrix.at(1,0);
 *         T result{};
 *         T sign{1};
 *         for (size_t j = 0; j < N; j++) {
 *             result += sign * matrix.at(0, j) * subMatrix(0, j).det();
 *             sign = sign * T{-1};
 *         }
 *         return result;
 *     }
 *
 *     T cofactor(size_t r, size_t c) const {
 *         T sign = ((r + c) % 2 == 0) ? T{1} : T{-1};
 *         return sign * subMatrix(r, c).det();
 *     }
 *
 *     SquareMatrix<T> adjugate() const {
 *         SquareMatrix<T> result(N);
 *         for (size_t i = 0; i < N; i++)
 *             for (size_t j = 0; j < N; j++)
 *                 result.at(j, i) = cofactor(i, j);
 *         return result;
 *     }
 *
 *     SquareMatrix<T> inverse() const {
 *         T d = det();
 *         if (d == T{})
 *             throw std::runtime_error("det = 0 → inverse does not exist");
 *         return adjugate() * (T{1} / d);
 *     }
 *
 *     SquareMatrix<T> pow(int n) const {
 *         if (n == 0) return identity(N);
 *         if (n < 0) return inverse().pow(-n);
 *         if (n == 1) return *this;
 *
 *         SquareMatrix<T> half = pow(n / 2);
 *         SquareMatrix<T> result = half * half;
 *         if (n % 2 == 1) result = result * (*this);
 *         return result;
 *     }
 *
 * private:
 *     Matrix<T> matrix;
 *     size_t N;
 * };
 */
