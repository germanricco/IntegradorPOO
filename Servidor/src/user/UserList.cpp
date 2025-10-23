#include "user/UserList.h"
#include "common/AuthZ.h"

using namespace XmlRpc;
namespace userrpc {

UserList::UserList(XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L)
: XmlRpcServerMethod("user.list", s), sessions_(sm), repo_(r), log_(L) {}

void UserList::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try {
        XmlRpcValue a = rpc_norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token"))
            throw XmlRpcException("BAD_REQUEST: falta token");

        std::string tok = std::string(a["token"]);

        //valida sesión y ADMIN
        const SessionView& sv = guardAdmin("list", sessions_, tok, log_);
        std::string actor = sv.user;

        XmlRpcValue arr; arr.setSize(0);
        int i = 0;
        for (const auto& u : repo_.list()) {
            XmlRpcValue item;
            item["id"]         = u.id;
            item["user"]       = u.username;
            item["privilegio"] = privToStr(u.priv);
            item["habilitado"] = u.habilitado;
            arr.setSize(i+1); arr[i++] = item;
        }

        result["ok"]   = true;
        result["users"]= arr;

        log_.info("[user] list OK — actor=" + actor + ", count=" + std::to_string(repo_.list().size()));
    }
    catch (const XmlRpcException&){ throw; }
    catch (...) {
        log_.error("[user] list ERROR — excepción inesperada");
        throw XmlRpcException("BAD_REQUEST: user.list");
    }
}

std::string UserList::help() {
    return "user.list({token:string}) -> {ok:bool, users:[{id:int,user:string,privilegio:string,habilitado:bool}]}";
}

} // namespace userrpc

