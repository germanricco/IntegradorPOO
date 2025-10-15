#ifndef PALOGGER_H
#define PALOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

/*
 Clase PALogger que registra eventos y los almacena en un archivo,
 haciendo uso interno de ofstream para la escritura.
 Clase sin modularizar sin header ni optimizaciones especiales.
 Gestiona mensajes de 4 tipos: depuracion, informacion, advertencia y error
 debug, info, warning, error: metodos para registrar mensajes con diferentes nivel.
 Pendiente el agregado de metodo para fallos criticos.
 log (privado): metodo interno a la clase, que da formato y guarda al mensaje.
 LogLevel enum: sirve para definir los diferentes niveles de registro.
 level_: atributo que evita el registro de los mensajes por debajo de el.
 logToFile_: atributo que indica si se generan mensajes para registro en
 archivo o solo para salida en la interfaz de usuario.
 El formato de los mensajes es fijo. Sin embargo, admite la actualizaci�n de
 los mensajes al formato de registro. Mejoraria con un metodo de personalizacion,
 por ejemplo setFormat().
 El control de errores por apertura y existencia del archivo deberia 
 hacerse usando excepciones.
 Deberia revisarse la conveniencia del cambio automatico de logToFile a false.
*/


enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class PALogger {
    private:
        LogLevel level_;
        bool logToFile_;
        std::ofstream logFile_;

    public:
        
        
        /**
         * @brief Constructor
         * @param level Nivel de registro (por defecto LogLevel::INFO)
         * @param logToFile Indica si se registra en archivo o no
         * @param filename Nombre del archivo log
         */
        PALogger(const LogLevel &level = LogLevel::INFO,
                 bool logToFile = false,
                 const std::string& filename = "servidor.log");

        /**
         * @brief Destructor
         */
        ~PALogger();
        
        /**
         * @brief Metodos para registrar mensajes
         * @param message Mensaje
         */
        void debug(const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);

    private:
        /**
         * @brief Metodo interno a la clase
         * @param level Nivel de registro
         * @param message Mensaje
         */
        void log(LogLevel level, const std::string& message);
};

#endif // PALOGGER_H