
#include "InputValidator.hpp"

#include <iostream>
#include <sstream>
#include <string>

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << "FAIL: " #condition \
                  << " (line " << __LINE__ << ")\n"; \
        ++failures; \
    } \
} while (false)

using InputValidator::parseDouble;
using InputValidator::readDouble;

void test_valid_doubles() {
    double result = 0.0;

    CHECK(parseDouble("0.30", 0.01, 1.0, result) && result == 0.30);
    CHECK(parseDouble("1.0", 0.01, 1.0, result) && result == 1.0);
    CHECK(parseDouble("  0.5  ", 0.01, 1.0, result) && result == 0.5);
    CHECK(parseDouble("0.01", 0.01, 1.0, result) && result == 0.01);
}

void test_invalid_doubles() {
    double result = 42.0;

    CHECK(!parseDouble("", 0.01, 1.0, result));
    CHECK(!parseDouble("abc", 0.01, 1.0, result));
    CHECK(!parseDouble("0.5abc", 0.01, 1.0, result));
    CHECK(!parseDouble("0.001", 0.01, 1.0, result));
    CHECK(!parseDouble("1.5", 0.01, 1.0, result));
    CHECK(!parseDouble("nan", 0.01, 1.0, result));
    CHECK(!parseDouble("inf", 0.01, 1.0, result));
    CHECK(!parseDouble("1e9999", 0.01, 1.0, result));
    CHECK(result == 42.0);
}

void test_read_double_retries() {
    std::istringstream in("abc\n2.5\n0.75\n");
    std::ostringstream out;
    double result = 0.0;

    CHECK(readDouble(in, out, "> ", 0.01, 1.0, result));
    CHECK(result == 0.75);
    CHECK(out.str().find("Invalid input") != std::string::npos);
}

void test_read_double_eof() {
    std::istringstream in("");
    std::ostringstream out;
    double result = 0.0;

    CHECK(!readDouble(in, out, "> ", 0.01, 1.0, result));
}

int main() {
    test_valid_doubles();
    test_invalid_doubles();
    test_read_double_retries();
    test_read_double_eof();

    if (failures == 0) {
        std::cout << "All double InputValidator tests passed.\n";
    } else {
        std::cout << failures << " test(s) failed.\n";
    }

    return failures == 0 ? 0 : 1;
}
