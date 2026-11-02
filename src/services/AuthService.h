#pragma once
#include <map>
#include <memory>

#include "data/IRepositories.h"
#include "services/PasswordHasher.h"

namespace lms {

class AuthService {
public:
    AuthService(IUserRepository& users, const PasswordHasher& hasher) : users_(users), hasher_(hasher) {}
    /// Returns the logged-in user or throws AccessDeniedException. Five wrong passwords lock the name until restart.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);
    void changePassword(const User& user, const std::string& oldPassword, const std::string& newPassword);

private:
    IUserRepository& users_;
    const PasswordHasher& hasher_;
    std::map<std::string, int> failures_;
};

}  // namespace lms
