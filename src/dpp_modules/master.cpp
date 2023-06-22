#include "unified.hpp"

// this is used whenever a stub is encountered
#define UNUSED_ERR_MSG ("`?l:" + std::to_string(__LINE__) + '`')

#define OK "Ok"
#define FAIL "Fail"

bool banana_man_valid();

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

std::string master_t::process_master_rule_give(int argc, std::string *argv, const dpp::message *context_msg, dpp::snowflake role)
{
  role_rule_t rr;
  rr.level_requirement = 0;
  rr.guild = context_msg->member.guild_id;
  for(int i = 0; i < argc; i++)
  {
    if(argv[i] == "level")
    {
      if(++i >= argc)
      {
        return FAIL;
      }
      std::stringstream tss(argv[i]);
      if(!(tss >> rr.level_requirement))
      {
        return FAIL;
      }
    }
  }
  for(auto it : context_msg->mention_roles)
  {
    if(it != role)
    {
      rr.blacklist_roles.insert(it);
    }
  }
  TRACE("adding rule for role %lu\n", (uint64_t) role);
  if(persistent_t::get()->dispatcher.give(role, rr))
  {
    return OK;
  }
  return FAIL;
}

std::string master_t::process_master_rule_forget(int argc, std::string *argv, const dpp::message *context_msg, dpp::snowflake role)
{
  if(persistent_t::get()->dispatcher.forget(role))
  {
    return OK;
  }
  return FAIL;
}

std::string master_t::process_master_rule(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    std::stringstream ss;
    ss << "List of rules;" << std::endl;
    persistent_t::get()->dispatcher.show_rules(ss);
    return ss.str();
  }
  if(argc <= 1)
  {
    return UNUSED_ERR_MSG;
  }
  uint64_t id;
  if(sscanf(argv[1].c_str(), "<@&%lu>", &id) != 1)
  {
    return FAIL;
  }
  dpp::snowflake role(id);
  if(argv[0] == "give")
  {
    return this->process_master_rule_give(argc - 2, argv + 2, context_msg, role);
  }
  if(argv[0] == "forget")
  {
    return this->process_master_rule_forget(argc - 2, argv + 2, context_msg, role);
  }
  return UNUSED_ERR_MSG;
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
  if(argv[0] == "level")
  {
    return this->process_master_level(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "rule")
  {
    return this->process_master_rule(argc - 1, argv + 1, context_msg);
  }
  return UNUSED_ERR_MSG;
}

std::string master_t::process_help(int argc, std::string *argv, const dpp::message *context_msg)
{
  std::stringstream ss;
  std::string prefix = persistent_t::get()->limiter.prefix_get();

  // help
  ss << '`' << prefix << " help` - Display this message. Duh." << std::endl;

  // fun
  ss << '`' << prefix << " opensource` - Show a link to my source code, for any wannabe contributors(contribute pls)." << std::endl;
  ss << '`' << prefix << " banana` - Summon the banana man." << std::endl;

  // level
  ss << '`' << prefix << " level [@user?]` - Show the level of the user. If user not specified, show your own level" << std::endl; // TODO
  ss << '`' << prefix << " level requirements` - Show the level xp requirements" << std::endl; // TODO

  // master, only if allowed
  if(persistent_t::get()->limiter.allow_master(context_msg->member, context_msg->author))
  {
    // limitation
    ss << '`' << prefix << " master prefix [key?]` - Get/set the current prefix in use." << std::endl;

    // allow
    ss << '`' << prefix << " master allow <add|remove> [@users]` - allow/deny certain users to use master api." << std::endl;
    ss << '`' << prefix << " master allow list` - list all users currenty allowed to use master api." << std::endl;

    // level
    ss << '`' << prefix << " master level base [#channel?]` - Get/set the base in the level formula." << std::endl;
    ss << '`' << prefix << " master level co [#channel?]` - Get/set the cutoff in the level formula." << std::endl;
    ss << '`' << prefix << " master level exp [#channel?]` - Get/set the exponent in the level formula." << std::endl;
    ss << '`' << prefix << " master level a [#channel?]` - Get/set the a in the level formula." << std::endl;
    ss << '`' << prefix << " master level b [#channel?]` - Get/set the b in the level formula." << std::endl;
    ss << '`' << prefix << " master level give [number] [@user]` - Give xp to a user." << std::endl;
    ss << '`' << prefix << " master level set [number] [@user]` - Set the level of a user." << std::endl;
    // ss << '`' << prefix << " master level log_channel [#channel?]` - Get/set the level log channel." << std::endl; // TODO

    // rule
    ss << '`' << prefix << " master rule ` - Show all rules" << std::endl;
    ss << '`' << prefix << " master rule give [@role] level [level] [@roles?]` - Create a rule" << std::endl;
    ss << '`' << prefix << " master rule forget [@role]` - Delte a rule" << std::endl;

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

std::string master_t::process_level_requirements(int argc, std::string *argv, const dpp::message *context_msg)
{
  std::stringstream ss;
  for(float level = 1; level < 30.5; level += 1)
  {
    float exp = persistent_t::get()->level.level_to_exp(level);
    ss << "For level " << (uint32_t)(level) << ": " << (uint32_t)(exp * 100.0f) << " exp is required." << std::endl;
  }
  return ss.str();
}

std::string master_t::process_level(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc >= 1 && argv[0] == "requirements")
  {
    return master_t::process_level_requirements(argc - 1, argv + 1, context_msg);
  }
  dpp::snowflake id = context_msg->author.id;
  // display first mention, of one exists
  for(auto it : context_msg->mentions)
  {
    id = it.first.id; break;
  }
  leveled_user_t lu = persistent_t::get()->level.checkout(id);
  float level = persistent_t::get()->level.exp_to_level(lu.exp);
  if(id == context_msg->author.id)
  {
    persistent_t::get()->dispatcher.check_level(context_msg->member, level);
  }
  return "Level of <@" + std::to_string((uint64_t) id) + ">: " + std::to_string((uint32_t)(level)) + "(exp:" + std::to_string((uint32_t)(lu.exp * 100.0f)) +")";
}

std::string master_t::process_master_level_co(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`level.co: " + std::to_string(persistent_t::get()->level.co_get()) + "`";
  }
  std::stringstream ss(argv[0]);
  float value;
  if(ss >> value)
  {
    if(persistent_t::get()->level.co_set(value))
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_level_base(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`level.base: " + std::to_string(persistent_t::get()->level.base_get()) + "`";
  }
  std::stringstream ss(argv[0]);
  float value;
  if(ss >> value)
  {
    if(persistent_t::get()->level.base_set(value))
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_level_exp(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`level.exp: " + std::to_string(persistent_t::get()->level.exp_get()) + "`";
  }
  std::stringstream ss(argv[0]);
  float value;
  if(ss >> value)
  {
    if(persistent_t::get()->level.exp_set(value))
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_level_a(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`level.a: " + std::to_string(persistent_t::get()->level.a_get()) + "`";
  }
  std::stringstream ss(argv[0]);
  float value;
  if(ss >> value)
  {
    if(persistent_t::get()->level.a_set(value))
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_level_b(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return "`level.b: " + std::to_string(persistent_t::get()->level.b_get()) + "`";
  }
  std::stringstream ss(argv[0]);
  float value;
  if(ss >> value)
  {
    if(persistent_t::get()->level.b_set(value))
    {
      return OK;
    }
  }
  return FAIL;
}

std::string master_t::process_master_level_give(int argc, std::string *argv, const dpp::message *context_msg)
{
  dpp::snowflake id;
  bool found  = false;
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  float xp_to_give;
  std::stringstream iss(argv[0]);
  if(!(iss >> xp_to_give))
  {
    return FAIL;
  }
  xp_to_give /= 100.0f;
  // affect first mention, if one exists
  for(auto it : context_msg->mentions)
  {
    id = it.first.id; found = true; break;
  }
  if(!found)
  {
    return FAIL;
  }
  leveled_user_t lu = persistent_t::get()->level.checkout(id);
  if(persistent_t::get()->level.set(id, lu.exp + xp_to_give))
  {
    return OK;
  }
  return FAIL;
}

std::string master_t::process_master_level_set(int argc, std::string *argv, const dpp::message *context_msg)
{
  dpp::snowflake id;
  bool found  = false;
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  float lvl_to_set;
  std::stringstream iss(argv[0]);
  if(!(iss >> lvl_to_set))
  {
    return FAIL;
  }
  // affect first mention, if one exists
  for(auto it : context_msg->mentions)
  {
    id = it.first.id; found = true; break;
  }
  if(!found)
  {
    return FAIL;
  }
  if(persistent_t::get()->level.set(id, persistent_t::get()->level.level_to_exp(lvl_to_set)))
  {
    return OK;
  }
  return FAIL;
}

std::string master_t::process_master_level(int argc, std::string *argv, const dpp::message *context_msg)
{
  if(argc <= 0)
  {
    return UNUSED_ERR_MSG;
  }
  if(argv[0] == "give")
  {
    return this->process_master_level_give(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "set")
  {
    return this->process_master_level_set(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "co")
  {
    return this->process_master_level_co(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "base")
  {
    return this->process_master_level_base(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "exp")
  {
    return this->process_master_level_exp(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "a")
  {
    return this->process_master_level_a(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "b")
  {
    return this->process_master_level_b(argc - 1, argv + 1, context_msg);
  }
  return UNUSED_ERR_MSG;
}

std::string master_t::process_banana(int args, std::string *argv, const dpp::message *context_msg)
{
  if(banana_man_valid())
  {
    return "https://media.discordapp.net/attachments/1109129070752571423/1121133147111833671/banana_man_cat.mp4";
  }
  else
  {
    return "Banana man is taking a *break*.";
  }
}

std::string master_t::process_opensource(int args, std::string *argv, const dpp::message *context_msg)
{
  return "This is my source code, please consider helping out ^^ <https://github.com/iarba/catbot>";
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
  if(argv[0] == "opensource")
  {
    return this->process_opensource(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "banana")
  {
    return this->process_banana(argc - 1, argv + 1, context_msg);
  }
  if(argv[0] == "level")
  {
    return this->process_level(argc - 1, argv + 1, context_msg);
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
