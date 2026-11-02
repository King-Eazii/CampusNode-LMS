#pragma once
#include <string>

namespace lms::validation {

/// Trims spaces; throws ValidationException when empty or longer than maxLength.
std::string requireText(const std::string& value, const std::string& fieldName, std::size_t maxLength);
void requireUsername(const std::string& username);   // 3 to 30 letters, digits or underscores
void requirePassword(const std::string& password);   // at least 8 characters
void requireMark(double mark, double maxMark);       // 0 <= mark <= maxMark
std::string trim(const std::string& value);

}  // namespace lms::validation
