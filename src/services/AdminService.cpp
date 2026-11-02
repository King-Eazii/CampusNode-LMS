#include "services/AdminService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "domain/UserFactory.h"

namespace lms {

bool AdminService::hasUsers() { return !users_.findAll().empty(); }

int AdminService::createFirstAdministrator(const std::string& username, const std::string& fullName,
                                           const std::string& password) {
    if (hasUsers()) throw AccessDeniedException("An administrator already exists.");
    validation::requireUsername(username);
    validation::requirePassword(password);
    UserData data;
    data.username = username;
    data.fullName = validation::requireText(fullName, "The name", 100);
    data.passwordHash = hasher_.hash(password);
    data.role = Role::Admin;
    return users_.save(*UserFactory::create(data));
}

int AdminService::createUser(const User& admin, const NewUser& n) {
    access_.require(admin, Permission::ManageUsers);
    validation::requireUsername(n.username);
    validation::requirePassword(n.password);
    UserData data;
    data.username = n.username;
    data.fullName = validation::requireText(n.fullName, "The name", 100);
    data.role = n.role;
    if (users_.findByUsername(n.username)) throw ValidationException("That username is already taken.");
    if (n.role == Role::Student) {
        if (!n.studentNumber) throw ValidationException("A student needs a student number.");
        data.studentNumber = validation::requireText(*n.studentNumber, "The student number", 30);
    }
    if (n.role == Role::Hod) {
        if (!n.departmentId || !courses_.findDepartment(*n.departmentId)) throw ValidationException("Choose an existing department.");
        for (const auto& u : users_.findAll()) {
            if (u->getRole() == Role::Hod && u->data().departmentId == n.departmentId) {
                throw ValidationException("That department already has a Head of Department.");
            }
        }
        data.departmentId = n.departmentId;
    }
    data.passwordHash = hasher_.hash(n.password);
    return users_.save(*UserFactory::create(data));
}

std::vector<std::unique_ptr<User>> AdminService::listUsers(const User& admin) {
    access_.require(admin, Permission::ManageUsers);
    return users_.findAll();
}

void AdminService::setActive(const User& admin, int userId, bool active) {
    access_.require(admin, Permission::ManageUsers);
    if (userId == admin.id()) throw ValidationException("You cannot deactivate your own account.");
    auto user = users_.findById(userId);
    if (!user) throw NotFoundException("No user has that number.");
    UserData data = user->data();
    data.active = active;
    users_.update(*UserFactory::create(data));
}

int AdminService::createDepartment(const User& admin, const std::string& name) {
    access_.require(admin, Permission::ManageCourses);
    return courses_.saveDepartment(Department{0, validation::requireText(name, "The department name", 100)});
}

int AdminService::createCourse(const User& admin, const std::string& code, const std::string& title, int departmentId) {
    access_.require(admin, Permission::ManageCourses);
    if (!courses_.findDepartment(departmentId)) throw NotFoundException("No department has that number.");
    return courses_.save(Course{0, validation::requireText(code, "The course code", 20),
                                validation::requireText(title, "The course title", 100), departmentId});
}

void AdminService::assignLecturer(const User& admin, int courseId, int lecturerId) {
    access_.require(admin, Permission::ManageCourses);
    if (!courses_.findById(courseId)) throw NotFoundException("No course has that number.");
    const auto lecturer = users_.findById(lecturerId);
    if (!lecturer || lecturer->getRole() != Role::Lecturer) throw ValidationException("That number is not a lecturer.");
    courses_.assignLecturer(courseId, lecturerId);
}

void AdminService::enrol(const User& admin, int studentId, int courseId) {
    access_.require(admin, Permission::EnrolStudents);
    if (!courses_.findById(courseId)) throw NotFoundException("No course has that number.");
    const auto student = users_.findById(studentId);
    if (!student || student->getRole() != Role::Student) throw ValidationException("That number is not a student.");
    courses_.enrol(studentId, courseId);
}

std::vector<Department> AdminService::departments(const User& admin) {
    access_.require(admin, Permission::ManageCourses);
    return courses_.findDepartments();
}

std::vector<Course> AdminService::courses(const User& admin) {
    access_.require(admin, Permission::ManageCourses);
    return courses_.findAll();
}

void AdminService::backup(const User& admin, const std::string& path) {
    access_.require(admin, Permission::BackupRestore);
    db_.backupTo(validation::requireText(path, "The file name", 260));
}

void AdminService::restore(const User& admin, const std::string& path) {
    access_.require(admin, Permission::BackupRestore);
    db_.restoreFrom(validation::requireText(path, "The file name", 260));
}

}  // namespace lms
