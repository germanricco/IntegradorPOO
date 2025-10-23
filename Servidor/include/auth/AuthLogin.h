#ifndef AUTHLOGIN_H
#define AUTHLOGIN_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "PALogger.h"
#include "user/UsersRepoCsv.h"

namespace auth {

class AuthLogin : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    PALogger&       logger_;
    UsersRepoCsv&   repo_;
public:
    AuthLogin(XmlRpc::XmlRpcServer* server, SessionManager& sm, PALogger& L, UsersRepoCsv& repo);
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace auth

#endif
