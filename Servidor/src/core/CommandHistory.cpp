#include "core/CommandHistory.h"
#include <chrono>   // Para obtener la hora actual
#include <sstream>  // Para formatear el string de la hora
#include <iomanip>  // Para formatear el string de la hora (setw, setfill)
#include <iostream> // (Opcional, para debug)

/**
 * @brief Función helper privada para obtener un timestamp actual en formato ISO 8601 (YYYY-MM-DDTHH:MM:SS).
 */
std::string CommandHistory::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    // std::put_time usa el 'struct tm' en formato local. 
    // Usamos gmtime_s o gmtime (dependiendo del compilador) para UTC si fuera necesario,
    // pero para un log local, la hora local suele ser más útil.
    
    // C++ moderno ofrece std::localtime, pero algunos compiladores pueden requerir
    // una versión más segura (localtime_s). Para compatibilidad, usamos std::localtime.
    
    #pragma warning(suppress : 4996) // Suprime la advertencia de C4996 sobre std::localtime
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");
    
    return ss.str();
}


/**
 * @brief Agrega una nueva entrada al historial de comandos.
 */
void CommandHistory::addEntry(const std::string& user, const std::string& service, 
                              const std::string& details, bool is_error) {
    
    // 1. Crear la entrada
    CommandEntry new_entry;
    new_entry.timestamp = get_current_timestamp();
    new_entry.username = user;
    new_entry.service_name = service;
    new_entry.details = details;
    new_entry.was_error = is_error;

    // --- ¡NUESTRA FUSIÓN! ---
    // 2. Llamar a PALogger para que guarde en el CSV
    std::string resultado = is_error ? ("ERROR: " + details) : "OK";
    // (Aún no tenemos el "nodo" (IP), así que lo dejamos default)
    logger_.logRequest(user, service, resultado);
    // --- FIN DE LA FUSIÓN ---

    // 3. Bloquear el mutex
    std::lock_guard<std::mutex> lock(mtx_);
    
    // 4. Modificar el vector
    entries_.push_back(new_entry);
    
    // 5. El mutex se desbloquea automáticamente
}


/**
 * @brief Obtiene una copia de todas las entradas de un usuario específico.
 */
std::vector<CommandEntry> CommandHistory::getEntriesForUser(const std::string& username) const {
    
    std::vector<CommandEntry> user_entries; // Lista de resultado

    // 1. Bloquear el mutex antes de LEER el vector
    // Usamos 'const_cast' si es necesario o 'mutable' en el .h (que ya hicimos)
    std::lock_guard<std::mutex> lock(mtx_);
    
    // 2. Recorrer la lista y copiar solo las entradas del usuario
    for (const auto& entry : entries_) {
        if (entry.username == username) {
            user_entries.push_back(entry);
        }
    }
    
    // 3. El mutex se desbloquea.
    
    return user_entries; // Devolvemos la copia
}


/**
 * @brief Obtiene una copia de TODAS las entradas (para el admin).
 */
std::vector<CommandEntry> CommandHistory::getAllEntries() const {
    // 1. Bloquear
    std::lock_guard<std::mutex> lock(mtx_);
    
    // 2. Devolver una copia del vector completo
    // Esto es seguro porque 'entries_' no se modificará mientras lo copiamos.
    return entries_; 
    
    // 3. Desbloquear (automático)
}


/**
 * @brief Limpia el historial de un usuario.
 */
void CommandHistory::clearUserHistory(const std::string& username) {
    
    // 1. Bloquear
    std::lock_guard<std::mutex> lock(mtx_);
    
    // 2. Usar el patrón "erase-remove idiom" de C++ para borrar elementos
    // entries_.erase(
    //     std::remove_if(entries_.begin(), entries_.end(), 
    //                    [&username](const CommandEntry& e) { return e.username == username; }),
    //     entries_.end()
    // );
    
    // O una forma más simple de entender (pero potencialmente menos eficiente):
    std::vector<CommandEntry> new_entries;
    for (const auto& entry : entries_) {
        if (entry.username != username) {
            new_entries.push_back(entry);
        }
    }
    entries_ = new_entries; // Reemplazamos el vector
    
    // 3. Desbloquear (automático)
}