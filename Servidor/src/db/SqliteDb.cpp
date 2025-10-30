#include "db/SqliteDb.h"
#include <stdexcept>

SqliteDb::SqliteDb(const std::string& path) : db_(nullptr) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error(std::string("SQLite open: ") + sqlite3_errmsg(db_));
    }
    exec("PRAGMA foreign_keys=ON;");
    exec("PRAGMA journal_mode=WAL;");
}

SqliteDb::~SqliteDb() {
    if (db_) sqlite3_close(db_);
}

void SqliteDb::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "SQLite exec error";
        sqlite3_free(err);
        throw std::runtime_error(e);
    }
}

void SqliteDb::withPrepared(const std::string& sql,
                            const std::function<void(sqlite3_stmt*)>& binder,
                            const std::function<void(sqlite3_stmt*)>& rowHandler) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Prepare: ") + sqlite3_errmsg(db_));
    }
    if (binder) binder(st);
    int rc;
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        if (rowHandler) rowHandler(st);
    }
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(st);
        throw std::runtime_error(std::string("Step: ") + sqlite3_errmsg(db_));
    }
    sqlite3_finalize(st);
}

