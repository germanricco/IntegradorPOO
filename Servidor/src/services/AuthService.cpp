#include "services/AuthService.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

static std::string sha256_hex(const std::string& s) {
    unsigned char h[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(s.c_str()), s.size(), h);
    std::ostringstream o;
    for (int i=0;i<SHA256_DIGEST_LENGTH;++i)
        o << std::hex << std::setw(2) << std::setfill('0') << (int)h[i];
    return o.str();
}

AuthService::AuthService(IUsersRepo& repo, const std::string& salt)
    : users_(repo), salt_(salt) {}

bool AuthService::verifyPassword(const std::string& clear, const std::string& hash) const {
    return sha256_hex(salt_ + clear) == hash;   // simple para el TP
}

std::optional<AuthUser> AuthService::login(const std::string& username,
                                           const std::string& password) {
    auto u = users_.findByUsername(username);
    if (!u || !u->is_active) return std::nullopt;
    if (!verifyPassword(password, u->password_hash)) return std::nullopt;
    AuthUser au{u->id, u->username, u->role};
    return au;
}


