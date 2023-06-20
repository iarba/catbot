#include "unified.hpp"

bool message_cache_t::save(std::ostream &ss)
{
  bool status = true;
  // do not save data, just settings
  this->mtx.lock();
  status = status && (ss << ' ' << this->cache_validity << std::endl);
  this->mtx.unlock();
  return status;
}

bool message_cache_t::load(std::istream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss >> this->cache_validity);
  this->mtx.unlock();
  return status;
}

void message_cache_t::get_message_cb(dpp::snowflake channel_id, dpp::snowflake message_id, std::function<void(dpp::message, bool)> cb)
{
  bool hit;
  dpp::message m = this->get_message(channel_id, message_id, &hit);
  if(hit)
  {
    cb(m, true);
  }
  else
  {
    persistent_t::get()->bot->message_get(message_id, channel_id, [channel_id, message_id, cb](const auto &gcb)
    {
      dpp::message m;
      if(gcb.is_error())
      {
        WARN("failed to get message %lu %lu\n", (uint64_t)channel_id, (uint64_t)message_id);
        cb(m, false);
        return;
      }
      m = std::get<dpp::message>(gcb.value);
      persistent_t::get()->cache.add_message(m);
      cb(m, true);
    });
  }
}

dpp::message message_cache_t::get_message(dpp::snowflake channel_id, dpp::snowflake message_id, bool *hit)
{
  this->mtx.lock();
  auto it = this->data.find(std::pair<dpp::snowflake, dpp::snowflake>(channel_id, message_id));
  if(it == this->data.end())
  {
    this->mtx.unlock();
    if(hit != NULL)
    {
      *hit = false;
    }
    TRACE("CACHE: miss %lu %lu\n", (uint64_t)channel_id, (uint64_t)message_id);
    return dpp::message();
  }
  // refresh timestamp on cache HIT
  it->second.first = std::chrono::system_clock::now();
  dpp::message rval = it->second.second;
  this->mtx.unlock();
  if(hit != NULL)
  {
    *hit = true;
  }
  TRACE("CACHE: hit %lu %lu\n", (uint64_t)channel_id, (uint64_t)message_id);
  return rval;
}

void message_cache_t::add_message(const dpp::message &message)
{
  bool hit;
  this->get_message(message.channel_id, message.id, &hit);
  if(hit)
  {
    return;
  }
  TRACE("CACHE: add %lu %lu\n", (uint64_t)message.channel_id, (uint64_t)message.id);
  this->mtx.lock();
  timestamp_t rn = std::chrono::system_clock::now();
  this->data[std::pair<dpp::snowflake, dpp::snowflake>(message.channel_id, message.id)] = std::pair<timestamp_t, dpp::message>(rn, message);
  this->mtx.unlock();
}

void message_cache_t::cleanup()
{
  std::vector<std::pair<dpp::snowflake, dpp::snowflake>> keys_to_delete;
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  for(auto it = this->data.begin(); it != this->data.end(); it++)
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - it->second.first);
    if(delta.count() > this->cache_validity)
    {
      keys_to_delete.push_back(it->first);
    }
  }
  for(auto it = keys_to_delete.begin(); it != keys_to_delete.end(); it++)
  {
    TRACE("CACHE: expire %lu %lu\n", (uint64_t)(*it).first, (uint64_t)(*it).second);
    this->data.erase(*it);
  }
  this->mtx.unlock();
}
