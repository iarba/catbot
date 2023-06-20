#include "unified.hpp"

limiter_t::limiter_t()
{
  this->creator = "iarba#0000";
  this->prefix = "nya";
}

bool limiter_t::save(std::ostream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss << ' ' << this->prefix << ' ' << this->authorised_masters.size() << std::endl);
  for(auto it = this->authorised_masters.begin(); it != this->authorised_masters.end(); it++)
  {
    status = status && (ss << ' ' << (uint64_t)(*it));
  }
  status = status && (ss << std::endl);
  this->mtx.unlock();
  return status;
}

bool limiter_t::load(std::istream &ss)
{
  bool status = true;
  this->mtx.lock();
  uint64_t snowflake_uint64;
  long count;
  status = status && (ss >> this->prefix >> count);
  while(count--)
  {
    status = status && (ss >> snowflake_uint64);
    dpp::snowflake snowflake(snowflake_uint64);
    this->authorised_masters.insert(snowflake);
  }
  this->mtx.unlock();
  return status;
}

std::string limiter_t::prefix_get()
{
  this->mtx.lock();
  std::string prefix = this->prefix;
  this->mtx.unlock();
  return prefix;
}

bool limiter_t::prefix_set(std::string prefix)
{
  this->mtx.lock();
  this->prefix = prefix;
  this->mtx.unlock();
  return true;
}

bool limiter_t::allow_master(dpp::user user)
{
  bool status = false;
  this->mtx.lock();
  if(user.format_username() == this->creator)
  {
    WARN("accept master call from %s(%lu] scenarion 1\n", user.format_username().c_str(), (uint64_t)user.id);
    status = true;
  }
  else if(this->authorised_masters.find(user.id) != this->authorised_masters.end())
  {
    WARN("accept master call from %s(%lu] scenarion 2\n", user.format_username().c_str(), (uint64_t)user.id);
    status = true;
  }
  else
  {
    WARN("reject master call from %s(%lu]\n", user.format_username().c_str(), (uint64_t)user.id);
  }
  this->mtx.unlock();
  return status;
}
