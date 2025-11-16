#ifndef AUTHZ_H
#define AUTHZ_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "../session/CurrentUser.h"
#include <string>

inline XmlRpc::XmlRpcValue rpc_norm(XmlRpc::XmlRpcValue& p) {
    if (p.getType()==XmlRpc::XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

inline const SessionView& requireSession(const SessionManager& sm, const std::string& token) {
    auto s = sm.get(token);
    if (!s) throw XmlRpc::XmlRpcException("AUTH_INVALID: token");
    return *s;
}

inline void requireAdmin(const SessionManager& sm, const std::string& token) {
    auto& s = requireSession(sm, token);
    if (s.privilegio != "admin") throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes");
}

inline const SessionView& guardSession(const char* op, SessionManager& sm, const std::string& token, PALogger& log) {
    const SessionView* sv = sm.get(token);
    if (!sv) {
        log.warning(std::string("[auth] token inválido — ") + op);
        CurrentUser::clear();
        throw XmlRpc::XmlRpcException("AUTH_INVALID: token");
    }

    // En cada petición válida, establecemos el ID del usuario en el hilo actual.
    CurrentUser::set(sv->id);
    
    return *sv;
}
inline const SessionView& guardAdmin(const char* op, SessionManager& sm, const std::string& token, PALogger& log) {
    const SessionView& sv = guardSession(op, sm, token, log);
    if (sv.privilegio != "admin") {
        log.warning(std::string("[user] ") + op + " FORBIDDEN — actor=" + sv.user);
        throw XmlRpc::XmlRpcException("FORBIDDEN: privilegios insuficientes");
    }
    return sv;
}

#endif
