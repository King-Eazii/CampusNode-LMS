#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "domain/Entities.h"
#include "domain/User.h"

namespace lms {

/// Repository interfaces. The business layer depends on these, never on SQLite.
class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::unique_ptr<User> findById(int id) = 0;  // nullptr when not found
    virtual std::unique_ptr<User> findByUsername(const std::string& username) = 0;
    virtual std::vector<std::unique_ptr<User>> findAll() = 0;
    virtual std::vector<std::unique_ptr<User>> findStudentsOfCourse(int courseId) = 0;
    virtual int save(const User& user) = 0;
    virtual void update(const User& user) = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual std::optional<Course> findById(int id) = 0;
    virtual std::vector<Course> findAll() = 0;
    virtual std::vector<Course> findByLecturer(int lecturerId) = 0;
    virtual std::vector<Course> findByStudent(int studentId) = 0;
    virtual int save(const Course& course) = 0;
    virtual void assignLecturer(int courseId, int lecturerId) = 0;
    virtual void enrol(int studentId, int courseId) = 0;
    virtual bool isEnrolled(int studentId, int courseId) = 0;
    virtual std::vector<Department> findDepartments() = 0;
    virtual std::optional<Department> findDepartment(int id) = 0;
    virtual int saveDepartment(const Department& department) = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual std::optional<Assignment> findById(int id) = 0;
    virtual std::vector<Assignment> findByCourse(int courseId) = 0;
    virtual int save(const Assignment& assignment) = 0;
    virtual void saveExtension(const Extension& extension) = 0;
    /// The latest extension a student has for an assignment, if any.
    virtual std::optional<Extension> findExtension(int assignmentId, int studentId) = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual std::optional<Submission> findById(int id) = 0;
    virtual std::vector<Submission> findByStudent(int studentId) = 0;
    virtual std::vector<Submission> findByAssignment(int assignmentId) = 0;
    virtual std::vector<Submission> findByDepartment(int departmentId, SubmissionStatus status) = 0;
    virtual int save(const Submission& submission) = 0;    // inserts the submission and its versions
    virtual void update(const Submission& submission) = 0;  // updates the row and adds new versions
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual void add(const AuditEntry& entry) = 0;
    virtual std::vector<AuditEntry> find(const AuditFilter& filter) = 0;
};

}  // namespace lms
