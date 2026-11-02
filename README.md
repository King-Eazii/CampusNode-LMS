# CampusNode LMS

CampusNode LMS is a console-based Learning Management System for a secondary school. It manages courses, assignments, submissions, marking, moderation by the Head of Department (HoD), and the release of marks.

The application is written in C++17 and uses SQLite for data persistence. It is designed to run offline on a single computer.

## Requirements

- A C++17 compiler:
  - GCC 9 or newer
  - Clang 10 or newer
  - Visual Studio 2019 or newer
- CMake 3.14 or newer
- SQLite 3 development files
- libsodium development files
- pkg-config

## Installation

### Ubuntu or Debian

Install the required dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libsqlite3-dev libsodium-dev
```

### Windows (Visual Studio and vcpkg)

Install Visual Studio with C++ development tools, CMake, and vcpkg.

Install the required libraries:

```cmd
vcpkg install sqlite3 libsodium pkgconf
```

Configure the project using the vcpkg CMake toolchain:

```cmd
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
```

Replace `<vcpkg-root>` with the actual location of your vcpkg installation.

## Build

From the project root directory, configure and build the application:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

On Windows, when using the Visual Studio generator, build with:

```cmd
cmake --build build --config Release
```

The executable is named `lms`. Its exact output location depends on the CMake generator. With a typical Visual Studio configuration, it is:

```text
build\Release\lms.exe
```

## Run

Run the executable from the project root directory.

Ubuntu or Debian:

```bash
./build/lms
./build/lms --db school.db
```

Windows (typical Visual Studio output):

```cmd
build\Release\lms.exe
build\Release\lms.exe --db school.db
```

By default, the application uses `lms.db` in the current working directory. The `--db` option allows another database file to be specified.

On the first run, the application creates the database and prompts the user to create the administrator account. There is no default administrator password. After setup, log in and use the menu corresponding to your role.

## Project Structure

```text
src/common/     Exceptions, date and time helpers, input checks
src/domain/     Users, roles, entities and UserFactory
src/data/       DatabaseManager, repository interfaces and SQLite repositories
src/services/   Business rules and application services
src/ui/         Console menus and input validation
sql/schema.sql  Database schema, embedded into the program during configuration
```

The application follows a layered architecture:

```text
UI → Services → Repository Interfaces ← SQLite Repositories
```

The services layer contains the business rules and determines what operations are permitted. SQL queries are implemented in `src/data/SqliteRepositories.cpp` and use bound parameters.

## Coding Standard

See `CODING_STANDARD.md` for the project's coding conventions and standards.