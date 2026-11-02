#pragma once
#include <functional>
#include <string>
#include <vector>

#include "services/AdminService.h"
#include "services/AssignmentService.h"
#include "services/AuditService.h"
#include "services/AuthService.h"
#include "services/MarkingService.h"
#include "services/ModerationService.h"
#include "services/ResultsService.h"
#include "services/SubmissionService.h"

namespace lms {

/// The services the menus call. The menus contain no business rules.
struct Services {
    AuthService& auth;
    AdminService& admin;
    AssignmentService& assignments;
    SubmissionService& submissions;
    MarkingService& marking;
    ModerationService& moderation;
    ResultsService& results;
    AuditService& audit;
};

class MenuBase {
public:
    virtual ~MenuBase() = default;
    virtual void run() = 0;  // returns when the user logs out

protected:
    struct Item {
        std::string label;
        std::function<void()> action;
    };
    void loop(const std::string& title, const std::vector<Item>& items);
    void changePassword(const User& user, Services& services);
    void showAuditLog(Services& services);
    bool loggedOut_ = false;
};

class AdminMenu : public MenuBase {
public:
    AdminMenu(const User& user, Services& services) : user_(user), s_(services) {}
    void run() override;

private:
    const User& user_;
    Services& s_;
};

class LecturerMenu : public MenuBase {
public:
    LecturerMenu(const User& user, Services& services) : user_(user), s_(services) {}
    void run() override;

private:
    const User& user_;
    Services& s_;
};

class HodMenu : public MenuBase {
public:
    HodMenu(const HeadOfDepartment& user, Services& services) : user_(user), s_(services) {}
    void run() override;

private:
    const HeadOfDepartment& user_;
    Services& s_;
};

class StudentMenu : public MenuBase {
public:
    StudentMenu(const User& user, Services& services) : user_(user), s_(services) {}
    void run() override;

private:
    const User& user_;
    Services& s_;
};

}  // namespace lms
