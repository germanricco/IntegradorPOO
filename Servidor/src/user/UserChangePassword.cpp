#include "user/UserChangePassword.h"
#include "common/AuthZ.h"
#include <stdexcept>

using namespace XmlRpc;
namespace userrpc {

UserChangePassword::UserChangePassword(XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L)
: XmlRpcServerMethod("user.changePassword", s), sessions_(sm), repo_(r), log_(L) {}

void UserChangePassword::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try {
        XmlRpcValue a = rpc_norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token") || !a.hasMember("newPass"))
            throw XmlRpcException("BAD_REQUEST: token,newPass obligatorios");

        std::string tok     = std::string(a["token"]);
        std::string newPass = std::string(a["newPass"]);

        //valida sesión con logging
        const auto& s = guardSession("changePassword", sessions_, tok, log_);

        std::string target;
        std::string mode;

        if (a.hasMember("user")) {
            // admin resetea la clave de otro
            if (s.privilegio != "admin") {
                log_.warning("[user] changePassword FORBIDDEN — actor=" + s.user + " (admin-reset)");
                throw XmlRpcException("FORBIDDEN: privilegios insuficientes");
            }
            target = std::string(a["user"]);
            mode = "admin-reset";
            repo_.changePass(target, newPass);
        } else {
            // autoservicio
            if (!a.hasMember("oldPass")) throw XmlRpcException("BAD_REQUEST: falta oldPass para autoservicio");
            std::string old = std::string(a["oldPass"]);
            auto ck = repo_.validate(s.user, old);
            if (!ck.ok) {
                log_.warning("[user] changePassword FAIL — actor=" + s.user + " (oldPass inválida)");
                throw XmlRpcException("AUTH_INVALID: oldPass");
            }
            target = s.user;
            mode = "self";
            repo_.changePass(target, newPass);
        }

        result["ok"] = true;
        log_.info("[user] changePassword OK — actor=" + s.user + ", target=" + target + " (" + mode + ")");
    }
    catch (const std::runtime_error& e){
        log_.warning(std::string("[user] changePassword FAIL — ") + e.what());
        throw XmlRpcException(e.what());
    }
    catch (const XmlRpcException&){ throw; }
    catch (...) {
        log_.error("[user] changePassword ERROR — excepción inesperada");
        throw XmlRpcException("BAD_REQUEST: user.changePassword");
    }
}

std::string UserChangePassword::help() {
    return "user.changePassword({token:string, user?:string, oldPass?:string, newPass:string}) -> {ok:bool}\n"
           "Admin: {token,user,newPass}  |  Auto-servicio: {token,oldPass,newPass}";
}

} // namespace userrpc
