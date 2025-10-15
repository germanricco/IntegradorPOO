#include "PALogger.h"

using namespace std;
	

PALogger::PALogger(const LogLevel &level, bool logToFile, const std::string& filename) 
	: level_(level), logToFile_(logToFile) {
			
	if (logToFile_) {
		logFile_.open(filename, std::ios::app); 
		if (!logFile_.is_open()) {
			std::cerr << "Error al abrir el archivo de log: " << filename << std::endl;
			std::cerr << "Se continua sin registro permanente. " << std::endl;
			logToFile_ = false;
		}
	}
}
		
PALogger::~PALogger() {
	if (logToFile_) {
		if (logFile_.is_open()) {
			logFile_.close();
		}
	}
}
		
void PALogger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
		
void PALogger::info(const std::string& message) { log(LogLevel::INFO, message); }
		
void PALogger::warning(const std::string& message) { log(LogLevel::WARNING, message); }
		
void PALogger::error(const std::string& message) { log(LogLevel::ERROR, message); }
		

			
void PALogger::log(LogLevel level, const std::string& message) {
	std::time_t now = std::time(0); // Hora actual
	std::tm timeinfo;
	localtime_r(&now, &timeinfo); 	// Lleva hora a local en timeinfo [POSIX]
			
	std::stringstream messageNew;
	messageNew << "[" << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << "] ";
	// tambien puede usar asctime_r(&timeinfo, buf) junto un char buffer[N];
			
	switch (level) {
		case LogLevel::DEBUG:
			messageNew << "[DEBUG] ";
			break;
		case LogLevel::INFO:
			messageNew << "[INFO] ";
			break;
		case LogLevel::WARNING:
			messageNew << "[WARNING] ";
			break;
		case LogLevel::ERROR:
			messageNew << "[ERROR] ";
			break;
	}

	messageNew << message;
	std::string finalMessage = messageNew.str();

	// Si el nivel del mensaje es menor que el nivel configurado, no lo mostramos
    if (static_cast<int>(level) < static_cast<int>(level_)) {
        return;
    }

    // Mostrar en consola
    std::cout << finalMessage << std::endl;
	
    // Escribir en archivo si esta activo
	if (logToFile_) {
		if (!logFile_.is_open()) {
			std::cerr << "El archivo de log no esta abierto." << std::endl;
			return;
		}
				
		logFile_ << finalMessage << std::endl;
        logFile_.flush(); // Asegurar que se escribe inmediatamente
	}			
}