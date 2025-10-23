#ifndef USERUPDATE_H
#define USERUPDATE_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "user/UsersRepoCsv.h"
#include "PALogger.h"

namespace userrpc {

class UserUpdate : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    UsersRepoCsv&   repo_;
    PALogger&       log_;  
public:
    UserUpdate(XmlRpc::XmlRpcServer* s, SessionManager& sm, UsersRepoCsv& r, PALogger& L);
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace userrpc


#endif
