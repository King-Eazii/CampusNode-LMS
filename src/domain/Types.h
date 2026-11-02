#pragma once
#include <string>

namespace lms {

enum class Role { Admin, Lecturer, Hod, Student };

enum class Permission {
    ManageUsers, ManageCourses, EnrolStudents, BackupRestore, ViewAuditLog, ChangePassword,
    CreateAssignment, GrantExtension, MarkSubmission, ExportMarks,
    ModerateMarks, ReleaseMarks,
    ViewAssignments, SubmitWork, ViewResults
};

enum class SubmissionStatus { Submitted, Late, Marked, Moderated, Released };

std::string toString(Role role);
Role roleFromString(const std::string& text);
std::string toString(SubmissionStatus status);
SubmissionStatus statusFromString(const std::string& text);

}  // namespace lms
