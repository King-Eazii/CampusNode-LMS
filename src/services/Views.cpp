#include "services/Views.h"

#include "common/Exceptions.h"

namespace lms {

SubmissionItem SubmissionDetails::describe(const Submission& submission) {
    SubmissionItem item;
    item.submission = submission;
    const auto assignment = assignments_.findById(submission.assignmentId);
    if (!assignment) throw NotFoundException("The assignment of this submission no longer exists.");
    item.assignmentTitle = assignment->title;
    item.maxMark = assignment->maxMark;
    if (const auto course = courses_.findById(assignment->courseId)) item.courseCode = course->code;
    if (const auto student = users_.findById(submission.studentId)) {
        item.studentName = student->fullName();
        if (student->data().studentNumber) item.studentNumber = *student->data().studentNumber;
    }
    return item;
}

}  // namespace lms
