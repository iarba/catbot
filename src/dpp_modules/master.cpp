#include "unified.hpp"

// this is used whenever a stub is encountered
#define UNUSED_ERR_MSG ("`?l:" + std::to_string(__LINE__) + '`')

#define OK "Ok"
#define FAIL "Fail"

std::string master_t::process_master_config_save(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    if(persistent_t::get()->save())
    {
      return OK;
    }
  }
  else
  {
    persistent_t::get()->save(argv[0]);
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_config(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  if(argv[0] == "save")
  {
    return this->process_master_config_save(argc - 1, argv + 1, context_msg);
  }
  return UNUSED_ERR_MSG;
}

std::string master_t::process_master_prefix(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`prefix: \"" + persistent_t::get()->limiter.prefix_get() + "\"`";
  }
  if(persistent_t::get()->limiter.prefix_set(argv[0]))
  {
    return OK;
  }
  return FAIL;
}

std::string master_t::process_master_allow(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  if(argv[0] == "list")
  {
    std::stringstream ss;
    ss << "List of certified catgirls:" << std::endl;
    persistent_t::get()->limiter.fill_masters(ss);
    return ss.str();
  }
  else if(argv[0] == "add")
  {
    bool status = true;
    for(auto it : context_msg->mentions)
    {
      status |= persistent_t::get()->limiter.allow(it.first.id);
    }
    for(auto it : context_msg->mention_roles)
    {
      status |= persistent_t::get()->limiter.allow_role(it);
    }
    if(status)
    {
      return OK;
    }
    return FAIL;
  }
  else if(argv[0] == "remove")
  {
    bool status = true;
    for(auto it : context_msg->mentions)
    {
      status |= persistent_t::get()->limiter.deny(it.first.id);
    }
    for(auto it : context_msg->mention_roles)
    {
      status |= persistent_t::get()->limiter.deny_role(it);
    }
    if(status)
    {
      return OK;
    }
    return FAIL;
  }
  return UNUSED_ERR_MSG;
}

std::string master_t::process_master_ydb(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  if(argv[0] == "threshold")
  {
    if(argc == 1)
    {
      return "`threshold: \"" + std::to_string(persistent_t::get()->ydb.threshold_get()) + "\"`";
    }
    std::stringstream ss(argv[1]);
    int count;
    if(ss >> count)
    {
      if(persistent_t::get()->ydb.threshold_set(count))
      {
        return OK;
      }
    }
  }
  else if(argv[0] == "persistence")
  {
    if(argc == 1)
    {
      return "`persistence: \"" + std::to_string(persistent_t::get()->ydb.persistence_get().count()) + "\"(seconds)`";
    }
    std::stringstream ss(argv[1]);
    uint32_t seconds;
    if(ss >> seconds)
    {
      if(persistent_t::get()->ydb.persistence_set(std::chrono::seconds(seconds)))
      {
        return OK;
      }
    }
  }
  else if(argv[0] == "timeout")
  {
    if(argc == 1)
    {
      return "`timeout: \"" + std::to_string(persistent_t::get()->ydb.timeout_get().count()) + "\"(seconds)`";
    }
    std::stringstream ss(argv[1]);
    uint32_t seconds;
    if(ss >> seconds)
    {
      if(persistent_t::get()->ydb.timeout_set(std::chrono::seconds(seconds)))
      {
        return OK;
      }
    }
  }
  return FAIL;
}

std::string master_t::process_master(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "Yes master?";
  }
  if(argv[0] == "config")
  {
    return this->process_master_config(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "prefix")
  {
    return this->process_master_prefix(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "ydb")
  {
    return this->process_master_ydb(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "allow")
  {
    return this->process_master_allow(argc - 1, argv + 1, context_msg);
  }
  return UNUSED_ERR_MSG;
}

std::string master_t::process_help(int argc, std::string *argv, const dpp::message *context_msg)
{
  std::stringstream ss;
  std::string prefix = persistent_t::get()->limiter.prefix_get();

  // help
  ss << '`' << prefix << " help` - Display this message. Duh." << std::endl;

  // master, only if allowed
  if(persistent_t::get()->limiter.allow_master(context_msg->member, context_msg->author))
  {
    // limitation
    ss << '`' << prefix << " master prefix [key?]` - Get/set the current prefix in use." << std::endl;

    // allow
    ss << '`' << prefix << " master allow <add|remove> [@users]` - allow/deny certain users to use master api." << std::endl;
    ss << '`' << prefix << " master allow list` - list all users currenty allowed to use master api." << std::endl;

    // level
    // ss << '`' << prefix << " master level_log_channel [#channel?]` - Get/set the level log channel." << std::endl; // TODO

    // internal
    // ss << '`' << prefix << " master internal cache validity [seconds?]` - Get/set the cache validity duration." << std::endl; // TODO

    // ydb
    ss << '`' << prefix << " master ydb threshold [count?]` - Get/set the yikes report count threshold." << std::endl;
    ss << '`' << prefix << " master ydb persistence [seconds?]` - Get/set the validity of yikes reports." << std::endl;
    ss << '`' << prefix << " master ydb timeout [seconds?]` - Get/set the duration of the automatic timeout." << std::endl;

    // config
    ss << '`' << prefix << " master config save [file?]` - Save persistent data at the given location. If file is unspecified, use active memfile." << std::endl;
  }
  return ss.str();
}

std::string master_t::process(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "";
  }
  if(argv[0] == "help")
  {
    return this->process_help(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "master")
  {
    INFO("attempt at using master command\n");
    if(persistent_t::get()->limiter.allow_master(context_msg->member, context_msg->author))
    {
      return this->process_master(argc - 1, argv + 1, context_msg);
    }
    else
    {
      return "I only listen to certified catgirls.";
    }
  }
  return UNUSED_ERR_MSG;
}
