#include "session/SessionManager.h"
#include <random>

std::string SessionManager::genToken() {
    static const char HEX[] = "0123456789abcdef";
    std::random_device rd; std::mt19937 rng(rd()); std::uniform_int_distribution<int> d(0,15);
    std::string t; t.reserve(32);
    for (int i=0;i<32;++i) t.push_back(HEX[d(rng)]);
    return t;
}

std::string SessionManager::create(const std::string& user, const std::string& priv) {
    std::string tok = genToken();
    map_[tok] = {user, priv};
    return tok;
}

bool SessionManager::remove(const std::string& token) {
    return map_.erase(token) > 0;
}

const SessionView* SessionManager::get(const std::string& token) const {
    auto it = map_.find(token);
    return it == map_.end() ? nullptr : &it->second;
}

