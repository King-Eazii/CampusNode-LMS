#include "services/ModerationService.h"

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "common/Validation.h"

namespace lms {

std::vector<SubmissionItem> ModerationService::pendingFor(const HeadOfDepartment& hod) {
    access_.require(hod, Permission::ModerateMarks);
    std::vector<SubmissionItem> items;
    for (const Submission& s : submissions_.findByDepartment(hod.departmentId(), SubmissionStatus::Marked)) {
        items.push_back(details_.describe(s));
    }
    return items;
}

std::vector<SubmissionItem> ModerationService::departmentMarks(const HeadOfDepartment& hod) {
    access_.require(hod, Permission::ModerateMarks);
    std::vector<SubmissionItem> items;
    for (SubmissionStatus status : {SubmissionStatus::Marked, SubmissionStatus::Moderated, SubmissionStatus::Released}) {
        for (const Submission& s : submissions_.findByDepartment(hod.departmentId(), status)) items.push_back(details_.describe(s));
    }
    return items;
}

Submission ModerationService::loadInDepartment(const HeadOfDepartment& hod, int submissionId, double* maxMark) {
    auto submission = submissions_.findById(submissionId);
    if (!submission) throw NotFoundException("No submission has that number.");
    const auto assignment = assignments_.findById(submission->assignmentId);
    const auto course = assignment ? courses_.findById(assignment->courseId) : std::nullopt;
    // A head of department only works on their own department (BR6).
    if (!course || course->departmentId != hod.departmentId()) throw AccessDeniedException("That submission is not in your department.");
    if (maxMark != nullptr) *maxMark = assignment->maxMark;
    return *submission;
}

void ModerationService::changeMark(const HeadOfDepartment& hod, int submissionId, double newMark, const std::string& reason) {
    access_.require(hod, Permission::ModerateMarks);
    double maxMark = 0;
    Submission submission = loadInDepartment(hod, submissionId, &maxMark);
    if (submission.status == SubmissionStatus::Submitted || submission.status == SubmissionStatus::Late) {
        throw ValidationException("This submission has not been marked yet.");
    }
    validation::requireMark(newMark, maxMark);
    const std::string text = validation::requireText(reason, "The reason", 500);
    const double oldMark = submission.finalMark.value_or(0.0);
    if (oldMark == newMark) throw ValidationException("The new mark is the same as the current mark.");

    submission.finalMark = newMark;
    Transaction tx(db_);  // the change and its audit entry are saved together or not at all (NFR6)
    submissions_.update(submission);
    audit_.record(AuditEntry{0, submission.id, hod.id(), datetime::now(), oldMark, newMark, text});
    tx.commit();
}

int ModerationService::approve(const HeadOfDepartment& hod, const std::vector<int>& submissionIds, bool release) {
    access_.require(hod, Permission::ModerateMarks);
    if (release) access_.require(hod, Permission::ReleaseMarks);
    if (submissionIds.empty()) throw ValidationException("Choose at least one submission.");
    Transaction tx(db_);  // all chosen submissions change together, or none does
    for (int id : submissionIds) {
        Submission s = loadInDepartment(hod, id);
        s.transitionTo(SubmissionStatus::Moderated);
        if (release) s.transitionTo(SubmissionStatus::Released);
        submissions_.update(s);
    }
    tx.commit();
    return static_cast<int>(submissionIds.size());
}

int ModerationService::releaseApproved(const HeadOfDepartment& hod, const std::vector<int>& submissionIds) {
    access_.require(hod, Permission::ReleaseMarks);
    if (submissionIds.empty()) throw ValidationException("Choose at least one submission.");
    Transaction tx(db_);
    for (int id : submissionIds) {
        Submission s = loadInDepartment(hod, id);
        s.transitionTo(SubmissionStatus::Released);
        submissions_.update(s);
    }
    tx.commit();
    return static_cast<int>(submissionIds.size());
}

}  // namespace lms
