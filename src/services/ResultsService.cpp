#include "services/ResultsService.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "common/Exceptions.h"
#include "common/Validation.h"

namespace lms {
namespace {
std::string csvCell(const std::string& text) {
    std::string out = "\"";
    for (char c : text) out += (c == '"') ? "\"\"" : std::string(1, c);
    return out + "\"";
}
}  // namespace

std::vector<SubmissionItem> ResultsService::viewReleasedMarks(const User& student) {
    access_.require(student, Permission::ViewResults);
    std::vector<SubmissionItem> items;
    for (const Submission& s : submissions_.findByStudent(student.id())) {
        if (s.status == SubmissionStatus::Released) items.push_back(details_.describe(s));
    }
    return items;
}

void ResultsService::exportCsv(const User& lecturer, int courseId, const std::string& path) {
    access_.require(lecturer, Permission::ExportMarks);
    const auto list = courses_.findByLecturer(lecturer.id());
    if (std::none_of(list.begin(), list.end(), [&](const Course& c) { return c.id == courseId; })) {
        throw AccessDeniedException("You do not teach that course.");
    }
    const std::string file = validation::requireText(path, "The file name", 260);
    if (std::filesystem::exists(file)) throw ValidationException("That file already exists. Choose a new file name.");
    std::ofstream out(file);
    if (!out) throw ValidationException("The file could not be created.");
    out << "student_number,student_name,assignment,status,final_mark\n";
    for (const Assignment& a : assignments_.findByCourse(courseId)) {
        for (const Submission& s : submissions_.findByAssignment(a.id)) {
            const SubmissionItem item = details_.describe(s);
            out << csvCell(item.studentNumber) << ',' << csvCell(item.studentName) << ',' << csvCell(a.title) << ','
                << toString(s.status) << ',' << (s.finalMark ? std::to_string(*s.finalMark) : "") << '\n';
        }
    }
}

}  // namespace lms
