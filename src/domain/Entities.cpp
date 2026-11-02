#include "domain/Entities.h"

#include "common/Exceptions.h"

namespace lms {

bool Submission::canTransitionTo(SubmissionStatus next) const {
    using S = SubmissionStatus;
    switch (status) {
        case S::Submitted: return next == S::Marked;
        case S::Late: return next == S::Marked || next == S::Submitted;  // Submitted again after an extension
        case S::Marked: return next == S::Moderated;
        case S::Moderated: return next == S::Released;
        case S::Released: return false;
    }
    return false;
}

void Submission::transitionTo(SubmissionStatus next) {
    if (!canTransitionTo(next)) {
        throw ValidationException("A submission cannot change from " + toString(status) + " to " + toString(next) + ".");
    }
    status = next;
}

const SubmissionVersion& Submission::latestVersion() const {
    if (versions.empty()) throw NotFoundException("The submission has no versions.");
    return versions.back();
}

}  // namespace lms
