#include "services/AuthBootstrap.h"
#include <memory>

static std::unique_ptr<SqliteDb>        G_DB;
static std::unique_ptr<UsersRepoSqlite> G_REPO;
static std::unique_ptr<AuthService>     G_AUTH;
static AuthWiring G_WIRING{};

void init_auth_layer(const std::string& dbPath, const std::string& salt) {
    if (!G_DB) {
        G_DB   = std::make_unique<SqliteDb>(dbPath);
        G_REPO = std::make_unique<UsersRepoSqlite>(*G_DB);
        G_AUTH = std::make_unique<AuthService>(*G_REPO, salt);
        G_WIRING = { G_DB.get(), G_REPO.get(), G_AUTH.get() };
    }
}

AuthWiring& auth_wiring() {
    return G_WIRING;
}

