#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Matrix.hpp"

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
        REQUIRE_THAT(m.at(0, 1), WithinAbs(5.0, EPS));
        REQUIRE_THAT(m.at(1, 0), WithinAbs(5.0, EPS));
        REQUIRE_THAT(m.at(1, 1), WithinAbs(5.0, EPS));
    }

    SECTION("at() write") {
        Matrix<double> m(2, 2);
        m.at(0, 1) = 3.14;
        REQUIRE_THAT(m.at(0, 1), WithinAbs(3.14, EPS));
    }
}

TEST_CASE("at() out of range exception", "[matrix][exception]") {
    Matrix<double> m(2, 3);
    REQUIRE_THROWS_AS(m.at(2, 0), std::out_of_range);
    REQUIRE_THROWS_AS(m.at(0, 3), std::out_of_range);
}

// ────────────────────────────────────────────
// Addition
// ────────────────────────────────────────────
TEST_CASE("matrix addition operator+", "[matrix][add]") {
    SECTION("valid addition") {
        Matrix<double> a(2, 2, 1.0);
        Matrix<double> b(2, 2, 2.0);
        auto c = a + b;
        REQUIRE(c.getRow() == 2);
        REQUIRE(c.getCol() == 2);
        REQUIRE_THAT(c.at(0, 0), WithinAbs(3.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(3.0, EPS));
    }

    SECTION("size mismatch exception") {
        Matrix<double> a(2, 2);
        Matrix<double> b(2, 3);
        REQUIRE_THROWS_AS(a + b, std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Subtraction
// ────────────────────────────────────────────
TEST_CASE("matrix subtraction operator-", "[matrix][sub]") {
    SECTION("valid subtraction") {
        Matrix<double> a(2, 2, 5.0);
        Matrix<double> b(2, 2, 3.0);
        auto c = a - b;
        REQUIRE_THAT(c.at(0, 0), WithinAbs(2.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(2.0, EPS));
    }

    SECTION("size mismatch exception") {
        Matrix<double> a(2, 2);
        Matrix<double> b(3, 2);
        REQUIRE_THROWS_AS(a - b, std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Scalar multiplication
// ────────────────────────────────────────────
TEST_CASE("scalar multiplication operator*(T)", "[matrix][scalar]") {
    Matrix<double> a(2, 3, 2.0);
    auto b = a * 3.0;
    REQUIRE(b.getRow() == 2);
    REQUIRE(b.getCol() == 3);
    REQUIRE_THAT(b.at(0, 0), WithinAbs(6.0, EPS));
    REQUIRE_THAT(b.at(1, 2), WithinAbs(6.0, EPS));
}

// ────────────────────────────────────────────
// Matrix multiplication
// ────────────────────────────────────────────
TEST_CASE("matrix multiplication operator*(Matrix)", "[matrix][mul]") {
    SECTION("2x2 * 2x2") {
        // A = [[1,2],[3,4]], B = [[5,6],[7,8]]
        // A*B = [[19,22],[43,50]]
        Matrix<double> a(2, 2);
        a.at(0, 0) = 1; a.at(0, 1) = 2;
        a.at(1, 0) = 3; a.at(1, 1) = 4;

        Matrix<double> b(2, 2);
        b.at(0, 0) = 5; b.at(0, 1) = 6;
        b.at(1, 0) = 7; b.at(1, 1) = 8;

        auto c = a * b;
        REQUIRE(c.getRow() == 2);
        REQUIRE(c.getCol() == 2);
        REQUIRE_THAT(c.at(0, 0), WithinAbs(19.0, EPS));
        REQUIRE_THAT(c.at(0, 1), WithinAbs(22.0, EPS));
        REQUIRE_THAT(c.at(1, 0), WithinAbs(43.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(50.0, EPS));
    }

    SECTION("2x3 * 3x2 -> 2x2") {
        // A = [[1,2,3],[4,5,6]], B = [[7,8],[9,10],[11,12]]
        // A*B = [[58,64],[139,154]]
        Matrix<double> a(2, 3);
        a.at(0, 0)=1; a.at(0, 1)=2; a.at(0, 2)=3;
        a.at(1, 0)=4; a.at(1, 1)=5; a.at(1, 2)=6;

        Matrix<double> b(3, 2);
        b.at(0, 0)=7;  b.at(0, 1)=8;
        b.at(1, 0)=9;  b.at(1, 1)=10;
        b.at(2, 0)=11; b.at(2, 1)=12;

        auto c = a * b;
        REQUIRE(c.getRow() == 2);
        REQUIRE(c.getCol() == 2);
        REQUIRE_THAT(c.at(0, 0), WithinAbs(58.0,  EPS));
        REQUIRE_THAT(c.at(0, 1), WithinAbs(64.0,  EPS));
        REQUIRE_THAT(c.at(1, 0), WithinAbs(139.0, EPS));
        REQUIRE_THAT(c.at(1, 1), WithinAbs(154.0, EPS));
    }

    SECTION("size mismatch exception") {
        Matrix<double> a(2, 3);
        Matrix<double> b(2, 2);
        REQUIRE_THROWS_AS(a * b, std::invalid_argument);
    }
}

// ────────────────────────────────────────────
// Transpose
// ────────────────────────────────────────────
TEST_CASE("matrix transpose()", "[matrix][transpose]") {
    SECTION("2x3 -> 3x2") {
        // A = [[1,2,3],[4,5,6]]
        // A^T = [[1,4],[2,5],[3,6]]
        Matrix<double> a(2, 3);
        a.at(0, 0)=1; a.at(0, 1)=2; a.at(0, 2)=3;
        a.at(1, 0)=4; a.at(1, 1)=5; a.at(1, 2)=6;

        auto t = a.transpose();
        REQUIRE(t.getRow() == 3);
        REQUIRE(t.getCol() == 2);
        REQUIRE_THAT(t.at(0, 0), WithinAbs(1.0, EPS));
        REQUIRE_THAT(t.at(1, 0), WithinAbs(2.0, EPS));
        REQUIRE_THAT(t.at(2, 0), WithinAbs(3.0, EPS));
        REQUIRE_THAT(t.at(0, 1), WithinAbs(4.0, EPS));
        REQUIRE_THAT(t.at(1, 1), WithinAbs(5.0, EPS));
        REQUIRE_THAT(t.at(2, 1), WithinAbs(6.0, EPS));
    }

    SECTION("transpose of transpose equals original") {
        Matrix<double> a(3, 2, 7.0);
        a.at(0, 1) = 99.0;
        auto tt = a.transpose().transpose();
        REQUIRE(tt.getRow() == a.getRow());
        REQUIRE(tt.getCol() == a.getCol());
        REQUIRE_THAT(tt.at(0, 1), WithinAbs(99.0, EPS));
    }
}
