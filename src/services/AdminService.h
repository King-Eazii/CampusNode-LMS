#pragma once
#include <memory>
#include <optional>
#include <vector>

#include "data/DatabaseManager.h"
#include "data/IRepositories.h"
#include "services/AccessControl.h"
#include "services/PasswordHasher.h"

namespace lms {

struct NewUser {
    std::string username;
    std::string fullName;
    std::string password;
    Role role = Role::Student;
    std::optional<std::string> studentNumber;  // required for students
    std::optional<int> departmentId;           // required for heads of department
};

class AdminService {
public:
    AdminService(IUserRepository& users, ICourseRepository& courses, const PasswordHasher& hasher,
                 const AccessControl& access, DatabaseManager& db)
        : users_(users), courses_(courses), hasher_(hasher), access_(access), db_(db) {}

    bool hasUsers();
    /// First run only: creates the first administrator when the database has no users.
    int createFirstAdministrator(const std::string& username, const std::string& fullName, const std::string& password);

    int createUser(const User& admin, const NewUser& newUser);
    std::vector<std::unique_ptr<User>> listUsers(const User& admin);
    void setActive(const User& admin, int userId, bool active);
    int createDepartment(const User& admin, const std::string& name);
    int createCourse(const User& admin, const std::string& code, const std::string& title, int departmentId);
    void assignLecturer(const User& admin, int courseId, int lecturerId);
    void enrol(const User& admin, int studentId, int courseId);
    std::vector<Department> departments(const User& admin);
    std::vector<Course> courses(const User& admin);
    void backup(const User& admin, const std::string& path);
    void restore(const User& admin, const std::string& path);

private:
    IUserRepository& users_;
    ICourseRepository& courses_;
    const PasswordHasher& hasher_;
    const AccessControl& access_;
    DatabaseManager& db_;
};

}  // namespace lms
