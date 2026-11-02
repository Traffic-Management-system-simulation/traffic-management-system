
#ifndef INPUT_VALIDATOR_HPP
#define INPUT_VALIDATOR_HPP

#include <iosfwd>
#include <string>

namespace InputValidator {

bool parseInt(const std::string& text, int minVal, int maxVal, int& result);
bool parseDouble(const std::string& text, double minVal, double maxVal, double& result);

bool readInt(std::istream& in, std::ostream& out, const std::string& prompt,
             int minVal, int maxVal, int& result);

bool readDouble(std::istream& in, std::ostream& out, const std::string& prompt,
                double minVal, double maxVal, double& result);

} // namespace InputValidator

#endif
