#ifndef USERSREPOSQLITE_H
#define USERSREPOSQLITE_H


#include "auth/IUsersRepo.h"
#include "db/SqliteDb.h"

class UsersRepoSqlite : public IUsersRepo {
public:
  explicit UsersRepoSqlite(SqliteDb& db);
  std::optional<UserDTO> findByUsername(const std::string&) override;
  std::optional<UserDTO> findById(int id) override;
  int insert(const UserDTO&) override;
  void setActive(int userId, bool active) override;
  void updatePasswordHash(int userId, const std::string& hash) override;
  std::vector<UserDTO> listAll() override;
private:
  SqliteDb& db_;
  static UserDTO row(sqlite3_stmt*);
};


#endif
