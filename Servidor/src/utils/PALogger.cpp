#include "utils/PALogger.h"

using namespace std;
	
// --- Constructor Actualizado ---
PALogger::PALogger(const LogLevel &level, bool logToFile, const std::string& debugFilename, const std::string& auditFilename) 
	: level_(level), logToFile_(logToFile) {
			
	if (logToFile_) {
        // 1. Abrir el log de depuración (servidor.log)
		logFile_.open(debugFilename, std::ios::app); 
		if (!logFile_.is_open()) {
			std::cerr << "Error al abrir el archivo de log (debug): " << debugFilename << std::endl;
			std::cerr << "Se continua sin registro permanente. " << std::endl;
			logToFile_ = false; 
		}

        // 2. Abrir el log de auditoría (audit.csv)
        bool isNewAuditFile = false;
        std::ifstream testFile(auditFilename);
        if (!testFile.good()) {
            isNewAuditFile = true;
        }
        testFile.close();

        auditFile_.open(auditFilename, std::ios::app);
        if (!auditFile_.is_open()) {
			std::cerr << "Error al abrir el archivo de log (audit): " << auditFilename << std::endl;
		} else if (isNewAuditFile) {
            // Si es nuevo, escribimos la cabecera CSV
            auditFile_ << "timestamp,peticion,usuario,nodo,respuesta" << std::endl;
            auditFile_.flush();
        }
	}
}
		
// --- Destructor Actualizado ---
PALogger::~PALogger() {
	if (logToFile_ && logFile_.is_open()) {
		logFile_.close();
	}
    if (auditFile_.is_open()) {
        auditFile_.close();
    }
}
		
// --- (Funciones de log de depuración: sin cambios) ---
void PALogger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
void PALogger::info(const std::string& message) { log(LogLevel::INFO, message); }
void PALogger::warning(const std::string& message) { log(LogLevel::WARNING, message); }
void PALogger::error(const std::string& message) { log(LogLevel::ERROR, message); }
			
void PALogger::log(LogLevel level, const std::string& message) {
	std::time_t now = std::time(0);
	std::tm timeinfo;
	localtime_r(&now, &timeinfo); 	
	std::stringstream messageNew;
	messageNew << "[" << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << "] ";
	
	switch (level) {
		case LogLevel::DEBUG:   messageNew << "[DEBUG] ";   break;
		case LogLevel::INFO:    messageNew << "[INFO] ";    break;
		case LogLevel::WARNING: messageNew << "[WARNING] "; break;
		case LogLevel::ERROR:   messageNew << "[ERROR] ";   break;
	}
	messageNew << message;
	std::string finalMessage = messageNew.str();

    if (static_cast<int>(level) < static_cast<int>(level_)) { return; }
    std::cout << finalMessage << std::endl;
	
	if (logToFile_) {
		if (!logFile_.is_open()) {
			std::cerr << "El archivo de log (debug) no esta abierto." << std::endl;
			return;
		}
		logFile_ << finalMessage << std::endl;
        logFile_.flush(); 
	}			
}

// --- NUEVO: Helper de Timestamp para CSV ---
std::string PALogger::getTimestampCsv() const {
    std::time_t now = std::time(0);
	std::tm timeinfo;
	localtime_r(&now, &timeinfo);
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// --- NUEVO: Implementación de logRequest (CSV) ---
void PALogger::logRequest(const std::string& usuario,
                        const std::string& peticion,
                        const std::string& respuesta,
                        const std::string& nodo) {
    
    if (!auditFile_.is_open()) {
        std::cerr << "El archivo de log (audit) no esta abierto. No se pudo registrar la petición." << std::endl;
        return;
    }

    auto escapeCsv = [](std::string s) {
        if (s.find(',') != std::string::npos || s.find('"') != std::string::npos) {
            size_t pos = 0;
            while ((pos = s.find('"', pos)) != std::string::npos) {
                s.replace(pos, 1, "\"\"");
                pos += 2;
            }
            s = "\"" + s + "\"";
        }
        return s;
    };

    std::stringstream ss;
    ss << getTimestampCsv() << ","
       << escapeCsv(peticion) << ","
       << escapeCsv(usuario) << ","
       << escapeCsv(nodo) << ","
       << escapeCsv(respuesta) << std::endl;

    auditFile_ << ss.str();
    auditFile_.flush();
}