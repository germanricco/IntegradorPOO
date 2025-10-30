#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include <sqlite3.h>
#include <string>
#include <functional>

class SqliteDb {
public:
    explicit SqliteDb(const std::string& path);
    ~SqliteDb();

    SqliteDb(const SqliteDb&) = delete;
    SqliteDb& operator=(const SqliteDb&) = delete;

    sqlite3* handle() { return db_; }

    void exec(const std::string& sql);

    void withPrepared(const std::string& sql,
                      const std::function<void(sqlite3_stmt*)>& binder,
                      const std::function<void(sqlite3_stmt*)>& rowHandler);

private:
    sqlite3* db_;
};

#endif // SQLITE_DB_H

