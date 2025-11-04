#ifndef AUTH_BOOTSTRAP_H
#define AUTH_BOOTSTRAP_H

#include "db/SqliteDb.h"
#include "storage/UsersRepoSqlite.h"
#include "services/AuthService.h"

struct AuthWiring {
    SqliteDb* db;
    UsersRepoSqlite* repo;
    AuthService* auth;
};

// Llamá esto 1 sola vez al arrancar el server
void init_auth_layer(const std::string& dbPath, const std::string& salt);

// Acceso global seguro (punteros ya inicializados por init_auth_layer)
AuthWiring& auth_wiring();

#endif // AUTH_BOOTSTRAP_H

