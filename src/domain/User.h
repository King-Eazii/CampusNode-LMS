#pragma once
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "domain/Types.h"

namespace lms {

/// Plain data needed to build any kind of user (used by UserFactory and the repositories).
struct UserData {
    int id = 0;
    std::string username;
    std::string fullName;
    std::string passwordHash;
    Role role = Role::Student;
    bool active = true;
    std::optional<std::string> studentNumber;  // students only
    std::optional<int> departmentId;           // heads of department only
};

/// Abstract base of the four roles. Each role decides what it is allowed to do (polymorphism).
class User {
public:
    explicit User(UserData data) : data_(std::move(data)) {}
    virtual ~User() = default;

    virtual Role getRole() const = 0;
    virtual std::vector<Permission> allowedActions() const = 0;

    bool can(Permission permission) const {
        const auto actions = allowedActions();
        return std::find(actions.begin(), actions.end(), permission) != actions.end();
    }
    int id() const { return data_.id; }
    const std::string& username() const { return data_.username; }
    const std::string& fullName() const { return data_.fullName; }
    const std::string& passwordHash() const { return data_.passwordHash; }
    bool isActive() const { return data_.active; }
    const UserData& data() const { return data_; }

protected:
    UserData data_;
};

class Administrator : public User {
public:
    using User::User;
    Role getRole() const override { return Role::Admin; }
    std::vector<Permission> allowedActions() const override {
        return {Permission::ManageUsers, Permission::ManageCourses, Permission::EnrolStudents,
                Permission::BackupRestore, Permission::ViewAuditLog, Permission::ChangePassword};
    }
};

class Lecturer : public User {
public:
    using User::User;
    Role getRole() const override { return Role::Lecturer; }
    std::vector<Permission> allowedActions() const override {
        return {Permission::CreateAssignment, Permission::GrantExtension, Permission::MarkSubmission,
                Permission::ExportMarks, Permission::ChangePassword};
    }
};

class HeadOfDepartment : public User {
public:
    using User::User;
    Role getRole() const override { return Role::Hod; }
    std::vector<Permission> allowedActions() const override {
        return {Permission::ModerateMarks, Permission::ReleaseMarks, Permission::ViewAuditLog,
                Permission::ChangePassword};
    }
    int departmentId() const { return data_.departmentId.value_or(0); }
};

class Student : public User {
public:
    using User::User;
    Role getRole() const override { return Role::Student; }
    std::vector<Permission> allowedActions() const override {
        return {Permission::ViewAssignments, Permission::SubmitWork, Permission::ViewResults,
                Permission::ChangePassword};
    }
    const std::string& studentNumber() const { return *data_.studentNumber; }
};

}  // namespace lms
