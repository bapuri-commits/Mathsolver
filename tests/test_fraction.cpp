#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <stdexcept>

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

TEST_CASE("zero numerator normalizes to 0/1", "[fraction][normalize]") {
    REQUIRE(Fraction(0, 5) == Fraction(0, 1));
    REQUIRE(Fraction(0, -5).getNum() == 0);
    REQUIRE(Fraction(0, -5).getDen() == 1);
}

TEST_CASE("negative denominator moves sign to numerator", "[fraction][normalize]") {
    Fraction f(2, -4);
    REQUIRE(f.getNum() == -1);
    REQUIRE(f.getDen() == 2);
}

TEST_CASE("equality and inequality", "[fraction][compare]") {
    REQUIRE(Fraction(1, 2) == Fraction(2, 4));
    REQUIRE(Fraction(1, 2) != Fraction(1, 3));
    REQUIRE_FALSE(Fraction(1, 2) == Fraction(1, 3));
    REQUIRE_FALSE(Fraction(2, 4) != Fraction(1, 2));
}

TEST_CASE("ordering operators <, >, <=, >=", "[fraction][compare]") {
    REQUIRE(Fraction(1, 3) < Fraction(1, 2));
    REQUIRE(Fraction(1, 2) > Fraction(1, 3));
    REQUIRE(Fraction(1, 2) <= Fraction(1, 2));
    REQUIRE(Fraction(1, 3) <= Fraction(1, 2));
    REQUIRE(Fraction(1, 2) >= Fraction(1, 2));
    REQUIRE(Fraction(1, 2) >= Fraction(1, 3));
    REQUIRE_FALSE(Fraction(1, 2) < Fraction(1, 3));
}

TEST_CASE("ordering with negatives", "[fraction][compare]") {
    REQUIRE(Fraction(-1, 2) < Fraction(1, 2));
    REQUIRE(Fraction(-1, 2) < Fraction(-1, 3));
    REQUIRE(Fraction(-3, 4) <= Fraction(-3, 4));
}

TEST_CASE("compound assignment +=, -=, *=, /=", "[fraction][assign]") {
    Fraction a(1, 2);
    a += Fraction(1, 3);
    REQUIRE(a == Fraction(5, 6));

    Fraction b(1, 2);
    b -= Fraction(1, 3);
    REQUIRE(b == Fraction(1, 6));

    Fraction c(2, 3);
    c *= Fraction(3, 4);
    REQUIRE(c == Fraction(1, 2));

    Fraction d(1, 2);
    d /= Fraction(3, 4);
    REQUIRE(d == Fraction(2, 3));
}

TEST_CASE("compound assignment returns reference (chaining)", "[fraction][assign]") {
    Fraction a(0, 1);
    (a += Fraction(1, 2)) += Fraction(1, 2);
    REQUIRE(a == Fraction(1));
}

TEST_CASE("zero denominator throws on construction", "[fraction][exception]") {
    REQUIRE_THROWS_AS(Fraction(1, 0), std::invalid_argument);
}

TEST_CASE("division by zero fraction throws", "[fraction][exception]") {
    REQUIRE_THROWS_AS(Fraction(1, 2) / Fraction(0, 1), std::invalid_argument);
}

TEST_CASE("toString format", "[fraction][output]") {
    REQUIRE(Fraction(1, 2).toString() == "1/2");
    REQUIRE(Fraction(6, 3).toString() == "2");
    REQUIRE(Fraction(-1, 3).toString() == "-1/3");
    REQUIRE(Fraction(1, -3).toString() == "-1/3");
}

TEST_CASE("stream output operator<<", "[fraction][output]") {
    std::ostringstream oss;
    oss << Fraction(2, 4);
    REQUIRE(oss.str() == "1/2");
}

TEST_CASE("multiplication stays exact for large values (overflow-safe)", "[fraction][mul][overflow]") {
    // 교차 약분이 없으면 분모 1e9 * 2e9 = 2e18 로 long long 한계(약 9.2e18)에 근접/초과 위험.
    // 교차 약분 후에는 곧바로 1/2 로 줄어 안전하다.
    Fraction big(1, 1000000000LL);
    Fraction big2(500000000LL, 1);
    REQUIRE(big * big2 == Fraction(1, 2));
}

// ===== 엣지 케이스 =====

TEST_CASE("edge: double negative normalizes to positive", "[fraction][edge][normalize]") {
    // 분자/분모 둘 다 음수면 양수 분수로 정규화돼야 한다
    REQUIRE(Fraction(-1, -2) == Fraction(1, 2));
    REQUIRE(Fraction(-6, -3) == Fraction(2));
}

TEST_CASE("edge: zero with negative/large denominator", "[fraction][edge][normalize]") {
    // 0 은 분모가 무엇이든 0/1 이어야 하고 음수 부호가 남으면 안 된다
    REQUIRE(Fraction(0, -7).getNum() == 0);
    REQUIRE(Fraction(0, -7).getDen() == 1);
    REQUIRE(Fraction(0, 999999999LL) == Fraction(0));
    REQUIRE(Fraction(0).toString() == "0");
}

TEST_CASE("edge: multiply by zero gives 0/1", "[fraction][edge][mul]") {
    REQUIRE(Fraction(3, 4) * Fraction(0) == Fraction(0));
    REQUIRE(Fraction(0) * Fraction(5, 7) == Fraction(0));
}

TEST_CASE("edge: sign combinations in multiplication", "[fraction][edge][mul]") {
    REQUIRE(Fraction(-1, 2) * Fraction(-2, 3) == Fraction(1, 3));   // (-)(-) = (+)
    REQUIRE(Fraction(-1, 2) * Fraction(2, 3) == Fraction(-1, 3));   // (-)(+) = (-)
    REQUIRE(Fraction(1, 2) * Fraction(-2, 3) == Fraction(-1, 3));   // (+)(-) = (-)
}

TEST_CASE("edge: sign combinations in division", "[fraction][edge][div]") {
    REQUIRE(Fraction(1, 2) / Fraction(-3, 4) == Fraction(-2, 3));   // (+)/(-) = (-)
    REQUIRE(Fraction(-1, 2) / Fraction(-1, 3) == Fraction(3, 2));   // (-)/(-) = (+)
}

TEST_CASE("edge: self division equals one", "[fraction][edge][div]") {
    REQUIRE(Fraction(3, 7) / Fraction(3, 7) == Fraction(1));
    REQUIRE(Fraction(-5, 8) / Fraction(-5, 8) == Fraction(1));
}

TEST_CASE("edge: subtraction crossing zero into negative", "[fraction][edge][sub]") {
    REQUIRE(Fraction(1, 3) - Fraction(1, 2) == Fraction(-1, 6));
}

TEST_CASE("edge: adding additive inverse gives zero", "[fraction][edge][add]") {
    REQUIRE(Fraction(1, 2) + Fraction(-1, 2) == Fraction(0));
    REQUIRE(Fraction(3, 7) - Fraction(3, 7) == Fraction(0));
}

TEST_CASE("edge: whole number arithmetic", "[fraction][edge]") {
    REQUIRE(Fraction(3) + Fraction(2) == Fraction(5));
    REQUIRE(Fraction(7) - Fraction(10) == Fraction(-3));
    REQUIRE(Fraction(4) * Fraction(3) == Fraction(12));
    REQUIRE(Fraction(10) / Fraction(4) == Fraction(5, 2));
}

TEST_CASE("edge: comparison around zero and equal values", "[fraction][edge][compare]") {
    REQUIRE(Fraction(-1, 2) < Fraction(0));
    REQUIRE(Fraction(0) < Fraction(1, 2));
    REQUIRE_FALSE(Fraction(1, 2) < Fraction(1, 2));   // 자기 자신과의 < 는 거짓 (비반사성)
    REQUIRE(Fraction(1, 2) <= Fraction(2, 4));        // 약분 후 같으면 <= 참
    REQUIRE(Fraction(1, 2) >= Fraction(2, 4));
}

TEST_CASE("edge: compound assignment flipping sign", "[fraction][edge][assign]") {
    Fraction a(1, 3);
    a += Fraction(-1, 2);   // 1/3 + (-1/2) = -1/6
    REQUIRE(a == Fraction(-1, 6));

    Fraction b(2, 3);
    b /= Fraction(-2, 3);   // (2/3)/(-2/3) = -1
    REQUIRE(b == Fraction(-1));
}

TEST_CASE("edge: negative fraction toString keeps single leading minus", "[fraction][edge][output]") {
    REQUIRE(Fraction(-1, -3).toString() == "1/3");
    REQUIRE(Fraction(3, -4).toString() == "-3/4");
    REQUIRE(Fraction(-2).toString() == "-2");
}
