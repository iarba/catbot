#include "unified.hpp"

static persistent_t *g_persistent;

persistent_t::persistent_t()
{
  timestamp_t rn = std::chrono::system_clock::now();
  this->last_cleanup = rn;
  this->cleanup_frequency = 1 * 60;
  this->last_backup = rn;
  this->backup_frequency = 15 * 60;
  this->memfile = DISCORD_BOT_PERSISTENT_MEMFILE;
}

persistent_t *persistent_t::get()
{
  if(!g_persistent)
  {
    g_persistent = new persistent_t();
  }
  return g_persistent;
}

bool persistent_t::save(std::string file)
{
  std::ofstream f;
  f.open(file, std::fstream::out);
  if(f.is_open())
  {
    if(!(f << VERSION << std::endl))
    {
      ERROR("failed to write version to memfile\n");
      f.close();
      return false;
    }
    if(!this->cache.save(f))
    {
      ERROR("failed to serialise cache\n");
      f.close();
      return false;
    }
    if(!this->ydb.save(f))
    {
      ERROR("failed to serialise ydb\n");
      f.close();
      return false;
    }
    if(!this->mtrack.save(f))
    {
      ERROR("failed to serialise mtrack\n");
      f.close();
      return false;
    }
    if(!this->limiter.save(f))
    {
      ERROR("failed to serialise limiter\n");
      f.close();
      return false;
    }
    INFO("successfully backed up file %s\n", file.c_str());
    f.close();
    return true;
  }
  else
  {
    ERROR("failed to open file %s\n", file.c_str());
    return false;
  }
}

bool persistent_t::save()
{
  return this->save(this->memfile);
}

bool persistent_t::load()
{
  return this->load(this->memfile);
}

bool persistent_t::load(std::string file)
{
  std::ifstream f;
  f.open(file, std::fstream::in);
  if(f.is_open())
  {
    uint32_t version;
    if(!(f >> version))
    {
      ERROR("failed to read memfile version\n");
      f.close();
      return false;
    }
    if(version != VERSION)
    {
      ERROR("mismatch version, abort\n");
      f.close();
      return false;
    }
    if(!this->cache.load(f))
    {
      ERROR("failed to deserialise cache\n");
      f.close();
      return false;
    }
    if(!this->ydb.load(f))
    {
      ERROR("failed to deserialise ydb\n");
      f.close();
      return false;
    }
    if(!this->mtrack.load(f))
    {
      ERROR("failed to deserialise mtrack\n");
      f.close();
      return false;
    }
    if(!this->limiter.load(f))
    {
      ERROR("failed to deserialise limiter\n");
      f.close();
      return false;
    }
    INFO("successfully loaded file %s\n", file.c_str());
    f.close();
    return true;
  }
  else
  {
    ERROR("failed to open file %s\n", file.c_str());
    return false;
  }
}

void persistent_t::periodic_cb()
{
  TRACE("periodic tick\n");
  if(!this->mtx.try_lock())
  {
    WARN("failed to acquire lock, skipping tick\n");
  }
  timestamp_t rn = std::chrono::system_clock::now();
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - this->last_cleanup);
    if(delta.count() > this->cleanup_frequency)
    {
      TRACE("execute cleanup\n");
      this->cache.cleanup();
      this->ydb.cleanup();
      this->last_cleanup = rn;
    }
  }
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - this->last_backup);
    if(delta.count() > this->backup_frequency)
    {
      TRACE("execute backup\n");
      if(this->save(this->memfile))
      {
        this->last_backup = rn;
      }
    }
  }
  this->mtx.unlock();
}

