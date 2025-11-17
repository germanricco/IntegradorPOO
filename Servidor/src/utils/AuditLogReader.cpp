#include "utils/AuditLogReader.h"
#include <iostream> // Para std::cerr

/**
 * @brief Helper interno para parsear una sola línea de CSV.
 * Esta es una función de parseo simple que maneja comillas.
 */
AuditEntry AuditLogReader::parseCsvLine(const std::string& line) const {
    AuditEntry entry;
    std::stringstream ss(line);
    std::string field;
    char c;

    std::vector<std::string> fields;
    bool in_quotes = false;
    std::string current_field;

    for (size_t i = 0; i < line.length(); ++i) {
        c = line[i];

        if (c == '"') {
            if (i + 1 < line.length() && line[i + 1] == '"') {
                // Es un escape de comillas ("")
                current_field += '"';
                i++; // Saltar la siguiente comilla
            } else {
                // Es el inicio o fin de un campo entrecomillado
                in_quotes = !in_quotes;
            }
        } else if (c == ',' && !in_quotes) {
            // Fin de un campo
            fields.push_back(current_field);
            current_field = "";
        } else {
            current_field += c;
        }
    }
    // Añadir el último campo
    fields.push_back(current_field);

    // Asignar los campos a la estructura
    if (fields.size() >= 5) {
        entry.timestamp = fields[0];
        entry.peticion = fields[1];
        entry.usuario = fields[2];
        entry.nodo = fields[3];
        entry.respuesta = fields[4];
    }

    return entry;
}

/**
 * @brief Obtiene las entradas del log, aplicando filtros.
 */
std::vector<AuditEntry> AuditLogReader::getEntries(
    const std::string& filter_user,
    const std::string& filter_response_contains) const {
    
    std::vector<AuditEntry> results;
    std::ifstream file(auditFilePath_);

    if (!file.is_open()) {
        logger_.error("AuditLogReader: No se pudo abrir el archivo de log: " + auditFilePath_);
        return results; // Devolver vector vacío
    }

    std::string line;
    // 1. Leer y descartar la línea de cabecera
    if (!std::getline(file, line)) {
        logger_.warning("AuditLogReader: El archivo de log CSV está vacío: " + auditFilePath_);
        return results; // Archivo vacío
    }

    // 2. Leer el resto del archivo línea por línea
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        AuditEntry entry = parseCsvLine(line);

        // 3. Aplicar filtros
        bool pass_user_filter = true;
        bool pass_response_filter = true;

        // Criterio 1: Filtrar por usuario (si se proveyó)
        if (!filter_user.empty()) {
            pass_user_filter = (entry.usuario == filter_user);
        }

        // Criterio 2: Filtrar por contenido de respuesta (si se proveyó)
        if (!filter_response_contains.empty()) {
            // Búsqueda "contiene" (case-insensitive sería mejor, pero esto funciona)
            pass_response_filter = (entry.respuesta.find(filter_response_contains) != std::string::npos);
        }

        // 4. Añadir a resultados si pasa ambos filtros
        if (pass_user_filter && pass_response_filter) {
            results.push_back(entry);
        }
    }

    file.close();
    return results;
}