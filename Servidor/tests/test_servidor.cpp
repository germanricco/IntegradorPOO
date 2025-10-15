#include <iostream>
#include <stdlib.h>
#include "../include/ServiciosBasicos.h"

using namespace XmlRpc;

int main(int argc, char** argv) {
    
    // Verificar que se haya proporcionado el puerto
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>" << std::endl;
        return 1;
    }

    // Buscar puerto en argumentos
    int port = atoi(argv[1]);

    // Crear servidor
    XmlRpcServer server;

    // Registrar metodos
    ServicioPrueba servicioPrueba(&server);
    
    // Iniciar Servidor (crear socket de servidor enlazado a puerto)
    if(!server.bindAndListen(port)) {
        std::cerr << "Error al enlazar el servidor al puerto " << port << std::endl;
        return 1;
    }
    // Habilitar introspección
    server.enableIntrospection(true);

    // Escuchar y atender clientes
    server.work(-1.0); // -1.0 = esperar indefinidamente
    
    return 0;
}