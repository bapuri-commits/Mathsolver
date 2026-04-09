#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Matrix.hpp"
#include <tuple>

using namespace Catch::Matchers;
constexpr double EPS = 1e-9;

// ────────────────────────────────────────────
// Construction & Access
// ────────────────────────────────────────────
TEST_CASE("Matrix construction and access", "[matrix][constructor]") {
    SECTION("default constructor 1x1") {
        Matrix<double> m;
        REQUIRE(m.getRow() == 1);
        REQUIRE(m.getCol() == 1);
    }

    SECTION("size constructor with zero init") {
        Matrix<double> m(3, 4);
        REQUIRE(m.getRow() == 3);
        REQUIRE(m.getCol() == 4);
        REQUIRE_THAT(m.at(0, 0), WithinAbs(0.0, EPS));
        REQUIRE_THAT(m.at(2, 3), WithinAbs(0.0, EPS));
    }

    SECTION("size constructor with init value") {
        Matrix<double> m(2, 2, 5.0);
        REQUIRE_THAT(m.at(0, 0), WithinAbs(5.0, EPS));
        REQUIRE_THAT(m.at(1, 1), WithinAbs(5.0, EPS));
    }

    SECTION("initializer_list constructor") {
        Matrix<double> m = {{1, 2, 3}, {4, 5, 6}};
        REQUIRE(m.getRow() == 2);
        REQUIRE(m.getCol() == 3);
        REQUIRE_THAT(m.at(0, 0), WithinAbs(1.0, EPS));
        REQUIRE_THAT(m.at(1, 2), WithinAbs(6.0, EPS));
    }

    SECTION("at() write") {
        Matrix<double> m(2, 2);
        m.at(0, 1) = 3.14;
        REQUIRE_THAT(m.at(0, 1), WithinAbs(3.14, EPS));
    }

    SECTION("operator[] access") {
        Matrix<double> m = {{1, 2}, {3, 4}};
        REQUIRE_THAT(m[0][0], WithinAbs(1.0, EPS));
        REQUIRE_THAT(m[1][1], WithinAbs(4.0, EPS));
        m[0][1] = 99.0;
        REQUIRE_THAT(m.at(0, 1), WithinAbs(99.0, EPS));
    }

    SECTION("isSquare") {
        Matrix<double> sq(3, 3);
        Matrix<double> rect(2, 3);
        REQUIRE(sq.isSquare());
        REQUIRE_FALSE(rect.isSquare());
    }
}

TEST_CASE("at() out of range exception", "[matrix][exception]") {
    Matrix<double> m(2, 3);
    REQUIRE_THROWS_AS(m.at(2, 0), std::out_of_range);
    REQUIRE_THROWS_AS(m.at(0, 3), std::out_of_range);
}

// ────────────────────────────────────────────
// Identity
// ────────────────────────────────────────────
TEST_CASE("identity matrix", "[matrix][identity]") {
    auto I = Matrix<double>::identity(3);
    REQUIRE(I.getRow() == 3);
    REQUIRE(I.getCol() == 3);
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            REQUIRE_THAT(I.at(i, j), WithinAbs(i == j ? 1.0 : 0.0, EPS));
}

// ────────────────────────────────────────────
// Addition
// ────────────────────────────────────────────
TEST_CASE("matrix addition", "[matrix][add]") {
    SECTION("valid addition") {
        Matrix<double> a = {{1, 2}, {3, 4}};
        Matrix<double> b = {{5, 6}, {7, 8}};
        auto c = a + b;
        REQUIRE_THAT(c.at(0, 0), WithinAbs(6.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(12.0, EPS));
    }

    SECTION("size mismatch") {
        Matrix<double> a(2, 2);
        Matrix<double> b(2, 3);
        REQUIRE_THROWS_AS(a + b, std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Subtraction
// ────────────────────────────────────────────
TEST_CASE("matrix subtraction", "[matrix][sub]") {
    Matrix<double> a = {{5, 6}, {7, 8}};
    Matrix<double> b = {{1, 2}, {3, 4}};
    auto c = a - b;
    REQUIRE_THAT(c.at(0, 0), WithinAbs(4.0, EPS));
    REQUIRE_THAT(c.at(1, 1), WithinAbs(4.0, EPS));
}

// ────────────────────────────────────────────
// Scalar multiplication
// ────────────────────────────────────────────
TEST_CASE("scalar multiplication", "[matrix][scalar]") {
    Matrix<double> a = {{1, 2}, {3, 4}};

    SECTION("right scalar") {
        auto b = a * 3.0;
        REQUIRE_THAT(b.at(0, 0), WithinAbs(3.0, EPS));
        REQUIRE_THAT(b.at(1, 1), WithinAbs(12.0, EPS));
    }

    SECTION("left scalar (friend)") {
        auto b = 3.0 * a;
        REQUIRE_THAT(b.at(0, 0), WithinAbs(3.0, EPS));
        REQUIRE_THAT(b.at(1, 1), WithinAbs(12.0, EPS));
    }
}

// ────────────────────────────────────────────
// Matrix multiplication
// ────────────────────────────────────────────
TEST_CASE("matrix multiplication", "[matrix][mul]") {
    SECTION("2x2 * 2x2") {
        Matrix<double> a = {{1, 2}, {3, 4}};
        Matrix<double> b = {{5, 6}, {7, 8}};
        auto c = a * b;
        REQUIRE_THAT(c.at(0, 0), WithinAbs(19.0, EPS));
        REQUIRE_THAT(c.at(0, 1), WithinAbs(22.0, EPS));
        REQUIRE_THAT(c.at(1, 0), WithinAbs(43.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(50.0, EPS));
    }

    SECTION("2x3 * 3x2 -> 2x2") {
        Matrix<double> a = {{1, 2, 3}, {4, 5, 6}};
        Matrix<double> b = {{7, 8}, {9, 10}, {11, 12}};
        auto c = a * b;
        REQUIRE_THAT(c.at(0, 0), WithinAbs(58.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(154.0, EPS));
    }

    SECTION("size mismatch") {
        Matrix<double> a(2, 3);
        Matrix<double> b(2, 2);
        REQUIRE_THROWS_AS(a * b, std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Equality
// ────────────────────────────────────────────
TEST_CASE("equality operators", "[matrix][eq]") {
    Matrix<double> a = {{1, 2}, {3, 4}};
    Matrix<double> b = {{1, 2}, {3, 4}};
    Matrix<double> c = {{1, 2}, {3, 5}};
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ────────────────────────────────────────────
// Transpose
// ────────────────────────────────────────────
TEST_CASE("transpose", "[matrix][transpose]") {
    Matrix<double> a = {{1, 2, 3}, {4, 5, 6}};
    auto t = a.transpose();
    REQUIRE(t.getRow() == 3);
    REQUIRE(t.getCol() == 2);
    REQUIRE_THAT(t.at(0, 0), WithinAbs(1.0, EPS));
    REQUIRE_THAT(t.at(2, 1), WithinAbs(6.0, EPS));

    auto tt = t.transpose();
    REQUIRE(tt == a);
}

// ────────────────────────────────────────────
// Trace
// ────────────────────────────────────────────
TEST_CASE("trace", "[matrix][trace]") {
    Matrix<double> a = {{1, 2}, {3, 4}};
    REQUIRE_THAT(a.trace(), WithinAbs(5.0, EPS));

    SECTION("non-square throws") {
        Matrix<double> b(2, 3);
        REQUIRE_THROWS_AS(b.trace(), std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Determinant
// ────────────────────────────────────────────
TEST_CASE("determinant", "[matrix][det]") {
    SECTION("1x1") {
        Matrix<double> m = {{7}};
        REQUIRE_THAT(m.det(), WithinAbs(7.0, EPS));
    }

    SECTION("2x2") {
        Matrix<double> m = {{1, 2}, {3, 4}};
        REQUIRE_THAT(m.det(), WithinAbs(-2.0, EPS));
    }

    SECTION("3x3") {
        Matrix<double> m = {{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
        REQUIRE_THAT(m.det(), WithinAbs(-306.0, EPS));
    }

    SECTION("singular matrix det = 0") {
        Matrix<double> m = {{1, 2}, {2, 4}};
        REQUIRE_THAT(m.det(), WithinAbs(0.0, EPS));
    }

    SECTION("non-square throws") {
        Matrix<double> m(2, 3);
        REQUIRE_THROWS_AS(m.det(), std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Inverse
// ────────────────────────────────────────────
TEST_CASE("inverse", "[matrix][inverse]") {
    SECTION("2x2 inverse") {
        Matrix<double> m = {{4, 7}, {2, 6}};
        auto inv = m.inverse();
        auto product = m * inv;
        auto I = Matrix<double>::identity(2);
        for (size_t i = 0; i < 2; i++)
            for (size_t j = 0; j < 2; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(I.at(i, j), 1e-6));
    }

    SECTION("3x3 inverse") {
        Matrix<double> m = {{1, 2, 3}, {0, 1, 4}, {5, 6, 0}};
        auto inv = m.inverse();
        auto product = m * inv;
        auto I = Matrix<double>::identity(3);
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(I.at(i, j), 1e-6));
    }

    SECTION("singular matrix throws") {
        Matrix<double> m = {{1, 2}, {2, 4}};
        REQUIRE_THROWS_AS(m.inverse(), std::runtime_error);
    }
}

// ────────────────────────────────────────────
// Power
// ────────────────────────────────────────────
TEST_CASE("pow", "[matrix][pow]") {
    Matrix<double> m = {{1, 1}, {0, 1}};

    SECTION("pow(0) = identity") {
        auto p = m.pow(0);
        REQUIRE(p == Matrix<double>::identity(2));
    }

    SECTION("pow(1) = self") {
        auto p = m.pow(1);
        REQUIRE(p == m);
    }

    SECTION("pow(3)") {
        auto p = m.pow(3);
        REQUIRE_THAT(p.at(0, 0), WithinAbs(1.0, EPS));
        REQUIRE_THAT(p.at(0, 1), WithinAbs(3.0, EPS));
        REQUIRE_THAT(p.at(1, 0), WithinAbs(0.0, EPS));
        REQUIRE_THAT(p.at(1, 1), WithinAbs(1.0, EPS));
    }
}

// ────────────────────────────────────────────
// RREF
// ────────────────────────────────────────────
TEST_CASE("rref", "[matrix][rref]") {
    SECTION("already reduced identity") {
        auto I = Matrix<double>::identity(3);
        auto r = I.rref();
        REQUIRE(r == I);
    }

    SECTION("2x3 system") {
        Matrix<double> m = {{1, 2, 3}, {4, 5, 6}};
        auto r = m.rref();
        REQUIRE_THAT(r.at(0, 0), WithinAbs(1.0, EPS));
        REQUIRE_THAT(r.at(0, 1), WithinAbs(0.0, EPS));
        REQUIRE_THAT(r.at(1, 0), WithinAbs(0.0, EPS));
        REQUIRE_THAT(r.at(1, 1), WithinAbs(1.0, EPS));
    }

    SECTION("3x3 non-singular") {
        Matrix<double> m = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
        auto r = m.rref();
        auto I = Matrix<double>::identity(3);
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(r.at(i, j), WithinAbs(I.at(i, j), 1e-6));
    }

    SECTION("singular matrix") {
        Matrix<double> m = {{1, 2, 3}, {2, 4, 6}, {1, 1, 1}};
        auto r = m.rref();
        REQUIRE_THAT(r.at(0, 0), WithinAbs(1.0, EPS));
        REQUIRE_THAT(r.at(2, 0), WithinAbs(0.0, EPS));
        REQUIRE_THAT(r.at(2, 1), WithinAbs(0.0, EPS));
        REQUIRE_THAT(r.at(2, 2), WithinAbs(0.0, EPS));
    }
}

// ────────────────────────────────────────────
// PLU Decomposition
// ────────────────────────────────────────────
TEST_CASE("PLU decomposition", "[matrix][plu]") {
    SECTION("2x2 requiring pivoting") {
        Matrix<double> m = {{0, 1}, {1, 0}};
        auto [P, L, U] = m.plu();
        auto product = L * U;
        auto PA = P * m;
        for (size_t i = 0; i < 2; i++)
            for (size_t j = 0; j < 2; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(PA.at(i, j), 1e-6));
    }

    SECTION("3x3 with pivoting") {
        Matrix<double> m = {{1, 2, 0}, {3, 5, 4}, {5, 6, 3}};
        auto [P, L, U] = m.plu();
        auto LU = L * U;
        auto PA = P * m;
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(LU.at(i, j), WithinAbs(PA.at(i, j), 1e-6));

        for (size_t i = 0; i < 3; i++)
            REQUIRE_THAT(L.at(i, i), WithinAbs(1.0, EPS));
    }

    SECTION("3x3 no pivoting needed") {
        Matrix<double> m = {{2, -1, 0}, {-1, 2, -1}, {0, -1, 2}};
        auto [P, L, U] = m.plu();
        auto LU = L * U;
        auto PA = P * m;
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(LU.at(i, j), WithinAbs(PA.at(i, j), 1e-6));
    }

    SECTION("singular matrix throws") {
        Matrix<double> m = {{1, 2}, {2, 4}};
        REQUIRE_THROWS_AS(m.plu(), std::runtime_error);
    }

    SECTION("non-square throws") {
        Matrix<double> m(2, 3);
        REQUIRE_THROWS_AS(m.plu(), std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// LU Decomposition
// ────────────────────────────────────────────
TEST_CASE("LU decomposition", "[matrix][lu]") {
    SECTION("2x2") {
        Matrix<double> m = {{2, 3}, {4, 7}};
        auto [L, U] = m.lu();
        auto product = L * U;
        for (size_t i = 0; i < 2; i++)
            for (size_t j = 0; j < 2; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(m.at(i, j), 1e-6));
    }

    SECTION("3x3") {
        Matrix<double> m = {{2, -1, 0}, {-1, 2, -1}, {0, -1, 2}};
        auto [L, U] = m.lu();
        auto product = L * U;
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(m.at(i, j), 1e-6));

        for (size_t i = 0; i < 3; i++)
            REQUIRE_THAT(L.at(i, i), WithinAbs(1.0, EPS));
    }

    SECTION("zero pivot throws") {
        Matrix<double> m = {{0, 1}, {1, 0}};
        REQUIRE_THROWS_AS(m.lu(), std::runtime_error);
    }

    SECTION("non-square throws") {
        Matrix<double> m(2, 3);
        REQUIRE_THROWS_AS(m.lu(), std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// LDU Decomposition
// ────────────────────────────────────────────
TEST_CASE("LDU decomposition", "[matrix][ldu]") {
    SECTION("3x3 L*D*U == original") {
        Matrix<double> m = {{2, -1, 0}, {-1, 2, -1}, {0, -1, 2}};
        auto [L, D, U] = m.ldu();
        auto product = L * D * U;
        for (size_t i = 0; i < 3; i++)
            for (size_t j = 0; j < 3; j++)
                REQUIRE_THAT(product.at(i, j), WithinAbs(m.at(i, j), 1e-6));

        for (size_t i = 0; i < 3; i++) {
            REQUIRE_THAT(L.at(i, i), WithinAbs(1.0, EPS));
            REQUIRE_THAT(U.at(i, i), WithinAbs(1.0, EPS));
        }
    }
}

// ────────────────────────────────────────────
// Cofactor & Adjugate
// ────────────────────────────────────────────
TEST_CASE("cofactor and adjugate", "[matrix][cofactor]") {
    Matrix<double> m = {{1, 2}, {3, 4}};

    SECTION("cofactor values") {
        REQUIRE_THAT(m.cofactor(0, 0), WithinAbs(4.0, EPS));
        REQUIRE_THAT(m.cofactor(0, 1), WithinAbs(-3.0, EPS));
        REQUIRE_THAT(m.cofactor(1, 0), WithinAbs(-2.0, EPS));
        REQUIRE_THAT(m.cofactor(1, 1), WithinAbs(1.0, EPS));
    }

    SECTION("adjugate = cofactor matrix transposed") {
        auto adj = m.adjugate();
        REQUIRE_THAT(adj.at(0, 0), WithinAbs(4.0, EPS));
        REQUIRE_THAT(adj.at(0, 1), WithinAbs(-2.0, EPS));
        REQUIRE_THAT(adj.at(1, 0), WithinAbs(-3.0, EPS));
        REQUIRE_THAT(adj.at(1, 1), WithinAbs(1.0, EPS));
    }
}
