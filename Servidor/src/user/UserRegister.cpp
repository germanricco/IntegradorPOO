#include "user/UserRegister.h"
#include "common/AuthZ.h"
#include <stdexcept>

using namespace XmlRpc;
namespace userrpc {

UserRegister::UserRegister(XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L)
: XmlRpcServerMethod("user.register", s), sessions_(sm), repo_(r), log_(L) {}

void UserRegister::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try {
        XmlRpcValue a = rpc_norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token") ||
            !a.hasMember("user") || !a.hasMember("pass") || !a.hasMember("privilegio"))
            throw XmlRpcException("BAD_REQUEST: token,user,pass,privilegio");

        std::string tok  = std::string(a["token"]);
        std::string user = std::string(a["user"]);
        std::string pass = std::string(a["pass"]);
        std::string prv  = std::string(a["privilegio"]);

        // valida sesión y ADMIN 
        const SessionView& sv = guardAdmin("register", sessions_, tok, log_);

        int id = repo_.create(user, pass, privFromStr(prv));
        result["ok"] = true;
        result["id"] = id;

        log_.info("[user] register OK — actor=" + sv.user + ", new=" + user + ", priv=" + prv);
    }
    catch (const std::runtime_error& e){
        log_.warning(std::string("[user] register FAIL — ") + e.what());
        throw XmlRpcException(e.what());
    }
    catch (const XmlRpcException&){ throw; }
    catch (...) {
        log_.error("[user] register ERROR — excepción inesperada");
        throw XmlRpcException("BAD_REQUEST: user.register");
    }
}

std::string UserRegister::help() {
    return "user.register({token:string, user:string, pass:string, privilegio:'admin'|'op'|'viewer'}) -> {ok:bool, id:int}";
}

} // namespace userrpc


