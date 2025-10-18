#include <iostream>
#include <stdlib.h>
#include "../include/ServiciosBasicos.h"
#include "../include/PALogger.h"

using namespace XmlRpc;

int main(int argc, char** argv) {
    
    // Instanciar Logger
    PALogger logger(LogLevel::INFO, true, "servidor.log");

    logger.info(" === INICIANDO SERVIDOR XML-RPC ===");
    // Verificar que se haya proporcionado el puerto
    if (argc != 2) {
        logger.error("Uso incorrecto");
        std::cerr << "Uso: " << argv[0] << " <puerto>" << std::endl;
        return 1;
    }

    // Buscar puerto en argumentos
    int port = atoi(argv[1]);
    logger.info("Iniciando servidor en puerto " + std::to_string(port));

    // Crear servidor
    XmlRpcServer server;

    // Registrar metodos
    logger.info("Registrando servicios");

    ServicioPrueba servicioPrueba(&server);
    logger.debug("Servicio de prueba registrado");
    
    // Iniciar Servidor (crear socket de servidor enlazado a puerto)
    logger.info("Enlazando servidor al puerto " + std::to_string(port));
    if(!server.bindAndListen(port)) {
        std::cerr << "Error al enlazar el servidor al puerto " << port << std::endl;
        return 1;
    }
    logger.info("Enlace realizado con exito");

    // Habilitar introspección
    server.enableIntrospection(true);
    logger.info("Introspeccion habilitada");

    // Escuchar y atender clientes
    server.work(-1.0); // -1.0 = esperar indefinidamente
    
    logger.info(" === SERVIDOR XML-RPC FINALIZADO ===");
    return 0;
}