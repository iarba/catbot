#include "unified.hpp"

bool dispatcher_t::save(std::ostream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss << ' ' << this->rulez.size() << std::endl);
  for(auto it : this->rulez)
  {
    status = status && ( ss << (uint64_t) it.first << ' ' << it.second.level_requirement << ' ' << (uint64_t) it.second.guild << ' ' << it.second.blacklist_roles.size());
    for(auto it2 : it.second.blacklist_roles)
    {
      status = status && ( ss << ' ' << (uint64_t) it2);
    }
    ss << std::endl;
  }
  this->mtx.unlock();
  return status;
}

bool dispatcher_t::load(std::istream &ss)
{
  bool status = true;
  uint64_t value;
  int count, count2, lvl_req;
  this->mtx.lock();
  this->rulez.clear();
  status = status && (ss >> count);
  while(count-- > 0)
  {
    role_rule_t rr;
    status = status && (ss >> value);
    dpp::snowflake role(value);
    status = status && (ss >> rr.level_requirement >> value);
    rr.guild = value;
    status = status && (ss >> count2);
    while(count2-- > 0)
    {
      status = status && (ss >> value);
      rr.blacklist_roles.insert(value);
    }
    this->rulez[role] = rr;
  }
  this->mtx.unlock();
  return status;
}

void dispatcher_t::check_level(const dpp::guild_member &member, float level)
{
  for(auto it : this->rulez)
  {
    if(member.guild_id != it.second.guild)
    {
      TRACE("user %lu in wrong guild for role %lu", (uint64_t) member.user_id, (uint64_t)it.first);
    }
    if(((float)(it.second.level_requirement)) > level)
    {
      TRACE("user %lu too low level for role %lu", (uint64_t) member.user_id, (uint64_t)it.first);
      continue;
    }
    bool already_has_role = false;
    bool blacklisted = false;
    for(auto it2 : member.roles)
    {
      if(it.second.blacklist_roles.find(it2) != it.second.blacklist_roles.end())
      {
        TRACE("user %lu blacklisted from acquiring role %lu", (uint64_t) member.user_id, (uint64_t)it.first);
        blacklisted = true;
        break;
      }
      if(it2 == it.first)
      {
        TRACE("user %lu already has role %lu", (uint64_t) member.user_id, (uint64_t)it.first);
        already_has_role = true;
        break;
      }
    }
    if(already_has_role || blacklisted)
    {
      continue;
    }
    INFO("giving role %lu to user %lu", (uint64_t) it.first, (uint64_t) member.user_id);
    this->give_role(member.guild_id, member.user_id, it.first);
  }
}

bool dispatcher_t::forget(dpp::snowflake role)
{
  bool status = false;
  this->mtx.lock();
  auto it = this->rulez.find(role);
  if(it != this->rulez.end())
  {
    status = true;
    this->rulez.erase(it);
  }
  this->mtx.unlock();
  return status;
}

bool dispatcher_t::give(dpp::snowflake role, role_rule_t rr)
{
  bool status = false;
  this->mtx.lock();
  auto it = this->rulez.find(role);
  if(it == this->rulez.end())
  {
    status = true;
    this->rulez[role] = rr;
  }
  this->mtx.unlock();
  return status;
}

void dispatcher_t::show_rules(std::stringstream &ss)
{
  this->mtx.lock();
  for(auto it : this->rulez)
  {
    ss << "Giving role <@&" << (uint64_t)it.first << "> to people level " << it.second.level_requirement << "+, except for people having roles [";
    for(auto it2 : it.second.blacklist_roles)
    {
      ss << "<@&" << (uint64_t)it2 << "> ";
    }
    ss << ']' << std::endl;
  }
  this->mtx.unlock();
}

void dispatcher_t::dispatch_message(dpp::snowflake gid, dpp::snowflake channel, std::string message)
{
  dpp::message m;
  m.set_guild_id(gid).set_channel_id(channel).set_content(message);
  persistent_t::get()->bot->message_create(m);
}

void dispatcher_t::give_role(dpp::snowflake gid, dpp::snowflake user, dpp::snowflake role)
{
  persistent_t::get()->bot->guild_member_add_role(gid, user, role);
}
