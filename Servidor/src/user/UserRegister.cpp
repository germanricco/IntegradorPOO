#include "user/UserRegister.h"
#include "services/AuthBootstrap.h"

using namespace XmlRpc;

namespace userrpc {

static XmlRpcValue norm(XmlRpcValue& p){
    if (p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

UserRegister::UserRegister(XmlRpcServer* s,
                           SessionManager& sm,
                           IUsersRepo& r,
                           PALogger& L)
: XmlRpcServerMethod("user.register", s)
, sessions_(sm)
, repo_(r)
, log_(L)
{}

void UserRegister::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        XmlRpcValue a = norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct ||
            !a.hasMember("user") ||
            !a.hasMember("pass")) {
            throw XmlRpcException("BAD_REQUEST: se esperaban {user, pass, [role], [active]}");
        }

        const std::string user   = std::string(a["user"]);
        const std::string pass   = std::string(a["pass"]);
        const std::string role   = a.hasMember("role")   ? std::string(a["role"])   : "operator";
        const bool        active = a.hasMember("active") ? bool(a["active"])        : true;

        // ¿ya existe?
        if (repo_.findByUsername(user)){
            throw XmlRpcException("CONFLICT: username existente");
        }

        auto& W = auth_wiring();

        UserDTO u{};
        u.username      = user;
        u.password_hash = W.auth->makeHash(pass);
        u.role          = role;
        u.is_active     = active;

        int id = repo_.insert(u);

        result["ok"]     = true;
        result["id"]     = id;
        result["user"]   = user;
        result["role"]   = role;
        result["active"] = active;
        log_.info("[user.register] OK -- user=" + user + " id=" + std::to_string(id));
    }
    catch(const XmlRpcException&){ throw; }
    catch(...){ throw XmlRpcException("INTERNAL_ERROR: user.register"); }
}

std::string UserRegister::help(){
    return "user.register {user, pass, [role='operator'], [active=true]} -> {ok,id,...}";
}

} // namespace userrpc


