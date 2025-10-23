#ifndef USERENTITY_H
#define USERENTITY_H


#include <string>
#include <stdexcept>

enum class Priv { Admin, Op, Viewer };

inline std::string privToStr(Priv p){
    switch(p){ case Priv::Admin: return "admin"; case Priv::Op: return "op"; default: return "viewer"; }
}
inline Priv privFromStr(const std::string& s){
    if(s=="admin") return Priv::Admin;
    if(s=="op") return Priv::Op;
    if(s=="viewer") return Priv::Viewer;
    throw std::invalid_argument("Privilegio inválido");
}

struct UserEntity {
    int         id{0};
    std::string username;
    std::string pass_hash;   // hash (no texto plano)
    Priv        priv{Priv::Viewer};
    bool        habilitado{true};
    std::string creado_en;   // ISO-8601 (texto)
};


#endif
