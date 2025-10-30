#ifndef IUSERSREPO_H
#define IUSERSREPO_H

#include <optional>
#include <string>
#include <vector>

struct UserDTO {
  int id{};
  std::string username;
  std::string password_hash;
  std::string role;      // "admin" | "operator" | "viewer"
  bool is_active{true};
};

class IUsersRepo {
public:
  virtual ~IUsersRepo() = default;
  virtual std::optional<UserDTO> findByUsername(const std::string&) = 0;
  virtual std::optional<UserDTO> findById(int id) = 0;
  virtual int insert(const UserDTO&) = 0;
  virtual void setActive(int userId, bool active) = 0;
  virtual void updatePasswordHash(int userId, const std::string& hash) = 0;
  virtual std::vector<UserDTO> listAll() = 0;
};

#endif
