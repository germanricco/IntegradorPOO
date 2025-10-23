#include "auth/AuthMe.h"
using namespace XmlRpc;

namespace auth {

static XmlRpcValue normalizeArgs(XmlRpcValue& p) {
    if (p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

AuthMe::AuthMe(XmlRpcServer* server, SessionManager& sm)
: XmlRpcServerMethod("auth.me", server), sessions_(sm) {}

void AuthMe::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try {
        XmlRpcValue a = normalizeArgs(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token"))
            throw XmlRpcException("BAD_REQUEST: falta 'token'");

        std::string tok = std::string(a["token"]);
        auto s = sessions_.get(tok);
        if (!s) throw XmlRpcException("AUTH_INVALID: token");

        result["ok"] = true;
        result["user"] = s->user;
        result["privilegio"] = s->privilegio;
    }
    catch (const XmlRpcException&) { throw; }
    catch (...) { throw XmlRpcException("BAD_REQUEST: auth.me"); }
}

std::string AuthMe::help() {
    return "auth.me({token:string}) -> {ok:bool, user:string, privilegio:string}";
}

} // namespace auth

