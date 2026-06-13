#pragma once
#include <iostream>
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
    long long getnum() { return num; }
    long long getden() { return den; }
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
        long long denominator = den*other.den;
        long long numerator = num*other.num;
        return Fraction(numerator,denominator);
    }
    Fraction operator/(const Fraction& other) const {
        long long denominator = den * other.num;
        long long numerator = num * other.den;
        return Fraction(numerator, denominator);
    }
    bool operator==(const Fraction& other) const {
        if (num == other.num && den == other.den)return true;
        return false;
    }
    bool operator!=(const Fraction& other) const {
        if (num == other.num && den == other.den)return false;
        return true;
    }
    bool operator<(const Fraction& other) const {
        return num * other.den < den * other.num;
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
