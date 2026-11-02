
#include "InputValidator.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace InputValidator {

static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t b = s.find_first_not_of(ws);

    if (b == std::string::npos) return "";

    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

bool parseInt(const std::string& text, int minVal, int maxVal, int& result) {
    if (minVal > maxVal) return false;

    std::string t = trim(text);
    if (t.empty()) return false;

    errno = 0;
    char* end = nullptr;
    long value = std::strtol(t.c_str(), &end, 10);

    if (errno == ERANGE || end == t.c_str() || *end != '\0') return false;
    if (value < INT_MIN || value > INT_MAX) return false;
    if (value < minVal || value > maxVal) return false;

    result = static_cast<int>(value);
    return true;
}

bool parseDouble(const std::string& text, double minVal, double maxVal,
                 double& result) {
    if (minVal > maxVal) return false;

    std::string t = trim(text);
    if (t.empty()) return false;

    errno = 0;
    char* end = nullptr;
    double value = std::strtod(t.c_str(), &end);

    if (errno == ERANGE || end == t.c_str() || *end != '\0') return false;
    if (!std::isfinite(value)) return false;
    if (value < minVal || value > maxVal) return false;

    result = value;
    return true;
}

bool readInt(std::istream& in, std::ostream& out, const std::string& prompt,
             int minVal, int maxVal, int& result) {
    std::string line;

    while (true) {
        out << prompt;

        if (!std::getline(in, line)) return false;

        if (parseInt(line, minVal, maxVal, result)) return true;

        out << "Invalid input. Enter a whole number between "
            << minVal << " and " << maxVal << ".\n";
    }
}

bool readDouble(std::istream& in, std::ostream& out, const std::string& prompt,
                double minVal, double maxVal, double& result) {
    std::string line;

    while (true) {
        out << prompt;

        if (!std::getline(in, line)) return false;

        if (parseDouble(line, minVal, maxVal, result)) return true;

        out << "Invalid input. Enter a number between "
            << minVal << " and " << maxVal << ".\n";
    }
}

} // namespace InputValidator
