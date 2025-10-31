#ifndef USER_UPDATE_H
#define USER_UPDATE_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "PALogger.h"
#include "auth/IUsersRepo.h"

namespace userrpc {

class UserUpdate : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    IUsersRepo&     repo_;
    PALogger&       log_;

public:
    UserUpdate(XmlRpc::XmlRpcServer* s,
               SessionManager& sm,
               IUsersRepo& r,
               PALogger& L);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace userrpc
#endif

