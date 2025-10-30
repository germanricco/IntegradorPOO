#ifndef CURRENT_USER_H
#define CURRENT_USER_H

// Contexto de usuario por hilo (request). Se setea en el login exitoso.
// Si no hay usuario, devuelve -1.

namespace CurrentUser {
    void set(int userId);   // setear al loguearse
    int  get();             // -1 si no hay
    void clear();           // limpiar al cerrar sesión o cuando quieras “sin contexto”
}

#endif // CURRENT_USER_H

