#ifndef ROBOT_GRIPPER_METHOD_H
#define ROBOT_GRIPPER_METHOD_H

#include "../../lib/xmlrpc/XmlRpc.h" // Ajusta si tu ruta es diferente
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../robot_model/RobotService.h"
#include "../common/AuthZ.h"
#include "../core/CommandHistory.h"


namespace robot_service_methods {

class RobotGripperMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_;
    CommandHistory& history_; // Referencia a la fachada del robot

public:
    RobotGripperMethod(XmlRpc::XmlRpcServer* server,
                      SessionManager& sm,
                      PALogger& L,
                      RobotService& rs,
                      CommandHistory& ch );

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_GRIPPER_METHOD_H