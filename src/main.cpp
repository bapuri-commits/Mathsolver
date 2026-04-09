#include <iostream>
#include <string>
#include <map>
#include "Matrix.hpp"
#include "CommandParser.hpp"

int main() {
    std::cout << "MathSolver v0.3.0  (type 'help' for commands)\n\n";

    std::map<std::string, Matrix<double>> variables;
    CommandParser parser(variables);

    std::string line;
    while (true) {
        std::cout << ">> ";
        if (!std::getline(std::cin, line)) break;
        if (!parser.execute(line)) break;
    }

    std::cout << "Bye.\n";
    return 0;
}
