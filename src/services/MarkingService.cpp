#include "services/MarkingService.h"

#include <algorithm>

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "common/Validation.h"
#include "services/Deadlines.h"

namespace lms {
namespace {
bool teaches(ICourseRepository& courses, int lecturerId, int courseId) {
    const auto list = courses.findByLecturer(lecturerId);
    return std::any_of(list.begin(), list.end(), [&](const Course& c) { return c.id == courseId; });
}
}  // namespace

std::vector<SubmissionItem> MarkingService::listForMarking(const User& lecturer, int assignmentId) {
    access_.require(lecturer, Permission::MarkSubmission);
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundException("No assignment has that number.");
    if (!teaches(courses_, lecturer.id(), assignment->courseId)) throw AccessDeniedException("You do not teach that course.");
    std::vector<SubmissionItem> items;
    for (const Submission& s : submissions_.findByAssignment(assignmentId)) {
        if (s.status == SubmissionStatus::Submitted || s.status == SubmissionStatus::Late) items.push_back(details_.describe(s));
    }
    return items;
}

void MarkingService::markSubmission(const User& lecturer, int submissionId, double mark, const std::string& feedback) {
    access_.require(lecturer, Permission::MarkSubmission);
    auto submission = submissions_.findById(submissionId);
    if (!submission) throw NotFoundException("No submission has that number.");
    const auto assignment = assignments_.findById(submission->assignmentId);
    if (!assignment) throw NotFoundException("The assignment no longer exists.");
    if (!teaches(courses_, lecturer.id(), assignment->courseId)) throw AccessDeniedException("You do not teach that course.");
    if (submission->status != SubmissionStatus::Submitted && submission->status != SubmissionStatus::Late) {
        throw ValidationException("This submission has already been marked. The Head of Department can change the mark.");
    }
    validation::requireMark(mark, assignment->maxMark);
    const std::string text = validation::requireText(feedback, "The feedback", 2000);

    const std::string deadline = lms::effectiveDeadline(*assignment, assignments_.findExtension(assignment->id, submission->studentId));
    const double penalty = policy_.calculatePenalty(assignment->maxMark, submission->latestVersion().submittedAt, deadline);
    const double finalMark = std::max(0.0, mark - penalty);

    submission->rawMark = mark;
    submission->penalty = penalty;
    submission->finalMark = finalMark;
    submission->feedback = text;
    submission->transitionTo(SubmissionStatus::Marked);

    Transaction tx(db_);  // the mark and its audit entry are saved together or not at all (NFR6)
    submissions_.update(*submission);
    audit_.record(AuditEntry{0, submission->id, lecturer.id(), datetime::now(), std::nullopt, finalMark, "Initial mark"});
    tx.commit();
}

}  // namespace lms
