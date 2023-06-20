#ifndef DPP_MODULES_YIKES_HPP
#define DPP_MODULES_YIKES_HPP

#include "common.hpp"

class yikes_database_t
{
  protected:
  int yikes_required = 2;
  std::chrono::seconds yikes_persistence = std::chrono::seconds(15 * 60);
  std::chrono::seconds yikes_timeout_duration = std::chrono::seconds(15 * 60);
  std::mutex mtx;
  std::map<std::pair<dpp::snowflake, dpp::snowflake>, timestamp_t> yikes_collection;
  std::map<dpp::snowflake, int> yikes_count;
  public:

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  std::chrono::seconds timeout_duration_get();
  bool report(dpp::snowflake reporting_user, dpp::snowflake reported_user);

  void cleanup();
};

#endif // DPP_MODULES_YIKES_HPP
