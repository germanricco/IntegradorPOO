#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <optional>
#include <string>
#include "auth/IUsersRepo.h"

struct AuthUser {
    int id;
    std::string username;
    std::string role;
};

class AuthService {
public:
    AuthService(IUsersRepo& repo, const std::string& salt);

    std::optional<AuthUser> login(const std::string& username,
                                  const std::string& password);

    bool verifyPassword(const std::string& clear, const std::string& hash) const;

private:
    IUsersRepo& users_;
    std::string salt_;
};

#endif // AUTH_SERVICE_H

