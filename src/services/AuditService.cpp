#include "services/AuditService.h"

#include "common/Validation.h"

namespace lms {

void AuditService::record(const AuditEntry& entry) {
    AuditEntry copy = entry;
    copy.reason = validation::requireText(entry.reason, "The reason", 500);
    repository_.add(copy);
}

std::vector<AuditEntry> AuditService::find(const AuditFilter& filter) { return repository_.find(filter); }

}  // namespace lms
