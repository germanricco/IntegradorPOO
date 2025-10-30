#include "storage/UsersRepoSqlite.h"

UsersRepoSqlite::UsersRepoSqlite(SqliteDb& db):db_(db){}

UserDTO UsersRepoSqlite::row(sqlite3_stmt* s){
  UserDTO u;
  u.id=sqlite3_column_int(s,0);
  u.username=(const char*)sqlite3_column_text(s,1);
  u.password_hash=(const char*)sqlite3_column_text(s,2);
  u.role=(const char*)sqlite3_column_text(s,3);
  u.is_active=sqlite3_column_int(s,4)!=0;
  return u;
}

std::optional<UserDTO> UsersRepoSqlite::findByUsername(const std::string& u){
  std::optional<UserDTO> out;
  db_.withPrepared(
    "SELECT id,username,password_hash,role,is_active FROM users WHERE username=?1",
    [&](sqlite3_stmt* st){ sqlite3_bind_text(st,1,u.c_str(),-1,SQLITE_TRANSIENT); },
    [&](sqlite3_stmt* st){ out=row(st); }
  );
  return out;
}

std::optional<UserDTO> UsersRepoSqlite::findById(int id){
  std::optional<UserDTO> out;
  db_.withPrepared(
    "SELECT id,username,password_hash,role,is_active FROM users WHERE id=?1",
    [&](sqlite3_stmt* st){ sqlite3_bind_int(st,1,id); },
    [&](sqlite3_stmt* st){ out=row(st); }
  );
  return out;
}

int UsersRepoSqlite::insert(const UserDTO& u){
  db_.withPrepared(
    "INSERT INTO users(username,password_hash,role,is_active) VALUES(?1,?2,?3,?4)",
    [&](sqlite3_stmt* st){
      sqlite3_bind_text(st,1,u.username.c_str(),-1,SQLITE_TRANSIENT);
      sqlite3_bind_text(st,2,u.password_hash.c_str(),-1,SQLITE_TRANSIENT);
      sqlite3_bind_text(st,3,u.role.c_str(),-1,SQLITE_TRANSIENT);
      sqlite3_bind_int(st,4,u.is_active?1:0);
    }, nullptr);
  return (int)sqlite3_last_insert_rowid(db_.handle());
}

void UsersRepoSqlite::setActive(int id,bool a){
  db_.withPrepared("UPDATE users SET is_active=?1 WHERE id=?2",
    [&](sqlite3_stmt* st){ sqlite3_bind_int(st,1,a?1:0); sqlite3_bind_int(st,2,id); }, nullptr);
}

void UsersRepoSqlite::updatePasswordHash(int id,const std::string& h){
  db_.withPrepared("UPDATE users SET password_hash=?1 WHERE id=?2",
    [&](sqlite3_stmt* st){ sqlite3_bind_text(st,1,h.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_int(st,2,id); }, nullptr);
}

std::vector<UserDTO> UsersRepoSqlite::listAll(){
  std::vector<UserDTO> v;
  db_.withPrepared(
    "SELECT id,username,password_hash,role,is_active FROM users ORDER BY id",
    nullptr, [&](sqlite3_stmt* st){ v.push_back(row(st)); });
  return v;
}

