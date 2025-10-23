#include "auth/AuthLogout.h"
#include "common/AuthZ.h"
using namespace XmlRpc;

namespace auth {

AuthLogout::AuthLogout(XmlRpcServer* s, SessionManager& sm, PALogger& L)
: XmlRpcServerMethod("auth.logout", s), sessions_(sm), logger_(L) {}

void AuthLogout::execute(XmlRpcValue& params, XmlRpcValue& result) {
    try{
        XmlRpcValue a = rpc_norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("token"))
            throw XmlRpcException("BAD_REQUEST: falta 'token'");

        std::string tok = std::string(a["token"]);
        if(!sessions_.remove(tok)) {
            logger_.warning("[auth] logout FAIL — token inválido");
            throw XmlRpcException("AUTH_INVALID: token");
        }

        result["ok"]=true;

        // Log de éxito sin token
        logger_.info("[auth] logout OK");
    }
    catch (const XmlRpcException&){ throw; }
    catch (...){
        logger_.error("[auth] logout ERROR — excepción inesperada");
        throw XmlRpcException("BAD_REQUEST: auth.logout");
    }
}

std::string AuthLogout::help(){
    return "auth.logout({token:string}) -> {ok:bool}";
}

} // namespace auth


