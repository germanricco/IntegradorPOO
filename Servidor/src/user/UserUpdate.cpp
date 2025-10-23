#include "user/UserUpdate.h"
#include "common/AuthZ.h"
#include <optional>
#include <stdexcept>

using namespace XmlRpc;
namespace userrpc {

UserUpdate::UserUpdate(XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L)
: XmlRpcServerMethod("user.update", s), sessions_(sm), repo_(r), log_(L) {}

void UserUpdate::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try {
        XmlRpcValue a = rpc_norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token") || !a.hasMember("user"))
            throw XmlRpcException("BAD_REQUEST: token,user obligatorios");

        std::string tok  = std::string(a["token"]);
        std::string user = std::string(a["user"]);

        //valida sesión y ADMIN c
        const SessionView& sv = guardAdmin("update", sessions_, tok, log_);
        std::string actor = sv.user;

        std::optional<std::string> newUser;
        std::optional<Priv> newPriv;
        std::optional<bool> hab;

        if (a.hasMember("newUser"))       newUser = std::string(a["newUser"]);
        if (a.hasMember("newPrivilegio")) newPriv = privFromStr(std::string(a["newPrivilegio"]));
        if (a.hasMember("habilitado"))    hab     = bool(a["habilitado"]);

        repo_.update(user,
                     newUser ? &*newUser : nullptr,
                     newPriv ? &*newPriv : nullptr,
                     hab     ? &*hab     : nullptr);

        result["ok"] = true;

        std::string info;
        if (newUser) info += "newUser=" + *newUser + "; ";
        if (newPriv) info += "newPriv=" + privToStr(*newPriv) + "; ";
        if (hab)     info += std::string("hab=") + (*hab ? "1" : "0") + "; ";
        if (info.empty()) info = "sin cambios";

        log_.info("[user] update OK — actor=" + actor + ", target=" + user + " (" + info + ")");
    }
    catch (const std::runtime_error& e){
        log_.warning(std::string("[user] update FAIL — ") + e.what());
        throw XmlRpcException(e.what());
    }
    catch (const XmlRpcException&){ throw; }
    catch (...) {
        log_.error("[user] update ERROR — excepción inesperada");
        throw XmlRpcException("BAD_REQUEST: user.update");
    }
}

std::string UserUpdate::help() {
    return "user.update({token:string, user:string, newUser?:string, newPrivilegio?:'admin'|'op'|'viewer', habilitado?:bool}) -> {ok:bool}";
}

} // namespace userrpc


