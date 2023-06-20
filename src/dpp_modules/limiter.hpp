#ifndef DPP_MODULES_LIMITER_HPP
#define DPP_MODULES_LIMITER_HPP

#include "common.hpp"

class limiter_t
{
  protected:
  std::mutex mtx;
  std::set<dpp::snowflake> authorised_masters;
  std::set<dpp::snowflake> authorised_roles;
  std::string creator;
  std::string prefix;
  public:
  limiter_t();

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  void fill_masters(std::stringstream &ss);
  bool allow(dpp::snowflake id);
  bool deny(dpp::snowflake id);
  bool allow_role(dpp::snowflake id);
  bool deny_role(dpp::snowflake id);

  std::string prefix_get();
  bool prefix_set(std::string prefix);
  bool allow_master(const dpp::guild_member &guser, const dpp::user &user);
};

#endif // DPP_MODULES_LIMITER_HPP
