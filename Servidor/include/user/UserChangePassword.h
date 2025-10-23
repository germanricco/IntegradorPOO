#ifndef USERCHANGEPASSWORD_H
#define USERCHANGEPASSWORD_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "user/UsersRepoCsv.h"
#include "PALogger.h"

namespace userrpc {

class UserChangePassword : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    UsersRepoCsv&   repo_;
    PALogger&       log_;
public:
    UserChangePassword(XmlRpc::XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace userrpc



#endif
