#ifndef DPP_MODULES_YIKES_HPP
#define DPP_MODULES_YIKES_HPP

#include "common.hpp"

class yikes_database_t
{
  protected:
  int threshold = 2;
  std::chrono::seconds persistence = std::chrono::seconds(15 * 60);
  std::chrono::seconds timeout = std::chrono::seconds(15 * 60);
  std::mutex mtx;
  std::map<std::pair<dpp::snowflake, dpp::snowflake>, timestamp_t> yikes_collection;
  std::map<dpp::snowflake, int> yikes_count;
  public:

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  int threshold_get();
  bool threshold_set(int val);
  std::chrono::seconds persistence_get();
  bool persistence_set(std::chrono::seconds val);
  std::chrono::seconds timeout_get();
  bool timeout_set(std::chrono::seconds val);

  bool report(dpp::snowflake reporting_user, dpp::snowflake reported_user);

  void cleanup();
};

#endif // DPP_MODULES_YIKES_HPP
