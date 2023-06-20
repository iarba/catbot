#ifndef DPP_MODULES_UNIFIED_HPP
#define DPP_MODULES_UNIFIED_HPP

#include "cache.hpp"
#include "yikes.hpp"
#include "mtrack.hpp"
#include "limiter.hpp"
#include "master.hpp"

class persistent_t
{
  // singleton pattern
  public:
  static persistent_t *get();
  protected:
  persistent_t();

  public:
  // modules
  message_cache_t cache;
  yikes_database_t ydb;
  mtrack_t mtrack;
  limiter_t limiter;
  master_t master;

  // dpp stuff
  dpp::cluster *bot;
  dpp::snowflake self;

  // structural stuff
  std::string memfile;
  bool save(std::string file);
  bool save();
  bool load(std::string file);
  bool load();
  void periodic_cb();

  protected:
  std::mutex mtx;

  timestamp_t last_cleanup;
  int cleanup_frequency;

  timestamp_t last_backup;
  int backup_frequency;
};

#endif // DPP_MODULES_UNIFIED_HPP
