#pragma once
#include <functional>

#include "data/DatabaseManager.h"
#include "data/IRepositories.h"

namespace lms {

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(DatabaseManager& db) : db_(db) {}
    std::unique_ptr<User> findById(int id) override;
    std::unique_ptr<User> findByUsername(const std::string& username) override;
    std::vector<std::unique_ptr<User>> findAll() override;
    std::vector<std::unique_ptr<User>> findStudentsOfCourse(int courseId) override;
    int save(const User& user) override;
    void update(const User& user) override;

private:
    DatabaseManager& db_;
};

class SqliteCourseRepository : public ICourseRepository {
public:
    explicit SqliteCourseRepository(DatabaseManager& db) : db_(db) {}
    std::optional<Course> findById(int id) override;
    std::vector<Course> findAll() override;
    std::vector<Course> findByLecturer(int lecturerId) override;
    std::vector<Course> findByStudent(int studentId) override;
    int save(const Course& course) override;
    void assignLecturer(int courseId, int lecturerId) override;
    void enrol(int studentId, int courseId) override;
    bool isEnrolled(int studentId, int courseId) override;
    std::vector<Department> findDepartments() override;
    std::optional<Department> findDepartment(int id) override;
    int saveDepartment(const Department& department) override;

private:
    DatabaseManager& db_;
};

class SqliteAssignmentRepository : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(DatabaseManager& db) : db_(db) {}
    std::optional<Assignment> findById(int id) override;
    std::vector<Assignment> findByCourse(int courseId) override;
    int save(const Assignment& assignment) override;
    void saveExtension(const Extension& extension) override;
    std::optional<Extension> findExtension(int assignmentId, int studentId) override;

private:
    DatabaseManager& db_;
};

class SqliteSubmissionRepository : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(DatabaseManager& db) : db_(db) {}
    std::optional<Submission> findById(int id) override;
    std::vector<Submission> findByStudent(int studentId) override;
    std::vector<Submission> findByAssignment(int assignmentId) override;
    std::vector<Submission> findByDepartment(int departmentId, SubmissionStatus status) override;
    int save(const Submission& submission) override;
    void update(const Submission& submission) override;

private:
    std::vector<Submission> query(const std::string& whereClause, const std::function<void(Statement&)>& bind);
    void loadVersions(Submission& submission);
    DatabaseManager& db_;
};

class SqliteAuditRepository : public IAuditRepository {
public:
    explicit SqliteAuditRepository(DatabaseManager& db) : db_(db) {}
    void add(const AuditEntry& entry) override;
    std::vector<AuditEntry> find(const AuditFilter& filter) override;

private:
    DatabaseManager& db_;
};

}  // namespace lms
