/**
 * @file ServiciosBasicos.cpp
 * @author German Ricco
 * @brief Implementación de los servicios básicos del servidor XML-RPC.
 * @date 2025-15-10
 */

#include "ServiciosAdmin/ServiciosBasicos.h"

using namespace XmlRpc;

/**
 * Implamentacion de ServicioPrueba
 */

ServicioPrueba::ServicioPrueba(XmlRpcServer* servidor) 
    : XmlRpcServerMethod("ServicioPrueba", servidor) {}

void ServicioPrueba::execute(XmlRpcValue& params, XmlRpcValue& result)
{
    result = "Hola, soy el servidor XML-RPC!";
}

std::string ServicioPrueba::help()
{
    return "ServicioPrueba: Retorna un saludo del servidor.";
}