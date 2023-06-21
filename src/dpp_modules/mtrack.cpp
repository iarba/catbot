#include "unified.hpp"

mtrack_t::mtrack_t()
{
  auto rn =  std::chrono::system_clock::now();
  this->last_mommy_message = rn;
  this->last_meow_message = rn;
}

bool mtrack_t::save(std::ostream &ss)
{
  bool status = true;
  this->mtx.lock();
  status = status && (ss << ' ' << std::chrono::system_clock::to_time_t(this->last_mommy_message) << std::endl);
  status = status && (ss << ' ' << std::chrono::system_clock::to_time_t(this->last_meow_message) << std::endl);
  this->mtx.unlock();
  return status;
}

bool mtrack_t::load(std::istream &ss)
{
  bool status = true;
  this->mtx.lock();
  std::time_t when;
  status = status && (ss >> when);
  this->last_mommy_message = std::chrono::system_clock::from_time_t(when);
  status = status && (ss >> when);
  this->last_meow_message = std::chrono::system_clock::from_time_t(when);
  this->mtx.unlock();
  return status;
}

std::string mtrack_t::report_mommy()
{
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(rn - this->last_mommy_message);
  this->last_mommy_message = rn;
  this->mtx.unlock();
  INFO("MTRACK: someone said mommy, delta: %ld seconds\n", seconds.count());
  std::stringstream ss;
  if(seconds.count() > 48 * 60 * 60)
  {
    ss.precision(1);
    ss << "Days since someone last begged for \"mommy\": **~~" << std::fixed << ((float)(seconds.count() / (24 * 6 * 6))) / 10 << "~~ 0**.";
  }
  else if(seconds.count() > 3 * 60 * 60)
  {
    ss.precision(1);
    ss << "Hours since someone last begged for \"mommy\": **~~" << std::fixed << ((float)(seconds.count() / (60 * 6))) / 10 << "~~ 0**.";
  }
  return ss.str();
}

std::string mtrack_t::report_meow()
{
  timestamp_t rn = std::chrono::system_clock::now();
  this->mtx.lock();
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(rn - this->last_meow_message);
  this->last_meow_message = rn;
  this->mtx.unlock();
  INFO("MTRACK: someone said meow, delta: %ld seconds\n", seconds.count());
  std::stringstream ss;
  if(seconds.count() > 42)
  {
    ss << "meow";
  }
  return ss.str();
}
