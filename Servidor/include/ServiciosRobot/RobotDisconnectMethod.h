#ifndef ROBOT_DISCONNECT_METHOD_H
#define ROBOT_DISCONNECT_METHOD_H

#include "../../lib/xmlrpc/XmlRpc.h"
#include "../session/SessionManager.h"
#include "../PALogger.h"
#include "../RobotService.h"
#include "../common/AuthZ.h"

namespace robot_service_methods {

class RobotDisconnectMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_;

public:
    RobotDisconnectMethod(XmlRpc::XmlRpcServer* server,
                       SessionManager& sm,
                       PALogger& L,
                       RobotService& rs);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_DISCONNECT_METHOD_H