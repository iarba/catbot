#ifndef DPP_MODULES_DISPATCHER_HPP
#define DPP_MODULES_DISPATCHER_HPP

#include "common.hpp"

typedef struct
{
  int level_requirement;
  dpp::snowflake guild;
  std::set<dpp::snowflake> blacklist_roles;
} role_rule_t;

class dispatcher_t
{
  protected:

  std::map<dpp::snowflake, role_rule_t> rulez;
  std::mutex mtx;
  public:

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  void check_level(const dpp::guild_member &member, float level);
  bool forget(dpp::snowflake role);
  bool give(dpp::snowflake role, role_rule_t rr);
  void show_rules(std::stringstream &ss);

  void dispatch_message(dpp::snowflake gid, dpp::snowflake channel, std::string message);
  void give_role(dpp::snowflake gid, dpp::snowflake user, dpp::snowflake role);
};

#endif // DPP_MODULES_DISPATCHER_HPP
