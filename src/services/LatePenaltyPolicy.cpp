#include "services/LatePenaltyPolicy.h"

#include "common/DateTime.h"

namespace lms {

double TenPercentPerDayPolicy::calculatePenalty(double maxMark, const std::string& submittedAt,
                                                const std::string& effectiveDeadline) const {
    const int days = datetime::daysLate(submittedAt, effectiveDeadline);
    if (days == 0) return 0.0;
    if (days > maxLateDays_) return maxMark;  // the whole mark is removed, so the final mark is zero
    return days * ratePerDay_ * maxMark;
}

}  // namespace lms
