#include "services/AuthService.h"

#include "common/Exceptions.h"
#include "common/Validation.h"
#include "domain/UserFactory.h"

namespace lms {
namespace {
constexpr int kMaxFailures = 5;
}

std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) {
    if (failures_[username] >= kMaxFailures) {
        throw AccessDeniedException("Too many wrong attempts. Restart the program and try again.");
    }
    auto user = users_.findByUsername(username);
    if (!user || !user->isActive() || !hasher_.verify(password, user->passwordHash())) {
        ++failures_[username];
        throw AccessDeniedException("Wrong username or password.");  // same message for every cause
    }
    failures_.erase(username);
    return user;
}

void AuthService::changePassword(const User& user, const std::string& oldPassword, const std::string& newPassword) {
    if (!user.can(Permission::ChangePassword)) throw AccessDeniedException("You are not allowed to do that.");
    if (!hasher_.verify(oldPassword, user.passwordHash())) throw AccessDeniedException("The current password is wrong.");
    validation::requirePassword(newPassword);
    UserData data = user.data();
    data.passwordHash = hasher_.hash(newPassword);
    users_.update(*UserFactory::create(data));
}

}  // namespace lms
