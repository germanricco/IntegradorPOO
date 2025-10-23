#ifndef USERSREPOCSV_H
#define USERSREPOCSV_H

#include "user/UserEntity.h"
#include <vector>
#include <unordered_map>
#include <string>

struct CredCheck { bool ok{false}; std::string priv; bool habilitado{false}; };

class UsersRepoCsv {
    std::string path_;
    std::vector<UserEntity> users_;
    std::unordered_map<std::string,int> byName_; // username -> idx en users_

    static std::string trim(const std::string& s);
    static std::string nowISO();
    static std::string hashPass(const std::string& plain);

    void rebuildIndex();
    int  nextId() const;
    int  countAdmins() const;

public:
    explicit UsersRepoCsv(std::string path);

    // carga/guarda CSV
    void load();
    void save() const;

    // asegura un admin inicial si la base está vacía o sin admin
    void ensureDefaultAdmin();

    // consultas
    bool exists(const std::string& user) const;
    CredCheck validate(const std::string& user, const std::string& passPlain) const;

    // (lo usaremos en el Paso 4)
    int  create(const std::string& user, const std::string& passPlain, Priv p);
    void update(const std::string& user, const std::string* newUser, const Priv* newPriv, const bool* habilitado);
    void changePass(const std::string& user, const std::string& newPass);
    const std::vector<UserEntity>& list() const { return users_; }
};


#endif
