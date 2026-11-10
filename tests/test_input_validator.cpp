#include "InputValidator.hpp"
#include <iostream>
#include <sstream>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

using InputValidator::parseInt;
using InputValidator::readInt;

void test_parse_valid() {
    int r = 0;
    CHECK(parseInt("5", 1, 6, r) && r == 5);
    CHECK(parseInt("  5  ", 1, 6, r) && r == 5);
    CHECK(parseInt("+3", 1, 6, r) && r == 3);
    CHECK(parseInt("1", 1, 6, r) && r == 1);
    CHECK(parseInt("6", 1, 6, r) && r == 6);
}

void test_parse_invalid() {
    int r = 42;
    CHECK(!parseInt("", 1, 6, r));
    CHECK(!parseInt("   ", 1, 6, r));
    CHECK(!parseInt("abc", 1, 6, r));
    CHECK(!parseInt("5x", 1, 6, r));
    CHECK(!parseInt("3.5", 1, 6, r));
    CHECK(!parseInt("0", 1, 6, r));
    CHECK(!parseInt("7", 1, 6, r));
    CHECK(!parseInt("-1", 1, 6, r));
    CHECK(!parseInt("99999999999999999999", 1, 6, r));
    CHECK(r == 42);   // result untouched on failure
}

void test_read_retries_until_valid() {
    std::istringstream in("abc\n\n9\n4\n");
    std::ostringstream out;
    int r = 0;
    CHECK(readInt(in, out, "> ", 1, 6, r));
    CHECK(r == 4);
    CHECK(out.str().find("Invalid input") != std::string::npos);
}

void test_read_eof_returns_false() {
    std::istringstream in("");
    std::ostringstream out;
    int r = 0;
    CHECK(!readInt(in, out, "> ", 1, 6, r));
}

void test_read_eof_after_bad_input() {
    std::istringstream in("abc\n");
    std::ostringstream out;
    int r = 0;
    CHECK(!readInt(in, out, "> ", 1, 6, r));
}

int main() {
    test_parse_valid();
    test_parse_invalid();
    test_read_retries_until_valid();
    test_read_eof_returns_false();
    test_read_eof_after_bad_input();

    if (failures == 0) std::cout << "All InputValidator tests passed.\n";
    else std::cout << failures << " check(s) failed.\n";
    return failures == 0 ? 0 : 1;
}
