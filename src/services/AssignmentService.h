#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "data/DatabaseManager.h"
#include "data/IRepositories.h"
#include "services/AccessControl.h"

namespace lms {

/// One line of a student's assignment list.
struct AssignmentView {
    Assignment assignment;
    std::string courseCode;
    std::string effectiveDeadline;
    bool extended = false;
    std::optional<SubmissionStatus> status;  // empty when the student has not submitted
};

class AssignmentService {
public:
    AssignmentService(IAssignmentRepository& assignments, ICourseRepository& courses, IUserRepository& users,
                      ISubmissionRepository& submissions, const AccessControl& access, DatabaseManager& db)
        : assignments_(assignments), courses_(courses), users_(users), submissions_(submissions), access_(access), db_(db) {}

    int createAssignment(const User& lecturer, int courseId, const std::string& title, const std::string& description,
                         const std::string& deadline, double maxMark);
    void grantExtension(const User& lecturer, int assignmentId, int studentId, const std::string& newDeadline,
                        const std::string& reason);
    std::vector<AssignmentView> listForStudent(const User& student);
    std::vector<AssignmentView> listForLecturer(const User& lecturer);
    std::vector<Course> coursesOf(const User& lecturer);
    std::vector<std::unique_ptr<User>> studentsOfCourse(const User& lecturer, int courseId);

private:
    void requireTeaches(const User& lecturer, int courseId);
    IAssignmentRepository& assignments_;
    ICourseRepository& courses_;
    IUserRepository& users_;
    ISubmissionRepository& submissions_;
    const AccessControl& access_;
    DatabaseManager& db_;
};

}  // namespace lms
