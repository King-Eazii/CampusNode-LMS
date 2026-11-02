#pragma once
#include <optional>
#include <string>

#include "common/DateTime.h"
#include "domain/Entities.h"

namespace lms {

/// The deadline that counts for one student: the extended deadline if one was granted (BR3).
inline std::string effectiveDeadline(const Assignment& assignment, const std::optional<Extension>& extension) {
    if (extension && datetime::toSeconds(extension->newDeadline) > datetime::toSeconds(assignment.deadline)) {
        return extension->newDeadline;
    }
    return assignment.deadline;
}

}  // namespace lms
