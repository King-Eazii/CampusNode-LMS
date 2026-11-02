#pragma once
#include <string>

#include "data/IRepositories.h"

namespace lms {

/// A submission together with the names the console needs to show it.
struct SubmissionItem {
    Submission submission;
    std::string studentName;
    std::string studentNumber;
    std::string courseCode;
    std::string assignmentTitle;
    double maxMark = 0;
};

class SubmissionDetails {
public:
    SubmissionDetails(IUserRepository& users, IAssignmentRepository& assignments, ICourseRepository& courses)
        : users_(users), assignments_(assignments), courses_(courses) {}
    SubmissionItem describe(const Submission& submission);

private:
    IUserRepository& users_;
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
};

}  // namespace lms
