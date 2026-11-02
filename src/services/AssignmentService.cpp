#include "services/AssignmentService.h"

#include <algorithm>

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "common/Validation.h"
#include "services/Deadlines.h"

namespace lms {

void AssignmentService::requireTeaches(const User& lecturer, int courseId) {
    const auto courses = courses_.findByLecturer(lecturer.id());
    const bool teaches = std::any_of(courses.begin(), courses.end(), [&](const Course& c) { return c.id == courseId; });
    if (!teaches) throw AccessDeniedException("You do not teach that course.");
}

std::vector<Course> AssignmentService::coursesOf(const User& lecturer) {
    access_.require(lecturer, Permission::CreateAssignment);
    return courses_.findByLecturer(lecturer.id());
}

std::vector<std::unique_ptr<User>> AssignmentService::studentsOfCourse(const User& lecturer, int courseId) {
    access_.require(lecturer, Permission::GrantExtension);
    requireTeaches(lecturer, courseId);
    return users_.findStudentsOfCourse(courseId);
}

int AssignmentService::createAssignment(const User& lecturer, int courseId, const std::string& title,
                                        const std::string& description, const std::string& deadline, double maxMark) {
    access_.require(lecturer, Permission::CreateAssignment);
    requireTeaches(lecturer, courseId);
    Assignment a;
    a.courseId = courseId;
    a.title = validation::requireText(title, "The title", 100);
    a.description = validation::trim(description);
    if (a.description.size() > 2000) throw ValidationException("The description is too long (maximum 2000 characters).");
    a.deadline = datetime::normalise(deadline);
    if (!(maxMark > 0) || maxMark > 1000) throw ValidationException("The maximum mark must be more than 0 and at most 1000.");
    a.maxMark = maxMark;
    return assignments_.save(a);
}

void AssignmentService::grantExtension(const User& lecturer, int assignmentId, int studentId,
                                       const std::string& newDeadline, const std::string& reason) {
    access_.require(lecturer, Permission::GrantExtension);
    const auto assignment = assignments_.findById(assignmentId);
    if (!assignment) throw NotFoundException("No assignment has that number.");
    requireTeaches(lecturer, assignment->courseId);
    if (!courses_.isEnrolled(studentId, assignment->courseId)) throw ValidationException("That student is not enrolled in this course.");
    const std::string text = validation::requireText(reason, "The reason", 500);
    const std::string deadline = datetime::normalise(newDeadline);
    const std::string current = effectiveDeadline(*assignment, assignments_.findExtension(assignmentId, studentId));
    if (datetime::toSeconds(deadline) <= datetime::toSeconds(current)) {
        throw ValidationException("The new deadline must be later than the current deadline (" + current + ").");
    }
    // A submission that has been marked can no longer be given an extension (A8).
    std::optional<Submission> existing;
    for (const Submission& s : submissions_.findByStudent(studentId)) {
        if (s.assignmentId == assignmentId) existing = s;
    }
    if (existing && existing->status != SubmissionStatus::Submitted && existing->status != SubmissionStatus::Late) {
        throw ValidationException("This work has already been marked, so it cannot be extended.");
    }
    Transaction tx(db_);
    assignments_.saveExtension(Extension{0, assignmentId, studentId, lecturer.id(), deadline, text, ""});
    // If the late work is now on time, it is no longer late (UC9, alternative flow A1).
    if (existing && existing->status == SubmissionStatus::Late &&
        datetime::toSeconds(existing->latestVersion().submittedAt) <= datetime::toSeconds(deadline)) {
        existing->transitionTo(SubmissionStatus::Submitted);
        submissions_.update(*existing);
    }
    tx.commit();
}

std::vector<AssignmentView> AssignmentService::listForStudent(const User& student) {
    access_.require(student, Permission::ViewAssignments);
    const auto submissions = submissions_.findByStudent(student.id());
    std::vector<AssignmentView> result;
    for (const Course& course : courses_.findByStudent(student.id())) {
        for (const Assignment& a : assignments_.findByCourse(course.id)) {
            AssignmentView v;
            v.assignment = a;
            v.courseCode = course.code;
            const auto extension = assignments_.findExtension(a.id, student.id());
            v.effectiveDeadline = effectiveDeadline(a, extension);
            v.extended = v.effectiveDeadline != a.deadline;
            for (const Submission& s : submissions) {
                if (s.assignmentId == a.id) v.status = s.status;
            }
            result.push_back(v);
        }
    }
    return result;
}

std::vector<AssignmentView> AssignmentService::listForLecturer(const User& lecturer) {
    access_.require(lecturer, Permission::CreateAssignment);
    std::vector<AssignmentView> result;
    for (const Course& course : courses_.findByLecturer(lecturer.id())) {
        for (const Assignment& a : assignments_.findByCourse(course.id)) {
            AssignmentView v;
            v.assignment = a;
            v.courseCode = course.code;
            v.effectiveDeadline = a.deadline;
            result.push_back(v);
        }
    }
    return result;
}

}  // namespace lms
