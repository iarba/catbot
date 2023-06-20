#ifndef DPP_MODULES_CACHE_HPP
#define DPP_MODULES_CACHE_HPP

#include "common.hpp"

class message_cache_t
{
  protected:
  int cache_validity = 5 * 60;
  std::mutex mtx;
  std::map<std::pair<dpp::snowflake, dpp::snowflake>, std::pair<timestamp_t, dpp::message>> data;
  public:

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  void get_message_cb(dpp::snowflake channel_id, dpp::snowflake message_id, std::function<void(dpp::message, bool)> cb);
  dpp::message get_message(dpp::snowflake channel_id, dpp::snowflake message_id, bool *hit = NULL);
  void add_message(const dpp::message &message);

  void cleanup();
};

#endif // DPP_MODULES_CACHE_HPP
