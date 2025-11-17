#ifndef AUDITLOGREADER_H
#define AUDITLOGREADER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "PALogger.h" // Lo usamos para el path

/**
 * @brief Estructura para almacenar una línea parseada del audit.csv
 * (Basado en el requisito del PDF)
 */
struct AuditEntry {
    std::string timestamp;
    std::string peticion;
    std::string usuario;
    std::string nodo;
    std::string respuesta;
};

/**
 * @brief Clase Helper para leer y filtrar el archivo audit.csv
 * Esta clase se encarga de todo el I/O y parsing del CSV.
 */
class AuditLogReader {
private:
    std::string auditFilePath_; // Ruta al archivo audit.csv
    PALogger& logger_;          // Para loguear errores si el archivo no existe

    /**
     * @brief Helper interno para parsear una sola línea de CSV.
     * Maneja comillas y comas.
     */
    AuditEntry parseCsvLine(const std::string& line) const;

public:
    /**
     * @brief Constructor
     * @param auditFilename El path al archivo audit.csv
     * @param logger Una referencia al PALogger para reportar errores
     */
    AuditLogReader(const std::string& auditFilename, PALogger& logger)
        : auditFilePath_(auditFilename), logger_(logger) {}

    /**
     * @brief Obtiene las entradas del log, aplicando filtros.
     * @param filter_user Filtra por este usuario (si no está vacío).
     * @param filter_response_contains Filtra si la respuesta contiene este texto (si no está vacío).
     * @return Un vector de entradas de auditoría que coinciden.
     */
    std::vector<AuditEntry> getEntries(const std::string& filter_user,
                                     const std::string& filter_response_contains) const;
};

#endif // AUDITLOGREADER_H