#include "db/SqliteDb.h"
#include "storage/UsersRepoSqlite.h"
#include "services/AuthService.h"
#include <iostream>

int main(){
    try {
        SqliteDb db("data/db/poo.db");
        UsersRepoSqlite repo(db);
        AuthService auth(repo, "palabra_secreta");

        // crea tablas por las dudas
        db.exec("PRAGMA foreign_keys=ON;");
        db.exec("CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT UNIQUE NOT NULL,"
                "password_hash TEXT NOT NULL,"
                "role TEXT NOT NULL,"
                "is_active INTEGER NOT NULL DEFAULT 1,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);");

        // inserta admin si no existe
        auto admin = repo.findByUsername("admin");
        if (!admin) {
            UserDTO u{};
            u.username="admin";
            u.password_hash = auth.makeHash("admin123");
            u.role="admin"; u.is_active=true;
            int id = repo.insert(u);
            std::cout << "admin creado id=" << id << "\n";
        } else {
            std::cout << "admin ya existia\n";
        }

        // login
        auto ok = auth.login("admin","admin123");
        std::cout << "login: " << (ok ? "OK":"FAIL") << "\n";
        return ok ? 0 : 1;
    } catch (const std::exception& e){
        std::cerr << "ERROR: " << e.what() << "\n";
        return 2;
    }
}

