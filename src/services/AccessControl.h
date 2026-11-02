#pragma once
#include "common/Exceptions.h"
#include "domain/User.h"

namespace lms {

/// Every service calls require() before it acts, so a menu cannot skip the role check.
class AccessControl {
public:
    void require(const User& user, Permission permission) const {
        if (!user.isActive() || !user.can(permission)) throw AccessDeniedException("You are not allowed to do that.");
    }
};

}  // namespace lms
