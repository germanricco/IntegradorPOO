#ifndef SERVICIOSROBOT_H
#define SERVICIOSROBOT_H

#include "../lib/xmlrpc/XmlRpc.h"
using namespace XmlRpc;

//! Hay que agregar todos los metodos de RobotService para
//! ser usados por el servidor XML-RPC

class HomingMethod : public XmlRpcServerMethod {
    public:
        HomingMethod(XmlRpcServer* servidor);

        void execute(XmlRpcValue& params, XmlRpcValue& result);
        std::string help();
};

#endif // SERVICIOSROBOT_H