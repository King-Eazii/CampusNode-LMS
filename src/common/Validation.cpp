#include "common/Validation.h"

#include <cctype>

#include "common/Exceptions.h"

namespace lms::validation {

std::string trim(const std::string& value) {
    std::size_t first = 0, last = value.size();
    while (first < last && std::isspace(static_cast<unsigned char>(value[first]))) ++first;
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) --last;
    return value.substr(first, last - first);
}

std::string requireText(const std::string& value, const std::string& fieldName, std::size_t maxLength) {
    const std::string text = trim(value);
    if (text.empty()) throw ValidationException(fieldName + " cannot be empty.");
    if (text.size() > maxLength) throw ValidationException(fieldName + " is too long (maximum " + std::to_string(maxLength) + " characters).");
    return text;
}

void requireUsername(const std::string& username) {
    bool ok = username.size() >= 3 && username.size() <= 30;
    for (unsigned char c : username) ok = ok && (std::isalnum(c) || c == '_');
    if (!ok) throw ValidationException("A username has 3 to 30 letters, digits or underscores.");
}

void requirePassword(const std::string& password) {
    if (password.size() < 8) throw ValidationException("A password needs at least 8 characters.");
    if (password.size() > 128) throw ValidationException("A password can have at most 128 characters.");
}

void requireMark(double mark, double maxMark) {
    if (!(mark >= 0) || mark > maxMark) {
        throw ValidationException("The mark must be between 0 and " + std::to_string(maxMark) + ".");
    }
}

}  // namespace lms::validation
