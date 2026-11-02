#pragma once
#include <string>
#include <vector>

namespace lms::input {

/// Each function asks again until the answer is valid, so bad input never reaches the services.
int integer(const std::string& prompt, int min, int max);
double number(const std::string& prompt, double min, double max);
std::string text(const std::string& prompt, std::size_t maxLength, bool allowEmpty = false);
std::string dateTime(const std::string& prompt);  // returns "YYYY-MM-DD HH:MM:SS"
bool yesNo(const std::string& prompt);
std::vector<int> idList(const std::string& prompt);  // for example "3, 5, 8"

}  // namespace lms::input
