#include "auth/AuthLogin.h"
#include "XmlRpc.h"
using namespace XmlRpc;

namespace auth {

static XmlRpcValue norm(XmlRpcValue& p){
    if(p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

AuthLogin::AuthLogin(XmlRpcServer* s, SessionManager& sm, PALogger& L, UsersRepoCsv& repo)
: XmlRpcServerMethod("auth.login", s), sessions_(sm), logger_(L), repo_(repo) {}

void AuthLogin::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        XmlRpcValue a = norm(params);
        if(a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("user") || !a.hasMember("pass"))
            throw XmlRpcException("BAD_REQUEST: se esperaban claves 'user' y 'pass'");

        std::string user = std::string(a["user"]);
        std::string pass = std::string(a["pass"]);

        auto ck = repo_.validate(user, pass);
        if(!ck.ok) {
            // Log claro y humano para credenciales inválidas
            logger_.warning("[auth] login FAIL — user=" + user + " (credenciales inválidas)");
            throw XmlRpcException("AUTH_INVALID: usuario/clave incorrectos");
        }
        if(!ck.habilitado) {
            logger_.warning("[auth] login FAIL — user=" + user + " (usuario deshabilitado)");
            throw XmlRpcException("USER_DISABLED: usuario deshabilitado");
        }

        std::string tok = sessions_.create(user, ck.priv);

        result["ok"]         = true;
        result["token"]      = tok;
        result["user"]       = user;
        result["privilegio"] = ck.priv;

        // Log de éxito (sin token en el log)
        logger_.info("[auth] login OK — user=" + user);
    }catch(const XmlRpcException&){ throw; }
     catch(...){ throw XmlRpcException("BAD_REQUEST: auth.login"); }
}

std::string AuthLogin::help(){
    return "auth.login({user:string, pass:string}) -> {ok:bool, token:string, user:string, privilegio:string}\n"
           "Valida contra CSV (crea admin/1234 si no existe).";
}

} // namespace auth

