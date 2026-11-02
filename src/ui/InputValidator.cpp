#include "ui/InputValidator.h"

#include <cstdlib>
#include <sstream>

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "common/Validation.h"
#include "ui/Console.h"

namespace lms::input {

int integer(const std::string& prompt, int min, int max) {
    while (true) {
        const std::string line = validation::trim(console::readLine(prompt));
        char* end = nullptr;
        const long value = std::strtol(line.c_str(), &end, 10);
        if (!line.empty() && *end == '\0' && value >= min && value <= max) return static_cast<int>(value);
        console::error("Enter a whole number from " + std::to_string(min) + " to " + std::to_string(max) + ".");
    }
}

double number(const std::string& prompt, double min, double max) {
    while (true) {
        const std::string line = validation::trim(console::readLine(prompt));
        char* end = nullptr;
        const double value = std::strtod(line.c_str(), &end);
        if (!line.empty() && *end == '\0' && value >= min && value <= max) return value;
        console::error("Enter a number from " + console::number(min) + " to " + console::number(max) + ".");
    }
}

std::string text(const std::string& prompt, std::size_t maxLength, bool allowEmpty) {
    while (true) {
        const std::string line = validation::trim(console::readLine(prompt));
        if (line.empty() && allowEmpty) return line;
        if (line.empty()) { console::error("This cannot be empty."); continue; }
        if (line.size() > maxLength) { console::error("At most " + std::to_string(maxLength) + " characters."); continue; }
        return line;
    }
}

std::string dateTime(const std::string& prompt) {
    while (true) {
        try {
            return datetime::normalise(validation::trim(console::readLine(prompt + " (YYYY-MM-DD HH:MM): ")));
        } catch (const ValidationException& e) {
            console::error(e.what());
        }
    }
}

bool yesNo(const std::string& prompt) {
    while (true) {
        const std::string line = validation::trim(console::readLine(prompt + " (y/n): "));
        if (line == "y" || line == "Y") return true;
        if (line == "n" || line == "N") return false;
        console::error("Type y or n.");
    }
}

std::vector<int> idList(const std::string& prompt) {
    while (true) {
        std::string line = console::readLine(prompt);
        for (char& c : line) if (c == ',') c = ' ';
        std::istringstream in(line);
        std::vector<int> ids;
        std::string token;
        bool ok = true;
        while (in >> token) {
            char* end = nullptr;
            const long v = std::strtol(token.c_str(), &end, 10);
            if (*end != '\0' || v <= 0) { ok = false; break; }
            ids.push_back(static_cast<int>(v));
        }
        if (ok && !ids.empty()) return ids;
        console::error("Enter one or more numbers separated by commas, for example 3, 5.");
    }
}

}  // namespace lms::input
