#pragma once
#include "Matrix.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <string>

namespace steps {

using Mat = Matrix<double>;

inline std::string fmtVal(double v) {
    if (std::abs(v) < 1e-12) v = 0.0;
    double r = std::round(v * 1e6) / 1e6;
    if (std::abs(r - std::round(r)) < 1e-9) {
        return std::to_string(static_cast<long long>(std::round(r)));
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << r;
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

inline void printIndented(std::ostream& os, const Mat& m, const std::string& prefix = "  ") {
    std::ostringstream buf;
    buf << m;
    std::string line;
    std::istringstream stream(buf.str());
    while (std::getline(stream, line))
        os << prefix << line << "\n";
}

inline std::string signStr(size_t idx) {
    return (idx % 2 == 0) ? "+" : "-";
}

// ─────────────────────────────────────────
// det -v
// ─────────────────────────────────────────
inline double detVerbose(const Mat& m, std::ostream& os) {
    if (!m.isSquare())
        throw std::invalid_argument("det requires a square matrix");
    size_t n = m.getRow();

    if (n == 1) {
        double val = m.at(0, 0);
        os << "det = " << fmtVal(val) << "  (1x1)\n";
        return val;
    }

    if (n == 2) {
        double a = m.at(0, 0), b = m.at(0, 1);
        double c = m.at(1, 0), d = m.at(1, 1);
        double det = a * d - b * c;
        os << "[Step 1] 2x2 formula: a*d - b*c\n";
        os << "  = (" << fmtVal(a) << ")(" << fmtVal(d) << ") - ("
           << fmtVal(b) << ")(" << fmtVal(c) << ")\n";
        os << "  = " << fmtVal(a * d) << " - " << fmtVal(b * c) << "\n\n";
        os << "Result: det = " << fmtVal(det) << "\n";
        return det;
    }

    // n >= 3: cofactor expansion along row 0 (top-level only)
    os << "[Step 1] Cofactor expansion along row 0:\n";
    os << "  det = ";
    for (size_t j = 0; j < n; j++) {
        if (j > 0) os << " + ";
        os << "(" << signStr(j) << "1)(" << fmtVal(m.at(0, j)) << ") * M0" << j;
    }
    os << "\n\n";

    os << "[Step 2] Minor determinants:\n";
    std::vector<double> minors(n);
    for (size_t j = 0; j < n; j++) {
        Mat sub = m.subMatrix(0, j);
        minors[j] = sub.det();
        os << "  M0" << j << " = det(";
        for (size_t r = 0; r < sub.getRow(); r++) {
            os << "[";
            for (size_t c = 0; c < sub.getCol(); c++) {
                if (c > 0) os << ", ";
                os << fmtVal(sub.at(r, c));
            }
            os << "]";
            if (r + 1 < sub.getRow()) os << ", ";
        }
        os << ") = " << fmtVal(minors[j]) << "\n";
    }
    os << "\n";

    os << "[Step 3] Sum:\n  = ";
    double result = 0;
    for (size_t j = 0; j < n; j++) {
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        double term = sign * m.at(0, j) * minors[j];
        result += term;
        if (j > 0) os << " + ";
        os << "(" << signStr(j) << "1)(" << fmtVal(m.at(0, j)) << ")(" << fmtVal(minors[j]) << ")";
    }
    os << "\n  = ";
    for (size_t j = 0; j < n; j++) {
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        double term = sign * m.at(0, j) * minors[j];
        if (j == 0) {
            os << fmtVal(term);
        } else if (term >= 0) {
            os << " + " << fmtVal(term);
        } else {
            os << " - " << fmtVal(-term);
        }
    }
    os << " = " << fmtVal(result) << "\n\n";
    os << "Result: det = " << fmtVal(result) << "\n";
    return result;
}

// ─────────────────────────────────────────
// inverse -v
// ─────────────────────────────────────────
inline Mat inverseVerbose(const Mat& m, std::ostream& os) {
    if (!m.isSquare())
        throw std::invalid_argument("inverse requires a square matrix");

    os << "[Step 1] Compute determinant:\n";
    double d = m.det();
    os << "  det = " << fmtVal(d) << "\n";
    if (std::abs(d) < 1e-12)
        throw std::runtime_error("det = 0 -> inverse does not exist");
    os << "\n";

    os << "[Step 2] Cofactor matrix:\n";
    Mat cof = m.cofactorMatrix();
    printIndented(os, cof);
    os << "\n";

    os << "[Step 3] Adjugate (transpose of cofactor):\n";
    Mat adj = m.adjugate();
    printIndented(os, adj);
    os << "\n";

    os << "[Step 4] Multiply by 1/det = " << fmtVal(1.0 / d) << ":\n";
    Mat inv = adj * (1.0 / d);
    printIndented(os, inv);
    os << "\n";

    os << "Result: inverse =\n";
    printIndented(os, inv);
    return inv;
}

// ─────────────────────────────────────────
// rref -v
// ─────────────────────────────────────────
inline Mat rrefVerbose(const Mat& m, std::ostream& os) {
    Mat result = m;
    size_t rows = result.getRow();
    size_t cols = result.getCol();
    size_t pivotRow = 0;
    int step = 0;

    os << "Initial matrix:\n";
    printIndented(os, result);
    os << "\n";

    for (size_t c = 0; c < cols && pivotRow < rows; c++) {
        size_t maxRow = pivotRow;
        double maxVal = std::abs(result[pivotRow][c]);
        for (size_t r = pivotRow + 1; r < rows; r++) {
            double val = std::abs(result[r][c]);
            if (val > maxVal) { maxVal = val; maxRow = r; }
        }

        if (maxVal < 1e-12) continue;

        if (maxRow != pivotRow) {
            step++;
            std::swap(result[pivotRow], result[maxRow]);
            os << "[Step " << step << "] Swap R" << (pivotRow + 1)
               << " <-> R" << (maxRow + 1) << "\n";
            printIndented(os, result);
            os << "\n";
        }

        double pivot = result[pivotRow][c];
        if (std::abs(pivot - 1.0) > 1e-12) {
            step++;
            for (size_t j = 0; j < cols; j++)
                result[pivotRow][j] /= pivot;
            os << "[Step " << step << "] R" << (pivotRow + 1)
               << " <- R" << (pivotRow + 1) << " / " << fmtVal(pivot) << "\n";
            printIndented(os, result);
            os << "\n";
        }

        for (size_t r = 0; r < rows; r++) {
            if (r == pivotRow) continue;
            double factor = result[r][c];
            if (std::abs(factor) < 1e-12) continue;
            step++;
            for (size_t j = 0; j < cols; j++)
                result[r][j] -= factor * result[pivotRow][j];
            os << "[Step " << step << "] R" << (r + 1) << " <- R" << (r + 1)
               << " - (" << fmtVal(factor) << ") * R" << (pivotRow + 1) << "\n";
            printIndented(os, result);
            os << "\n";
        }
        pivotRow++;
    }

    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < cols; j++)
            if (std::abs(result[i][j]) < 1e-12)
                result[i][j] = 0.0;

    os << "Result: RREF =\n";
    printIndented(os, result);
    return result;
}

// ─────────────────────────────────────────
// lu -v
// ─────────────────────────────────────────
inline std::pair<Mat, Mat> luVerbose(const Mat& m, std::ostream& os) {
    if (!m.isSquare())
        throw std::invalid_argument("lu requires a square matrix");
    size_t n = m.getRow();
    Mat L = Mat::identity(n);
    Mat U = m;
    int step = 0;

    os << "Initial U = A:\n";
    printIndented(os, U);
    os << "\n";

    for (size_t k = 0; k < n; k++) {
        if (std::abs(U[k][k]) < 1e-12)
            throw std::runtime_error("LU decomposition failed: zero pivot encountered");

        bool anyElim = false;
        for (size_t i = k + 1; i < n; i++) {
            if (std::abs(U[i][k]) < 1e-12) continue;
            anyElim = true;
            double factor = U[i][k] / U[k][k];
            L[i][k] = factor;
            step++;
            os << "[Step " << step << "] Column " << k << ": L[" << i << "][" << k
               << "] = " << fmtVal(U[i][k]) << " / " << fmtVal(U[k][k])
               << " = " << fmtVal(factor) << "\n";
            os << "  R" << (i + 1) << " <- R" << (i + 1) << " - ("
               << fmtVal(factor) << ") * R" << (k + 1) << "\n";
            for (size_t j = k; j < n; j++)
                U[i][j] -= factor * U[k][j];
        }
        if (anyElim) {
            os << "  U after step:\n";
            printIndented(os, U, "    ");
            os << "\n";
        }
    }

    os << "Result:\nL =\n";
    printIndented(os, L);
    os << "\nU =\n";
    printIndented(os, U);
    return {L, U};
}

// ─────────────────────────────────────────
// plu -v
// ─────────────────────────────────────────
inline std::tuple<Mat, Mat, Mat> pluVerbose(const Mat& m, std::ostream& os) {
    if (!m.isSquare())
        throw std::invalid_argument("plu requires a square matrix");
    size_t n = m.getRow();
    Mat P = Mat::identity(n);
    Mat L(n, n);
    Mat U = m;
    int step = 0;

    os << "Initial U = A:\n";
    printIndented(os, U);
    os << "\n";

    for (size_t k = 0; k < n; k++) {
        size_t maxRow = k;
        double maxVal = std::abs(U[k][k]);
        for (size_t i = k + 1; i < n; i++) {
            double v = std::abs(U[i][k]);
            if (v > maxVal) { maxVal = v; maxRow = i; }
        }

        if (maxVal < 1e-12)
            throw std::runtime_error("PLU decomposition failed: matrix is singular");

        if (maxRow != k) {
            step++;
            std::swap(U[k], U[maxRow]);
            std::swap(P[k], P[maxRow]);
            for (size_t j = 0; j < k; j++)
                std::swap(L[k][j], L[maxRow][j]);
            os << "[Step " << step << "] Pivot: max in column " << k
               << " is row " << maxRow << " (|" << fmtVal(maxVal)
               << "|), swap R" << (k + 1) << " <-> R" << (maxRow + 1) << "\n";
            os << "  U:\n";
            printIndented(os, U, "    ");
            os << "\n";
        }

        L[k][k] = 1.0;
        bool anyElim = false;
        for (size_t i = k + 1; i < n; i++) {
            if (std::abs(U[i][k]) < 1e-12) continue;
            anyElim = true;
            double factor = U[i][k] / U[k][k];
            L[i][k] = factor;
            step++;
            os << "[Step " << step << "] L[" << i << "][" << k
               << "] = " << fmtVal(factor)
               << ",  R" << (i + 1) << " <- R" << (i + 1) << " - ("
               << fmtVal(factor) << ") * R" << (k + 1) << "\n";
            for (size_t j = k; j < n; j++)
                U[i][j] -= factor * U[k][j];
        }
        if (anyElim) {
            os << "  U:\n";
            printIndented(os, U, "    ");
            os << "\n";
        }
    }

    os << "Result (PA = LU):\nP =\n";
    printIndented(os, P);
    os << "\nL =\n";
    printIndented(os, L);
    os << "\nU =\n";
    printIndented(os, U);
    return {P, L, U};
}

// ─────────────────────────────────────────
// ldu -v
// ─────────────────────────────────────────
inline std::tuple<Mat, Mat, Mat> lduVerbose(const Mat& m, std::ostream& os) {
    os << "--- LU decomposition phase ---\n\n";
    auto [L, U] = luVerbose(m, os);
    size_t n = m.getRow();

    os << "\n--- Extract D from U diagonal ---\n\n";
    Mat D(n, n);
    Mat Up = Mat::identity(n);
    int step = 0;

    os << "  D diagonal: [";
    for (size_t i = 0; i < n; i++) {
        D[i][i] = U[i][i];
        if (i > 0) os << ", ";
        os << fmtVal(D[i][i]);
    }
    os << "]\n\n";

    for (size_t i = 0; i < n; i++) {
        if (std::abs(D[i][i]) < 1e-12)
            throw std::runtime_error("LDU decomposition failed: zero diagonal in U");
        step++;
        os << "[Step " << step << "] U' row " << i << " = U row " << i
           << " / " << fmtVal(D[i][i]) << "\n";
        for (size_t j = i; j < n; j++)
            Up[i][j] = U[i][j] / D[i][i];
    }

    os << "\nResult:\nL =\n";
    printIndented(os, L);
    os << "\nD =\n";
    printIndented(os, D);
    os << "\nU' =\n";
    printIndented(os, Up);
    return {L, D, Up};
}

} // namespace steps
