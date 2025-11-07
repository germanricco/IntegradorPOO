#ifndef SERVIDOR_CONFIG_H
#define SERVIDOR_CONFIG_H

#include <string>

struct ServidorConfig {
    // === Configuracion de red ===
    int puerto = 8080;
    int maxConexiones = 32;

    // === Configuracion de base de datos ===
    std::string rutaBaseDatos = "data/db/poo.db";

    // Configuración de autenticación
    std::string salt = "cambia_este_salt";
    const char* envSalt = std::getenv("AUTH_SALT");

    // === Configuracion de logging ===
    std::string archivoLog = "servidor.log";
    bool logEnConsola = true;

    // === Configuracion del robot ===
    std::string puertoSerial = "/dev/ttyUSB0";
    int baudrate = 115200;
    std::string directorioTrayectorias = "data/trayectorias/";

    // === Configuracion de modulos ===
    bool moduloRobotHabilitado = true;
    bool moduloUsuariosHabilitado = true;

    // === Métodos de carga de configuración ===
    static ServidorConfig cargarDesdeArchivo(const std::string& rutaArchivo);
    static ServidorConfig cargarDesdeArgumentos(int argc, char** argv);
    void validar() const;
};

#endif // SERVIDOR_CONFIG_H