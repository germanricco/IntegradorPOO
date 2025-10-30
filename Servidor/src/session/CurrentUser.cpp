#include "session/CurrentUser.h"
#include <thread>

// thread_local: cada request usa su copia
static thread_local int g_current_user_id = -1;

namespace CurrentUser {
    void set(int userId) { g_current_user_id = userId; }
    int  get()           { return g_current_user_id; }
    void clear()         { g_current_user_id = -1; }
}

