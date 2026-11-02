#pragma once
#include "data/IRepositories.h"
#include "services/AccessControl.h"
#include "services/Views.h"

namespace lms {

class ResultsService {
public:
    ResultsService(ISubmissionRepository& submissions, IAssignmentRepository& assignments, ICourseRepository& courses,
                   IUserRepository& users, SubmissionDetails& details, const AccessControl& access)
        : submissions_(submissions), assignments_(assignments), courses_(courses), users_(users), details_(details), access_(access) {}

    /// A student sees a mark and feedback only after the HoD has released it (BR1).
    std::vector<SubmissionItem> viewReleasedMarks(const User& student);
    /// Writes the marks of one course to a new CSV file.
    void exportCsv(const User& lecturer, int courseId, const std::string& path);

private:
    ISubmissionRepository& submissions_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    SubmissionDetails& details_;
    const AccessControl& access_;
};

}  // namespace lms
