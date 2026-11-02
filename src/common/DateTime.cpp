#include "common/DateTime.h"

#include <cstdio>
#include <ctime>

#include "common/Exceptions.h"

namespace lms::datetime {
namespace {

constexpr long long kSecondsPerDay = 86400;

// Days since 1970-01-01 for a calendar date (Howard Hinnant's algorithm).
long long daysFromCivil(int y, int m, int d) {
    y -= m <= 2 ? 1 : 0;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const int yoe = y - era * 400;
    const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<long long>(era) * 146097 + doe - 719468;
}

void civilFromDays(long long z, int& y, int& m, int& d) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const int doe = static_cast<int>(z - era * 146097);
    const int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe + era * 400);
    const int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += m <= 2 ? 1 : 0;
}

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : days[m - 1];
}

}  // namespace

std::string now() {
    const std::time_t t = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

std::string normalise(const std::string& text) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0, used = 0;
    int count = std::sscanf(text.c_str(), "%d-%d-%d %d:%d:%d%n", &y, &mo, &d, &h, &mi, &s, &used);
    if (count != 6) {
        s = 0;
        count = std::sscanf(text.c_str(), "%d-%d-%d %d:%d%n", &y, &mo, &d, &h, &mi, &used);
        if (count != 5) throw ValidationException("Use the format YYYY-MM-DD HH:MM, for example 2026-11-20 17:00.");
    }
    if (static_cast<std::size_t>(used) != text.size() || y < 2000 || y > 2100 || mo < 1 || mo > 12 || d < 1 ||
        d > daysInMonth(y, mo) || h < 0 || h > 23 || mi < 0 || mi > 59 || s < 0 || s > 59) {
        throw ValidationException("That is not a valid date and time. Use YYYY-MM-DD HH:MM.");
    }
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%04d-%02d-%02d %02d:%02d:%02d", y, mo, d, h, mi, s);
    return buffer;
}

long long toSeconds(const std::string& iso) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (std::sscanf(iso.c_str(), "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s) != 6) {
        throw ValidationException("Invalid stored date: " + iso);
    }
    return daysFromCivil(y, mo, d) * kSecondsPerDay + h * 3600 + mi * 60 + s;
}

std::string fromSeconds(long long seconds) {
    long long days = seconds / kSecondsPerDay;
    long long rest = seconds % kSecondsPerDay;
    if (rest < 0) { rest += kSecondsPerDay; --days; }
    int y = 0, m = 0, d = 0;
    civilFromDays(days, y, m, d);
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%04d-%02d-%02d %02d:%02d:%02d", y, m, d,
                  static_cast<int>(rest / 3600), static_cast<int>(rest % 3600 / 60), static_cast<int>(rest % 60));
    return buffer;
}

int daysLate(const std::string& submittedAt, const std::string& deadline) {
    const long long diff = toSeconds(submittedAt) - toSeconds(deadline);
    if (diff <= 0) return 0;
    return static_cast<int>((diff + kSecondsPerDay - 1) / kSecondsPerDay);
}

}  // namespace lms::datetime
