#include "user/UserList.h"

using namespace XmlRpc;

namespace userrpc {

static XmlRpcValue norm(XmlRpcValue& p){
    if (p.getType()==XmlRpcValue::TypeArray && p.size()==1) return p[0];
    return p;
}

UserList::UserList(XmlRpcServer* s,
                   SessionManager& sm,
                   IUsersRepo& r,
                   PALogger& L)
: XmlRpcServerMethod("user.list", s)
, sessions_(sm)
, repo_(r)
, log_(L)
{}

void UserList::execute(XmlRpcValue& params, XmlRpcValue& result){
    try{
        (void)params; // sin args

        auto all = repo_.listAll();

        XmlRpcValue arr;
        arr.setSize(int(all.size()));
        for (int i=0;i<int(all.size());++i){
            XmlRpcValue u;
            u["id"]       = all[i].id;
            u["username"] = all[i].username;
            u["role"]     = all[i].role;
            u["active"]   = all[i].is_active;
            arr[i] = u;
        }
        result["ok"]    = true;
        result["users"] = arr;
    }
    catch(const XmlRpcException&){ throw; }
    catch(...){ throw XmlRpcException("INTERNAL_ERROR: user.list"); }
}

std::string UserList::help(){
    return "user.list -> {ok, users:[{id,username,role,active}...] }";
}

} // namespace userrpc

