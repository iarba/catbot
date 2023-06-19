#include "macro.hpp"

#include <dpp/dpp.h>

#include <set>

typedef std::chrono::time_point<std::chrono::system_clock> timestamp_t;

class message_cache_t
{
  protected:
  int cache_validity = 5 * 60;
  std::mutex mtx;
  std::map<std::pair<dpp::snowflake, dpp::snowflake>, std::pair<timestamp_t, dpp::message>> data;
  public:

  bool save(std::ostream &ss)
  {
    bool status = true;
    // do not save data, just settings
    this->mtx.lock();
    status = status && (ss << ' ' << this->cache_validity << std::endl);
    this->mtx.unlock();
    return status;
  }

  bool load(std::istream &ss)
  {
    bool status = true;
    this->mtx.lock();
    status = status && (ss >> this->cache_validity);
    this->mtx.unlock();
    return status;
  }

  dpp::message get_message(dpp::snowflake channel_id, dpp::snowflake message_id, bool *hit = NULL)
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

  void add_message(const dpp::message &message)
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

  void cleanup()
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
};

class yikes_database_t
{
  protected:
  int yikes_required = 2;
  std::chrono::seconds yikes_persistence = std::chrono::seconds(15 * 60);
  std::chrono::seconds yikes_timeout_duration = std::chrono::seconds(15 * 60);
  std::mutex mtx;
  std::map<std::pair<dpp::snowflake, dpp::snowflake>, timestamp_t> yikes_collection;
  std::map<dpp::snowflake, int> yikes_count;
  public:

  bool save(std::ostream &ss)
  {
    bool status = true;
    // do not save data, just settings
    this->mtx.lock();
    status = status && (ss << ' ' << this->yikes_required << ' ');
    status = status && (ss << ' ' << this->yikes_persistence.count() << ' ');
    status = status && (ss << ' ' << this->yikes_timeout_duration.count() << ' ');
    status = status && (ss << ' ' << this->yikes_collection.size() << std::endl);
    for(auto it = this->yikes_collection.begin(); it != this->yikes_collection.end(); it++)
    {
      status = status && (ss << ' ' << (uint64_t)it->first.first << ' ' << (uint64_t)it->first.second << ' ' << std::chrono::system_clock::to_time_t(it->second) << std::endl);
    }
    // yikes_count can be deduced.
    this->mtx.unlock();
    return status;
  }

  bool load(std::istream &ss)
  {
    long val;
    bool status = true;
    this->mtx.lock();
    status = status && (ss >> this->yikes_required);
    status = status && (ss >> val); this->yikes_persistence = std::chrono::seconds(val);
    status = status && (ss >> val); this->yikes_timeout_duration = std::chrono::seconds(val);
    status = status && (ss >> val);
    this->yikes_collection.clear();
    this->yikes_count.clear();
    while(val--)
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

  std::chrono::seconds timeout_duration_get()
  {
    return this->yikes_timeout_duration;
  }

  bool report(dpp::snowflake reporting_user, dpp::snowflake reported_user)
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
    bool rval = ycount >= this->yikes_required;
    this->mtx.unlock();
    TRACE("YDB: add %lu report of %lu, has %d yikeses, modified %d, rval %d\n", (uint64_t)reporting_user, (uint64_t)reported_user, ycount, increase_count, rval);
    return rval;
  }

  void cleanup()
  {
    std::vector<std::pair<dpp::snowflake, dpp::snowflake>> keys_to_delete;
    timestamp_t rn = std::chrono::system_clock::now();
    this->mtx.lock();
    for(auto it = this->yikes_collection.begin(); it != this->yikes_collection.end(); it++)
    {
      auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - it->second);
      if(delta > this->yikes_persistence)
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
};

class mommy_tracker_t
{
  protected:
  std::mutex mtx;
  timestamp_t last_mommy_message;
  timestamp_t last_meow_message;
  public:
  mommy_tracker_t()
  {
    auto rn =  std::chrono::system_clock::now();
    this->last_mommy_message = rn;
    this->last_meow_message = rn;
  }

  bool save(std::ostream &ss)
  {
    bool status = true;
    this->mtx.lock();
    status = status && (ss << ' ' << std::chrono::system_clock::to_time_t(this->last_mommy_message) << std::endl);
    status = status && (ss << ' ' << std::chrono::system_clock::to_time_t(this->last_meow_message) << std::endl);
    this->mtx.unlock();
    return status;
  }

  bool load(std::istream &ss)
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

  std::string report_mommy()
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
      ss.precision(2);
      ss << "Days since someone last begged for \"mommy\": ~~" << ((float)(seconds.count() / (24 * 6 * 6))) / 100 << "~~ 0.";
    }
    else if(seconds.count() > 3 * 60 * 60)
    {
      ss << "Hours since someone last begged for \"mommy\": ~~" << seconds.count() << "~~ 0.";
    }
    return ss.str();
  }

  std::string report_meow()
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
};

class limiter_t
{
  protected:
  std::mutex mtx;
  std::set<dpp::snowflake> authorised_masters;
  std::string creator;
  std::string prefix;
  public:
  limiter_t()
  {
    this->creator = "iarba#0000";
    this->prefix = "nya";
  }

  bool save(std::ostream &ss)
  {
    bool status = true;
    this->mtx.lock();
    status = status && (ss << ' ' << this->prefix << ' ' << this->authorised_masters.size() << std::endl);
    for(auto it = this->authorised_masters.begin(); it != this->authorised_masters.end(); it++)
    {
      status = status && (ss << ' ' << (uint64_t)(*it));
    }
    status = status && (ss << std::endl);
    this->mtx.unlock();
    return status;
  }

  bool load(std::istream &ss)
  {
    bool status = true;
    this->mtx.lock();
    uint64_t snowflake_uint64;
    long count;
    status = status && (ss >> this->prefix >> count);
    while(count--)
    {
      status = status && (ss >> snowflake_uint64);
      dpp::snowflake snowflake(snowflake_uint64);
      this->authorised_masters.insert(snowflake);
    }
    this->mtx.unlock();
    return status;
  }

  std::string prefix_get()
  {
    this->mtx.lock();
    std::string prefix = this->prefix;
    this->mtx.unlock();
    return prefix;
  }

  bool prefix_set(std::string prefix)
  {
    this->mtx.lock();
    this->prefix = prefix;
    this->mtx.unlock();
    return true;
  }

  bool allow_master(dpp::user user)
  {
    bool status = false;
    this->mtx.lock();
    if(user.format_username() == this->creator)
    {
      WARN("accept master call from %s(%lu] scenarion 1\n", user.format_username().c_str(), (uint64_t)user.id);
      status = true;
    }
    else if(this->authorised_masters.find(user.id) != this->authorised_masters.end())
    {
      WARN("accept master call from %s(%lu] scenarion 2\n", user.format_username().c_str(), (uint64_t)user.id);
      status = true;
    }
    else
    {
      WARN("reject master call from %s(%lu]\n", user.format_username().c_str(), (uint64_t)user.id);
    }
    this->mtx.unlock();
    return status;
  }
};

typedef struct
{
  message_cache_t cache;
  yikes_database_t ydb;
  mommy_tracker_t mtrack;
  limiter_t limiter;

  uint64_t tick_period;
  dpp::snowflake self;
  dpp::cluster *bot;

  timestamp_t last_cleanup;
  int cleanup_frequency;
  std::string memfile;
  timestamp_t last_backup;
  int backup_frequency;
} persistent_t;

persistent_t g_persistent;

bool g_save(std::string file)
{
  std::ofstream f;
  f.open(file, std::fstream::out);
  if(f.is_open())
  {
    if(!g_persistent.cache.save(f))
    {
      ERROR("failed to serialise cache\n");
      f.close();
      return false;
    }
    if(!g_persistent.ydb.save(f))
    {
      ERROR("failed to serialise ydb\n");
      f.close();
      return false;
    }
    if(!g_persistent.mtrack.save(f))
    {
      ERROR("failed to serialise mtrack\n");
      f.close();
      return false;
    }
    if(!g_persistent.limiter.save(f))
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

bool g_load(std::string file)
{
  std::ifstream f;
  f.open(file, std::fstream::in);
  if(f.is_open())
  {
    if(!g_persistent.cache.load(f))
    {
      ERROR("failed to deserialise cache\n");
      f.close();
      return false;
    }
    if(!g_persistent.ydb.load(f))
    {
      ERROR("failed to deserialise ydb\n");
      f.close();
      return false;
    }
    if(!g_persistent.mtrack.load(f))
    {
      ERROR("failed to deserialise mtrack\n");
      f.close();
      return false;
    }
    if(!g_persistent.limiter.load(f))
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

void g_periodic_cb()
{
  TRACE("periodic tick\n");
  timestamp_t rn = std::chrono::system_clock::now();
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - g_persistent.last_cleanup);
    if(delta.count() > g_persistent.cleanup_frequency)
    {
      TRACE("execute cleanup\n");
      g_persistent.cache.cleanup();
      g_persistent.ydb.cleanup();
      g_persistent.last_cleanup = rn;
    }
  }
  {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(rn - g_persistent.last_backup);
    if(delta.count() > g_persistent.cleanup_frequency)
    {
      TRACE("execute backup\n");
      if(g_save(g_persistent.memfile))
      {
        g_persistent.last_backup = rn;
      }
    }
  }
}

int dpp_main(int argc, char **argv)
{
  bool load = true;
  for(int i = 0; i < argc; i++)
  {
    if(strcmp(argv[i], "-dpp_no_load") == 0)
    {
      load = false;
    }
  }
  // initialise persistent structures
  timestamp_t rn = std::chrono::system_clock::now();
  g_persistent.last_cleanup = rn;
  g_persistent.cleanup_frequency = 1 * 60;
  g_persistent.last_backup = rn;
  g_persistent.backup_frequency = 15 * 60;
  g_persistent.memfile = DISCORD_BOT_PERSISTENT_MEMFILE;
  if(load)
  {
    INFO("memfile load\n");
    g_load(g_persistent.memfile);
  }
  else
  {
    WARN("skipping memfile load\n");
  }
  // load api key
  std::string apikey;
  INFO("fetching api key\n");
  std::ifstream f;
  std::string keyfile = DISCORD_BOT_API_KEY_FILE;
  f.open(keyfile, std::fstream::in);
  if(f.is_open())
  {
    std::stringstream transfer;
    transfer << f.rdbuf();
    apikey = transfer.str();
    f.close();
  }
  else
  {
    WARN("cannot open apikey file %s\n", keyfile.c_str());
    return 0;
  }
  // file system adds a new line
  apikey.erase(std::remove_if(apikey.begin(), apikey.end(), [](unsigned char x) { return std::isspace(x); }), apikey.end());

  // create the bot
  g_persistent.bot = new dpp::cluster(apikey, dpp::i_default_intents | dpp::i_message_content);

  // attach a logger
  g_persistent.bot->on_log(dpp::utility::cout_logger());

  g_persistent.bot->on_message_create([](const auto & event)
  {
    if(event.msg.author.is_bot()) // only ignore self messages?
    {
      TRACE("not replying to self\n");
      return;
    }
    g_persistent.cache.add_message(event.msg);
    std::string msg = event.msg.content;
    for (auto & c: msg) c = tolower(c);
    INFO("processing message %lu %lu\n", (uint64_t)event.msg.channel_id, (uint64_t)event.msg.id);
    if(msg.find("yikes") != std::string::npos)
    {
      bool hit;
      auto target_msg = g_persistent.cache.get_message(event.msg.message_reference.channel_id, event.msg.message_reference.message_id, &hit);
      if(!hit)
      {
        g_persistent.bot->message_get(event.msg.message_reference.message_id, event.msg.message_reference.channel_id, [reporting_user = event.msg.author.id](const auto &gcb)
        {
          if(gcb.is_error())
          {
            WARN("%lu reported bad message\n", (uint64_t) reporting_user);
            return;
          }
          // TODO; somehow figure out how to use T dpp::confirmation_callback_t::get()
          dpp::message target_msg = std::get<dpp::message>(gcb.value);
          g_persistent.cache.add_message(target_msg);
          dpp::snowflake reported_user = target_msg.author.id;
          if(g_persistent.ydb.report(reporting_user, reported_user))
          {
            // take action against said user
            WARN("timing out user %lu\n", (uint64_t)reported_user);
            g_persistent.bot->guild_member_timeout(target_msg.guild_id, reported_user, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + g_persistent.ydb.timeout_duration_get()));
          }
        });
      }
      else
      {
        dpp::snowflake reported_user = target_msg.author.id;
        if(g_persistent.ydb.report(event.msg.author.id, reported_user))
        {
          // take action against said user
          WARN("timing out user %lu\n", (uint64_t)reported_user);
          if(reported_user == g_persistent.self)
          {
            ERROR("someone tried to get me to time myself out, rude\n");
            return;
          }
          else
          {
            g_persistent.bot->guild_member_timeout(target_msg.guild_id, reported_user, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + g_persistent.ydb.timeout_duration_get()));
          }
        }
      }
    }
    if(msg.find("mommy") != std::string::npos || msg.find("mummy") != std::string::npos)
    {
      std::string remark = g_persistent.mtrack.report_mommy();
      if(remark.size() != 0)
      {
        event.reply(remark);
      }
      return;
    }
    if(msg.find("meow") != std::string::npos)
    {
      std::string remark = g_persistent.mtrack.report_meow();
      if(remark.size() != 0)
      {
        event.send(remark);
      }
      return;
    }
    std::string match = g_persistent.limiter.prefix_get() + ' ';
    int consumed = 0;
    if(msg.compare(consumed, match.size(), match) == 0)
    {
      consumed += match.size();
      match = "master ";
      if(msg.compare(consumed, match.size(), match) == 0)
      {
        consumed += match.size();
        if(!g_persistent.limiter.allow_master(event.msg.author))
        {
          event.reply("I only listen to certified catgirls.");
          return;
        }
        std::stringstream css;
        css.str(msg.substr(consumed));
        std::string token;
#define CONFUSED           \
{                          \
  event.reply("?");        \
  return;                  \
}
#define GET_TOK            \
if(!(css >> token))        \
CONFUSED
#define GET_TOK_OPTIONAL   \
token.clear();             \
css >> token;
        GET_TOK;
        if(token == "config")
        {
          GET_TOK;
          if(token == "save")
          {
            if(g_save(g_persistent.memfile))
            {
              event.reply("ok");
            }
            else
            {
              event.reply("failed");
            }
            return;
          }
          CONFUSED;
        }
        else if(token == "prefix")
        {
          GET_TOK_OPTIONAL;
          if(token == "set")
          {
            GET_TOK;
            if(g_persistent.limiter.prefix_set(token))
            {
              event.reply("ok");
            }
            else
            {
              event.reply("failed");
            }
            return;
          }
          event.reply("`prefix: \"" + g_persistent.limiter.prefix_get() + "\"`");
          return;
        }
        CONFUSED;
      }
    }
  });
  g_persistent.bot->start_timer([](const auto & _event)
  {
    g_periodic_cb();
  }, 5);
  g_persistent.bot->on_ready([](const dpp::ready_t& event) {      
    INFO("ready and eager\n");
    g_persistent.bot->current_user_get([](const auto &gcb)
    {
      if(gcb.is_error())
      {
        ERROR("I'm having an identity crysis\n");
        return;
      }
      // TODO; somehow figure out how to use T dpp::confirmation_callback_t::get()
      dpp::user_identified target_user = std::get<dpp::user_identified>(gcb.value);
      WARN("I am %s(%lu)\n", target_user.format_username().c_str(), (uint64_t)target_user.id);
      g_persistent.self = target_user.id;
    });
  });
  g_persistent.bot->start(dpp::st_wait);

  return 0;
}
