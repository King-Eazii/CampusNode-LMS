#include "domain/Types.h"

#include "common/Exceptions.h"

namespace lms {

std::string toString(Role role) {
    switch (role) {
        case Role::Admin: return "ADMIN";
        case Role::Lecturer: return "LECTURER";
        case Role::Hod: return "HOD";
        case Role::Student: return "STUDENT";
    }
    return "";
}

Role roleFromString(const std::string& text) {
    if (text == "ADMIN") return Role::Admin;
    if (text == "LECTURER") return Role::Lecturer;
    if (text == "HOD") return Role::Hod;
    if (text == "STUDENT") return Role::Student;
    throw ValidationException("Unknown role: " + text);
}

std::string toString(SubmissionStatus status) {
    switch (status) {
        case SubmissionStatus::Submitted: return "SUBMITTED";
        case SubmissionStatus::Late: return "LATE";
        case SubmissionStatus::Marked: return "MARKED";
        case SubmissionStatus::Moderated: return "MODERATED";
        case SubmissionStatus::Released: return "RELEASED";
    }
    return "";
}

SubmissionStatus statusFromString(const std::string& text) {
    if (text == "SUBMITTED") return SubmissionStatus::Submitted;
    if (text == "LATE") return SubmissionStatus::Late;
    if (text == "MARKED") return SubmissionStatus::Marked;
    if (text == "MODERATED") return SubmissionStatus::Moderated;
    if (text == "RELEASED") return SubmissionStatus::Released;
    throw ValidationException("Unknown status: " + text);
}

}  // namespace lms
