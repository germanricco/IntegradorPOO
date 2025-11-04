#include "auth/AuthLogin.h"
#include "services/AuthBootstrap.h"
#include "session/CurrentUser.h"
#include "XmlRpc.h"
using namespace XmlRpc;

namespace auth {

static XmlRpcValue norm(XmlRpcValue& p){
    if(p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

AuthLogin::AuthLogin(XmlRpcServer* s, SessionManager& sm, PALogger& L, IUsersRepo& repo)
: XmlRpcServerMethod("auth.login", s), sessions_(sm), logger_(L), repo_(repo) {}

void AuthLogin::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        XmlRpcValue a = norm(params);
        if(a.getType()!=XmlRpcValue::TypeStruct || !a.hasMember("user") || !a.hasMember("pass"))
            throw XmlRpcException("BAD_REQUEST: se esperaban claves 'user' y 'pass'");

        std::string user = std::string(a["user"]);
        std::string pass = std::string(a["pass"]);

        auto& W = auth_wiring();  // accedemos a { db, repo, auth }

        // usamos AuthService::login (consulta + hash)
        auto authUser = W.auth->login(user, pass);
        if (!authUser) {
          logger_.warning("[auth] login FAIL -- user=" + user + " (credenciales inválidas)");
          throw XmlRpcException("AUTH_INVALID: usuario/clave incorrectos");
        }

        // usuario activo (por si agregás lógica adicional)
        CurrentUser::set(authUser->id);

        // si querés, log de éxito  
        logger_.info("[auth] login OK -- user=" + user);

        // Datos “fuertes” del usuario
        result["id"]       = authUser->id;
        result["username"] = authUser->username;
        result["role"]     = authUser->role;

        // Token de sesión: SessionManager::create(username, role)
        std::string tok = sessions_.create(authUser->username, authUser->role);

        // Campos de salida del método (mantengo tus claves originales)
        result["ok"]        = true;
        result["token"]     = tok;
        result["user"]      = authUser->username;
        result["privilegio"]= authUser->role; 

        CurrentUser::clear();

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

