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
  return UNUSED_ERR_MSG;
}

std::string master_t::process_help(int argc, std::string *argv, const dpp::message *context_msg)
{
  std::stringstream ss;
  std::string prefix = persistent_t::get()->limiter.prefix_get();

  // help
  ss << '`' << prefix << " help` - Display this message. Duh." << std::endl;

  // master
  ss << '`' << prefix << " master config save [file]` - Save persistent data at the given location. If file is unspecified, use active memfile." << std::endl;
  ss << '`' << prefix << " master prefix` - Display the current prefix in use." << std::endl;
  ss << '`' << prefix << " master prefix [key]` - Replace the current prefix with the provided one." << std::endl;
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
    if(persistent_t::get()->limiter.allow_master(context_msg->author))
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
