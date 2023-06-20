#ifndef DPP_MODULES_MTRACK_HPP
#define DPP_MODULES_MTRACK_HPP

#include "common.hpp"

class mtrack_t
{
  protected:
  std::mutex mtx;
  timestamp_t last_mommy_message;
  timestamp_t last_meow_message;
  public:
  mtrack_t();

  bool save(std::ostream &ss);
  bool load(std::istream &ss);

  std::string report_mommy();
  std::string report_meow();
};

#endif // DPP_MODULES_MTRACK_HPP
