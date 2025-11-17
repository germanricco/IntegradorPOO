#ifndef AUTHLOGOUT_H
#define AUTHLOGOUT_H

#include "XmlRpc.h"
#include "session/SessionManager.h"
#include "utils/PALogger.h"

namespace auth {

class AuthLogout : public XmlRpc::XmlRpcServerMethod {
    SessionManager& sessions_;
    PALogger& logger_;
public:
    AuthLogout(XmlRpc::XmlRpcServer* server, SessionManager& sm, PALogger& L);
    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace auth


#endif
