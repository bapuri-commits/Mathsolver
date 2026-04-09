#pragma once
#include <vector>
#include <stdexcept>
#include <string>
#include <initializer_list>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <tuple>
#include <algorithm>

template<typename T>
class Matrix {
public:
    Matrix(size_t row, size_t col, T init = T{})
        : row_(row), col_(col), data_(row, std::vector<T>(col, init)) {}

    Matrix() : Matrix(1, 1) {}

    Matrix(std::initializer_list<std::initializer_list<T>> init) {
        if (init.size() == 0)
            throw std::invalid_argument("Cannot create matrix from empty initializer list");
        row_ = init.size();
        col_ = init.begin()->size();
        data_.reserve(row_);
        for (const auto& rowData : init) {
            if (rowData.size() != col_)
                throw std::invalid_argument("All rows must have the same number of columns");
            data_.emplace_back(rowData);
        }
    }

    ~Matrix() = default;

    // --- Access ---
    T& at(size_t r, size_t c) {
        if (r >= row_ || c >= col_)
            throw std::out_of_range("Index Over");
        return data_[r][c];
    }

    const T& at(size_t r, size_t c) const {
        if (r >= row_ || c >= col_)
            throw std::out_of_range("Index Over");
        return data_[r][c];
    }

    std::vector<T>& operator[](size_t r) { return data_[r]; }
    const std::vector<T>& operator[](size_t r) const { return data_[r]; }

    size_t getRow() const { return row_; }
    size_t getCol() const { return col_; }
    bool isSquare() const { return row_ == col_; }

    // --- Static Factory ---
    static Matrix<T> identity(size_t n) {
        Matrix<T> I(n, n);
        for (size_t i = 0; i < n; i++)
            I.data_[i][i] = T{1};
        return I;
    }

    // --- Arithmetic ---
    Matrix<T> operator+(const Matrix<T>& other) const {
        if (row_ != other.row_ || col_ != other.col_)
            throw std::invalid_argument("Matrix sizes do not match for addition");
        Matrix<T> result(row_, col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[i][j] = data_[i][j] + other.data_[i][j];
        return result;
    }

    Matrix<T> operator-(const Matrix<T>& other) const {
        if (row_ != other.row_ || col_ != other.col_)
            throw std::invalid_argument("Matrix sizes do not match for subtraction");
        Matrix<T> result(row_, col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[i][j] = data_[i][j] - other.data_[i][j];
        return result;
    }

    Matrix<T> operator*(T scalar) const {
        Matrix<T> result(row_, col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[i][j] = data_[i][j] * scalar;
        return result;
    }

    friend Matrix<T> operator*(T scalar, const Matrix<T>& m) {
        return m * scalar;
    }

    Matrix<T> operator*(const Matrix<T>& other) const {
        if (col_ != other.row_)
            throw std::invalid_argument("Matrix sizes do not match for product");
        Matrix<T> result(row_, other.col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < other.col_; j++) {
                T sum{};
                for (size_t k = 0; k < col_; k++)
                    sum += data_[i][k] * other.data_[k][j];
                result.data_[i][j] = sum;
            }
        return result;
    }

    bool operator==(const Matrix<T>& other) const {
        if (row_ != other.row_ || col_ != other.col_) return false;
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                if (data_[i][j] != other.data_[i][j]) return false;
        return true;
    }

    bool operator!=(const Matrix<T>& other) const { return !(*this == other); }

    // --- General ---
    Matrix<T> transpose() const {
        Matrix<T> result(col_, row_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[j][i] = data_[i][j];
        return result;
    }

    // --- Square Matrix ---
    T trace() const {
        assertSquare("trace");
        T sum{};
        for (size_t i = 0; i < row_; i++)
            sum += data_[i][i];
        return sum;
    }

    Matrix<T> subMatrix(size_t excludeRow, size_t excludeCol) const {
        if (row_ <= 1 || col_ <= 1)
            throw std::invalid_argument("Cannot create submatrix of 1x1 or smaller");
        Matrix<T> result(row_ - 1, col_ - 1);
        size_t ri = 0;
        for (size_t i = 0; i < row_; i++) {
            if (i == excludeRow) continue;
            size_t ci = 0;
            for (size_t j = 0; j < col_; j++) {
                if (j == excludeCol) continue;
                result.data_[ri][ci] = data_[i][j];
                ci++;
            }
            ri++;
        }
        return result;
    }

    T det() const {
        assertSquare("det");
        if (row_ == 1) return data_[0][0];
        if (row_ == 2) return data_[0][0] * data_[1][1] - data_[0][1] * data_[1][0];

        T result{};
        T sign{1};
        for (size_t j = 0; j < col_; j++) {
            result += sign * data_[0][j] * subMatrix(0, j).det();
            sign = sign * T{-1};
        }
        return result;
    }

    T cofactor(size_t r, size_t c) const {
        assertSquare("cofactor");
        T sign = ((r + c) % 2 == 0) ? T{1} : T{-1};
        return sign * subMatrix(r, c).det();
    }

    Matrix<T> cofactorMatrix() const {
        assertSquare("cofactorMatrix");
        Matrix<T> result(row_, col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[i][j] = cofactor(i, j);
        return result;
    }

    Matrix<T> adjugate() const {
        assertSquare("adjugate");
        Matrix<T> result(row_, col_);
        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                result.data_[j][i] = cofactor(i, j);
        return result;
    }

    Matrix<T> inverse() const {
        assertSquare("inverse");
        T d = det();
        if (std::abs(d) < T{1e-12})
            throw std::runtime_error("det = 0 -> inverse does not exist");
        return adjugate() * (T{1} / d);
    }

    Matrix<T> pow(int n) const {
        assertSquare("pow");
        if (n == 0) return identity(row_);
        if (n < 0) return inverse().pow(-n);
        if (n == 1) return *this;

        Matrix<T> half = pow(n / 2);
        Matrix<T> result = half * half;
        if (n % 2 == 1) result = result * (*this);
        return result;
    }

    // --- RREF (Gauss-Jordan with partial pivoting) ---
    Matrix<T> rref() const {
        Matrix<T> result = *this;
        size_t pivotRow = 0;

        for (size_t c = 0; c < col_ && pivotRow < row_; c++) {
            size_t maxRow = pivotRow;
            T maxVal = std::abs(result.data_[pivotRow][c]);
            for (size_t r = pivotRow + 1; r < row_; r++) {
                T val = std::abs(result.data_[r][c]);
                if (val > maxVal) {
                    maxVal = val;
                    maxRow = r;
                }
            }

            if (maxVal < T{1e-12}) continue;

            std::swap(result.data_[pivotRow], result.data_[maxRow]);

            T pivot = result.data_[pivotRow][c];
            for (size_t j = 0; j < col_; j++)
                result.data_[pivotRow][j] /= pivot;

            for (size_t r = 0; r < row_; r++) {
                if (r == pivotRow) continue;
                T factor = result.data_[r][c];
                if (std::abs(factor) < T{1e-12}) continue;
                for (size_t j = 0; j < col_; j++)
                    result.data_[r][j] -= factor * result.data_[pivotRow][j];
            }
            pivotRow++;
        }

        for (size_t i = 0; i < row_; i++)
            for (size_t j = 0; j < col_; j++)
                if (std::abs(result.data_[i][j]) < T{1e-12})
                    result.data_[i][j] = T{0};

        return result;
    }

    // --- LU Decomposition (Doolittle, square only) ---
    std::pair<Matrix<T>, Matrix<T>> lu() const {
        assertSquare("lu");
        size_t n = row_;
        Matrix<T> L = identity(n);
        Matrix<T> U = *this;

        for (size_t k = 0; k < n; k++) {
            if (std::abs(U.data_[k][k]) < T{1e-12})
                throw std::runtime_error("LU decomposition failed: zero pivot encountered");
            for (size_t i = k + 1; i < n; i++) {
                T factor = U.data_[i][k] / U.data_[k][k];
                L.data_[i][k] = factor;
                for (size_t j = k; j < n; j++)
                    U.data_[i][j] -= factor * U.data_[k][j];
            }
        }

        return {L, U};
    }

    // --- PLU Decomposition (LU with partial pivoting, square only) ---
    // PA = LU
    std::tuple<Matrix<T>, Matrix<T>, Matrix<T>> plu() const {
        assertSquare("plu");
        size_t n = row_;
        Matrix<T> P = identity(n);
        Matrix<T> L = Matrix<T>(n, n);
        Matrix<T> U = *this;

        for (size_t k = 0; k < n; k++) {
            size_t maxRow = k;
            T maxVal = std::abs(U.data_[k][k]);
            for (size_t i = k + 1; i < n; i++) {
                T v = std::abs(U.data_[i][k]);
                if (v > maxVal) { maxVal = v; maxRow = i; }
            }

            if (maxVal < T{1e-12})
                throw std::runtime_error("PLU decomposition failed: matrix is singular");

            if (maxRow != k) {
                std::swap(U.data_[k], U.data_[maxRow]);
                std::swap(P.data_[k], P.data_[maxRow]);
                for (size_t j = 0; j < k; j++)
                    std::swap(L.data_[k][j], L.data_[maxRow][j]);
            }

            L.data_[k][k] = T{1};
            for (size_t i = k + 1; i < n; i++) {
                T factor = U.data_[i][k] / U.data_[k][k];
                L.data_[i][k] = factor;
                for (size_t j = k; j < n; j++)
                    U.data_[i][j] -= factor * U.data_[k][j];
            }
        }

        return {P, L, U};
    }

    // --- LDU Decomposition (square only) ---
    std::tuple<Matrix<T>, Matrix<T>, Matrix<T>> ldu() const {
        auto [L, U] = lu();
        size_t n = row_;
        Matrix<T> D = Matrix<T>(n, n);
        Matrix<T> Uprime = identity(n);

        for (size_t i = 0; i < n; i++) {
            D.data_[i][i] = U.data_[i][i];
            if (std::abs(D.data_[i][i]) < T{1e-12})
                throw std::runtime_error("LDU decomposition failed: zero diagonal in U");
            for (size_t j = i; j < n; j++)
                Uprime.data_[i][j] = U.data_[i][j] / D.data_[i][i];
        }

        return {L, D, Uprime};
    }

    // --- Output ---
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& m) {
        std::vector<std::vector<std::string>> strs(m.row_, std::vector<std::string>(m.col_));
        std::vector<size_t> colWidths(m.col_, 0);

        for (size_t i = 0; i < m.row_; i++) {
            for (size_t j = 0; j < m.col_; j++) {
                strs[i][j] = formatValue(m.data_[i][j]);
                colWidths[j] = std::max(colWidths[j], strs[i][j].size());
            }
        }

        for (size_t i = 0; i < m.row_; i++) {
            os << "[ ";
            for (size_t j = 0; j < m.col_; j++) {
                os << std::setw(static_cast<int>(colWidths[j])) << std::right << strs[i][j];
                if (j + 1 < m.col_) os << "  ";
            }
            os << " ]";
            if (i + 1 < m.row_) os << "\n";
        }
        return os;
    }

private:
    std::vector<std::vector<T>> data_;
    size_t row_;
    size_t col_;

    void assertSquare(const std::string& op) const {
        if (row_ != col_)
            throw std::invalid_argument(op + " requires a square matrix");
    }

    static std::string formatValue(T val) {
        if (std::abs(val) < 1e-12) val = T{0};
        double rounded = std::round(val * 1e6) / 1e6;
        if (std::abs(rounded - std::round(rounded)) < 1e-9) {
            std::ostringstream oss;
            oss << static_cast<long long>(std::round(rounded));
            return oss.str();
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4) << rounded;
        std::string s = oss.str();
        size_t dot = s.find('.');
        if (dot != std::string::npos) {
            size_t last = s.find_last_not_of('0');
            if (last != std::string::npos && last > dot)
                s = s.substr(0, last + 1);
            else if (last == dot)
                s = s.substr(0, dot);
        }
        return s;
    }
};
