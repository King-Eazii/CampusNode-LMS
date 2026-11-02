#pragma once
#include <memory>

#include "common/Exceptions.h"
#include "domain/User.h"

namespace lms {

/// Remembers who is logged in for the current session.
class SessionManager {
public:
    void start(std::unique_ptr<User> user) { user_ = std::move(user); }
    void end() { user_.reset(); }
    bool isActive() const { return user_ != nullptr; }
    const User& user() const {
        if (!user_) throw AccessDeniedException("Nobody is logged in.");
        return *user_;
    }

private:
    std::unique_ptr<User> user_;
};

}  // namespace lms
