# Coding Standard

**Language and build.** C++17, built with CMake. Code must compile without warnings using
`-Wall -Wextra -Wpedantic` (or `/W4` on Visual Studio).

**Naming.**
- Classes, structs, enums and files: `PascalCase` (`SubmissionService.cpp`).
- Functions and variables: `camelCase`. Private data members end with an underscore (`db_`).
- Constants: `kPascalCase`. Namespaces: `lowercase`. Enum values: `PascalCase`.

**Layout.** 4 spaces, no tabs. At most 120 characters per line. Braces on the same line.
One class per header and source file (small related helpers may share a file). Headers use `#pragma once`.

**Layers (from the design).**
- `ui` calls `services` only. It contains no business rules and no SQL.
- `services` contain every business rule. They call repository interfaces, never SQLite.
- SQL appears only in `SqliteRepositories.cpp`, and values are always bound as parameters. Never build SQL from user text.
- Every service method starts with `AccessControl::require()`.

**Safety.**
- No raw owning pointers: use values, references, `std::unique_ptr` or `std::optional`.
- Pass objects by `const&`. Mark functions and parameters `const` when they do not change data.
- Errors are reported with the exceptions in `common/Exceptions.h`. Do not return error codes.
  The console catches `AppException`, shows the message and returns to the menu. The program must not crash.
- Passwords are only handled by `PasswordHasher`. A password is never stored, logged or printed.
- A change to a mark and its audit entry are saved inside one `Transaction`.

**Comments.** Comment why, not what. Every class and every public function that is not obvious has one short `///` comment.

**Git.** Work on a feature branch, make small commits with a clear message (for example `Add late penalty strategy`),
and merge to `main` by pull request.
