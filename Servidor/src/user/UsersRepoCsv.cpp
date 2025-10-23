#include "user/UsersRepoCsv.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <functional> // std::hash

static constexpr const char* CSV_HEADER = "id,username,pass_hash,privilegio,habilitado,creado_en\n";

std::string UsersRepoCsv::trim(const std::string& s){
    size_t a = s.find_first_not_of(" \t\r\n");
    if(a==std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}
std::string UsersRepoCsv::nowISO(){
    std::time_t t = std::time(nullptr);
    char buf[32]; std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&t));
    return buf;
}
// Hash MUY simple para TP (no usar en producción)
std::string UsersRepoCsv::hashPass(const std::string& plain){
    static const std::string SALT = "_tp2025_salt_";
    return std::to_string(std::hash<std::string>{}(plain + SALT));
}

void UsersRepoCsv::rebuildIndex(){
    byName_.clear();
    for(size_t i=0;i<users_.size();++i) byName_[users_[i].username] = (int)i;
}
int UsersRepoCsv::nextId() const {
    int m=0; for(auto& u: users_) m = std::max(m, u.id); return m+1;
}
int UsersRepoCsv::countAdmins() const {
    int c=0; for(auto& u: users_) if(u.priv==Priv::Admin) ++c; return c;
}

UsersRepoCsv::UsersRepoCsv(std::string path) : path_(std::move(path)) {
    load();
    ensureDefaultAdmin();
}

void UsersRepoCsv::load(){
    users_.clear(); byName_.clear();
    std::ifstream in(path_);
    if(!in) return; // no existe todavía
    std::string line;
    std::getline(in, line); // header opcional
    while(std::getline(in, line)){
        if(line.empty()) continue;
        std::stringstream ss(line);
        std::string sId,sUser,sHash,sPriv,sHab,sCreado;
        std::getline(ss, sId, ',');
        std::getline(ss, sUser, ',');
        std::getline(ss, sHash, ',');
        std::getline(ss, sPriv, ',');
        std::getline(ss, sHab, ',');
        std::getline(ss, sCreado, ',');

        UserEntity u;
        u.id        = std::stoi(trim(sId));
        u.username  = trim(sUser);
        u.pass_hash = trim(sHash);
        u.priv      = privFromStr(trim(sPriv));
        u.habilitado= (trim(sHab)=="1");
        u.creado_en = trim(sCreado);

        users_.push_back(u);
    }
    rebuildIndex();
}

void UsersRepoCsv::save() const {
    std::ofstream out(path_);
    out << CSV_HEADER;
    for(const auto& u: users_){
        out << u.id << "," << u.username << "," << u.pass_hash << ","
            << privToStr(u.priv) << "," << (u.habilitado?"1":"0") << ","
            << u.creado_en << "\n";
    }
}

void UsersRepoCsv::ensureDefaultAdmin(){
    // si no hay admin, crea admin/1234 habilitado
    if(countAdmins() == 0){
        UserEntity u;
        u.id        = nextId();
        u.username  = "admin";
        u.pass_hash = hashPass("1234");
        u.priv      = Priv::Admin;
        u.habilitado= true;
        u.creado_en = nowISO();
        users_.push_back(u);
        rebuildIndex();
        save();
    }
}

bool UsersRepoCsv::exists(const std::string& user) const {
    return byName_.count(user) > 0;
}

CredCheck UsersRepoCsv::validate(const std::string& user, const std::string& passPlain) const {
    auto it = byName_.find(user);
    if(it == byName_.end()) return {};
    const auto& u = users_.at(it->second);
    if(u.pass_hash != hashPass(passPlain)) return {};
    return { true, privToStr(u.priv), u.habilitado };
}

// (estos se usarán en el Paso 4)
int UsersRepoCsv::create(const std::string& user, const std::string& passPlain, Priv p){
    if(exists(user)) throw std::runtime_error("USER_EXISTS");
    if(p==Priv::Admin && countAdmins()>=1) throw std::runtime_error("SINGLE_ADMIN_VIOLATION");
    UserEntity u;
    u.id        = nextId();
    u.username  = user;
    u.pass_hash = hashPass(passPlain);
    u.priv      = p;
    u.habilitado= true;
    u.creado_en = nowISO();
    users_.push_back(u);
    rebuildIndex();
    save();
    return u.id;
}

void UsersRepoCsv::update(const std::string& user, const std::string* newUser, const Priv* newPriv, const bool* habilitado){
    auto it = byName_.find(user);
    if(it==byName_.end()) throw std::runtime_error("USER_NOT_FOUND");
    UserEntity& u = users_.at(it->second);

    if(newUser){
        if(exists(*newUser)) throw std::runtime_error("USER_EXISTS");
        u.username = *newUser;
    }
    if(newPriv){
        if(*newPriv==Priv::Admin && u.priv!=Priv::Admin && countAdmins()>=1) throw std::runtime_error("SINGLE_ADMIN_VIOLATION");
        if(*newPriv!=Priv::Admin && u.priv==Priv::Admin && countAdmins()<=1) throw std::runtime_error("SINGLE_ADMIN_VIOLATION");
        u.priv = *newPriv;
    }
    if(habilitado){
        u.habilitado = *habilitado;
    }
    rebuildIndex();
    save();
}

void UsersRepoCsv::changePass(const std::string& user, const std::string& newPass){
    auto it = byName_.find(user);
    if(it==byName_.end()) throw std::runtime_error("USER_NOT_FOUND");
    users_.at(it->second).pass_hash = hashPass(newPass);
    save();
}

