#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include <string>
#include <vector>
#include <mutex>
#include "utils/PALogger.h" // <-- AÑADIR ESTE INCLUDE

struct CommandEntry {
    std::string timestamp;
    std::string username;
    std::string service_name;
    std::string details;
    bool was_error;
};

class CommandHistory {
private:
    PALogger& logger_; // <-- AÑADIR REFERENCIA AL LOGGER
    std::vector<CommandEntry> entries_;
    mutable std::mutex mtx_;
    std::string get_current_timestamp() const;

public:
    // Modificar el constructor para recibir el logger
    CommandHistory(PALogger& logger) : logger_(logger) {} // <-- MODIFICAR CONSTRUCTOR

    void addEntry(const std::string& user, const std::string& service, 
                  const std::string& details, bool is_error);

    std::vector<CommandEntry> getEntriesForUser(const std::string& username) const;
    std::vector<CommandEntry> getAllEntries() const;
    void clearUserHistory(const std::string& username);

    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;
};

#endif // COMMANDHISTORY_H