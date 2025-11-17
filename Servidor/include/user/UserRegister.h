#ifndef USERREGISTER_H
#define USERREGISTER_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "utils/PALogger.h"
#include "auth/IUsersRepo.h"

namespace userrpc {

class UserRegister : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    IUsersRepo&     repo_;
    PALogger&       log_;

public:
    UserRegister(XmlRpc::XmlRpcServer* s,
                 SessionManager& sm,
                 IUsersRepo& r,
                 PALogger& L);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace userrpc
#endif

