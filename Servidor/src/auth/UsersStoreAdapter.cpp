#include "auth/UsersStoreAdapter.h"
#include <sqlite3.h>

UsersStoreAdapter::UsersStoreAdapter(SqliteDb& db, UsersRepoSqlite& repo, AuthService& auth)
: db_(db), repo_(repo), auth_(auth) {}

std::optional<UserInfo> UsersStoreAdapter::map(const UserDTO& u){
    return UserInfo{u.id, u.username, u.role, u.is_active};
}

bool UsersStoreAdapter::userExists(const std::string& username){
    return repo_.findByUsername(username).has_value();
}

std::optional<UserInfo> UsersStoreAdapter::getByUsername(const std::string& username){
    auto u = repo_.findByUsername(username);
    if (!u) return std::nullopt;
    return map(*u);
}

std::optional<UserInfo> UsersStoreAdapter::getById(int id){
    auto u = repo_.findById(id);
    if (!u) return std::nullopt;
    return map(*u);
}

int UsersStoreAdapter::addUser(const std::string& username, const std::string& clearPassword,
                               const std::string& role, bool active){
    if (userExists(username)) return -1;
    UserDTO dto{};
    dto.username = username;
    dto.password_hash = auth_.makeHash(clearPassword);
    dto.role = role;
    dto.is_active = active;
    try {
        return repo_.insert(dto);
    } catch (...) {
        return -1;
    }
}

bool UsersStoreAdapter::setActive(int userId, bool active){
    try {
        repo_.setActive(userId, active);
        return true;
    } catch (...) {
        return false;
    }
}

bool UsersStoreAdapter::changePassword(int userId, const std::string& newClearPassword){
    try {
        repo_.updatePasswordHash(userId, auth_.makeHash(newClearPassword));
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<UserInfo> UsersStoreAdapter::listAll(){
    std::vector<UserInfo> out;
    for (auto& u : repo_.listAll()){
        out.push_back(UserInfo{u.id, u.username, u.role, u.is_active});
    }
    return out;
}

// ---------- Tokens/sesiones opcional ----------

bool UsersStoreAdapter::saveToken(const std::string& token, int userId, const std::string& expiresAtIso){
    try {
        db_.withPrepared(
            "INSERT INTO sessions(id,user_id,expires_at) VALUES(?1,?2,?3)",
            [&](sqlite3_stmt* st){
                sqlite3_bind_text(st,1, token.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(st,2, userId);
                sqlite3_bind_text(st,3, expiresAtIso.c_str(), -1, SQLITE_TRANSIENT);
            }, nullptr);
        return true;
    } catch (...) { return false; }
}

bool UsersStoreAdapter::revokeToken(const std::string& token){
    try {
        db_.withPrepared("DELETE FROM sessions WHERE id=?1",
            [&](sqlite3_stmt* st){ sqlite3_bind_text(st,1, token.c_str(), -1, SQLITE_TRANSIENT); }, nullptr);
        return true;
    } catch (...) { return false; }
}

std::optional<int> UsersStoreAdapter::getUserIdByToken(const std::string& token){
    std::optional<int> out;
    try {
        db_.withPrepared(
            "SELECT user_id FROM sessions WHERE id=?1",
            [&](sqlite3_stmt* st){ sqlite3_bind_text(st,1, token.c_str(), -1, SQLITE_TRANSIENT); },
            [&](sqlite3_stmt* st){ out = sqlite3_column_int(st,0); });
    } catch (...) {}
    return out;
}

std::optional<UserInfo> UsersStoreAdapter::loginAndFetch(const std::string& username, const std::string& clearPassword){
    auto u = repo_.findByUsername(username);
    if (!u || !u->is_active) return std::nullopt;
    if (!auth_.verifyPassword(clearPassword, u->password_hash)) return std::nullopt;
    return map(*u);
}

