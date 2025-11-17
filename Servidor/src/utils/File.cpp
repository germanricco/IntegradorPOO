#include "utils/File.h"

using namespace std;

File::File(const std::string& filename, const std::string& directory)
    : filePath(directory + "/" + filename), isOpenFlag(false) {

        // Crear directorios anidados si es necesario
        if (!directory.empty()){
            error_code ec;
            filesystem::create_directories(directory, ec);
            // Ignoramos errores de directorio ya existente
        }
}


File::~File() {
    if (isOpenFlag) {
        close();
    }
}

void File::ensureOpen(FileMode mode) {
    if (!isOpen() || currentMode != mode) {
        if (isOpen()) {
            close();
        }
        open(mode);
    }
}

void File::open(FileMode mode) {
    if (isOpen()) {
        close();
    }
    
    ios_base::openmode openMode = ios::binary;
    
    switch (mode) {
        case FileMode::READ:
            openMode |= ios::in;
            break;
        case FileMode::WRITE:
            openMode |= ios::out | ios::trunc;
            break;
        case FileMode::APPEND:
            openMode |= ios::out | ios::app;
            break;
    }
    
    fileStream.open(filePath, openMode);
    if (!fileStream.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + filePath);
    }
    
    // Trackear el modo de apertura
    isOpenFlag = true;
    currentMode = mode;
}

void File::close() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
    isOpenFlag = false;
}

bool File::isOpen() const {
    return isOpenFlag && fileStream.is_open();
}

bool File::exists() const {
    error_code ec;
    return filesystem::exists(filePath, ec);
}

void File::write(const std::string& content) {
    // Verificar que esta abierto en el modo correcto
    ensureOpen(FileMode::WRITE);
    
    fileStream << content;
    fileStream.flush();
    
    if (!fileStream.good()) {
        throw runtime_error("Error al escribir en el archivo: " + filePath);
    }
}

void File::append(const std::string& content) {
    // Verificarr que esta abierto en el modo correcto
    ensureOpen(FileMode::APPEND);
    
    fileStream << content;
    fileStream.flush();
    
    if (!fileStream.good()) {
        throw runtime_error("Error al añadir al archivo: " + filePath);
    }
}

std::string File::readAll() {
    // No reabrir si esta abierto en el modo correcto
    ensureOpen(FileMode::READ);

    fileStream.seekg(0, ios::end);
    size_t size = fileStream.tellg();
    fileStream.seekg(0, ios::beg);
    
    if (size == 0) {
        return "";
    }
    
    string content(size, '\0');
    fileStream.read(&content[0], size);
    
    if (!fileStream.good() && !fileStream.eof()) {
        throw runtime_error("Error al leer el archivo: " + filePath);
    }
    
    return content;
}

std::vector<std::string> File::readLines() {
    string content = readAll();
    vector<string> lines;
    stringstream ss(content);
    string line;
    
    while (getline(fileStream, line)) {
        lines.push_back(line);
    }
    
    while (getline(ss, line)) {
        // Remove trailing \r if present (for Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    
    return lines;
}

std::string File::getFilePath() const {
    return filePath;
}

std::string File::getFilename() const {
    filesystem::path pathObj(filePath);
    return pathObj.filename().string();
}

std::string File::getDirectory() const {
    filesystem::path pathObj(filePath);
    return pathObj.parent_path().string();
}

size_t File::getSize() const {
    error_code ec;
    return filesystem::file_size(filePath, ec);
}

bool File::isEmpty() const {
    if (!exists()) {
        return true;
    }
    return getSize() == 0;
}