#include "ui/Menus.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "common/DateTime.h"
#include "common/Exceptions.h"
#include "ui/Console.h"
#include "ui/InputValidator.h"

namespace lms {
using console::fit;
using console::number;
using console::print;

namespace {

std::string deadlineText(const AssignmentView& v) { return v.effectiveDeadline + (v.extended ? " (extended)" : ""); }

void printAssignments(const std::vector<AssignmentView>& list, bool showStatus) {
    if (list.empty()) { print("  (none)"); return; }
    print("  " + fit("No", 5) + fit("Course", 10) + fit("Assignment", 28) + fit("Deadline", 32) + fit("Max", 6) + (showStatus ? "Status" : ""));
    for (const AssignmentView& v : list) {
        print("  " + fit(std::to_string(v.assignment.id), 5) + fit(v.courseCode, 10) + fit(v.assignment.title, 28) +
              fit(deadlineText(v), 32) + fit(number(v.assignment.maxMark), 6) +
              (showStatus ? (v.status ? toString(*v.status) : std::string("not submitted")) : ""));
    }
}

void printItems(const std::vector<SubmissionItem>& items) {
    if (items.empty()) { print("  (none)"); return; }
    print("  " + fit("No", 5) + fit("Student", 22) + fit("Course", 9) + fit("Assignment", 20) + fit("Status", 11) +
          fit("Raw", 6) + fit("Penalty", 8) + fit("Final", 6) + "Late?");
    for (const SubmissionItem& i : items) {
        const Submission& s = i.submission;
        print("  " + fit(std::to_string(s.id), 5) + fit(i.studentName + " (" + i.studentNumber + ")", 22) + fit(i.courseCode, 9) +
              fit(i.assignmentTitle, 20) + fit(toString(s.status), 11) + fit(s.rawMark ? number(*s.rawMark) : "-", 6) +
              fit(number(s.penalty), 8) + fit(s.finalMark ? number(*s.finalMark) + "/" + number(i.maxMark) : "-", 6) +
              (s.penalty > 0 ? "yes" : ""));
    }
}

void printCourses(const std::vector<Course>& courses) {
    if (courses.empty()) { print("  (none)"); return; }
    for (const Course& c : courses) print("  " + fit(std::to_string(c.id), 5) + fit(c.code, 10) + c.title);
}

void printUsers(const std::vector<std::unique_ptr<User>>& users, bool onlyStudents = false, bool onlyLecturers = false) {
    for (const auto& u : users) {
        if (onlyStudents && u->getRole() != Role::Student) continue;
        if (onlyLecturers && u->getRole() != Role::Lecturer) continue;
        print("  " + fit(std::to_string(u->id()), 5) + fit(u->username(), 16) + fit(u->fullName(), 26) + fit(toString(u->getRole()), 10) +
              (u->isActive() ? "" : "(deactivated)"));
    }
}

std::string readWork() {
    const std::string line = input::text("Type your answer, or the path of a text file: ", 20000);
    std::error_code ec;
    if (std::filesystem::is_regular_file(line, ec)) {
        std::ifstream in(line, std::ios::binary);
        std::ostringstream out;
        out << in.rdbuf();
        if (out.str().empty()) throw ValidationException("That file is empty.");
        return out.str();
    }
    return line;
}

}  // namespace

// ---------------------------------------------------------------- MenuBase
void MenuBase::loop(const std::string& title, const std::vector<Item>& items) {
    while (!loggedOut_) {
        console::heading(title);
        for (std::size_t i = 0; i < items.size(); ++i) print("  " + std::to_string(i + 1) + ". " + items[i].label);
        print("  0. Log out");
        const int choice = input::integer("Choose: ", 0, static_cast<int>(items.size()));
        if (choice == 0) { loggedOut_ = true; return; }
        try {
            items[static_cast<std::size_t>(choice) - 1].action();
        } catch (const AppException& e) {
            console::error(e.what());
        } catch (const console::InputClosed&) {
            throw;
        } catch (const std::exception& e) {
            console::error(std::string("Something went wrong and the action was cancelled. (") + e.what() + ")");
        }
    }
}

void MenuBase::changePassword(const User& user, Services& services) {
    const std::string oldPw = console::readSecret("Current password: ");
    const std::string newPw = console::readSecret("New password (at least 8 characters): ");
    if (newPw != console::readSecret("Repeat the new password: ")) throw ValidationException("The two passwords are different.");
    services.auth.changePassword(user, oldPw, newPw);
    print("Password changed. Please log in again.");
    loggedOut_ = true;
}

void MenuBase::showAuditLog(Services& services) {
    AuditFilter filter;
    const int id = input::integer("Submission number (0 for all): ", 0, 1000000000);
    if (id > 0) filter.submissionId = id;
    const auto entries = services.audit.find(filter);
    if (entries.empty()) { print("  (no entries)"); return; }
    print("  " + fit("Submission", 11) + fit("By (user)", 10) + fit("When", 21) + fit("Old", 7) + fit("New", 7) + "Reason");
    for (const AuditEntry& e : entries) {
        print("  " + fit(std::to_string(e.submissionId), 11) + fit(std::to_string(e.changedBy), 10) + fit(e.changedAt, 21) +
              fit(e.oldValue ? number(*e.oldValue) : "-", 7) + fit(number(e.newValue), 7) + e.reason);
    }
}

// ---------------------------------------------------------------- AdminMenu
void AdminMenu::run() {
    loop("Administrator: " + user_.fullName(), {
        {"Create a user", [&] {
            NewUser n;
            print("Roles: 1 Administrator, 2 Lecturer, 3 Head of Department, 4 Student");
            const int r = input::integer("Role: ", 1, 4);
            n.role = r == 1 ? Role::Admin : r == 2 ? Role::Lecturer : r == 3 ? Role::Hod : Role::Student;
            n.username = input::text("Username: ", 30);
            n.fullName = input::text("Full name: ", 100);
            if (n.role == Role::Student) n.studentNumber = input::text("Student number: ", 30);
            if (n.role == Role::Hod) {
                for (const Department& d : s_.admin.departments(user_)) print("  " + fit(std::to_string(d.id), 5) + d.name);
                n.departmentId = input::integer("Department number: ", 1, 1000000000);
            }
            n.password = console::readSecret("Password (at least 8 characters): ");
            print("Created user number " + std::to_string(s_.admin.createUser(user_, n)) + ".");
        }},
        {"List users", [&] { printUsers(s_.admin.listUsers(user_)); }},
        {"Activate or deactivate a user", [&] {
            printUsers(s_.admin.listUsers(user_));
            const int id = input::integer("User number: ", 1, 1000000000);
            const bool active = input::yesNo("Activate this user? (n deactivates)");
            s_.admin.setActive(user_, id, active);
            print("Done.");
        }},
        {"Create a department", [&] {
            print("Created department number " + std::to_string(s_.admin.createDepartment(user_, input::text("Department name: ", 100))) + ".");
        }},
        {"Create a course", [&] {
            for (const Department& d : s_.admin.departments(user_)) print("  " + fit(std::to_string(d.id), 5) + d.name);
            const std::string code = input::text("Course code: ", 20);
            const std::string title = input::text("Course title: ", 100);
            const int dept = input::integer("Department number: ", 1, 1000000000);
            print("Created course number " + std::to_string(s_.admin.createCourse(user_, code, title, dept)) + ".");
        }},
        {"Assign a lecturer to a course", [&] {
            printCourses(s_.admin.courses(user_));
            const int course = input::integer("Course number: ", 1, 1000000000);
            printUsers(s_.admin.listUsers(user_), false, true);
            s_.admin.assignLecturer(user_, course, input::integer("Lecturer number: ", 1, 1000000000));
            print("Done.");
        }},
        {"Enrol a student in a course", [&] {
            printCourses(s_.admin.courses(user_));
            const int course = input::integer("Course number: ", 1, 1000000000);
            printUsers(s_.admin.listUsers(user_), true, false);
            s_.admin.enrol(user_, input::integer("Student number: ", 1, 1000000000), course);
            print("Done.");
        }},
        {"View the audit log", [&] { showAuditLog(s_); }},
        {"Back up the database", [&] {
            s_.admin.backup(user_, input::text("New backup file name: ", 260));
            print("Backup saved.");
        }},
        {"Restore the database from a backup", [&] {
            if (!input::yesNo("This replaces all current data. Continue?")) return;
            s_.admin.restore(user_, input::text("Backup file name: ", 260));
            print("Database restored. Please log in again.");
            loggedOut_ = true;
        }},
        {"Change my password", [&] { changePassword(user_, s_); }},
    });
}

// ---------------------------------------------------------------- LecturerMenu
void LecturerMenu::run() {
    loop("Lecturer: " + user_.fullName(), {
        {"My courses and assignments", [&] {
            print("Courses:");
            printCourses(s_.assignments.coursesOf(user_));
            print("Assignments:");
            printAssignments(s_.assignments.listForLecturer(user_), false);
        }},
        {"Create an assignment", [&] {
            printCourses(s_.assignments.coursesOf(user_));
            const int course = input::integer("Course number: ", 1, 1000000000);
            const std::string title = input::text("Title: ", 100);
            const std::string description = input::text("Description (optional): ", 2000, true);
            const std::string deadline = input::dateTime("Deadline");
            const double maxMark = input::number("Maximum mark: ", 0.5, 1000);
            print("Created assignment number " + std::to_string(s_.assignments.createAssignment(user_, course, title, description, deadline, maxMark)) + ".");
        }},
        {"Grant an extension to one student", [&] {
            printCourses(s_.assignments.coursesOf(user_));
            const int course = input::integer("Course number: ", 1, 1000000000);
            const auto students = s_.assignments.studentsOfCourse(user_, course);
            printUsers(students);
            const int student = input::integer("Student number: ", 1, 1000000000);
            std::vector<AssignmentView> inCourse;
            for (const AssignmentView& v : s_.assignments.listForLecturer(user_)) if (v.assignment.courseId == course) inCourse.push_back(v);
            printAssignments(inCourse, false);
            const int assignment = input::integer("Assignment number: ", 1, 1000000000);
            const std::string deadline = input::dateTime("New deadline");
            const std::string reason = input::text("Reason: ", 500);
            if (!input::yesNo("Grant this extension?")) return;
            s_.assignments.grantExtension(user_, assignment, student, deadline, reason);
            print("Extension granted.");
        }},
        {"Mark a submission", [&] {
            printAssignments(s_.assignments.listForLecturer(user_), false);
            const int assignment = input::integer("Assignment number: ", 1, 1000000000);
            const auto waiting = s_.marking.listForMarking(user_, assignment);
            printItems(waiting);
            if (waiting.empty()) return;
            const int id = input::integer("Submission number: ", 1, 1000000000);
            for (const SubmissionItem& i : waiting) {
                if (i.submission.id == id) {
                    print("Latest version (" + i.submission.latestVersion().submittedAt + "):");
                    print(i.submission.latestVersion().content.substr(0, 600));
                    const double mark = input::number("Mark (0 to " + number(i.maxMark) + "): ", 0, i.maxMark);
                    const std::string feedback = input::text("Written feedback: ", 2000);
                    s_.marking.markSubmission(user_, id, mark, feedback);
                    print("Marked. The mark goes to the Head of Department for moderation.");
                    return;
                }
            }
            throw NotFoundException("That submission is not waiting for a mark.");
        }},
        {"Export the marks of a course to a CSV file", [&] {
            printCourses(s_.assignments.coursesOf(user_));
            const int course = input::integer("Course number: ", 1, 1000000000);
            s_.results.exportCsv(user_, course, input::text("New file name (for example marks.csv): ", 260));
            print("File written.");
        }},
        {"Change my password", [&] { changePassword(user_, s_); }},
    });
}

// ---------------------------------------------------------------- HodMenu
void HodMenu::run() {
    loop("Head of Department: " + user_.fullName(), {
        {"Marks waiting for moderation", [&] { printItems(s_.moderation.pendingFor(user_)); }},
        {"All marks in my department", [&] { printItems(s_.moderation.departmentMarks(user_)); }},
        {"Change a mark", [&] {
            const auto items = s_.moderation.departmentMarks(user_);
            printItems(items);
            const int id = input::integer("Submission number: ", 1, 1000000000);
            double maxMark = 1000;
            for (const SubmissionItem& i : items) if (i.submission.id == id) maxMark = i.maxMark;
            const double mark = input::number("New mark (0 to " + number(maxMark) + "): ", 0, maxMark);
            const std::string reason = input::text("Reason: ", 500);
            if (!input::yesNo("Save this change?")) return;
            s_.moderation.changeMark(user_, id, mark, reason);
            print("Mark changed and recorded in the audit log.");
        }},
        {"Approve and release marks", [&] {
            printItems(s_.moderation.pendingFor(user_));
            const auto ids = input::idList("Submission numbers to approve and release: ");
            if (!input::yesNo("Release " + std::to_string(ids.size()) + " mark(s) to the students?")) return;
            print(std::to_string(s_.moderation.approve(user_, ids, true)) + " mark(s) released.");
        }},
        {"Approve only (release later)", [&] {
            printItems(s_.moderation.pendingFor(user_));
            const auto ids = input::idList("Submission numbers to approve: ");
            print(std::to_string(s_.moderation.approve(user_, ids, false)) + " mark(s) approved.");
        }},
        {"Release approved marks", [&] {
            std::vector<SubmissionItem> approved;
            for (const SubmissionItem& i : s_.moderation.departmentMarks(user_)) if (i.submission.status == SubmissionStatus::Moderated) approved.push_back(i);
            printItems(approved);
            const auto ids = input::idList("Submission numbers to release: ");
            print(std::to_string(s_.moderation.releaseApproved(user_, ids)) + " mark(s) released.");
        }},
        {"View the audit log", [&] { showAuditLog(s_); }},
        {"Change my password", [&] { changePassword(user_, s_); }},
    });
}

// ---------------------------------------------------------------- StudentMenu
void StudentMenu::run() {
    loop("Student: " + user_.fullName(), {
        {"My assignments", [&] { printAssignments(s_.assignments.listForStudent(user_), true); }},
        {"Submit or resubmit work", [&] {
            const auto list = s_.assignments.listForStudent(user_);
            printAssignments(list, true);
            const int id = input::integer("Assignment number: ", 1, 1000000000);
            const AssignmentView* chosen = nullptr;
            for (const AssignmentView& v : list) if (v.assignment.id == id) chosen = &v;
            if (chosen == nullptr) throw NotFoundException("That assignment is not in your courses.");
            const auto existing = s_.submissions.findFor(user_, id);
            const bool late = datetime::toSeconds(datetime::now()) > datetime::toSeconds(chosen->effectiveDeadline);
            if (existing && !input::yesNo("You already submitted this. Replace it with new work? (the old version is kept)")) return;
            if (late && !existing) {
                print("The deadline has passed. Your work will be accepted but marked as late.");
                print("A penalty of 10% of the maximum mark applies for each day or part of a day late, up to 3 days. After that the mark is zero.");
                if (!input::yesNo("Submit anyway?")) return;
            }
            const std::string work = readWork();
            const Submission result = existing ? s_.submissions.resubmit(user_, id, work) : s_.submissions.submit(user_, id, work);
            print("Saved as version " + std::to_string(result.versions.size()) + " at " + result.latestVersion().submittedAt + ". Status: " + toString(result.status) + ".");
        }},
        {"My marks and feedback", [&] {
            const auto items = s_.results.viewReleasedMarks(user_);
            if (items.empty()) { print("  No marks have been released yet."); return; }
            for (const SubmissionItem& i : items) {
                print("  " + i.courseCode + " - " + i.assignmentTitle + ": " + number(*i.submission.finalMark) + " / " + number(i.maxMark) +
                      (i.submission.penalty > 0 ? "  (late penalty " + number(i.submission.penalty) + ")" : ""));
                print("    Feedback: " + i.submission.feedback);
            }
        }},
        {"Change my password", [&] { changePassword(user_, s_); }},
    });
}

}  // namespace lms
