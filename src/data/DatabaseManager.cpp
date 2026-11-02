#include "data/DatabaseManager.h"

#include <sqlite3.h>

#include <filesystem>
#include <fstream>

#include "SchemaSql.h"
#include "common/Exceptions.h"

namespace lms {
namespace {

void check(int result, sqlite3* db, int expected = SQLITE_OK) {
    if (result != expected) throw DatabaseException(std::string("Database error: ") + sqlite3_errmsg(db));
}

}  // namespace

// ---------------------------------------------------------------- Statement
Statement::Statement(sqlite3* db, const std::string& sql) : db_(db), stmt_(nullptr) {
    check(sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr), db_);
}

Statement::~Statement() {
    if (stmt_ != nullptr) sqlite3_finalize(stmt_);
}

Statement::Statement(Statement&& other) noexcept : db_(other.db_), stmt_(other.stmt_) { other.stmt_ = nullptr; }

void Statement::bindInt(int index, long long value) { check(sqlite3_bind_int64(stmt_, index, value), db_); }
void Statement::bindDouble(int index, double value) { check(sqlite3_bind_double(stmt_, index, value), db_); }
void Statement::bindText(int index, const std::string& value) {
    check(sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT), db_);
}
void Statement::bindNull(int index) { check(sqlite3_bind_null(stmt_, index), db_); }
void Statement::bindOptionalInt(int index, const std::optional<int>& value) {
    value ? bindInt(index, *value) : bindNull(index);
}
void Statement::bindOptionalDouble(int index, const std::optional<double>& value) {
    value ? bindDouble(index, *value) : bindNull(index);
}
void Statement::bindOptionalText(int index, const std::optional<std::string>& value) {
    value ? bindText(index, *value) : bindNull(index);
}

bool Statement::step() {
    const int result = sqlite3_step(stmt_);
    if (result == SQLITE_ROW) return true;
    if (result == SQLITE_DONE) return false;
    throw DatabaseException(std::string("Database error: ") + sqlite3_errmsg(db_));
}

void Statement::run() { step(); }

long long Statement::columnInt(int index) const { return sqlite3_column_int64(stmt_, index); }
double Statement::columnDouble(int index) const { return sqlite3_column_double(stmt_, index); }
std::string Statement::columnText(int index) const {
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    return text == nullptr ? std::string() : std::string(reinterpret_cast<const char*>(text));
}
bool Statement::isNull(int index) const { return sqlite3_column_type(stmt_, index) == SQLITE_NULL; }

// ---------------------------------------------------------------- DatabaseManager
DatabaseManager::~DatabaseManager() { close(); }

void DatabaseManager::open(const std::string& path) {
    close();
    path_ = path;
    if (sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        const std::string message = db_ != nullptr ? sqlite3_errmsg(db_) : "cannot open the file";
        close();
        throw DatabaseException("Cannot open the database '" + path + "': " + message);
    }
    execute("PRAGMA foreign_keys = ON;");  // SQLite ignores foreign keys unless this is switched on
    Statement check(db_, "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = 'users'");
    check.step();
    if (check.columnInt(0) == 0) execute(kSchemaSql);  // first run: create all tables
}

void DatabaseManager::close() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    depth_ = 0;
}

Statement DatabaseManager::prepare(const std::string& sql) {
    if (db_ == nullptr) throw DatabaseException("The database is not open.");
    return Statement(db_, sql);
}

void DatabaseManager::execute(const std::string& sql) {
    if (db_ == nullptr) throw DatabaseException("The database is not open.");
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error != nullptr ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseException("Database error: " + message);
    }
}

long long DatabaseManager::lastInsertId() const { return sqlite3_last_insert_rowid(db_); }

void DatabaseManager::beginTransaction() {
    if (depth_ == 0) execute("BEGIN");
    ++depth_;
}

void DatabaseManager::commit() {
    if (depth_ == 0) return;
    if (--depth_ == 0) execute("COMMIT");
}

void DatabaseManager::rollback() {
    if (depth_ == 0) return;
    depth_ = 0;
    execute("ROLLBACK");
}

void DatabaseManager::backupTo(const std::string& path) {
    if (std::filesystem::exists(path)) throw ValidationException("That file already exists. Choose a new file name.");
    Statement statement = prepare("VACUUM INTO ?");
    statement.bindText(1, path);
    statement.run();
}

void DatabaseManager::restoreFrom(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    char header[16] = {};
    if (!in || !in.read(header, 15) || std::string(header, 15) != "SQLite format 3") {
        throw ValidationException("That file is not a valid database backup.");
    }
    in.close();
    const std::string target = path_;
    close();
    std::filesystem::copy_file(path, target, std::filesystem::copy_options::overwrite_existing);
    open(target);
}

// ---------------------------------------------------------------- Transaction
Transaction::Transaction(DatabaseManager& db) : db_(db) { db_.beginTransaction(); }

Transaction::~Transaction() {
    if (done_) return;
    try {
        db_.rollback();
    } catch (...) {
        // A destructor must not throw; the original error is already being reported.
    }
}

void Transaction::commit() {
    db_.commit();
    done_ = true;
}

}  // namespace lms
