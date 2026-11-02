#pragma once
#include "data/IRepositories.h"

namespace lms {

class AuditService {
public:
    explicit AuditService(IAuditRepository& repository) : repository_(repository) {}
    /// Adds one entry. Callers run this inside the same transaction as the mark change.
    void record(const AuditEntry& entry);
    std::vector<AuditEntry> find(const AuditFilter& filter);

private:
    IAuditRepository& repository_;
};

}  // namespace lms
