#include "services/PasswordHasher.h"

#include <sodium.h>

#include "common/Exceptions.h"

namespace lms {
namespace {
void init() {
    if (sodium_init() < 0) throw AppException("The security library could not start.");
}
}  // namespace

std::string PasswordHasher::hash(const std::string& password) const {
    init();
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(out, password.c_str(), password.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw AppException("The computer ran out of memory while protecting the password.");
    }
    return out;
}

bool PasswordHasher::verify(const std::string& password, const std::string& storedHash) const {
    init();
    return crypto_pwhash_str_verify(storedHash.c_str(), password.c_str(), password.size()) == 0;
}

}  // namespace lms
