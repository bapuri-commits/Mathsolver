#pragma once
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "Matrix.hpp"
#include "StepPrinter.hpp"

class CommandParser {
public:
    using MatrixD = Matrix<double>;
    using VarMap = std::map<std::string, MatrixD>;

    explicit CommandParser(VarMap& vars) : vars_(vars) {}

    bool execute(const std::string& line) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) return true;

        if (trimmed == "exit" || trimmed == "quit") return false;
        if (trimmed == "help") { printHelp(); return true; }
        if (trimmed == "list") { listVars(); return true; }

        try {
            size_t eqPos = trimmed.find('=');
            bool hasAssign = (eqPos != std::string::npos);

            if (hasAssign) {
                std::string lhs = trim(trimmed.substr(0, eqPos));
                std::string rhs = trim(trimmed.substr(eqPos + 1));

                if (!isValidName(lhs)) {
                    std::cerr << "Error: '" << lhs << "' is not a valid variable name\n";
                    return true;
                }
                MatrixD result = evaluateExpr(rhs);
                vars_[lhs] = result;
                std::cout << lhs << " =\n" << result << "\n";
                return true;
            }

            if (startsWith(trimmed, "new ")) {
                std::string name = trim(trimmed.substr(4));
                if (!isValidName(name)) {
                    std::cerr << "Error: '" << name << "' is not a valid variable name\n";
                    return true;
                }
                vars_[name] = inputMatrix();
                std::cout << name << " =\n" << vars_[name] << "\n";
                return true;
            }

            if (startsWith(trimmed, "show ")) {
                std::string name = trim(trimmed.substr(5));
                std::cout << name << " =\n" << getVar(name) << "\n";
                return true;
            }

            if (startsWith(trimmed, "delete ")) {
                std::string name = trim(trimmed.substr(7));
                if (vars_.erase(name))
                    std::cout << "'" << name << "' deleted\n";
                else
                    std::cerr << "Error: variable '" << name << "' not found\n";
                return true;
            }

            if (startsWith(trimmed, "det ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(4));
                if (verbose) {
                    steps::detVerbose(getVar(name), std::cout);
                } else {
                    double d = getVar(name).det();
                    std::cout << "det(" << name << ") = " << formatScalar(d) << "\n";
                }
                return true;
            }

            if (startsWith(trimmed, "trace ")) {
                std::string name = trim(trimmed.substr(6));
                double t = getVar(name).trace();
                std::cout << "trace(" << name << ") = " << formatScalar(t) << "\n";
                return true;
            }

            if (startsWith(trimmed, "transpose ")) {
                std::string name = trim(trimmed.substr(10));
                printResult("transpose(" + name + ")", getVar(name).transpose());
                return true;
            }

            if (startsWith(trimmed, "inverse ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(8));
                if (verbose) {
                    auto inv = steps::inverseVerbose(getVar(name), std::cout);
                    vars_["ans"] = inv;
                } else {
                    printResult("inverse(" + name + ")", getVar(name).inverse());
                }
                return true;
            }

            if (startsWith(trimmed, "cofactor ")) {
                std::string name = trim(trimmed.substr(9));
                printResult("cofactor(" + name + ")", getVar(name).cofactorMatrix());
                return true;
            }

            if (startsWith(trimmed, "adjugate ")) {
                std::string name = trim(trimmed.substr(9));
                printResult("adjugate(" + name + ")", getVar(name).adjugate());
                return true;
            }

            if (startsWith(trimmed, "rref ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(5));
                if (verbose) {
                    auto r = steps::rrefVerbose(getVar(name), std::cout);
                    vars_["ans"] = r;
                } else {
                    printResult("rref(" + name + ")", getVar(name).rref());
                }
                return true;
            }

            if (startsWith(trimmed, "plu ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(4));
                if (verbose) {
                    (void)steps::pluVerbose(getVar(name), std::cout);
                } else {
                    auto [P, L, U] = getVar(name).plu();
                    std::cout << "P =\n" << P << "\n\n";
                    std::cout << "L =\n" << L << "\n\n";
                    std::cout << "U =\n" << U << "\n";
                    std::cout << "\n(PA = LU)\n";
                }
                return true;
            }

            if (startsWith(trimmed, "lu ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(3));
                if (verbose) {
                    (void)steps::luVerbose(getVar(name), std::cout);
                } else {
                    auto [L, U] = getVar(name).lu();
                    std::cout << "L =\n" << L << "\n\n";
                    std::cout << "U =\n" << U << "\n";
                }
                return true;
            }

            if (startsWith(trimmed, "ldu ")) {
                auto [verbose, name] = parseVerboseFlag(trimmed.substr(4));
                if (verbose) {
                    (void)steps::lduVerbose(getVar(name), std::cout);
                } else {
                    auto [L, D, U] = getVar(name).ldu();
                    std::cout << "L =\n" << L << "\n\n";
                    std::cout << "D =\n" << D << "\n\n";
                    std::cout << "U =\n" << U << "\n";
                }
                return true;
            }

            if (startsWith(trimmed, "pow ")) {
                std::string rest = trim(trimmed.substr(4));
                std::istringstream iss(rest);
                std::string name;
                int n;
                iss >> name >> n;
                if (iss.fail()) {
                    std::cerr << "Usage: pow <matrix> <exponent>\n";
                    return true;
                }
                printResult("pow(" + name + ", " + std::to_string(n) + ")", getVar(name).pow(n));
                return true;
            }

            if (startsWith(trimmed, "identity ")) {
                std::string rest = trim(trimmed.substr(9));
                int n = std::stoi(rest);
                if (n <= 0) {
                    std::cerr << "Error: size must be a positive integer\n";
                    return true;
                }
                printResult("I(" + std::to_string(n) + ")", MatrixD::identity(static_cast<size_t>(n)));
                return true;
            }

            MatrixD result = evaluateExpr(trimmed);
            printResult("ans", result);

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }

        return true;
    }

private:
    VarMap& vars_;

    void printResult(const std::string& label, const MatrixD& m) {
        vars_["ans"] = m;
        std::cout << label << " =\n" << m << "\n";
    }

    // Recursive descent expression parser with correct operator precedence:
    //   lowest:  + -
    //   higher:  *
    //   highest: unary commands (transpose, inverse, ...)
    //   base:    variable name, number
    MatrixD evaluateExpr(const std::string& expr) {
        std::string e = trim(expr);
        if (e.empty()) throw std::runtime_error("empty expression");

        // 1) +/- (lowest precedence, left-associative -> split at rightmost)
        auto [addPos, addOp] = findLastOp(e, "+-");
        if (addPos != std::string::npos) {
            auto lhs = evaluateExpr(trim(e.substr(0, addPos)));
            auto rhs = evaluateExpr(trim(e.substr(addPos + 1)));
            return (addOp == '+') ? lhs + rhs : lhs - rhs;
        }

        // 2) * (higher precedence)
        auto [mulPos, mulOp] = findLastOp(e, "*");
        if (mulPos != std::string::npos) {
            auto lhs = evaluateExpr(trim(e.substr(0, mulPos)));
            auto rhs = evaluateExpr(trim(e.substr(mulPos + 1)));
            bool lScalar = (lhs.getRow() == 1 && lhs.getCol() == 1);
            bool rScalar = (rhs.getRow() == 1 && rhs.getCol() == 1);
            if (lScalar && !rScalar) return rhs * lhs.at(0, 0);
            if (!lScalar && rScalar) return lhs * rhs.at(0, 0);
            return lhs * rhs;
        }

        // 3) Unary commands (highest precedence, bind to next expression)
        if (startsWith(e, "transpose ")) return evaluateExpr(trim(e.substr(10))).transpose();
        if (startsWith(e, "inverse "))   return evaluateExpr(trim(e.substr(8))).inverse();
        if (startsWith(e, "cofactor "))  return evaluateExpr(trim(e.substr(9))).cofactorMatrix();
        if (startsWith(e, "adjugate "))  return evaluateExpr(trim(e.substr(9))).adjugate();
        if (startsWith(e, "rref "))      return evaluateExpr(trim(e.substr(5))).rref();

        if (startsWith(e, "pow ")) {
            std::istringstream iss(e.substr(4));
            std::string name; int n;
            iss >> name >> n;
            return getVar(name).pow(n);
        }

        if (startsWith(e, "identity ")) {
            int n = std::stoi(trim(e.substr(9)));
            return MatrixD::identity(static_cast<size_t>(n));
        }

        // 4) Base case: number -> 1x1 matrix, otherwise variable
        if (isNumber(e)) return MatrixD(1, 1, std::stod(e));
        return getVar(e);
    }

    struct OpInfo {
        size_t pos = std::string::npos;
        char op = 0;
    };

    OpInfo findLastOp(const std::string& s, const std::string& ops) const {
        OpInfo result;
        for (size_t i = 1; i + 1 < s.size(); i++) {
            if (s[i - 1] == ' ' && s[i + 1] == ' ' && ops.find(s[i]) != std::string::npos)
                result = {i, s[i]};
        }
        return result;
    }

    struct VerboseResult {
        bool verbose;
        std::string arg;
    };

    static VerboseResult parseVerboseFlag(const std::string& rest) {
        std::string s = trim(rest);
        if (s.size() > 3 && s[0] == '-' && s[1] == 'v' && s[2] == ' ')
            return {true, trim(s.substr(3))};
        return {false, s};
    }

    const MatrixD& getVar(const std::string& name) const {
        auto it = vars_.find(name);
        if (it == vars_.end())
            throw std::runtime_error("variable '" + name + "' not found");
        return it->second;
    }

    MatrixD inputMatrix() const {
        size_t rows, cols;

        std::cout << "Rows: ";
        if (!(std::cin >> rows) || rows == 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            throw std::runtime_error("invalid row count");
        }

        std::cout << "Cols: ";
        if (!(std::cin >> cols) || cols == 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            throw std::runtime_error("invalid column count");
        }

        MatrixD m(rows, cols);
        for (size_t i = 0; i < rows; i++) {
            std::cout << "[Row " << (i + 1) << "] enter "
                      << cols << " values (space-separated): ";
            for (size_t j = 0; j < cols; j++) {
                if (!(std::cin >> m.at(i, j))) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    throw std::runtime_error("invalid number input");
                }
            }
        }
        std::cin.ignore(10000, '\n');
        return m;
    }

    void listVars() const {
        if (vars_.empty()) {
            std::cout << "(no variables stored)\n";
            return;
        }
        for (const auto& [name, mat] : vars_)
            std::cout << "  " << name << " : " << mat.getRow() << "x" << mat.getCol() << "\n";
    }

    void printHelp() const {
        std::cout << R"(
=== MathSolver CLI v0.3.0 ===

[Variables]
  new <name>            Create matrix (guided input)
  show <name>           Print matrix (or just type the name)
  list                  List all stored variables
  delete <name>         Delete a variable

[Binary ops]  (spaces required around operators)
  A + B                 Addition
  A - B                 Subtraction
  A * B                 Matrix multiplication
  A * 3  /  3 * A       Scalar multiplication
  * Complex expressions: A + B * C, 2 * A - B, etc.
  * Precedence: * > +/-  (standard math)

[Unary ops]
  transpose <name>      Transpose
  det <name>            Determinant (scalar)
  inverse <name>        Inverse
  trace <name>          Trace (scalar)
  cofactor <name>       Cofactor matrix
  adjugate <name>       Adjugate matrix
  pow <name> <n>        Power
  rref <name>           Reduced row echelon form
  plu <name>            PLU decomposition (PA = LU, with pivoting)
  lu <name>             LU decomposition (no pivoting)
  ldu <name>            LDU decomposition

[Step-by-step]  (add -v flag for detailed steps)
  det -v <name>         Show cofactor expansion steps
  inverse -v <name>     Show det -> cofactor -> adjugate -> scale
  rref -v <name>        Show each row operation
  lu -v <name>          Show elimination steps
  plu -v <name>         Show pivot selection + elimination
  ldu -v <name>         Show LU steps + D extraction

[Utility]
  identity <n>          n*n identity matrix
  <name> = <expr>       Store result  (e.g. C = A + B * D)
  ans                   Last result (auto-stored)
  help                  Show this help
  exit / quit           Quit

)";
    }

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    static bool isValidName(const std::string& s) {
        if (s.empty()) return false;
        if (!std::isalpha(static_cast<unsigned char>(s[0])) && s[0] != '_') return false;
        for (char c : s)
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
        static const char* reserved[] = {
            "new", "show", "list", "delete", "det", "trace", "transpose",
            "inverse", "cofactor", "adjugate", "rref", "plu", "lu", "ldu",
            "pow", "identity", "help", "exit", "quit", nullptr
        };
        for (const char** r = reserved; *r; r++)
            if (s == *r) return false;
        return true;
    }

    static bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        try {
            size_t pos;
            (void)std::stod(s, &pos);
            return pos == s.size();
        } catch (...) {
            return false;
        }
    }

    static std::string formatScalar(double val) {
        if (std::abs(val) < 1e-12) val = 0.0;
        double rounded = std::round(val * 1e6) / 1e6;
        if (std::abs(rounded - std::round(rounded)) < 1e-9) {
            return std::to_string(static_cast<long long>(std::round(rounded)));
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
