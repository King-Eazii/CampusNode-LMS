#pragma once
#include <string>

namespace lms::datetime {

/// Current local time as "YYYY-MM-DD HH:MM:SS".
std::string now();
/// Checks "YYYY-MM-DD HH:MM" or "YYYY-MM-DD HH:MM:SS" and returns it as "YYYY-MM-DD HH:MM:SS".
/// Throws ValidationException when the text is not a real date and time.
std::string normalise(const std::string& text);
/// Seconds since 1970-01-01 00:00:00 for a normalised time (no time zone is applied).
long long toSeconds(const std::string& iso);
std::string fromSeconds(long long seconds);
/// Whole or part days between a deadline and a later submission time (0 when not late).
int daysLate(const std::string& submittedAt, const std::string& deadline);

}  // namespace lms::datetime
