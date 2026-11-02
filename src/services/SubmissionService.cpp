#include "services/SubmissionService.h"

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "services/Deadlines.h"

namespace lms {
namespace {
constexpr std::size_t kMaxContentBytes = 10 * 1024 * 1024;  // 10 MB (SRS: to be confirmed)
}

std::string SubmissionService::effectiveDeadline(int assignmentId, int studentId) {
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundException("No assignment has that number.");
    return lms::effectiveDeadline(*assignment, assignments_.findExtension(assignmentId, studentId));
}

std::optional<Submission> SubmissionService::findFor(const User& student, int assignmentId) {
    access_.require(student, Permission::SubmitWork);
    for (const Submission& s : submissions_.findByStudent(student.id())) {
        if (s.assignmentId == assignmentId) return s;
    }
    return std::nullopt;
}

Submission SubmissionService::submit(const User& student, int assignmentId, const std::string& content) {
    return store(student, assignmentId, content, false);
}

Submission SubmissionService::resubmit(const User& student, int assignmentId, const std::string& content) {
    return store(student, assignmentId, content, true);
}

Submission SubmissionService::store(const User& student, int assignmentId, const std::string& content, bool mustExist) {
    access_.require(student, Permission::SubmitWork);
    if (content.empty()) throw ValidationException("The work cannot be empty.");
    if (content.size() > kMaxContentBytes) throw ValidationException("The work is too large (maximum 10 MB).");
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundException("No assignment has that number.");
    if (!courses_.isEnrolled(student.id(), assignment->courseId)) throw AccessDeniedException("You are not enrolled in this course.");

    const std::string deadline = effectiveDeadline(assignmentId, student.id());
    const std::string now = datetime::now();
    const bool late = datetime::toSeconds(now) > datetime::toSeconds(deadline);
    std::optional<Submission> existing = findFor(student, assignmentId);

    if (existing) {
        if (existing->status != SubmissionStatus::Submitted && existing->status != SubmissionStatus::Late) {
            throw ValidationException("This work has already been marked, so it cannot be replaced.");  // E3
        }
        if (late) throw ValidationException("The deadline has passed, so you cannot resubmit.");  // E2
        Submission updated = *existing;
        updated.versions.push_back(SubmissionVersion{static_cast<int>(updated.versions.size()) + 1, content, now});
        Transaction tx(db_);
        submissions_.update(updated);
        tx.commit();
        return updated;
    }
    if (mustExist) throw NotFoundException("You have not submitted this assignment yet.");

    Submission fresh;
    fresh.assignmentId = assignmentId;
    fresh.studentId = student.id();
    fresh.status = late ? SubmissionStatus::Late : SubmissionStatus::Submitted;
    fresh.versions.push_back(SubmissionVersion{1, content, now});
    Transaction tx(db_);
    fresh.id = submissions_.save(fresh);
    tx.commit();
    return fresh;
}

}  // namespace lms
