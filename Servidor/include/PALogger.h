#ifndef PALOGGER_H
#define PALOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

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
        std::ofstream logFile_;      // Para servidor.log
        std::ofstream auditFile_;  // Para audit.csv

    public:
        
        /**
         * @brief Constructor
         * @param level Nivel de registro (por defecto LogLevel::INFO)
         * @param logToFile Indica si se registra en archivo o no
         * @param debugFilename Nombre del archivo log de depuración (ej. servidor.log)
         * @param auditFilename Nombre del archivo log de auditoría (ej. audit.csv)
         */
        PALogger(const LogLevel &level = LogLevel::INFO,
                 bool logToFile = false,
                 const std::string& debugFilename = "servidor.log",
                 const std::string& auditFilename = "audit.csv"); // <-- CAMBIO

        /**
         * @brief Destructor
         */
        ~PALogger();
        
        // Métodos para log de depuración (Formato 2)
        void debug(const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);

        /**
         * @brief Metodo para registrar peticiones de auditoría (Formato 1 - CSV)
         */
        void logRequest(const std::string& usuario,          // <-- NUEVA FUNCIÓN
                        const std::string& peticion,
                        const std::string& respuesta,
                        const std::string& nodo = "N/A");

    private:
        // Método interno para log de depuración
        void log(LogLevel level, const std::string& message);
        
        // Helper para el timestamp del CSV
        std::string getTimestampCsv() const; // <-- NUEVO HELPER
};

#endif // PALOGGER_H