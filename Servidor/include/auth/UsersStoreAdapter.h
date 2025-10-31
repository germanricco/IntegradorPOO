#ifndef USERS_STORE_ADAPTER_H
#define USERS_STORE_ADAPTER_H

#include <string>
#include <vector>
#include <optional>
#include "db/SqliteDb.h"
#include "storage/UsersRepoSqlite.h"
#include "services/AuthService.h"

// Estructura que ya usabas en tu código (ajústala a tus campos reales)
struct UserInfo {
    int id;
    std::string username;
    std::string role;    // "admin" | "operator" | "viewer"
    bool is_active;
};

class UsersStoreAdapter {
public:
    // Inyectamos dependencias ya inicializadas
    UsersStoreAdapter(SqliteDb& db, UsersRepoSqlite& repo, AuthService& auth);

    // ----- API equivalente a la que usabas con CSV -----
    bool userExists(const std::string& username);
    std::optional<UserInfo> getByUsername(const std::string& username);
    std::optional<UserInfo> getById(int id);

    // Alta usuario (recibe contraseña en claro)
    // devuelve id nuevo o -1 si falla (username duplicado, etc.)
    int addUser(const std::string& username, const std::string& clearPassword,
                const std::string& role, bool active = true);

    bool setActive(int userId, bool active);
    bool changePassword(int userId, const std::string& newClearPassword);
    std::vector<UserInfo> listAll();

    // ---- Tokens/sesiones (opcional) ----
    bool saveToken(const std::string& token, int userId, const std::string& expiresAtIso);
    bool revokeToken(const std::string& token);
    std::optional<int> getUserIdByToken(const std::string& token);

    // Login que retorna UserInfo (si querés centralizar)
    std::optional<UserInfo> loginAndFetch(const std::string& username, const std::string& clearPassword);

private:
    SqliteDb& db_;
    UsersRepoSqlite& repo_;
    AuthService& auth_;

    std::optional<UserInfo> map(const UserDTO& u);
};

#endif // USERS_STORE_ADAPTER_H

