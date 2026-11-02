#pragma once
#include "data/DatabaseManager.h"
#include "data/IRepositories.h"
#include "services/AccessControl.h"
#include "services/AuditService.h"
#include "services/Views.h"

namespace lms {

class ModerationService {
public:
    ModerationService(ISubmissionRepository& submissions, IAssignmentRepository& assignments, ICourseRepository& courses,
                      AuditService& audit, SubmissionDetails& details, const AccessControl& access, DatabaseManager& db)
        : submissions_(submissions), assignments_(assignments), courses_(courses), audit_(audit), details_(details),
          access_(access), db_(db) {}

    /// Marked submissions of the HoD's own department that wait for moderation.
    std::vector<SubmissionItem> pendingFor(const HeadOfDepartment& hod);
    /// Every submission of the department that has a mark, with its status (FR11a).
    std::vector<SubmissionItem> departmentMarks(const HeadOfDepartment& hod);
    /// Changes a mark with a reason and writes an audit entry. Works before and after release (A9).
    void changeMark(const HeadOfDepartment& hod, int submissionId, double newMark, const std::string& reason);
    /// Approves marked submissions and optionally releases them to the students. Returns how many were changed.
    int approve(const HeadOfDepartment& hod, const std::vector<int>& submissionIds, bool release);
    /// Releases submissions that were approved earlier.
    int releaseApproved(const HeadOfDepartment& hod, const std::vector<int>& submissionIds);

private:
    Submission loadInDepartment(const HeadOfDepartment& hod, int submissionId, double* maxMark = nullptr);
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    AuditService& audit_;
    SubmissionDetails& details_;
    const AccessControl& access_;
    DatabaseManager& db_;
};

}  // namespace lms
