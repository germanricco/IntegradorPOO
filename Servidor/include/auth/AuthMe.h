#ifndef AUTHME_H
#define AUTHME_H

#include "XmlRpc.h"
#include "session/SessionManager.h"

namespace auth {

class AuthMe : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
public:
    AuthMe(XmlRpc::XmlRpcServer* server, SessionManager& sm);
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace auth

#endif
