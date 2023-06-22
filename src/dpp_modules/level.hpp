#ifndef DPP_MODULES_LEVEL_HPP
#define DPP_MODULES_LEVEL_HPP

#include "common.hpp"

struct leveled_user_t
{
  timestamp_t last_message;
  float exp;
};

// x = delta since last msg(s)
// base co exp = float
// gain(x) = (x / co) ^ exp            when x < co
// gain(x) = log base(x * base / co)   when x > co
//

class level_t
{
  protected:
  float base = 10;
  float co = 60;
  float exp = 1.3;
  float a = 6;
  float b = 1.45;
  std::map<dpp::snowflake, leveled_user_t *> user_db;
  leveled_user_t *get_user(dpp::snowflake id, timestamp_t rn);
  std::mutex mtx;
  public:

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  float base_get();
  bool base_set(float value);
  float co_get();
  bool co_set(float value);
  float exp_get();
  bool exp_set(float value);
  float a_get();
  bool a_set(float value);
  float b_get();
  bool b_set(float value);

  float exp_to_level(float exp);
  float level_to_exp(float level);
  void report_message(const dpp::message &msg);
  leveled_user_t checkout(dpp::snowflake id);
  bool set(dpp::snowflake id, float exp);
};

#endif // DPP_MODULES_LEVEL_HPP
