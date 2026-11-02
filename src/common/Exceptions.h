#pragma once
#include <stdexcept>

namespace lms {

/// Base class of every error the application reports to the user.
class AppException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class ValidationException : public AppException {
public:
    using AppException::AppException;
};
class AccessDeniedException : public AppException {
public:
    using AppException::AppException;
};
class NotFoundException : public AppException {
public:
    using AppException::AppException;
};
class DatabaseException : public AppException {
public:
    using AppException::AppException;
};

}  // namespace lms
