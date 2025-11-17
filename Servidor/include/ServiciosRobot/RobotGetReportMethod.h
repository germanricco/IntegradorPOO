#ifndef ROBOT_GET_REPORT_METHOD_H
#define ROBOT_GET_REPORT_METHOD_H

#include "../../lib/xmlrpc/XmlRpc.h" 
#include "../session/SessionManager.h"
#include "../utils/PALogger.h"
#include "../common/AuthZ.h"
#include "../core/CommandHistory.h"

namespace robot_service_methods {

class RobotGetReportMethod : public XmlRpc::XmlRpcServerMethod {
private:
    SessionManager& sessions_;
    PALogger&       logger_;
    CommandHistory& history_; // La herramienta principal de este servicio

public:
    RobotGetReportMethod(XmlRpc::XmlRpcServer* server,
                         SessionManager& sm,
                         PALogger& L,
                         CommandHistory& ch);

    void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result) override;
    std::string help() override;
};

} // namespace robot_service_methods

#endif // ROBOT_GET_REPORT_METHOD_H