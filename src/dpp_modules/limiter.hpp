#ifndef DPP_MODULES_LIMITER_HPP
#define DPP_MODULES_LIMITER_HPP

#include "common.hpp"

class limiter_t
{
  protected:
  std::mutex mtx;
  std::set<dpp::snowflake> authorised_masters;
  std::string creator;
  std::string prefix;
  public:
  limiter_t();

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  std::string prefix_get();
  bool prefix_set(std::string prefix);
  bool allow_master(dpp::user user);
};

#endif // DPP_MODULES_LIMITER_HPP
