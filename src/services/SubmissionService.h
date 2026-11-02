#pragma once
#include <optional>

#include "data/DatabaseManager.h"
#include "data/IRepositories.h"
#include "services/AccessControl.h"

namespace lms {

class SubmissionService {
public:
    SubmissionService(ISubmissionRepository& submissions, IAssignmentRepository& assignments, ICourseRepository& courses,
                      const AccessControl& access, DatabaseManager& db)
        : submissions_(submissions), assignments_(assignments), courses_(courses), access_(access), db_(db) {}

    /// First submission. Late work is accepted and flagged (BR2).
    Submission submit(const User& student, int assignmentId, const std::string& content);
    /// Replaces the work before the effective deadline. Earlier versions are kept (BR5).
    Submission resubmit(const User& student, int assignmentId, const std::string& content);
    std::optional<Submission> findFor(const User& student, int assignmentId);
    std::string effectiveDeadline(int assignmentId, int studentId);

private:
    Submission store(const User& student, int assignmentId, const std::string& content, bool mustExist);
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    const AccessControl& access_;
    DatabaseManager& db_;
};

}  // namespace lms
