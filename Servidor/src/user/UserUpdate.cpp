#include "user/UserUpdate.h"

using namespace XmlRpc;

namespace userrpc {

static XmlRpcValue norm(XmlRpcValue& p){
    if (p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

UserUpdate::UserUpdate(XmlRpcServer* s,
                       SessionManager& sm,
                       IUsersRepo& r,
                       PALogger& L)
: XmlRpcServerMethod("user.update", s)
, sessions_(sm)
, repo_(r)
, log_(L)
{}

void UserUpdate::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        XmlRpcValue a = norm(params);
        if (a.getType()!=XmlRpcValue::TypeStruct ||
            !a.hasMember("id") ||
            !a.hasMember("active")) {
            throw XmlRpcException("BAD_REQUEST: se esperaban {id, active}");
        }

        const int  id     = int(a["id"]);
        const bool active = bool(a["active"]);

        repo_.setActive(id, active);

        result["ok"]     = true;
        result["id"]     = id;
        result["active"] = active;
        log_.info("[user.update] OK -- id=" + std::to_string(id) + " active=" + (active?"1":"0"));
    }
    catch(const XmlRpcException&){ throw; }
    catch(...){ throw XmlRpcException("INTERNAL_ERROR: user.update"); }
}

std::string UserUpdate::help(){
    return "user.update {id, active} -> ok";
}

} // namespace userrpc

