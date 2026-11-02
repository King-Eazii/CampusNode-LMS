#pragma once
#include <memory>

#include "domain/User.h"

namespace lms {

/// Factory pattern: the only place that decides which User subclass to create.
class UserFactory {
public:
    static std::unique_ptr<User> create(const UserData& data);
};

}  // namespace lms
