#include "macro.hpp"
#include "dpp_modules/unified.hpp"

#define MAX_COMMAND_LEN 1024

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
  persistent_t::get();
  if(load)
  {
    INFO("memfile load\n");
    persistent_t::get()->load();
  }
  else
  {
    WARN("skipping memfile load\n");
  }
  {
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
    persistent_t::get()->bot = new dpp::cluster(apikey, dpp::i_default_intents | dpp::i_message_content);

  }

  // attach a logger
  persistent_t::get()->bot->on_log(dpp::utility::cout_logger());

  // this is the big boy
  persistent_t::get()->bot->on_message_create([](const auto & event)
  {
    if(event.msg.author.is_bot())
    {
      // sorry systems...
      TRACE("ignore self/bot\n");
      return;
    }
    persistent_t::get()->cache.add_message(event.msg);
    std::string msg = event.msg.content;
    for (auto & c: msg) c = tolower(c);
    INFO("processing message %lu %lu\n", (uint64_t)event.msg.channel_id, (uint64_t)event.msg.id);
    if(msg.find("yikes") != std::string::npos)
    {
      persistent_t::get()->cache.get_message_cb(event.msg.message_reference.channel_id, event.msg.message_reference.message_id, [reporting_user = event.msg.author.id](dpp::message target_msg, bool hit)
      {
        if(!hit)
        {
          WARN("%lu reported inexistent message\n", (uint64_t) reporting_user);
          return;
        }
        dpp::snowflake reported_user = target_msg.author.id;
        if(persistent_t::get()->ydb.report(reporting_user, reported_user))
        {
          // take action against said user
          WARN("timing out user %lu\n", (uint64_t)reported_user);
          if(reported_user == persistent_t::get()->self)
          {
            ERROR("someone tried to get me to time myself out, rude\n");
            return;
          }
          persistent_t::get()->bot->guild_member_timeout(target_msg.guild_id, reported_user, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + persistent_t::get()->ydb.timeout_duration_get()));
        }
      });
    }
    if(msg.find("mommy") != std::string::npos || msg.find("mummy") != std::string::npos)
    {
      std::string remark = persistent_t::get()->mtrack.report_mommy();
      if(remark.size() != 0)
      {
        event.reply(remark);
      }
      return;
    }
    if(msg.find("meow") != std::string::npos)
    {
      std::string remark = persistent_t::get()->mtrack.report_meow();
      if(remark.size() != 0)
      {
        event.send(remark);
      }
      return;
    }
    std::string prefix = persistent_t::get()->limiter.prefix_get();
    if(msg.compare(0, prefix.size(), prefix) == 0)
    {
      int argc = 0;
      std::string *argv = new std::string[MAX_COMMAND_LEN];
      std::stringstream css(msg.substr(prefix.size()));
      std::string token;
      // this is a command
      while(css >> token)
      {
        if(argc >= MAX_COMMAND_LEN)
        {
          WARN("failed to process too large command\n");
          event.reply("too much man, too much...");
          delete[] argv;
          return;
        }
        argv[argc++] = token;
      }
      token = persistent_t::get()->master.process(argc, argv, &event.msg);
      if(token.size() != 0)
      {
        TRACE("replying to command\n");
        event.reply(persistent_t::get()->master.process(argc, argv, &event.msg));
      }
      delete[] argv;
      return;
    }
  });

  persistent_t::get()->bot->start_timer([](const auto & _event)
  {
    persistent_t::get()->periodic_cb();
  }, 5);

  persistent_t::get()->bot->on_ready([](const dpp::ready_t& event) {      
    INFO("ready and eager\n");
    persistent_t::get()->bot->current_user_get([](const auto &gcb)
    {
      if(gcb.is_error())
      {
        ERROR("I'm having an identity crysis\n");
        return;
      }
      // TODO; somehow figure out how to use T dpp::confirmation_callback_t::get()
      dpp::user_identified target_user = std::get<dpp::user_identified>(gcb.value);
      WARN("I am %s(%lu)\n", target_user.format_username().c_str(), (uint64_t)target_user.id);
      persistent_t::get()->self = target_user.id;
    });
  });
  persistent_t::get()->bot->start(dpp::st_wait);

  return 0;
}
