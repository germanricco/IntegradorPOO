#ifndef ROBOT_MOTORS_METHOD_H // 
#define ROBOT_MOTORS_METHOD_H // 

#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../PALogger.h"
#include "../RobotService.h"
#include "../common/AuthZ.h" 
#include "../CommandHistory.h"

namespace robot_service_methods {

// --- CAMBIO: Nombre de la clase ---
class RobotMotorsMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    RobotService&   robotService_;
    CommandHistory& history_; 

public:
    // --- CAMBIO: Nombre del constructor ---
    RobotMotorsMethod(XmlRpc::XmlRpcServer* server,
                      SessionManager& sm,
                      PALogger& L,
                      RobotService& rs,
                      CommandHistory& ch);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_MOTORS_METHOD_H //