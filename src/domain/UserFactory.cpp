#include "domain/UserFactory.h"

namespace lms {

std::unique_ptr<User> UserFactory::create(const UserData& data) {
    switch (data.role) {
        case Role::Admin: return std::make_unique<Administrator>(data);
        case Role::Lecturer: return std::make_unique<Lecturer>(data);
        case Role::Hod: return std::make_unique<HeadOfDepartment>(data);
        case Role::Student: return std::make_unique<Student>(data);
    }
    return nullptr;
}

}  // namespace lms
