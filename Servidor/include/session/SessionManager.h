#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <unordered_map>

struct SessionView {
    std::string user;
    std::string privilegio; // "admin" | "op" | "viewer"
};

class SessionManager {
    std::unordered_map<std::string, SessionView> map_;
public:
    std::string create(const std::string& user, const std::string& priv); // devuelve token
    bool        remove(const std::string& token);
    const SessionView* get(const std::string& token) const;
    static std::string genToken(); // 32 chars hex
};


#endif
