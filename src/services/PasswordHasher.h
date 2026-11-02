#pragma once
#include <string>

namespace lms {

/// The only code that touches the hashing library (libsodium, Argon2id with a random salt).
class PasswordHasher {
public:
    std::string hash(const std::string& password) const;  // salted hash, the salt is inside the result
    bool verify(const std::string& password, const std::string& storedHash) const;
};

}  // namespace lms
