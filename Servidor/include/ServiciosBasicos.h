#ifndef SERVICIOS_BASICOS_H
#define SERVICIOS_BASICOS_H

#include "../lib/xmlrpc/XmlRpc.h"
using namespace XmlRpc;

class ServicioPrueba : public XmlRpcServerMethod {
    public:
        ServicioPrueba(XmlRpcServer* servidor);

        void execute(XmlRpcValue& params, XmlRpcValue& result);
        std::string help();

};

#endif // SERVICIOS_BASICOS_H