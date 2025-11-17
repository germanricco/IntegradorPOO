#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include <string>
#include <vector>
#include <mutex> // Necesitamos un mutex para proteger la lista de accesos simultáneos

// Esta estructura define una entrada en nuestro historial.
// Es la información que necesitamos para el reporte.
struct CommandEntry {
    std::string timestamp;
    std::string username;     // El usuario que ejecutó
    std::string service_name; // ej. "robot.move"
    std::string details;      // ej. "X:100 Y:50 Z:20"
    bool was_error;
};

class CommandHistory {
private:
    std::vector<CommandEntry> entries_; // La lista de todas las entradas
    mutable std::mutex mtx_;            // Para proteger 'entries_' (thread-safety)

    std::string get_current_timestamp() const; // Función helper

public:
    CommandHistory() = default; // Constructor por defecto

    /**
     * @brief Agrega una nueva entrada al historial de comandos.
     * Es 'thread-safe'.
     */
    void addEntry(const std::string& user, const std::string& service, 
                  const std::string& details, bool is_error);

    /**
     * @brief Obtiene una copia de todas las entradas de un usuario específico.
     * Es 'thread-safe'.
     * @param username El usuario por el cual filtrar.
     * @return Un vector de CommandEntry solo para ese usuario.
     */
    std::vector<CommandEntry> getEntriesForUser(const std::string& username) const;

    /**
     * @brief Obtiene una copia de TODAS las entradas (para el admin).
     * Es 'thread-safe'.
     */
    std::vector<CommandEntry> getAllEntries() const;

    /**
     * @brief Limpia el historial de un usuario (podría ser útil).
     */
    void clearUserHistory(const std::string& username);

    // Prevenimos que la clase se copie (buena práctica para clases con mutex)
    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;
};

#endif // COMMANDHISTORY_H