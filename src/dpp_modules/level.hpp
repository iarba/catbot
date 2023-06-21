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
// mfa
// level = log mfa (sum(gains) / quantum * mfa)

class level_t
{
  protected:
  float base = 10;
  float co = 60;
  float exp = 2;
  float quantum = 3;
  float mfa = 2.4;
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
  float quantum_get();
  bool quantum_set(float value);
  float mfa_get();
  bool mfa_set(float value);

  float exp_to_level(float exp);
  float level_to_exp(float level);
  void report_message(const dpp::message &msg);
  leveled_user_t checkout(dpp::snowflake id);
};

#endif // DPP_MODULES_LEVEL_HPP
