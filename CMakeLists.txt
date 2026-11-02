cmake_minimum_required(VERSION 3.14)
project(CampusNodeLMS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# Embed sql/schema.sql in the program so the database can be created on first run.
file(READ "${CMAKE_SOURCE_DIR}/sql/schema.sql" LMS_SCHEMA_SQL)
configure_file("${CMAKE_SOURCE_DIR}/src/data/SchemaSql.h.in" "${CMAKE_BINARY_DIR}/generated/SchemaSql.h" @ONLY)

# Core library: domain, data access and business logic (no console code).
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
     src/common/*.cpp src/domain/*.cpp src/data/*.cpp src/services/*.cpp)
add_library(lms_core STATIC ${CORE_SOURCES})
target_include_directories(lms_core PUBLIC src "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(lms_core PUBLIC SQLite::SQLite3 PkgConfig::SODIUM)
if(MSVC)
  target_compile_options(lms_core PRIVATE /W4)
else()
  target_compile_options(lms_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Console application.
file(GLOB UI_SOURCES CONFIGURE_DEPENDS src/ui/*.cpp)
add_executable(lms src/main.cpp ${UI_SOURCES})
target_link_libraries(lms PRIVATE lms_core)
if(NOT MSVC)
  target_compile_options(lms PRIVATE -Wall -Wextra -Wpedantic)
endif()
