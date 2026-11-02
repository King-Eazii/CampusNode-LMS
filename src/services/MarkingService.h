#pragma once
#include "data/DatabaseManager.h"
#include "data/IRepositories.h"
#include "services/AccessControl.h"
#include "services/AuditService.h"
#include "services/LatePenaltyPolicy.h"
#include "services/Views.h"

namespace lms {

class MarkingService {
public:
    MarkingService(ISubmissionRepository& submissions, IAssignmentRepository& assignments, ICourseRepository& courses,
                   const LatePenaltyPolicy& policy, AuditService& audit, SubmissionDetails& details,
                   const AccessControl& access, DatabaseManager& db)
        : submissions_(submissions), assignments_(assignments), courses_(courses), policy_(policy), audit_(audit),
          details_(details), access_(access), db_(db) {}

    /// Submissions of one assignment that are waiting for a mark.
    std::vector<SubmissionItem> listForMarking(const User& lecturer, int assignmentId);
    /// Marks a submission: calculates the late penalty, saves the mark and writes the first audit entry.
    void markSubmission(const User& lecturer, int submissionId, double mark, const std::string& feedback);

private:
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    const LatePenaltyPolicy& policy_;
    AuditService& audit_;
    SubmissionDetails& details_;
    const AccessControl& access_;
    DatabaseManager& db_;
};

}  // namespace lms
