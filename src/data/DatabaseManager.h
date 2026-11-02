#pragma once
#include <optional>
#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace lms {

/// One prepared SQL statement. Values are always bound as parameters, never joined into the SQL text.
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(Statement&& other) noexcept;
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement& operator=(Statement&&) = delete;

    // Parameter numbers start at 1; column numbers start at 0.
    void bindInt(int index, long long value);
    void bindDouble(int index, double value);
    void bindText(int index, const std::string& value);
    void bindNull(int index);
    void bindOptionalInt(int index, const std::optional<int>& value);
    void bindOptionalDouble(int index, const std::optional<double>& value);
    void bindOptionalText(int index, const std::optional<std::string>& value);

    /// Runs the statement. Returns true while a row is available, false when finished.
    bool step();
    /// Runs a statement that returns no rows (INSERT, UPDATE, DELETE).
    void run();

    long long columnInt(int index) const;
    double columnDouble(int index) const;
    std::string columnText(int index) const;
    bool isNull(int index) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_;
};

/// Opens the SQLite file, creates the tables on first use and runs transactions.
class DatabaseManager {
public:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void open(const std::string& path);
    void close();
    Statement prepare(const std::string& sql);
    void execute(const std::string& sql);
    long long lastInsertId() const;

    // Transactions can be nested: only the outermost begin and commit reach the database.
    void beginTransaction();
    void commit();
    void rollback();

    /// Copies the whole database to a new file. Fails if the file already exists.
    void backupTo(const std::string& path);
    /// Replaces the current database with a backup file.
    void restoreFrom(const std::string& path);
    const std::string& path() const { return path_; }

private:
    sqlite3* db_ = nullptr;
    std::string path_;
    int depth_ = 0;
};

/// RAII transaction: rolls back automatically unless commit() was called.
class Transaction {
public:
    explicit Transaction(DatabaseManager& db);
    ~Transaction();
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    void commit();

private:
    DatabaseManager& db_;
    bool done_ = false;
};

}  // namespace lms
