#include <iostrem>
#include <stdexcept>

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

    return 
}

class Fraction {
private:
    long long num;
    long long den;
    void normalize();          
public:
    Fraction(long long num, long long den = 1) {
        if (den == 0) {
            throw std::invalid_argument("denominator cannot be zero");
        }
        this->num = num;
        this->den = den;
        normalize();
    }
    Fraction operator+(const Fraction& other) const {
    }
    Fraction operator-(const Fraction& other) const {
    }
    Fraction operator*(const Fraction& other) const {
    }
    Fraction operator/(const Fraction& other) const {
    }
    bool operator==(const Fraction& other) const {
    }
    bool operator!=(const Fraction& other) const {
    }
    bool operator<(const Fraction& other) const {
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
