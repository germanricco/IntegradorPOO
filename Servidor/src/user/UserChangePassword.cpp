#include "user/UserChangePassword.h"
#include "services/AuthBootstrap.h"   // auth_wiring()
#include "session/CurrentUser.h"

using namespace XmlRpc;

namespace userrpc {

static XmlRpcValue norm(XmlRpcValue& p){
    if (p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

UserChangePassword::UserChangePassword(XmlRpcServer* s,
                                       SessionManager& sm,
                                       IUsersRepo& r,
                                       PALogger& L)
: XmlRpcServerMethod("user.changePassword", s)
, sessions_(sm)
, repo_(r)
, log_(L)
{}

void UserChangePassword::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        XmlRpcValue a = norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct ||
            !a.hasMember("user") ||
            !a.hasMember("old")  ||
            !a.hasMember("new")) {
            throw XmlRpcException("BAD_REQUEST: se esperaban {user, old, new}");
        }

        const std::string user = std::string(a["user"]);
        const std::string oldp = std::string(a["old"]);
        const std::string newp = std::string(a["new"]);

        // 1) Validar credenciales con AuthService (reemplaza repo_.validate del CSV)
        auto& W = auth_wiring();
        auto au = W.auth->login(user, oldp);
        if (!au){
            log_.warning("[user.changePassword] FAIL -- user=" + user + " (credenciales inválidas)");
            throw XmlRpcException("AUTH_INVALID: usuario/clave incorrectos");
        }

        // 2) Actualizar hash (reemplaza changePass del CSV)
        const std::string newHash = W.auth->makeHash(newp);
        repo_.updatePasswordHash(au->id, newHash);

        // (opcional) invalidar sesiones previas: sessions_.invalidateUser(au->id);

        result["ok"]   = true;
        result["user"] = user;
        log_.info("[user.changePassword] OK -- user=" + user);
    }
    catch(const XmlRpcException&){ throw; }
    catch(...){ throw XmlRpcException("INTERNAL_ERROR: user.changePassword"); }
}

std::string UserChangePassword::help(){
    return "user.changePassword {user, old, new} -> ok";
}

} // namespace userrpc

