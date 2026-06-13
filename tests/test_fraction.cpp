#include <catch2/catch_test_macros.hpp>

#include "Fraction.hpp"

TEST_CASE("addition: 1/2 + 1/3 == 5/6", "[fraction][add]") {
    REQUIRE(Fraction(1, 2) + Fraction(1, 3) == Fraction(5, 6));
}

TEST_CASE("subtraction: 1/2 - 1/3 == 1/6", "[fraction][sub]") {
    REQUIRE(Fraction(1, 2) - Fraction(1, 3) == Fraction(1, 6));
}

TEST_CASE("multiplication: 2/3 * 3/4 == 1/2", "[fraction][mul]") {
    REQUIRE(Fraction(2, 3) * Fraction(3, 4) == Fraction(1, 2));
}

TEST_CASE("division: (1/2) / (3/4) == 2/3", "[fraction][div]") {
    REQUIRE(Fraction(1, 2) / Fraction(3, 4) == Fraction(2, 3));
}

TEST_CASE("reduce: Fraction(2,4) == Fraction(1,2)", "[fraction][normalize]") {
    REQUIRE(Fraction(2, 4) == Fraction(1, 2));
}

TEST_CASE("negative sign normalization: Fraction(1,-3) == Fraction(-1,3)", "[fraction][normalize]") {
    REQUIRE(Fraction(1, -3) == Fraction(-1, 3));
}

TEST_CASE("integer reduction: Fraction(6,3) == Fraction(2)", "[fraction][normalize]") {
    REQUIRE(Fraction(6, 3) == Fraction(2));
}
