#pragma once
#include <ostream>     // [수정] operator<< 추가에 필요. 실제로 안 쓰이던 <iostream> 대신 더 가벼운 <ostream> 사용
#include <stdexcept>
#include <string>

static long long gcd(long long a, long long b) {
    a = a < 0 ? -a : a;
    b = b < 0 ? -b : b;
    while (b != 0) {
        long long r = a % b;// 이거 없으면 a가 덮여씌어짐.
        a = b;
        b = r;
    }
    return a;
}

static long long lcm(long long a, long long b) {
    long long g = gcd(a, b);
    return a / g * b;
}

class Fraction {
private:
    long long num;
    long long den;
    void normalize();          
public:
    // [수정] 읽기 전용 접근자에 const 추가 → const Fraction& 에서도 호출 가능. 이름도 명확하게(getnum→getNum)
    long long getNum() const { return num; }
    long long getDen() const { return den; }
    std::string toString() const {
        if (den == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(den);
    }
    Fraction(long long num, long long den = 1) {
        if (den == 0) {
            throw std::invalid_argument("denominator cannot be zero");
        }
        this->num = num;
        this->den = den;
        normalize();
    }
    Fraction operator+(const Fraction& other) const {
        long long dengcd = gcd(den, other.den);
        long long denominator = lcm(den, other.den);
        long long numerator = num * (other.den / dengcd) + other.num * (den/dengcd);
        return Fraction(numerator, denominator);
    }
    Fraction operator-(const Fraction& other) const {
        long long dengcd = gcd(den, other.den);
        long long denominator = lcm(den, other.den);
        long long numerator = num * (other.den / dengcd) - other.num * (den / dengcd);
        return Fraction(numerator, denominator);
    }
    Fraction operator*(const Fraction& other) const {
        // [수정] 오버플로 대비: 곱하기 전에 '교차 약분'을 먼저 한다.
        //  - 내 분자와 상대 분모(num, other.den), 상대 분자와 내 분모(other.num, den)를 각각 gcd로 미리 나눠
        //    중간 곱셈값이 불필요하게 커지는 것을 막는다. (결과 자체는 기존과 동일)
        long long g1 = gcd(num, other.den);
        long long g2 = gcd(other.num, den);
        long long numerator = (num / g1) * (other.num / g2);
        long long denominator = (den / g2) * (other.den / g1);
        return Fraction(numerator, denominator);
    }
    Fraction operator/(const Fraction& other) const {
        // [수정] 0으로 나누기를 '분모 0' 예외에 떠넘기지 않고 여기서 의미가 명확한 메시지로 직접 차단
        if (other.num == 0) {
            throw std::invalid_argument("division by zero fraction");
        }
        // [수정] 나눗셈 = 역수 곱이므로 곱셈과 동일하게 교차 약분으로 오버플로 대비
        long long g1 = gcd(num, other.num);
        long long g2 = gcd(other.den, den);
        long long numerator = (num / g1) * (other.den / g2);
        long long denominator = (den / g2) * (other.num / g1);
        return Fraction(numerator, denominator);
    }
    // [수정] 복합 대입 연산자 추가 — 기존 이항 연산자에 위임하여 로직 중복 없음
    Fraction& operator+=(const Fraction& other) { *this = *this + other; return *this; }
    Fraction& operator-=(const Fraction& other) { *this = *this - other; return *this; }
    Fraction& operator*=(const Fraction& other) { *this = *this * other; return *this; }
    Fraction& operator/=(const Fraction& other) { *this = *this / other; return *this; }
    bool operator==(const Fraction& other) const {
        // [수정] if(조건) return true; ... 대신 조건식을 바로 반환.
        //  생성자가 항상 '기약분수 + 분모 양수'로 정규화하므로 멤버 직접 비교로 충분하다.
        return num == other.num && den == other.den;
    }
    // [수정] != 는 == 에 위임 (중복 제거)
    bool operator!=(const Fraction& other) const { return !(*this == other); }
    bool operator<(const Fraction& other) const {
        // 분모가 항상 양수라 교차 곱 비교 시 부등호 방향이 뒤집히지 않는다
        return num * other.den < den * other.num;
    }
    // [수정] 나머지 비교 연산자 추가 — 모두 < 하나에 위임
    bool operator>(const Fraction& other) const { return other < *this; }
    bool operator<=(const Fraction& other) const { return !(other < *this); }
    bool operator>=(const Fraction& other) const { return !(*this < other); }
    // [수정] 스트림 출력 연산자 추가 — toString() 재사용 (std::cout << frac 가능)
    friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
        return os << f.toString();
    }
};

void Fraction::normalize() {
    long long n = this->num;
    long long d = this->den;

    if (d < 0) {   // 분모가 음수면 둘 다 뒤집기 결론이 같음.
        n = -n;
        d = -d;
    }

    long long div = gcd(n, d);
    this->num = n / div;
    this->den = d / div;
}
