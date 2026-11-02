#pragma once
#include <optional>
#include <string>
#include <vector>

#include "domain/Types.h"

namespace lms {

struct Department { int id = 0; std::string name; };

struct Course { int id = 0; std::string code; std::string title; int departmentId = 0; };

struct Assignment {
    int id = 0;
    int courseId = 0;
    std::string title;
    std::string description;
    std::string deadline;  // original deadline, "YYYY-MM-DD HH:MM:SS"
    double maxMark = 0;
};

struct Extension {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    int grantedBy = 0;
    std::string newDeadline;
    std::string reason;
    std::string grantedAt;
};

struct SubmissionVersion {
    int versionNo = 1;
    std::string content;
    std::string submittedAt;
};

/// A student's work for one assignment. Owns the life-cycle rules (FR10).
struct Submission {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    SubmissionStatus status = SubmissionStatus::Submitted;
    std::optional<double> rawMark;    // lecturer's mark before the penalty
    double penalty = 0;               // marks removed for lateness
    std::optional<double> finalMark;  // the mark the student receives
    std::string feedback;
    std::vector<SubmissionVersion> versions;

    bool canTransitionTo(SubmissionStatus next) const;
    /// Moves to the next status or throws ValidationException if the change is not allowed.
    void transitionTo(SubmissionStatus next);
    const SubmissionVersion& latestVersion() const;
};

struct AuditEntry {
    int id = 0;
    int submissionId = 0;
    int changedBy = 0;
    std::string changedAt;
    std::optional<double> oldValue;  // empty for the first mark
    double newValue = 0;
    std::string reason;
};

struct AuditFilter {
    std::optional<int> submissionId;
    std::optional<int> changedBy;
    std::optional<std::string> from;  // inclusive, "YYYY-MM-DD HH:MM:SS"
    std::optional<std::string> to;    // inclusive
};

}  // namespace lms
