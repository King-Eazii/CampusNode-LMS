#include "data/SqliteRepositories.h"

#include "domain/UserFactory.h"

namespace lms {
namespace {

// ---- users
const char* const kUserColumns =
    "user_id, username, full_name, password_hash, role, student_number, department_id, active";

std::unique_ptr<User> mapUser(Statement& s) {
    UserData d;
    d.id = static_cast<int>(s.columnInt(0));
    d.username = s.columnText(1);
    d.fullName = s.columnText(2);
    d.passwordHash = s.columnText(3);
    d.role = roleFromString(s.columnText(4));
    if (!s.isNull(5)) d.studentNumber = s.columnText(5);
    if (!s.isNull(6)) d.departmentId = static_cast<int>(s.columnInt(6));
    d.active = s.columnInt(7) != 0;
    return UserFactory::create(d);
}

Course mapCourse(Statement& s) {
    return Course{static_cast<int>(s.columnInt(0)), s.columnText(1), s.columnText(2), static_cast<int>(s.columnInt(3))};
}

Assignment mapAssignment(Statement& s) {
    return Assignment{static_cast<int>(s.columnInt(0)), static_cast<int>(s.columnInt(1)), s.columnText(2),
                      s.columnText(3), s.columnText(4), s.columnDouble(5)};
}

const char* const kSubmissionColumns =
    "s.submission_id, s.assignment_id, s.student_id, s.status, s.raw_mark, s.penalty, s.final_mark, s.feedback";

Submission mapSubmission(Statement& s) {
    Submission sub;
    sub.id = static_cast<int>(s.columnInt(0));
    sub.assignmentId = static_cast<int>(s.columnInt(1));
    sub.studentId = static_cast<int>(s.columnInt(2));
    sub.status = statusFromString(s.columnText(3));
    if (!s.isNull(4)) sub.rawMark = s.columnDouble(4);
    sub.penalty = s.columnDouble(5);
    if (!s.isNull(6)) sub.finalMark = s.columnDouble(6);
    sub.feedback = s.columnText(7);
    return sub;
}

}  // namespace

// ================================================================ users
std::unique_ptr<User> SqliteUserRepository::findById(int id) {
    Statement s = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users WHERE user_id = ?");
    s.bindInt(1, id);
    return s.step() ? mapUser(s) : nullptr;
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) {
    Statement s = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users WHERE username = ?");
    s.bindText(1, username);
    return s.step() ? mapUser(s) : nullptr;
}

std::vector<std::unique_ptr<User>> SqliteUserRepository::findAll() {
    Statement s = db_.prepare(std::string("SELECT ") + kUserColumns + " FROM users ORDER BY user_id");
    std::vector<std::unique_ptr<User>> result;
    while (s.step()) result.push_back(mapUser(s));
    return result;
}

std::vector<std::unique_ptr<User>> SqliteUserRepository::findStudentsOfCourse(int courseId) {
    Statement s = db_.prepare(std::string("SELECT ") + kUserColumns +
                              " FROM users WHERE user_id IN (SELECT student_id FROM enrolment WHERE course_id = ?) ORDER BY full_name");
    s.bindInt(1, courseId);
    std::vector<std::unique_ptr<User>> result;
    while (s.step()) result.push_back(mapUser(s));
    return result;
}

int SqliteUserRepository::save(const User& user) {
    const UserData& d = user.data();
    Statement s = db_.prepare(
        "INSERT INTO users (username, full_name, password_hash, role, student_number, department_id, active) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");
    s.bindText(1, d.username);
    s.bindText(2, d.fullName);
    s.bindText(3, d.passwordHash);
    s.bindText(4, toString(d.role));
    s.bindOptionalText(5, d.studentNumber);
    s.bindOptionalInt(6, d.departmentId);
    s.bindInt(7, d.active ? 1 : 0);
    s.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteUserRepository::update(const User& user) {
    Statement s = db_.prepare("UPDATE users SET full_name = ?, password_hash = ?, active = ? WHERE user_id = ?");
    s.bindText(1, user.fullName());
    s.bindText(2, user.passwordHash());
    s.bindInt(3, user.isActive() ? 1 : 0);
    s.bindInt(4, user.id());
    s.run();
}

// ================================================================ courses
std::optional<Course> SqliteCourseRepository::findById(int id) {
    Statement s = db_.prepare("SELECT course_id, code, title, department_id FROM course WHERE course_id = ?");
    s.bindInt(1, id);
    if (!s.step()) return std::nullopt;
    return mapCourse(s);
}

std::vector<Course> SqliteCourseRepository::findAll() {
    Statement s = db_.prepare("SELECT course_id, code, title, department_id FROM course ORDER BY code");
    std::vector<Course> result;
    while (s.step()) result.push_back(mapCourse(s));
    return result;
}

std::vector<Course> SqliteCourseRepository::findByLecturer(int lecturerId) {
    Statement s = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.department_id FROM course c "
        "JOIN course_lecturer cl ON cl.course_id = c.course_id WHERE cl.lecturer_id = ? ORDER BY c.code");
    s.bindInt(1, lecturerId);
    std::vector<Course> result;
    while (s.step()) result.push_back(mapCourse(s));
    return result;
}

std::vector<Course> SqliteCourseRepository::findByStudent(int studentId) {
    Statement s = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.department_id FROM course c "
        "JOIN enrolment e ON e.course_id = c.course_id WHERE e.student_id = ? ORDER BY c.code");
    s.bindInt(1, studentId);
    std::vector<Course> result;
    while (s.step()) result.push_back(mapCourse(s));
    return result;
}

int SqliteCourseRepository::save(const Course& course) {
    Statement s = db_.prepare("INSERT INTO course (code, title, department_id) VALUES (?, ?, ?)");
    s.bindText(1, course.code);
    s.bindText(2, course.title);
    s.bindInt(3, course.departmentId);
    s.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteCourseRepository::assignLecturer(int courseId, int lecturerId) {
    Statement s = db_.prepare("INSERT OR IGNORE INTO course_lecturer (course_id, lecturer_id) VALUES (?, ?)");
    s.bindInt(1, courseId);
    s.bindInt(2, lecturerId);
    s.run();
}

void SqliteCourseRepository::enrol(int studentId, int courseId) {
    Statement s = db_.prepare("INSERT OR IGNORE INTO enrolment (student_id, course_id) VALUES (?, ?)");
    s.bindInt(1, studentId);
    s.bindInt(2, courseId);
    s.run();
}

bool SqliteCourseRepository::isEnrolled(int studentId, int courseId) {
    Statement s = db_.prepare("SELECT 1 FROM enrolment WHERE student_id = ? AND course_id = ?");
    s.bindInt(1, studentId);
    s.bindInt(2, courseId);
    return s.step();
}

std::vector<Department> SqliteCourseRepository::findDepartments() {
    Statement s = db_.prepare("SELECT department_id, name FROM department ORDER BY name");
    std::vector<Department> result;
    while (s.step()) result.push_back(Department{static_cast<int>(s.columnInt(0)), s.columnText(1)});
    return result;
}

std::optional<Department> SqliteCourseRepository::findDepartment(int id) {
    Statement s = db_.prepare("SELECT department_id, name FROM department WHERE department_id = ?");
    s.bindInt(1, id);
    if (!s.step()) return std::nullopt;
    return Department{static_cast<int>(s.columnInt(0)), s.columnText(1)};
}

int SqliteCourseRepository::saveDepartment(const Department& department) {
    Statement s = db_.prepare("INSERT INTO department (name) VALUES (?)");
    s.bindText(1, department.name);
    s.run();
    return static_cast<int>(db_.lastInsertId());
}

// ================================================================ assignments
std::optional<Assignment> SqliteAssignmentRepository::findById(int id) {
    Statement s = db_.prepare(
        "SELECT assignment_id, course_id, title, description, deadline, max_mark FROM assignment WHERE assignment_id = ?");
    s.bindInt(1, id);
    if (!s.step()) return std::nullopt;
    return mapAssignment(s);
}

std::vector<Assignment> SqliteAssignmentRepository::findByCourse(int courseId) {
    Statement s = db_.prepare(
        "SELECT assignment_id, course_id, title, description, deadline, max_mark FROM assignment "
        "WHERE course_id = ? ORDER BY deadline");
    s.bindInt(1, courseId);
    std::vector<Assignment> result;
    while (s.step()) result.push_back(mapAssignment(s));
    return result;
}

int SqliteAssignmentRepository::save(const Assignment& a) {
    Statement s = db_.prepare("INSERT INTO assignment (course_id, title, description, deadline, max_mark) VALUES (?, ?, ?, ?, ?)");
    s.bindInt(1, a.courseId);
    s.bindText(2, a.title);
    s.bindText(3, a.description);
    s.bindText(4, a.deadline);
    s.bindDouble(5, a.maxMark);
    s.run();
    return static_cast<int>(db_.lastInsertId());
}

void SqliteAssignmentRepository::saveExtension(const Extension& e) {
    Statement s = db_.prepare(
        "INSERT INTO extension (assignment_id, student_id, granted_by, new_deadline, reason) VALUES (?, ?, ?, ?, ?)");
    s.bindInt(1, e.assignmentId);
    s.bindInt(2, e.studentId);
    s.bindInt(3, e.grantedBy);
    s.bindText(4, e.newDeadline);
    s.bindText(5, e.reason);
    s.run();
}

std::optional<Extension> SqliteAssignmentRepository::findExtension(int assignmentId, int studentId) {
    Statement s = db_.prepare(
        "SELECT extension_id, assignment_id, student_id, granted_by, new_deadline, reason, granted_at FROM extension "
        "WHERE assignment_id = ? AND student_id = ? ORDER BY new_deadline DESC LIMIT 1");
    s.bindInt(1, assignmentId);
    s.bindInt(2, studentId);
    if (!s.step()) return std::nullopt;
    return Extension{static_cast<int>(s.columnInt(0)), static_cast<int>(s.columnInt(1)), static_cast<int>(s.columnInt(2)),
                     static_cast<int>(s.columnInt(3)), s.columnText(4), s.columnText(5), s.columnText(6)};
}

// ================================================================ submissions
void SqliteSubmissionRepository::loadVersions(Submission& submission) {
    Statement s = db_.prepare(
        "SELECT version_no, content, submitted_at FROM submission_version WHERE submission_id = ? ORDER BY version_no");
    s.bindInt(1, submission.id);
    while (s.step()) {
        submission.versions.push_back(SubmissionVersion{static_cast<int>(s.columnInt(0)), s.columnText(1), s.columnText(2)});
    }
}

std::vector<Submission> SqliteSubmissionRepository::query(const std::string& whereClause,
                                                          const std::function<void(Statement&)>& bind) {
    Statement s = db_.prepare(std::string("SELECT ") + kSubmissionColumns + " FROM submission s " + whereClause);
    bind(s);
    std::vector<Submission> result;
    while (s.step()) result.push_back(mapSubmission(s));
    for (Submission& sub : result) loadVersions(sub);
    return result;
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) {
    auto rows = query("WHERE s.submission_id = ?", [&](Statement& st) { st.bindInt(1, id); });
    if (rows.empty()) return std::nullopt;
    return rows.front();
}

std::vector<Submission> SqliteSubmissionRepository::findByStudent(int studentId) {
    return query("WHERE s.student_id = ? ORDER BY s.submission_id", [&](Statement& st) { st.bindInt(1, studentId); });
}

std::vector<Submission> SqliteSubmissionRepository::findByAssignment(int assignmentId) {
    return query("WHERE s.assignment_id = ? ORDER BY s.submission_id", [&](Statement& st) { st.bindInt(1, assignmentId); });
}

std::vector<Submission> SqliteSubmissionRepository::findByDepartment(int departmentId, SubmissionStatus status) {
    return query(
        "JOIN assignment a ON a.assignment_id = s.assignment_id JOIN course c ON c.course_id = a.course_id "
        "WHERE c.department_id = ? AND s.status = ? ORDER BY s.submission_id",
        [&](Statement& st) {
            st.bindInt(1, departmentId);
            st.bindText(2, toString(status));
        });
}

int SqliteSubmissionRepository::save(const Submission& sub) {
    Transaction tx(db_);
    Statement s = db_.prepare(
        "INSERT INTO submission (assignment_id, student_id, status, raw_mark, penalty, final_mark, feedback) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");
    s.bindInt(1, sub.assignmentId);
    s.bindInt(2, sub.studentId);
    s.bindText(3, toString(sub.status));
    s.bindOptionalDouble(4, sub.rawMark);
    s.bindDouble(5, sub.penalty);
    s.bindOptionalDouble(6, sub.finalMark);
    s.bindText(7, sub.feedback);
    s.run();
    const int id = static_cast<int>(db_.lastInsertId());
    for (const SubmissionVersion& v : sub.versions) {
        Statement vs = db_.prepare("INSERT INTO submission_version (submission_id, version_no, content, submitted_at) VALUES (?, ?, ?, ?)");
        vs.bindInt(1, id);
        vs.bindInt(2, v.versionNo);
        vs.bindText(3, v.content);
        vs.bindText(4, v.submittedAt);
        vs.run();
    }
    tx.commit();
    return id;
}

void SqliteSubmissionRepository::update(const Submission& sub) {
    Transaction tx(db_);
    Statement s = db_.prepare(
        "UPDATE submission SET status = ?, raw_mark = ?, penalty = ?, final_mark = ?, feedback = ? WHERE submission_id = ?");
    s.bindText(1, toString(sub.status));
    s.bindOptionalDouble(2, sub.rawMark);
    s.bindDouble(3, sub.penalty);
    s.bindOptionalDouble(4, sub.finalMark);
    s.bindText(5, sub.feedback);
    s.bindInt(6, sub.id);
    s.run();
    Statement maxv = db_.prepare("SELECT COALESCE(MAX(version_no), 0) FROM submission_version WHERE submission_id = ?");
    maxv.bindInt(1, sub.id);
    maxv.step();
    const long long stored = maxv.columnInt(0);
    for (const SubmissionVersion& v : sub.versions) {
        if (v.versionNo <= stored) continue;
        Statement vs = db_.prepare("INSERT INTO submission_version (submission_id, version_no, content, submitted_at) VALUES (?, ?, ?, ?)");
        vs.bindInt(1, sub.id);
        vs.bindInt(2, v.versionNo);
        vs.bindText(3, v.content);
        vs.bindText(4, v.submittedAt);
        vs.run();
    }
    tx.commit();
}

// ================================================================ audit
void SqliteAuditRepository::add(const AuditEntry& e) {
    Statement s = db_.prepare("INSERT INTO audit_entry (submission_id, changed_by, changed_at, old_value, new_value, reason) VALUES (?, ?, ?, ?, ?, ?)");
    s.bindInt(1, e.submissionId);
    s.bindInt(2, e.changedBy);
    s.bindText(3, e.changedAt);
    s.bindOptionalDouble(4, e.oldValue);
    s.bindDouble(5, e.newValue);
    s.bindText(6, e.reason);
    s.run();
}

std::vector<AuditEntry> SqliteAuditRepository::find(const AuditFilter& f) {
    // Only fixed SQL fragments are added here. Every value is bound as a parameter.
    std::string sql = "SELECT audit_id, submission_id, changed_by, changed_at, old_value, new_value, reason FROM audit_entry WHERE 1 = 1";
    if (f.submissionId) sql += " AND submission_id = ?";
    if (f.changedBy) sql += " AND changed_by = ?";
    if (f.from) sql += " AND changed_at >= ?";
    if (f.to) sql += " AND changed_at <= ?";
    sql += " ORDER BY audit_id";
    Statement s = db_.prepare(sql);
    int i = 1;
    if (f.submissionId) s.bindInt(i++, *f.submissionId);
    if (f.changedBy) s.bindInt(i++, *f.changedBy);
    if (f.from) s.bindText(i++, *f.from);
    if (f.to) s.bindText(i++, *f.to);
    std::vector<AuditEntry> result;
    while (s.step()) {
        AuditEntry e;
        e.id = static_cast<int>(s.columnInt(0));
        e.submissionId = static_cast<int>(s.columnInt(1));
        e.changedBy = static_cast<int>(s.columnInt(2));
        e.changedAt = s.columnText(3);
        if (!s.isNull(4)) e.oldValue = s.columnDouble(4);
        e.newValue = s.columnDouble(5);
        e.reason = s.columnText(6);
        result.push_back(e);
    }
    return result;
}

}  // namespace lms
