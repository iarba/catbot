#include "unified.hpp"

float level_t::exp_to_level(float exp)
{
  float level;
  this->mtx.lock();
  level = (-this->b + std::sqrt(this->b * this->b + 4.0f * this->a * exp)) / (2.0f * this->a);
  this->mtx.unlock();
  return level;
}

float level_t::level_to_exp(float level)
{
  float exp;
  this->mtx.lock();
  exp = level * (level * this->a + b);
  this->mtx.unlock();
  return exp;
}

bool level_t::save(std::ostream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss << ' ' << this->base << ' ' << this->co << ' ' << this->exp << ' ' << this->a << ' ' << this->b << ' ' << this->user_db.size() << std::endl);
  for(auto it : user_db)
  {
    status = status && (ss << (uint64_t) it.first << ' ' << std::chrono::system_clock::to_time_t(it.second->last_message) << ' ' << it.second->exp << std::endl);
  }
  this->mtx.unlock();
  return status;
}

bool level_t::load(std::istream &ss)
{
  bool status = true;
  int count;
  uint64_t id;
  std::time_t time;
  float exp;
  this->mtx.lock();
  status = status && (ss >> this->base >> this->co >> this->exp >> this->a >> this->b >> count);
  for(auto it : this->user_db)
  {
    delete it.second;
  }
  this->user_db.clear();
  while(count-- > 0)
  {
    status = status && (ss >> id >> time >> exp);
    leveled_user_t *lu = new leveled_user_t;
    lu->last_message = std::chrono::system_clock::from_time_t(time);
    lu->exp = exp;
    this->user_db[id] = lu;
  }
  this->mtx.unlock();
  return status;
}

float level_t::base_get()
{
  this->mtx.lock();
  float value = this->base;
  this->mtx.unlock();
  return value;
}

bool level_t::base_set(float value)
{
  this->mtx.lock();
  this->base = value;
  this->mtx.unlock();
  return true;
}

float level_t::co_get()
{
  this->mtx.lock();
  float value = this->co;
  this->mtx.unlock();
  return value;
}

bool level_t::co_set(float value)
{
  this->mtx.lock();
  this->co = value;
  this->mtx.unlock();
  return true;
}

float level_t::exp_get()
{
  this->mtx.lock();
  float value = this->exp;
  this->mtx.unlock();
  return value;
}

bool level_t::exp_set(float value)
{
  this->mtx.lock();
  this->exp = value;
  this->mtx.unlock();
  return true;
}

float level_t::a_get()
{
  this->mtx.lock();
  float value = this->a;
  this->mtx.unlock();
  return value;
}

bool level_t::a_set(float value)
{
  this->mtx.lock();
  this->a = value;
  this->mtx.unlock();
  return true;
}

float level_t::b_get()
{
  this->mtx.lock();
  float value = this->b;
  this->mtx.unlock();
  return value;
}

bool level_t::b_set(float value)
{
  this->mtx.lock();
  this->b = value;
  this->mtx.unlock();
  return true;
}

leveled_user_t *level_t::get_user(dpp::snowflake id, timestamp_t rn)
{
  auto it = this->user_db.find(id);
  if(it == this->user_db.end())
  {
    leveled_user_t *lu = new leveled_user_t;
    lu->last_message = rn;
    lu->exp = 0;
    this->user_db[id] = lu;
    return lu;
  }
  return it->second;
}

void level_t::report_message(const dpp::message &msg)
{
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  leveled_user_t *lu = this->get_user(msg.author.id, rn);
  auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - lu->last_message);
  float x = delta.count();
  float exp_gain;
  if(x < this->co)
  {
    exp_gain = std::pow(x / this->co, this->exp);
  }
  else
  {
    exp_gain = std::log(x * this->base / this->co) / std::log(base);
  }
  lu->last_message = rn;
  lu->exp += exp_gain;
  this->mtx.unlock();
}

leveled_user_t level_t::checkout(dpp::snowflake id)
{
  leveled_user_t lu;
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  leveled_user_t *actual_lu = this->get_user(id, rn);
  lu = *actual_lu;
  this->mtx.unlock();
  return lu;
}

bool level_t::set(dpp::snowflake id, float exp)
{
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  leveled_user_t *lu = this->get_user(id, rn);
  lu->exp = exp;
  this->mtx.unlock();
  return true;
}
