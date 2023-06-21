#include "unified.hpp"

bool yikes_database_t::save(std::ostream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss << ' ' << this->threshold << ' ');
  status = status && (ss << ' ' << this->persistence.count() << ' ');
  status = status && (ss << ' ' << this->timeout.count() << ' ');
  status = status && (ss << ' ' << this->yikes_collection.size() << std::endl);
  for(auto it = this->yikes_collection.begin(); it != this->yikes_collection.end(); it++)
  {
    status = status && (ss << ' ' << (uint64_t)it->first.first << ' ' << (uint64_t)it->first.second << ' ' << std::chrono::system_clock::to_time_t(it->second) << std::endl);
  }
  // yikes_count can be deduced.
  this->mtx.unlock();
  return status;
}

bool yikes_database_t::load(std::istream &ss)
{
  long val;
  bool status = true;
  this->mtx.lock();
  status = status && (ss >> this->threshold);
  status = status && (ss >> val); this->persistence = std::chrono::seconds(val);
  status = status && (ss >> val); this->timeout = std::chrono::seconds(val);
  status = status && (ss >> val);
  this->yikes_collection.clear();
  this->yikes_count.clear();
  while(val-- > 0)
  {
    uint64_t first_snowflake_uint64, second_snowflake_uint64;
    std::time_t when;
    status = status && (ss >> first_snowflake_uint64 >> second_snowflake_uint64 >> when);
    dpp::snowflake first_snowflake(first_snowflake_uint64);
    dpp::snowflake second_snowflake(first_snowflake_uint64);
    this->yikes_collection[std::pair<dpp::snowflake, dpp::snowflake>(first_snowflake, second_snowflake)] = std::chrono::system_clock::from_time_t(when);
    auto it = this->yikes_count.find(second_snowflake);
    if(it == this->yikes_count.end())
    {
      this->yikes_count[second_snowflake] = 1;
    }
    else
    {
      it->second++;
    }
  }
  this->mtx.unlock();
  return status;
}

int yikes_database_t::threshold_get()
{
  this->mtx.lock();
  auto rval = this->threshold;
  this->mtx.unlock();
  return rval;
}

bool yikes_database_t::threshold_set(int val)
{
  this->mtx.lock();
  this->threshold = val;
  this->mtx.unlock();
  return true;
}

std::chrono::seconds yikes_database_t::persistence_get()
{
  this->mtx.lock();
  auto rval = this->persistence;
  this->mtx.unlock();
  return rval;
}

bool yikes_database_t::persistence_set(std::chrono::seconds val)
{
  this->mtx.lock();
  this->persistence = val;
  this->mtx.unlock();
  return true;
}

std::chrono::seconds yikes_database_t::timeout_get()
{
  this->mtx.lock();
  auto rval = this->timeout;
  this->mtx.unlock();
  return rval;
}

bool yikes_database_t::timeout_set(std::chrono::seconds val)
{
  this->mtx.lock();
  this->timeout = val;
  this->mtx.unlock();
  return true;
}

bool yikes_database_t::report(dpp::snowflake reporting_user, dpp::snowflake reported_user)
{
  int increase_count = 0;
  this->mtx.lock();
  if(this->yikes_collection.find(std::pair<dpp::snowflake, dpp::snowflake>(reporting_user, reported_user)) == this->yikes_collection.end())
  {
    increase_count = 1;
  }
  this->yikes_collection[std::pair<dpp::snowflake, dpp::snowflake>(reporting_user, reported_user)] = std::chrono::system_clock::now();
  auto it = this->yikes_count.find(reported_user);
  int ycount;
  if(it == this->yikes_count.end())
  {
    ycount = (this->yikes_count[reported_user] = increase_count);
  }
  else
  {
    ycount = (it->second += increase_count);
  }
  bool rval = ycount >= this->threshold;
  this->mtx.unlock();
  TRACE("YDB: add %lu report of %lu, has %d yikeses, modified %d, rval %d\n", (uint64_t)reporting_user, (uint64_t)reported_user, ycount, increase_count, rval);
  return rval;
}

void yikes_database_t::cleanup()
{
  std::vector<std::pair<dpp::snowflake, dpp::snowflake>> keys_to_delete;
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  for(auto it = this->yikes_collection.begin(); it != this->yikes_collection.end(); it++)
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - it->second);
    if(delta > this->persistence)
    {
      keys_to_delete.push_back(it->first);
    }
  }
  for(auto it = keys_to_delete.begin(); it != keys_to_delete.end(); it++)
  {
    TRACE("YDB: expire %lu report of %lu\n", (uint64_t)(*it).first, (uint64_t)(*it).second);
    this->yikes_collection.erase(*it);
    this->yikes_count[(*it).second]--;
  }
  this->mtx.unlock();
}
