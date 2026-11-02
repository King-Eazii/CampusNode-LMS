#pragma once
#include <string>

namespace lms {

/// Strategy pattern: the rule that turns lateness into a penalty can be replaced without touching the services.
class LatePenaltyPolicy {
public:
    virtual ~LatePenaltyPolicy() = default;
    virtual double calculatePenalty(double maxMark, const std::string& submittedAt,
                                    const std::string& effectiveDeadline) const = 0;
};

/// The school's rule: 10% of the maximum mark for each day or part of a day late, up to 3 days, then zero.
class TenPercentPerDayPolicy : public LatePenaltyPolicy {
public:
    double calculatePenalty(double maxMark, const std::string& submittedAt,
                            const std::string& effectiveDeadline) const override;

private:
    double ratePerDay_ = 0.10;
    int maxLateDays_ = 3;
};

}  // namespace lms
